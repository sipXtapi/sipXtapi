//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpJitterBuffer_h_
#define _MpJitterBuffer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
// #include "mp/MpBuf.h"
#include "mp/MpDecoderBase.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

   //static const int JbLatencyInit = 0;  // No real delay in this section. That is handled in MprDejitter
   static const int JbPayloadMapSize = 128;
   static const int JbQueueSize = (8 * (2 * 80)); // 8 packets, 20 mS each

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

class MpSipxDecoder;

//:class for managing dejitter/decode of incoming RTP.
class MpJitterBuffer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MpJitterBuffer(void);
     //:Constructor
     // Returns a new jitter buffer object.
     ////!param: ARGs - (in? out?) What?

   virtual
   ~MpJitterBuffer(void);
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   int ReceivePacket(JB_uchar* RTPpacket, JB_size RTPlength, JB_ulong TS);

   int GetSamples(Sample *voiceSamples, JB_size *pLength);

   int SetCodepoint(const JB_char* codec, JB_size sampleRate,
      JB_code codepoint);

   int SetCodecList(MpDecoderBase** codecList, int codecCount);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpJitterBuffer(const MpJitterBuffer& rMpJitterBuffer);
     //:Copy constructor

   MpJitterBuffer& operator=(const MpJitterBuffer& rhs);
     //:Assignment operator

   //int JbQWait;  // fixed latency delay control
   int JbPacketsAvail;
   int JbQCount;
   int JbQIn;
   int JbQOut;
   Sample JbQ[JbQueueSize];
   //MpSipxDecoder* payloadMap[JbPayloadMapSize];
   MpDecoderBase* payloadMap[JbPayloadMapSize];
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpJitterBuffer_h_
