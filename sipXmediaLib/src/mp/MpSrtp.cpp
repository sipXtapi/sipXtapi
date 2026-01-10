//  
// Copyright (C) 2026 SIP Specturn, Inc.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#ifdef ENABLE_SRTP
  // Note: RTCP is send from RTCPManager thread, and not yet implemented for SRTP encryption.
  //       It needs to share the same mSrtp session (ideally) as MpToNet, which is not thread-safe, so 
  //       we need to do some locking.  Since RTCP logic is currently buggy and typically disabled,
  //       we will leave this for later.
  #ifdef ENABLE_RTCP
    #error "Conflict detected: ENABLE_SRTP and ENABLE_RTCP cannot both be defined, since SRTP is not implemented for RTCP sending. Please choose one."
  #endif

#include <srtp.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "mp/MpSrtp.h"
#include "mp/MpSetSrtpParamsMsg.h"

// EXTERNAL FUNCTIONS

#ifndef ENABLE_SRTP
// define empty struct to allow compilation without SRTP
struct srtp_policy_t {};
#endif

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

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

OsStatus MpSrtp::setSrtpParams(const UtlString& resourceName, OsMsgQ& flowgraphMessageQueue, SdpMediaLine::SdpCryptoSuiteType cryptoSuite, const UtlString& cryptoKey)
{
#ifdef ENABLE_SRTP
   // Note:  this message is handled in MpRtp(Input/Output)Connection::handleMessage and relayed to Mpr(From/To)Net::setSrtpParams
   MpSetSrtpParamsMsg message(resourceName, cryptoSuite, cryptoKey);
   return(flowgraphMessageQueue.send(message, OsTime::OS_INFINITY /*sOperationQueueTimeout*/));
#else
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

