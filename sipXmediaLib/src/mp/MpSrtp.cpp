//  
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#ifdef ENABLE_SRTP
  // Note: RTCP is sent from RTCPManager thread, and not yet implemented for SRTP encryption.
  //       It needs to share the same mSrtp session (ideally) as MpToNet, which is not thread-safe, so 
  //       we need to do some locking.  Since RTCP logic is currently buggy and typically disabled,
  //       we will leave this for later.
  #ifndef EXCLUDE_RTCP
    #error "Conflict detected: ENABLE_SRTP defined without EXCLUDE_RTCP.  Both SRTP and RTCP cannot both be enabled, since SRTP is not implemented for RTCP sending. Please choose one.  Define EXCLUDE_RTCP to disable RTCP."
  #endif
#ifdef WIN32
#include <srtp.h>
#else
#include <srtp2/srtp.h>	
#endif
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
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
            srtp_crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_80:
            srtp_crypto_policy_set_aes_cm_192_hmac_sha1_80(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_192_hmac_sha1_80(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_32:
            srtp_crypto_policy_set_aes_cm_192_hmac_sha1_32(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_192_hmac_sha1_32(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_80:
            srtp_crypto_policy_set_aes_cm_256_hmac_sha1_80(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_256_hmac_sha1_80(&policy.rtcp);
            break;
         case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_32:
            srtp_crypto_policy_set_aes_cm_256_hmac_sha1_32(&policy.rtp);
            srtp_crypto_policy_set_aes_cm_256_hmac_sha1_32(&policy.rtcp);
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
}
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpSrtp::MpSrtp() :
   mCryptoSuite(SdpMediaLine::CRYPTO_SUITE_TYPE_NONE),
   mSrtpSessionCreated(FALSE)
{
}

// Destructor
MpSrtp::~MpSrtp()
{
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

OsStatus MpSrtp::setSrtpParams(const UtlString& resourceName, OsMsgQ& flowgraphMessageQueue, SdpMediaLine::SdpCryptoSuiteType cryptoSuite, const UtlString& cryptoKey)
{
#ifdef ENABLE_SRTP
   // Note:  this message is handled in MpRtp(Input/Output)Connection::handleMessage and relayed to Mpr(From/To)Net::setSrtpParams
   MpSetSrtpParamsMsg message(resourceName, cryptoSuite, cryptoKey);
   return(flowgraphMessageQueue.send(message, OsTime::OS_INFINITY /*sOperationQueueTimeout*/));
#else
   if (cryptoSuite != SdpMediaLine::CRYPTO_SUITE_TYPE_NONE)
   {
      OsSysLog::add(FAC_CP, PRI_WARNING, "setSrtpParams: a cryptoSuite was passed in but program was NOT compiled with ENABLE_SRTP flag");
   }
   return OS_FAILED;
#endif
}

UtlBoolean MpSrtp::setSrtpParams(SdpMediaLine::SdpCryptoSuiteType cryptoSuite, const UtlString& cryptoKey, UtlBoolean forUnprotect)
{
#ifdef ENABLE_SRTP
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
   switch (cryptoSuite)
   {
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80:
         srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32:
         srtp_crypto_policy_set_aes_cm_128_hmac_sha1_32(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_128_hmac_sha1_32(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_80:
         srtp_crypto_policy_set_aes_cm_192_hmac_sha1_80(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_192_hmac_sha1_80(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_192_HMAC_SHA1_32:
         srtp_crypto_policy_set_aes_cm_192_hmac_sha1_32(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_192_hmac_sha1_32(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_80:
         srtp_crypto_policy_set_aes_cm_256_hmac_sha1_80(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_256_hmac_sha1_80(&srtpPolicy.rtcp);
         break;
      case SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_256_HMAC_SHA1_32:
         srtp_crypto_policy_set_aes_cm_256_hmac_sha1_32(&srtpPolicy.rtp);
         srtp_crypto_policy_set_aes_cm_256_hmac_sha1_32(&srtpPolicy.rtcp);
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
// 1. Minimum length for AES - 128_HMAC_SHA1_32 is 20 bytes
//    - 8 bytes : Fixed RTCP Header(Header + SSRC)
//    - 4 bytes : SRTCP Index(Mandatory 31 - bit index + 1 - bit E - flag)
//    - 4 bytes : Authentication Tag(Truncated HMAC - SHA1 - 32)
// 2. Version must be 2 (0x80 mask)
// 3. Packet Type (Byte 1) must be in the RTCP range (192-223)
bool MpSrtp::isValidSrtcp(const uint8_t* buf, size_t len)
{
   if (len < 20) return false;

   // Byte 0: Version check (must be 2)
   if ((buf[0] & 0xC0) != 0x80) return false;

   // Byte 1: Packet Type.
   // Common types: 200 (SR), 201 (RR), 202 (SDES), 203 (BYE), 204 (APP)
   uint8_t pt = buf[1];
   return (pt >= 192 && pt <= 223);
}

// Note: The passed in buffer is modified in place to contain the unprotected data.
//       The returned size will always be <= the passed in size.
UtlBoolean MpSrtp::srtpUnprotectIfNeeded(const uint8_t* data, int* size, UtlBoolean rtcp)
{
#ifdef ENABLE_SRTP
   srtp_err_status_t status;
   if (!mSrtpSessionCreated)
   {
      // No SRTP configured for this session
      return TRUE;
   }
   if (rtcp == FALSE)
   {
      if (!isValidSrtp(data, *size))
      {
         // Failed basic SRTP validation - likely not an SRTP packet.
         // Avoid srtp_err_status_bad_param error in srtp_unprotect
         return TRUE;
      }
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
      status = srtp_unprotect_rtcp(mSrtpSession, (void*)data, size);
   }
   if (status)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::srtpUnprotectIfNeeded: srtp_unprotect error = %d", status);
      return FALSE;
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
      OsSysLog::add(FAC_MP, PRI_ERR, "MpSrtp::srtpProtectIfNeeded: srtp_protect error = %d", status);
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

