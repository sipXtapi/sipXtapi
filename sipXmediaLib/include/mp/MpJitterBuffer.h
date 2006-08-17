//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpJitterBuffer_h_
#define _MpJitterBuffer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpRtpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

   static const int JbLatencyInit = 5 ;
   static const int JbPayloadMapSize = 128;
   static const int JbQueueSize = (16 * (2 * 80)); // 16 packets, 20 mS each

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

class MpSipxDecoder;

/// Class for managing dejitter/decode of incoming RTP.
class MpJitterBuffer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpJitterBuffer();

     /// Destructor
   virtual
   ~MpJitterBuffer();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   int ReceivePacket(MpRtpBufPtr &rtpPacket);

   int GetSamples(MpAudioSample *voiceSamples, JB_size *pLength);

   int SetCodepoint(const JB_char* codec, JB_size sampleRate,
                    JB_code codepoint);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor
   MpJitterBuffer(const MpJitterBuffer& rMpJitterBuffer);

     /// Assignment operator
   MpJitterBuffer& operator=(const MpJitterBuffer& rhs);

   int JbQWait;  ///< Fixed latency delay control.
   int JbQCount;
   int JbQIn;
   int JbQOut;
   MpAudioSample JbQ[JbQueueSize];

   MpSipxDecoder* payloadMap[JbPayloadMapSize];
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpJitterBuffer_h_
