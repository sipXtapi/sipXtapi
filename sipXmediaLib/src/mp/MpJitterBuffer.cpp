//  
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include "assert.h"
#include "string.h"

#include "os/OsDefs.h" // for min macro
#include "mp/MpJitterBuffer.h"
#include "mp/MpDecoderBase.h"

#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#  ifdef RTL_AUDIO_ENABLED
#     include <SeScopeAudioBuffer.h>
#  endif
#endif

/* ============================ CREATORS ================================== */

MpJitterBuffer::MpJitterBuffer(unsigned int sampleRate)
: mSampleRate(sampleRate)
, mResampler(1, mSampleRate, mSampleRate)
{
   for (int i=0; i<JbPayloadMapSize; i++)
      payloadMap[i] = NULL;

   JbQCount = 0;
   JbQIn = 0;
   JbQOut = 0;
}

// Destructor
MpJitterBuffer::~MpJitterBuffer()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpJitterBuffer::pushPacket(MpRtpBufPtr &rtpPacket)
{
   int bufferSize;          // number of samples could be written to decoded buffer
   unsigned decodedSamples; // number of samples, returned from decoder
   uint8_t payloadType;     // RTP packet payload type
   MpDecoderBase* decoder;  // decoder for the packet
   unsigned codecSampleRate; // Codec sample rate

   payloadType = rtpPacket->getRtpPayloadType();

   // Ignore illegal payload types
   if (payloadType >= JbPayloadMapSize)
      return OS_FAILED;

   // Get decoder
   decoder = payloadMap[payloadType];
   if (decoder == NULL)
      return OS_FAILED; // If we can't decode it, we must ignore it?

   // Calculate space available for decoded samples
   if (JbQIn > JbQOut || JbQCount == 0)
   {
      bufferSize = JbQueueSize-JbQIn;
   } else {
      bufferSize = JbQOut-JbQIn;
   }

   codecSampleRate = decoder->getInfo()->getSampleRate();

   // Check if resampling needed
   if (codecSampleRate == mSampleRate)
   {
      // Decode packet
      decodedSamples = decoder->decode(rtpPacket, bufferSize, JbQ+JbQIn);
      // TODO:: If packet jitter buffer size is not integer multiple of decoded size,
      //        then part of the packet will be lost here. We should consider one of
      //        two ways: set JB size on creation depending on packet size, reported 
      //        by codec, OR push packet into decoder and then pull decoded data in
      //        chunks.

#ifdef RTL_AUDIO_ENABLED
      UtlString outputLabel("MpJitterBuffer_pushPacket");
      RTL_RAW_AUDIO(outputLabel,
                    codecSampleRate,
                    decodedSamples,
                    JbQ+JbQIn,
                    0);
#endif

      // Update buffer state
      JbQCount += decodedSamples;
      JbQIn += decodedSamples;
      // Reset write pointer if we reach end of buffer
      if (JbQIn >= JbQueueSize)
         JbQIn = 0;
   }
   else
   {
      // Update sample rate if changed.
      if (mResampler.getInputRate() != codecSampleRate)
      {
         mResampler.setInputRate(codecSampleRate);
      }

      // Decode packet to temporary resample buffer.
      decodedSamples = decoder->decode(rtpPacket, JbResampleBufSize, JbResampleBuf);
#ifdef RTL_AUDIO_ENABLED
      UtlString outputLabel("MpJitterBuffer_pushPacket");
      RTL_RAW_AUDIO(outputLabel,
                    codecSampleRate,
                    decodedSamples,
                    JbResampleBuf,
                    0);
#endif

      // Resample samples to actual buffer.
      uint32_t inSamplesResampled = 0;
      uint32_t outSamplesResampled = 0;
      const MpAudioSample *pResampleBufPtr = JbResampleBuf;
      while (decodedSamples > 0 && bufferSize > 0)
      {
         mResampler.resample(0,
                             JbResampleBuf, decodedSamples, inSamplesResampled,
                             JbQ+JbQIn, bufferSize, outSamplesResampled);
         decodedSamples -= inSamplesResampled;
         // Update buffer state
         JbQCount += outSamplesResampled;
         JbQIn += outSamplesResampled;
         if (JbQIn >= JbQueueSize)
         {
            // If write pointer reached end of buffer, there may be more space
            // at the beginning.
            JbQIn = 0;
            bufferSize = JbQOut;
         }
         else
         {
            // Write pointer have not reached and of buffer, so we either
            // out of space or are done with resampling.
            bufferSize -= outSamplesResampled;
         }
      }
   }

   return OS_SUCCESS;
}

int MpJitterBuffer::getSamples(MpAudioSample *samplesBuffer, int samplesNumber)
{
   // Check does we have available decoded data
   if (JbQCount != 0) {
      // We could not return more then we have
      samplesNumber = sipx_min(samplesNumber,JbQCount);

#ifdef RTL_AUDIO_ENABLED
      UtlString outputLabel("MpJitterBuffer_getSamples");
      RTL_RAW_AUDIO(outputLabel,
                    8000,
                    samplesNumber,
                    JbQ+JbQOut,
                    0);
#endif

      memcpy(samplesBuffer, JbQ+JbQOut, samplesNumber * sizeof(MpAudioSample));

      JbQCount -= samplesNumber;
      JbQOut += samplesNumber;
      if (JbQOut >= JbQueueSize)
         JbQOut -= JbQueueSize;
   }

   return samplesNumber;
}


int MpJitterBuffer::setCodecList(MpDecoderBase** codecList, int codecCount)
{
	// For every payload type, load in a codec pointer, or a NULL if it isn't there
	for(int i=0; i<codecCount; i++)
   {
		int payloadType = codecList[i]->getPayloadType();
		if(payloadType < JbPayloadMapSize) {
			payloadMap[payloadType] = codecList[i];
		}
	}

   // Reset resampler preparing it to resample new audio stream.
   // Actually we should have MpJitterBuffer::resetStream() function for this.
   mResampler.resetStream();

   return 0;
}
