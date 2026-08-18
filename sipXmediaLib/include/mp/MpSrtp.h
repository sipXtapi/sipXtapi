//  
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpSrtp_h_
#define _MpSrtp_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <sdp/SdpMediaLine.h>
#include <utl/UtlString.h>
#include <utl/UtlBool.h>
#include <os/OsStatus.h>
#include <os/OsMsgQ.h>
#include <os/OsMutex.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// Forwward declarations to keep from needing to include srtp.h here
#ifdef __cplusplus
extern "C" {
#endif
   struct srtp_ctx_t_;
   typedef struct srtp_ctx_t_* srtp_t;
#ifdef __cplusplus
}
#endif

// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Which of a connection's two protected streams a given piece of SRTP key
/// material applies to.
///
/// This is not a stylistic distinction -- it follows the keying protocol:
///
///   - SDES (RFC 4568) carries ONE master key per media line, and it protects
///     both SRTP and SRTCP no matter how the ports are laid out.  That is
///     MP_SRTP_KEY_USE_RTP_AND_RTCP.
///
///   - DTLS-SRTP (RFC 5764) keys a DTLS association, and section 3 requires a
///     separate association for each source/destination port pair.  Without
///     rtcp-mux that means two handshakes and two unrelated master keys, and
///     section 4.2 is explicit that each association discards the half it does
///     not need: the RTP association's SRTCP keys, and the RTCP association's
///     SRTP keys.  Those are MP_SRTP_KEY_USE_RTP_ONLY and
///     MP_SRTP_KEY_USE_RTCP_ONLY respectively.
///
///   - With rtcp-mux there is one association covering one port and both
///     halves are used -- MP_SRTP_KEY_USE_RTP_AND_RTCP again.
enum MpSrtpKeyUse
{
   MP_SRTP_KEY_USE_RTP_AND_RTCP = 0,  ///< SDES, or DTLS-SRTP with rtcp-mux
   MP_SRTP_KEY_USE_RTP_ONLY,          ///< DTLS-SRTP, RTP transport association
   MP_SRTP_KEY_USE_RTCP_ONLY          ///< DTLS-SRTP, RTCP transport association
};

/// Human readable name, for logging.
const char* MpSrtpKeyUseString(MpSrtpKeyUse keyUse);


