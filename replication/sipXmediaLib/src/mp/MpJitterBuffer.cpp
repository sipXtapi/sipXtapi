//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef HAVE_GIPS /* [ */

#include "assert.h"
#include "string.h"

#include "mp/JB/JB_API.h"
#include "mp/MpJitterBuffer.h"
#include "mp/MpSipxDecoders.h"
#include "mp/NetInTask.h" // for definition of RTP packet

static int debugCount = 0;

/* ============================ CREATORS ================================== */

MpJitterBuffer::MpJitterBuffer(void)
{
   int i;

   for (i=0; i<JbPayloadMapSize; i++) payloadMap[i] = NULL;
   //JbQWait = JbLatencyInit;
   JbQCount = 0;
   JbQIn = 0;
   JbQOut = 0;

   debugCount = 0;
}

//:Destructor
MpJitterBuffer::~MpJitterBuffer()
{
}

/* ============================ MANIPULATORS ============================== */

int MpJitterBuffer::SetCodecList(MpDecoderBase** codecList, int codecCount) {
	// For every payload type, load in a codec pointer, or a NULL if it isn't there
	for(int i=0;i<codecCount;i++) {
		int payloadType = codecList[i]->getPayloadType();
		if(payloadType < JbPayloadMapSize) {
			payloadMap[payloadType] = codecList[i];
		}
	}
	return 1;
}

int MpJitterBuffer::ReceivePacket(JB_uchar* RTPpacket, JB_size RTPlength, JB_ulong TS)
{
   int numSamples = 0;
   unsigned char* pRtpData = NULL;
   struct rtpHeader* pHdr = (struct rtpHeader*) RTPpacket;
   int cc;
   int payloadType;
   int overhead;
   // TS appears to be set to 0 by the caller

   payloadType = (pHdr->mpt) & 0x7f;
   cc = (pHdr->vpxcc) & 0x0f;

   overhead = sizeof(struct rtpHeader) + (cc*sizeof(int));
      numSamples = RTPlength - overhead;;
      pRtpData = RTPpacket + overhead;

   if(payloadType >= JbPayloadMapSize) return 0;  // Ignore illegal payload types
   MpDecoderBase* decoder = payloadMap[payloadType];
   // JbQ is a buffer of "Sample"
   if(decoder != NULL) {
      decoder->decode(pRtpData,numSamples,JbQ+JbQIn);
   } else {
	   return 0; // If we can't decode it, we must ignore it?
   }
   int outSamples = decoder->getInfo()->getNumSamplesPerFrame();
   JbQCount += outSamples;
   JbQIn += outSamples;
   if (JbQIn >= JbQueueSize) JbQIn -= JbQueueSize;
   // This will blow up if Samples Per Frame is not an exact multiple of 80

   //if (JbQWait > 0) {
   //   JbQWait--;
   //}
   return 0;
}

int MpJitterBuffer::GetSamples(Sample *voiceSamples, JB_size *pLength)
{
    int numSamples = 80;

   //if (0 >= JbQCount) {
   //   JbQWait = JbLatencyInit; // No data, prime the buffer (again).
	//  JbQCount=0; 
   //}
  // if (JbQWait > 0) {
   //   memset((char*) voiceSamples, 0, 80 * sizeof(Sample));
   //} else {
   if(JbQOut == JbQIn) {
		// No packet available
   } else {
        memcpy(voiceSamples, JbQ+JbQOut, numSamples * sizeof(Sample));

        JbQCount -= numSamples;
        JbQOut += numSamples;
      if (JbQOut >= JbQueueSize) JbQOut -= JbQueueSize;
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
                      JB_uchar* RTPpacket,
                      JB_size RTPlength,
                      JB_ulong timeStamp)
{
   return JB_inst->ReceivePacket(RTPpacket, RTPlength, timeStamp);
}

JB_ret JB_RecOut(JB_inst *JB_inst,
                      Sample *voiceSamples,
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
