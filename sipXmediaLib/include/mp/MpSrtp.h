//  
// Copyright (C) 2026 SIP Specturn, Inc.
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
   static OsStatus globalInitialize();

   // Static helper method to shutdown the SRTP library. Only called once
   // on application shutdown.
   static OsStatus globalShutdown();

   // Static helper method to send a MpSetSrtpParamsMsg to media resources for processing.
   // Eventually to be consumed by MprFromNet and MprToNet media processors.
   static OsStatus setSrtpParams(const UtlString& resourceName, 
      OsMsgQ& flowgraphMessageQueue, 
      SdpMediaLine::SdpCryptoSuiteType cryptoSuite, 
      const UtlString& cryptoKey);

   // Set the SRTP parameters for this Srtp session.  Note:  You use a seperate instance of this 
   // class for each direction (Protect/Outbound/ToNet vs. Unprotect/Inbound/FromNet). The instance
   // mode is specified with the forUnprotect parameter.
   // Note:  The cryptoKey for unprotect come from the remote partys SDP offer/answer, and
   //        the cryptoKey for protect comes from our local SDP offer/answer.
   UtlBoolean setSrtpParams(SdpMediaLine::SdpCryptoSuiteType cryptoSuite, const UtlString& cryptoKey, UtlBoolean forUnprotect);

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
   // 1. Minimum length for AES - 128_HMAC_SHA1_32 is 20 bytes
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

   SdpMediaLine::SdpCryptoSuiteType mCryptoSuite;
   UtlString mCryptoKey;

   UtlBoolean mSrtpSessionCreated;
   srtp_t mSrtpSession;

   void deallocateSrtpSession();

     /// Copy constructor (not implemented for this class)
   MpSrtp(const MpSrtp& rMpSrtp);

     /// Assignment operator (not implemented for this class)
   MpSrtp& operator=(const MpSrtp& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpSrtp_h_