/// Per-direction SRTP/SRTCP context.
///
/// Instances are thread-safe: every method that touches the underlying
/// libsrtp session takes an internal lock.  libsrtp itself provides no
/// serialization for a single srtp_t, and each of the three instances in
/// the media stack is genuinely shared across threads:
///
///   - MprFromNet's instance unprotects on the NetInTask thread while the
///     media thread installs keys.
///   - MprToNet's instance protects RTP on the media thread, which is also
///     where keys are installed.
///   - CRTCPRender's instance protects RTCP on the CRTCManager thread while
///     the media thread installs keys.
class MpSrtp
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpSrtp();

     /// Destructor
   virtual ~MpSrtp();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   // Static helper method to initialize the SRTP library. The SRTP library
   // only needs to be initialized once per applciation instance, on application startup.
   // Also probes libsrtp at first call to determine which SRTP crypto suites
   // the linked-in libsrtp build actually supports (GCM suites in particular
   // require libsrtp to have been built against an external crypto backend
   // such as OpenSSL or NSS); results are cached for isCryptoSuiteSupported().
   static OsStatus globalInitialize();

   // Static helper method to shutdown the SRTP library. Only called once
   // on application shutdown.
   static OsStatus globalShutdown();

   // Returns TRUE if the linked-in libsrtp build supports the given SRTP
   // crypto suite, FALSE otherwise. Useful for SDES- and DTLS-SRTP code
   // that wants to filter or validate suite lists against runtime capability.
   //
   // The result is determined by a one-time probe run at globalInitialize()
   // that attempts srtp_create() with each suite. Returns FALSE for suites
   // that have no libsrtp policy mapping at all (e.g. F8) and for
   // CRYPTO_SUITE_TYPE_NONE.
   //
   // Must not be called before globalInitialize(); doing so returns FALSE
   // for every suite.
   static bool isCryptoSuiteSupported(SdpMediaLine::SdpCryptoSuiteType suite);

   // Static helper method to send a MpSetSrtpParamsMsg to media resources for processing.
   // Eventually to be consumed by MprFromNet and MprToNet media processors.
   static OsStatus setSrtpParams(const UtlString& resourceName, 
      OsMsgQ& flowgraphMessageQueue, 
      SdpMediaLine::SdpCryptoSuiteType cryptoSuite, 
      const UtlString& cryptoKey,
      MpSrtpKeyUse keyUse = MP_SRTP_KEY_USE_RTP_AND_RTCP);

   // Set the SRTP parameters for this Srtp session.  Note:  You use a seperate instance of this 
   // class for each direction (Protect/Outbound/ToNet vs. Unprotect/Inbound/FromNet). The instance
   // mode is specified with the forUnprotect parameter.
   // Note:  The cryptoKey for unprotect come from the remote partys SDP offer/answer, and
   //        the cryptoKey for protect comes from our local SDP offer/answer.
   UtlBoolean setSrtpParams(SdpMediaLine::SdpCryptoSuiteType cryptoSuite, const UtlString& cryptoKey, UtlBoolean forUnprotect);

   UtlBoolean isSessionCreated();

   // Note: The passed in buffer is modified in place to contain the unprotected data.
   //       The returned size will always be <= the passed in size.
   // Warning: This method assumes that you called setSrtpParams with forUnprotect = TRUE
   UtlBoolean srtpUnprotectIfNeeded(const uint8_t* data, int* size, UtlBoolean rtcp);

   // Note: The passed in buffer is modified in place to contain the protected data.
   //       The returned size will always be 4 or 10 bytes larger the passed in size for RTP,
   //       and up to 14 bytes for RTCP.  It is recommended to have at least 20 extra 
   //       bytes available in the buffer to also allow room for a 4-byte MKI value.
   // The available buffer size is passed in as maxBufferSize to prevent buffer overruns.
   // Warning: This method assumes that you called setSrtpParams with forUnprotect = FALSE
   UtlBoolean srtpProtectIfNeeded(const uint8_t* data, int* size, UtlBoolean rtcp, int maxBufferSize);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{
 
   // Fast-path validation for Inbound SRTP (Media)
   // 1. Minimum length for AES-128_HMAC_SHA1_32 is 16 bytes
   //    - 12 bytes: Fixed RTP Header
   //    -  4 bytes: Authentication Tag (Truncated HMAC-SHA1-32)
   // 2. Version must be 2 (0x80 mask)
   // 3. Payload Type (PT) must be between 0 and 127
   static bool isValidSrtp(const uint8_t* buf, size_t len);

   // Fast-path validation for Inbound SRTCP (Control)
   // 1. Minimum length for AES - 128_HMAC_SHA1_32 is 16 bytes
   //    - 8 bytes : Fixed RTCP Header(Header + SSRC)
   //    - 4 bytes : SRTCP Index(Mandatory 31 - bit index + 1 - bit E - flag)
   //    - 4 bytes : Authentication Tag(Truncated HMAC - SHA1 - 32)
   // 2. Version must be 2 (0x80 mask)
   // 3. Packet Type (Byte 1) must be in the RTCP range (192-223)
   static bool isValidSrtcp(const uint8_t* buf, size_t len);

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Serializes access to mSrtpSession and the parameters describing it.
     /// Declared first so it is constructed before -- and destroyed after --
     /// everything it guards.
   OsMutex mLock;

   SdpMediaLine::SdpCryptoSuiteType mCryptoSuite;
   UtlString mCryptoKey;

   UtlBoolean mSrtpSessionCreated;
   srtp_t mSrtpSession;

     /// Tear down the libsrtp session.  Caller must hold mLock (or be the
     /// destructor, where no other thread may still be using the object).
   void deallocateSrtpSession();

     /// Copy constructor (not implemented for this class)
   MpSrtp(const MpSrtp& rMpSrtp);

     /// Assignment operator (not implemented for this class)
   MpSrtp& operator=(const MpSrtp& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpSrtp_h_
