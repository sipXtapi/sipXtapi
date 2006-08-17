//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef HAVE_GIPS /* [ */

#include "assert.h"
#include "string.h"

#include "mp/JB/JB_API.h"
#include "mp/MpJitterBuffer.h"
#include "mp/MpSipxDecoders.h"
#include "mp/NetInTask.h" // for definition of RTP packet

static int debugCount = 0;

/* ============================ CREATORS ================================== */

MpJitterBuffer::MpJitterBuffer()
{
   for (int i=0; i<JbPayloadMapSize; i++)
      payloadMap[i] = NULL;

   JbQWait = JbLatencyInit;
   JbQCount = 0;
   JbQIn = 0;
   JbQOut = 0;

   debugCount = 0;
}

// Destructor
MpJitterBuffer::~MpJitterBuffer()
{
}

/* ============================ MANIPULATORS ============================== */

int MpJitterBuffer::ReceivePacket(MpRtpBufPtr &rtpPacket)
{
   int numSamples = 0;

   switch (rtpPacket->getRtpPayloadType()) {
   case 0: // G.711 u-Law
   case 8: // G.711 a-Law
      numSamples = rtpPacket->getPayloadSize();
      break;
   default:
      return 0;
   }

   if (JbQWait > 0) 
   {
      JbQWait--;
   }

   if (JbQueueSize == JbQCount) 
   { 
      // discard some data...
      JbQOut = JbQIn + numSamples;
      JbQCount -= numSamples;
   }

   switch (rtpPacket->getRtpPayloadType())
   {
   case 0: // G.711 u-Law
      G711U_Decoder(numSamples, (JB_uchar*)rtpPacket->getPayload(), JbQ+JbQIn);
      break;
   case 8: // G.711 a-Law
      G711A_Decoder(numSamples, (JB_uchar*)rtpPacket->getPayload(), JbQ+JbQIn);
      break;
   default:
      break;
   }

   JbQCount += numSamples;
   JbQIn += numSamples;

   if (JbQIn >= JbQueueSize) 
   {
       JbQIn -= JbQueueSize;
   }
   return 0;
}

int MpJitterBuffer::GetSamples(MpAudioSample *voiceSamples, JB_size *pLength)
{
    int numSamples = 80;

    if (JbQCount == 0) 
    {
        JbQWait = JbLatencyInit; // No data, prime the buffer (again).
        memset((char*) voiceSamples, 0x00, numSamples * sizeof(MpAudioSample));
    }
    else
    {
        memcpy(voiceSamples, JbQ+JbQOut, numSamples * sizeof(MpAudioSample));

        JbQCount -= numSamples;
        JbQOut += numSamples;
        if (JbQOut >= JbQueueSize) 
        {
            JbQOut -= JbQueueSize;
        }
    }

    *pLength = numSamples;
    return 0;
}

int MpJitterBuffer::SetCodepoint(const JB_char* codec, JB_size sampleRate,
   JB_code codepoint)
{
   return 0;
}

/* ===================== Jitter Buffer API Functions ====================== */

JB_ret JB_initCodepoint(JB_inst *JB_inst,
                        const JB_char* codec,
                        JB_size sampleRate,
                        JB_code codepoint)
{
   return JB_inst->SetCodepoint(codec, sampleRate, codepoint);
}

JB_ret JB_RecIn(JB_inst *JB_inst,
                MpRtpBufPtr &rtpPacket)
{
   return JB_inst->ReceivePacket(rtpPacket);
}

JB_ret JB_RecOut(JB_inst *JB_inst,
                 MpAudioSample *voiceSamples,
                 JB_size *pLength)
{
   return JB_inst->GetSamples(voiceSamples, pLength);
}

JB_ret JB_create(JB_inst **pJB)
{
   *pJB = new MpJitterBuffer();
   return 0;
}

JB_ret JB_init(JB_inst *pJB, int fs)
{
   return 0;
}

JB_ret JB_free(JB_inst *pJB)
{
   delete pJB;
   return 0;
}
#endif /* NOT(HAVE_GIPS) ] */
