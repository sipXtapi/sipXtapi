//  
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#ifdef ENABLE_SRTP
#ifdef WIN32
#include <srtp.h>
#else
#include <srtp2/srtp.h>	
#endif
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "mp/MpSrtp.h"
#include "mp/MpSetSrtpParamsMsg.h"

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Capability cache populated by globalInitialize(). Indexed by
// SdpMediaLine::SdpCryptoSuiteType. A 'true' entry means we ran a
// successful srtp_create() probe with that suite at startup.
//
// Suites with no MpSrtp policy mapping (CRYPTO_SUITE_TYPE_NONE,
// F8_128_HMAC_SHA1_80) are never probed and stay false. Suites that
// have a mapping but whose underlying cipher is missing from the
// linked-in libsrtp build (typically GCM on a libsrtp built with no
// external crypto backend) also stay false.
//
// Sized to the largest enumerator + 1.
namespace
{
   const int kCryptoSuiteCount =
      SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM + 1;
}
static bool sSuiteSupported[kCryptoSuiteCount] = { false };
static bool sCapabilitiesProbed = false;

#ifdef ENABLE_SRTP
namespace
{
   /// Try to construct a one-off srtp_t with the given suite to see if
   /// the linked-in libsrtp can actually do it. Returns true on
   /// srtp_create() success. Used only by globalInitialize().
   bool probeCryptoSuite(SdpMediaLine::SdpCryptoSuiteType suite)
   {
      srtp_policy_t policy;
      memset(&policy, 0, sizeof(policy));

      switch (suite)
      {
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80:
            srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32:
            // 80 bit tag on .rtcp mirrors setSrtpParams() - see the RFC 4568 /
            // RFC 6188 note there.  The probe has to build the same policy it
            // is vouching for.
            srtp_crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_80:
            srtp_crypto_policy_set_aes_cm_192_hmac_sha1_80(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_192_hmac_sha1_80(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_32:
            srtp_crypto_policy_set_aes_cm_192_hmac_sha1_32(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_192_hmac_sha1_80(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_80:
            srtp_crypto_policy_set_aes_cm_256_hmac_sha1_80(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_256_hmac_sha1_80(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_32:
            srtp_crypto_policy_set_aes_cm_256_hmac_sha1_32(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_256_hmac_sha1_80(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM:
            srtp_crypto_policy_set_aes_gcm_128_16_auth(&policy.rtp);
            srtp_crypto_policy_set_aes_gcm_128_16_auth(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM:
            srtp_crypto_policy_set_aes_gcm_256_16_auth(&policy.rtp);
            srtp_crypto_policy_set_aes_gcm_256_16_auth(&policy.rtcp);
            break;
         default:
            // No libsrtp mapping (NONE, F8) -- nothing to probe.
            return false;
      }

      // Provide a key buffer of the exact length the policy expects;
      // the contents don't matter for srtp_create's purposes -- it just
      // needs a non-NULL pointer to copy from. Stack-allocated upper
      // bound covers all currently mapped suites (32-byte key + 14-byte
      // salt for AEAD_AES_256_GCM is the largest).
      uint8_t keyBuf[64] = { 0 };
      policy.ssrc.type = ssrc_any_inbound;
      policy.key       = keyBuf;
      policy.window_size = 64;
      policy.next      = NULL;

      srtp_t session = NULL;
      srtp_err_status_t status = srtp_create(&session, &policy);
      if (status == srtp_err_status_ok)
      {
         srtp_dealloc(session);
         return true;
      }
      // srtp_err_status_algo_fail is the typical "not built with this
      // cipher" return; treat any non-OK status as unsupported.
      return false;
   }

   /// Run the probe for every suite we care about, populate the cache,
   /// and emit a single INFO log line summarizing the support matrix.
   void probeAllCryptoSuites()
   {
      static const SdpMediaLine::SdpCryptoSuiteType probeList[] = {
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80,
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32,
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_80,
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_32,
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_80,
         SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_32,
         SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM,
         SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM
      };
      const int probeCount = (int)(sizeof(probeList) / sizeof(probeList[0]));

      UtlString summary;
      for (int i = 0; i < probeCount; i++)
      {
         SdpMediaLine::SdpCryptoSuiteType suite = probeList[i];
         bool ok = probeCryptoSuite(suite);
         sSuiteSupported[suite] = ok;

         if (!summary.isNull())
         {
            summary.append(", ");
         }
         summary.append(SdpMediaLine::SdpCryptoSuiteTypeString[suite]);
         summary.append(ok ? "=yes" : "=no");
      }

      OsSysLog::add(FAC_MP, PRI_INFO,
         "MpSrtp: libsrtp crypto suite capabilities: %s",
         summary.data());
   }

   /// Human readable name for the libsrtp statuses that can come back from
   /// srtp_unprotect()/srtp_unprotect_rtcp().  The numeric value is logged
   /// alongside this, so a status not listed here still identifies itself.
   const char* srtpStatusName(srtp_err_status_t status)
   {
      switch (status)
      {
         case srtp_err_status_ok:          return "ok";
         case srtp_err_status_fail:        return "fail";
         case srtp_err_status_bad_param:   return "bad_param";
         case srtp_err_status_auth_fail:   return "auth_fail";
         case srtp_err_status_cipher_fail: return "cipher_fail";
         case srtp_err_status_replay_fail: return "replay_fail";
         case srtp_err_status_replay_old:  return "replay_old";
         case srtp_err_status_no_ctx:      return "no_ctx";
         case srtp_err_status_cant_check:  return "cant_check";
         case srtp_err_status_key_expired: return "key_expired";
         case srtp_err_status_nonce_bad:   return "nonce_bad";
         case srtp_err_status_bad_mki:     return "bad_mki";
         case srtp_err_status_pkt_idx_old: return "pkt_idx_old";
         case srtp_err_status_pkt_idx_adv: return "pkt_idx_adv";
         default:                          return "unknown";
      }
   }

   /// Read a 32-bit big-endian (network order) field.
   uint32_t readUint32(const uint8_t* buf)
   {
      return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
             ((uint32_t)buf[2] <<  8) |  (uint32_t)buf[3];
   }

   bool isAeadSuite(SdpMediaLine::SdpCryptoSuiteType suite)
   {
      return suite == SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM ||
             suite == SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM;
   }

   /// Length of the authentication tag libsrtp appends to SRTCP, for the .rtcp
   /// policy setSrtpParams() installs.  Only used to locate the SRTCP index for
   /// logging -- if it is ever wrong the index reads as garbage, it has no
   /// bearing on what libsrtp itself does with the packet.
   ///
   /// Note this is the SRTCP tag, so the _32 suites report 10 and not 4: they
   /// truncate SRTP only and keep SRTCP at 80 bits.  See setSrtpParams().
   int srtcpAuthTagLen(SdpMediaLine::SdpCryptoSuiteType suite)
   {
      switch (suite)
      {
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80:
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_80:
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_80:
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32:
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_32:
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_32:
            return 10;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM:
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM:
            return 16;
         default:
            return 0;
      }
   }

   /// Pull the 31-bit SRTCP index out of the packet trailer.  For the AES-CM
   /// suites the trailer sits ahead of the auth tag; for the AEAD suites it is
   /// the last four octets of the packet (RFC 7714 section 9.3).  Returns false
   /// when the packet is too short to hold one.
   bool srtcpIndexFromPacket(SdpMediaLine::SdpCryptoSuiteType suite,
                             const uint8_t* buf, int len, uint32_t& index)
   {
      const int tagLen = srtcpAuthTagLen(suite);
      const int trailerOffset = isAeadSuite(suite) ? len - 4 : len - tagLen - 4;

      // Needs to leave room for the fixed 8 byte RTCP header ahead of it.
      if (trailerOffset < 8)
      {
         return false;
      }

      // Top bit of the trailer is the E (encrypted) flag, not part of the index.
      index = readUint32(buf + trailerOffset) & 0x7FFFFFFF;
      return true;
   }
}
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

const char* MpSrtpKeyUseString(MpSrtpKeyUse keyUse)
{
   switch (keyUse)
   {
      case MP_SRTP_KEY_USE_RTP_AND_RTCP: return "RTP+RTCP";
      case MP_SRTP_KEY_USE_RTP_ONLY:     return "RTP";
      case MP_SRTP_KEY_USE_RTCP_ONLY:    return "RTCP";
      default:                           return "unknown";
   }
}

/* ============================ CREATORS ================================== */

// Constructor
MpSrtp::MpSrtp() :
   mLock(OsMutex::Q_PRIORITY | OsMutex::INVERSION_SAFE),
   mCryptoSuite(SdpMediaLine::CRYPTO_SUITE_TYPE_NONE),
   mSrtpSessionCreated(FALSE),
   mLoggedUnkeyedPassthrough(FALSE),
   mSrtpSession(NULL)
{
}

// Destructor
MpSrtp::~MpSrtp()
{
   // No lock: the object is being destroyed, so no other thread may still
   // hold a reference to it.  Taking mLock here would only mask a caller
   // bug (and destroying a held mutex is undefined anyway).
   deallocateSrtpSession();
}

/* ============================ MANIPULATORS ============================== */

void MpSrtp::deallocateSrtpSession()
{
   if (mSrtpSessionCreated)
   {
#ifdef ENABLE_SRTP
      srtp_dealloc(mSrtpSession);
#endif
      mSrtpSession = NULL;

      // Reset members
      mSrtpSessionCreated = FALSE;
      mCryptoSuite = SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;
      mCryptoKey = UtlString::Empty;
   }
}

#ifdef ENABLE_SRTP
static void srtpEventHandler(srtp_event_data_t* data)
{
   switch (data->event) {
      case event_ssrc_collision:
         OsSysLog::add(FAC_MP, PRI_WARNING, "MpSrtp::srtpEventHandler: SSRC collision detected for SSRC: 0x%x", data->ssrc);
         break;
      case event_key_soft_limit:
         OsSysLog::add(FAC_MP, PRI_WARNING, "MpSrtp::srtpEventHandler: Key soft limit reached for SSRC: 0x%x, re-keying/re-negotiation needed soon.", data->ssrc);
         break;
      case event_key_hard_limit:
         OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::srtpEventHandler: Key hard limit reached for SSRC: 0x%x, session will fail.", data->ssrc);
         break;
      case event_packet_index_limit:
         OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::srtpEventHandler: Packet index limit reached for SSRC: 0x%x.", data->ssrc);
         break;
      default:
         OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::srtpEventHandler: Unknown event (%d), received for SSRC: 0x%x", data->event, data->ssrc);
         break;
   }
}
#endif

OsStatus MpSrtp::globalInitialize()
{
#ifdef ENABLE_SRTP
   // Initialize SRTP library - we only need to do this once per application
   // instance
   srtp_err_status_t status = srtp_init();
   if (status)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::globalInitialize: srtp_init failed, status=%d", status);
   }
   else
   {
      status = srtp_install_event_handler(srtpEventHandler);
      if(status)
      {
         OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::globalInitialize: srtp_install_event_handler failed, status=%d", status);
      }
   }

   // Probe libsrtp crypto suite support exactly once, regardless of
   // whether the event-handler install above failed (that's a non-fatal
   // diagnostic concern; it doesn't affect crypto availability). After
   // this returns, isCryptoSuiteSupported() is a pure cache read.
   if (!sCapabilitiesProbed)
   {
      probeAllCryptoSuites();
      sCapabilitiesProbed = true;
   }
   return (status ? OS_FAILED : OS_SUCCESS);
#else
   return OS_FAILED;
#endif
}

OsStatus MpSrtp::globalShutdown()
{
#ifdef ENABLE_SRTP
   return srtp_shutdown() ? OS_FAILED : OS_SUCCESS;
#else
   return OS_FAILED;
#endif
}

bool MpSrtp::isCryptoSuiteSupported(SdpMediaLine::SdpCryptoSuiteType suite)
{
   if (suite < 0 || suite >= kCryptoSuiteCount)
   {
      return false;
   }
   // If globalInitialize() has not run yet, the cache is all-false,
   // which is the safe answer (callers will treat the suite as
   // unsupported and either skip it or reject it).
   return sSuiteSupported[suite];
}

OsStatus MpSrtp::setSrtpParams(const UtlString& resourceName, OsMsgQ& flowgraphMessageQueue, SdpMediaLine::SdpCryptoSuiteType cryptoSuite, const UtlString& cryptoKey, MpSrtpKeyUse keyUse)
{
#ifdef ENABLE_SRTP
   // Note:  this message is handled in MpRtp(Input/Output)Connection::handleMessage and relayed to Mpr(From/To)Net::setSrtpParams
   MpSetSrtpParamsMsg message(resourceName, cryptoSuite, cryptoKey, keyUse);
   return(flowgraphMessageQueue.send(message, OsTime::OS_INFINITY /*sOperationQueueTimeout*/));
#else
   if (cryptoSuite != SdpMediaLine::CRYPTO_SUITE_TYPE_NONE)
   {
      OsSysLog::add(FAC_CP, PRI_WARNING, "setSrtpParams: a cryptoSuite was passed in but program was NOT compiled with ENABLE_SRTP flag");
   }
   return OS_FAILED;
#endif
}

UtlBoolean MpSrtp::isSessionCreated()
{
   OsLock lock(mLock);
   return mSrtpSessionCreated;
}

UtlBoolean MpSrtp::setSrtpParams(SdpMediaLine::SdpCryptoSuiteType cryptoSuite, const UtlString& cryptoKey, UtlBoolean forUnprotect)
{
#ifdef ENABLE_SRTP
   OsLock lock(mLock);
   srtp_err_status_t status;
   srtp_policy_t srtpPolicy;

   if (mSrtpSessionCreated)
   {
      // Check if settings are the same - if so just return true
      if (cryptoSuite == mCryptoSuite && cryptoKey == mCryptoKey)
      {
         // SRTP params unchanged - nothing to do
         return TRUE;
      }
      else
      {
         OsSysLog::add(FAC_MP, PRI_INFO, "MpSrtp::setSrtpParams: deallocating SRTP session: purpose=%s", forUnprotect ? "Inbound/Unprotect" : "Outbound/Protect");
         deallocateSrtpSession();
      }
   }

   if (cryptoSuite == SdpMediaLine::CRYPTO_SUITE_TYPE_NONE)
   {
      // No SRTP
      return TRUE;
   }

   // Clear out the policy struct
   memset(&srtpPolicy, 0, sizeof(srtp_policy_t));

   // Load default srtp/srtcp policy settings.
   //
   // The _32 suites are asymmetric on purpose: they truncate the SRTP tag to 32
   // bits but leave SRTCP at 80.  RFC 4568 section 6.2 spells that out for
   // AES_CM_128_HMAC_SHA1_32, RFC 6188 sections 3.2 and 3.4 repeat it for the
   // 192 and 256 bit variants, and it follows from RFC 3711 section 7.5, which
   // argues against short tags on control traffic because a forged RTCP BYE or
   // report costs far more than a forged media packet.  libsrtp says the same in
   // the header comment on srtp_crypto_policy_set_aes_cm_128_hmac_sha1_32:
   // "This crypto policy is intended for use in SRTP, but not in SRTCP."
   //
   // So the _32 cases deliberately install the matching _80 policy on .rtcp.
   // Handing .rtcp the _32 policy produces a 40 bit SRTCP tag that no conformant
   // peer will authenticate, in either direction.
   switch (cryptoSuite)
   {
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80:
         srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32:
         srtp_crypto_policy_set_aes_cm_128_hmac_sha1_32(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_80:
         srtp_crypto_policy_set_aes_cm_192_hmac_sha1_80(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_192_hmac_sha1_80(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_32:
         srtp_crypto_policy_set_aes_cm_192_hmac_sha1_32(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_192_hmac_sha1_80(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_80:
         srtp_crypto_policy_set_aes_cm_256_hmac_sha1_80(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_256_hmac_sha1_80(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_32:
         srtp_crypto_policy_set_aes_cm_256_hmac_sha1_32(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_256_hmac_sha1_80(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_128_GCM:
         // RFC 7714 - AES-128 GCM with 16-byte auth tag (128-bit tag).
         // Key material: 16-byte master key + 12-byte master salt = 28 bytes.
         srtp_crypto_policy_set_aes_gcm_128_16_auth(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_gcm_128_16_auth(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AEAD_AES_256_GCM:
         // RFC 7714 - AES-256 GCM with 16-byte auth tag (128-bit tag).
         // Key material: 32-byte master key + 12-byte master salt = 44 bytes.
         srtp_crypto_policy_set_aes_gcm_256_16_auth(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_gcm_256_16_auth(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_F8_128_HMAC_SHA1_80:
      default:
         // Not Supported
         OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::setSrtpParams: unsupported crypto suite = %d", cryptoSuite);
         return FALSE;
         break;
   }

   // Ensure key length matches expected length for selected crypto suite
   if (cryptoKey.length() != srtpPolicy.rtp.cipher_key_len)
   {
      // Invalid key size
      OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::setSrtpParams: invalid key size = %d (expected %d)", cryptoKey.length(), srtpPolicy.rtp.cipher_key_len);
      return FALSE;
   }

   // All good, copy params locally
   mCryptoSuite = cryptoSuite;
   mCryptoKey = cryptoKey;

   // set remaining policy settings
   if(forUnprotect)
   {
      srtpPolicy.ssrc.type = ssrc_any_inbound;
   }
   else
   {
      srtpPolicy.ssrc.type = ssrc_any_outbound;
   }
   srtpPolicy.key = (uint8_t*)mCryptoKey.data();
   srtpPolicy.window_size = 64;

   // Allocate and initailize the SRTP sessions
   status = srtp_create(&mSrtpSession, &srtpPolicy);
   if (status)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::setSrtpParams: srtp_create error = %d", status);

      // Unable to create srtp in session - reset info 
      mCryptoSuite = SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;
      mCryptoKey = UtlString::Empty;
      return FALSE;
   }

   OsSysLog::add(FAC_MP, PRI_INFO, "MpSrtp::setSrtpParams: Srtp enabled, cryptoSuite=%s, purpose=%s", SdpMediaLine::SdpCryptoSuiteTypeString[mCryptoSuite], forUnprotect ? "Inbound/Unprotect" : "Outbound/Protect");
   mSrtpSessionCreated = TRUE;

   return TRUE;
#else
   return FALSE;
#endif
}

// Fast-path validation for Inbound SRTP (Media)
// 1. Minimum length for AES-128_HMAC_SHA1_32 is 16 bytes
//    - 12 bytes: Fixed RTP Header
//    -  4 bytes: Authentication Tag (Truncated HMAC-SHA1-32)
// 2. Version must be 2 (0x80 mask)
// 3. Payload Type (PT) must be between 0 and 127
bool MpSrtp::isValidSrtp(const uint8_t* buf, size_t len)
{
   if (len < 16) return false;

   // Byte 0: Version (bits 0-1) must be 2. 
   // We mask with 0xC0 (1100 0000) to isolate the version.
   if ((buf[0] & 0xC0) != 0x80) return false;

   // Byte 1: PT (bits 1-7). 
   // For RTP, the most significant bit (Marker) can be 0 or 1.
   // The PT itself must be < 128. This is always true for an 8-bit field,
   // but in RTCP-mux scenarios, we check that it's NOT in the RTCP range (192-223).
   return (buf[1] < 192); // Basic demux logic: if it's not RTCP range, it's RTP
}

// Fast-path validation for Inbound SRTCP (Control)
// 1. Minimum length is 16 bytes
//    - 8 bytes : Fixed RTCP Header(Header + SSRC)
//    - 4 bytes : SRTCP Index(Mandatory 31 - bit index + 1 - bit E - flag)
//    - 4 bytes : Authentication Tag
//    The smallest packet the suites in setSrtpParams() can actually produce is
//    22 bytes -- a bare Receiver Report with no report blocks, under an 80 bit
//    SRTCP tag, which is what every AES-CM suite here uses including the _32
//    ones.  This floor is left below that on purpose: it is a cheap screen to
//    keep obviously-not-SRTCP out of libsrtp, not a conformance check, and
//    libsrtp validates the exact length against the negotiated policy itself.
//    The floor used to be 20, which was above the real minimum at the time and
//    silently discarded bare Receiver Reports.
// 2. Version must be 2 (0x80 mask)
// 3. Packet Type (Byte 1) must be in the RTCP range (192-223)
bool MpSrtp::isValidSrtcp(const uint8_t* buf, size_t len)
{
   if (len < 16) return false;

   // Byte 0: Version check (must be 2)
   if ((buf[0] & 0xC0) != 0x80) return false;

   // Byte 1: Packet Type.
   // Common types: 200 (SR), 201 (RR), 202 (SDES), 203 (BYE), 204 (APP)
   uint8_t pt = buf[1];
   return (pt >= 192 && pt <= 223);
}

bool MpSrtp::isRtcpPacket(const uint8_t* buf, size_t len)
{
   if (buf == NULL || len < 2)
   {
      return false;
   }

   // Must be in the RTP/RTCP band at all (RFC 7983): rules out STUN and DTLS.
   if (buf[0] < 128 || buf[0] > 191)
   {
      return false;
   }

   // RFC 5761 section 4: RTCP packet types occupy 192-223, disjoint from every
   // RTP payload type including those with the marker bit set.
   return (buf[1] >= 192 && buf[1] <= 223);
}

// Note: The passed in buffer is modified in place to contain the unprotected data.
//       The returned size will always be <= the passed in size.
UtlBoolean MpSrtp::srtpUnprotectIfNeeded(const uint8_t* data, int* size, UtlBoolean rtcp)
{
#ifdef ENABLE_SRTP
   OsLock lock(mLock);
   srtp_err_status_t status;
   if (!mSrtpSessionCreated)
   {
      // No SRTP configured -- the packet is handed on untouched.  Worth saying
      // once, because downstream this is indistinguishable from a successful
      // unprotect and yet means the opposite: on a session that IS meant to be
      // encrypted, it passes ciphertext (and the auth tag) to the RTP parser.
      if (!mLoggedUnkeyedPassthrough)
      {
         mLoggedUnkeyedPassthrough = TRUE;
         OsSysLog::add(FAC_MP, PRI_DEBUG,
            "MpSrtp::srtpUnprotectIfNeeded: no SRTP session on this context, "
            "passing %s through unmodified (size=%d)",
            rtcp ? "RTCP" : "RTP", *size);
      }
      return TRUE;
   }
   // Captured before the unprotect call, which rewrites the buffer in place and
   // shrinks *size.  Both are needed to make sense of a failure: libsrtp keys
   // its stream lookup and its replay window off the SSRC, and the index is
   // what the replay window actually rejects.
   const int protectedSize = *size;
   uint32_t ssrc = 0;
   uint32_t index = 0;
   bool haveIndex = false;

   if (rtcp == FALSE)
   {
      if (!isValidSrtp(data, *size))
      {
         // Failed basic SRTP validation - likely not an SRTP packet.
         // Avoid srtp_err_status_bad_param error in srtp_unprotect
         return TRUE;
      }
      // isValidSrtp() guarantees at least a full 12 byte RTP header.
      ssrc = readUint32(data + 8);
      index = (uint32_t)((data[2] << 8) | data[3]);   // RTP sequence number
      haveIndex = true;

      status = srtp_unprotect(mSrtpSession, (void*)data, size);
   }
   else
   {
      if (!isValidSrtcp(data, *size))
      {
         // Failed basic SRTCP validation - likely not an SRTCP packet.
         // Avoid srtp_err_status_bad_param error in srtp_unprotect_rtcp
         return TRUE;
      }
      // isValidSrtcp() guarantees at least a full 8 byte RTCP header.
      ssrc = readUint32(data + 4);
      haveIndex = srtcpIndexFromPacket(mCryptoSuite, data, protectedSize, index);

      status = srtp_unprotect_rtcp(mSrtpSession, (void*)data, size);
   }

   if (status)
   {
      // Warning rather than error: a rejected inbound packet is this function
      // working, not failing.  Anything that can reach the media port can
      // provoke one, so treating each as an error both overstates a peer's
      // one-off quirk and hands anyone who can send us a datagram a way to fill
      // the log.  The protect side stays at error, since a failure there is
      // ours.
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "MpSrtp::srtpUnprotectIfNeeded: %s error = %s (%d), suite=%s, ssrc=0x%08x, %s=%u, size=%d",
         rtcp ? "srtp_unprotect_rtcp" : "srtp_unprotect",
         srtpStatusName(status), status,
         SdpMediaLine::SdpCryptoSuiteTypeString[mCryptoSuite],
         ssrc,
         rtcp ? "srtcpIndex" : "rtpSeq",
         haveIndex ? index : 0,
         protectedSize);
      return FALSE;
   }

   if (rtcp)
   {
      // RTCP arrives on the order of one packet every few seconds, so logging
      // every accepted one costs nothing and gives the successful indexes to
      // compare a rejected one against -- an index that repeats or jumps
      // backwards is what separates a duplicated packet from a peer that
      // restarted its SRTP context out from under our replay window.
      OsSysLog::add(FAC_MP, PRI_DEBUG,
         "MpSrtp::srtpUnprotectIfNeeded: srtp_unprotect_rtcp ok, ssrc=0x%08x, srtcpIndex=%u, size=%d->%d",
         ssrc, haveIndex ? index : 0, protectedSize, *size);
   }
#endif
   return TRUE;
}

// Note: The passed in buffer is modified in place to contain the protected data.
//       The returned size will always be 4 or 10 bytes larger the passed in size for RTP,
//       and up to 14 bytes larget for RTCP.  It is recommended to have at least 20 extra 
//       bytes available in the buffer to also allow room for a 4-byte MKI value.
UtlBoolean MpSrtp::srtpProtectIfNeeded(const uint8_t* data, int* size, UtlBoolean rtcp, int maxBufferSize)
{
#ifdef ENABLE_SRTP
   OsLock lock(mLock);
   srtp_err_status_t status;
   if (!mSrtpSessionCreated)
   {
      // No SRTP configured for this session
      return TRUE;
   }

   // Ensure there is enough buffer room to SRTP encrypt into
   // The returned size will always be 4 or 10 bytes larger the passed in size for RTP,
   // and up to 14 bytes for RTCP.  It is recommended to have at least 20 extra 
   // bytes available in the buffer to also allow room for a 4-byte MKI value.
   if (*size + 20 > maxBufferSize)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::srtpProtectIfNeeded: not enough buffer room for SRTP protect, max=%d, want=%d", maxBufferSize, *size + 20);
      return FALSE;
   }

   if (rtcp == FALSE)
   {
      status = srtp_protect(mSrtpSession, (void*)data, size);
   }
   else
   {
      status = srtp_protect_rtcp(mSrtpSession, (void*)data, size);
   }
   if (status)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpSrtp::srtpProtectIfNeeded: %s error = %s (%d), suite=%s, size=%d",
         rtcp ? "srtp_protect_rtcp" : "srtp_protect",
         srtpStatusName(status), status,
         SdpMediaLine::SdpCryptoSuiteTypeString[mCryptoSuite],
         *size);
      return FALSE;
   }
#endif
   return TRUE;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

