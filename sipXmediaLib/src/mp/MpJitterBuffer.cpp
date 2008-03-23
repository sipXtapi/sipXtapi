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


// SYSTEM INCLUDES
//#include "assert.h"
//#include "string.h"

// APPLICATION INCLUDES
#include "os/OsDefs.h" // for min macro
#include "mp/MpJitterBuffer.h"
#include "mp/MpDecoderPayloadMap.h"
#include "mp/MpDecoderBase.h"
#include "mp/MpPlcBase.h"

#define RTL_ENABLED
#define RTL_AUDIO_ENABLED

#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#  ifdef RTL_AUDIO_ENABLED
#     include <SeScopeAudioBuffer.h>
#  endif
#else
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpJitterBuffer::MpJitterBuffer(const UtlString &plcName,
                               MpDecoderPayloadMap *pPayloadMap)
: mSampleRate(0)
, mSamplesPerFrame(0)
, mpResampler(NULL)
, mIsFirstPacket(TRUE)
, mOldestFrameNum(0)
, mpPayloadMap(pPayloadMap)
, mpPlc(MpPlcBase::createPlc(plcName))
{
}

void MpJitterBuffer::init(unsigned int samplesPerSec, unsigned int samplesPerFrame)
{
   mSampleRate = samplesPerSec;
   mSamplesPerFrame = samplesPerFrame;

   assert(mpPlc != NULL);
   mpPlc->init(mSampleRate, mSamplesPerFrame);

   assert(mpResampler == NULL);
   mpResampler = MpResamplerBase::createResampler(1, mSampleRate, mSampleRate);
}

MpJitterBuffer::~MpJitterBuffer()
{
   delete mpResampler;
   delete mpPlc;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpJitterBuffer::pushPacket(MpRtpBufPtr &rtpPacket)
{
   unsigned decodedSamples; // number of samples, returned from decoder
   MpDecoderBase* decoder;  // decoder for the packet
   unsigned codecSampleRate; // Codec sample rate
   unsigned frameIndex; // Index of frame in mFrames[] array.

   // If initialization was not completed properly.
   if (mpPayloadMap == NULL || mSampleRate == 0)
   {
      assert(FALSE);
      return OS_FAILED;
   }

   // Get decoder
   decoder = mpPayloadMap->mapPayloadType(rtpPacket->getRtpPayloadType());
   // If we can't decode it, we should ignore it.
   if (decoder == NULL)
   {
      return OS_FAILED;
   }
   codecSampleRate = decoder->getInfo()->getSampleRate();

   RTL_EVENT("MpJitterBuffer_pushPacket_loss_patern", !rtpPacket.isValid());

   // Do nothing if there is no incoming packet (i.e. it is lost) and
   // decoder is not capable of doing PLC. We'll do PLC later, so there is
   // nothing wrong wit it and we're returning OS_SUCCESS.
   if (!rtpPacket.isValid() && !decoder->getInfo()->haveInternalPLC())
   {
      return OS_SUCCESS;
   }

   // If this packet is first we've received, initialize timestamp.
   if (mIsFirstPacket == TRUE)
   {
      mIsFirstPacket = FALSE;
      mOldestTimestamp = rtpPacket->getRtpTimestamp();
   }

   // Decode packet to temporary resample and slice buffer.
   decodedSamples = decoder->decode(rtpPacket, DECODED_DATA_MAX_LENGTH,
                                    mDecodedData);
#ifdef RTL_AUDIO_ENABLED
   UtlString outputLabel("MpJitterBuffer_pushPacket");
   RTL_RAW_AUDIO(outputLabel,
                 codecSampleRate,
                 decodedSamples,
                 mDecodedData,
                 0);
#endif

   // Get source sample rate and update sample rate if changed.
   if (  (codecSampleRate != mSampleRate)
      && (mpResampler->getInputRate() != codecSampleRate))
   {
      mpResampler->setInputRate(codecSampleRate);
   }

   // Get RTP packet timestamp and convert it to mFrames[] array index.
   {
      RtpTimestamp audioTimestamp;

      // Note, that this subtraction allows to handle timestamp wraparound
      // correctly. Also note, that this subtraction is possibly only
      // because mOldestTimestamp is not too far in past from current timestamp.
      audioTimestamp = rtpPacket->getRtpTimestamp() - mOldestTimestamp;
      
      // Change timestamp according to resampling if needed.
      if (codecSampleRate != mSampleRate)
      {
         // Here we assume that (audioTimestamp*mSampleRate < 2^32).
         audioTimestamp = (audioTimestamp * mSampleRate) / codecSampleRate;
      }

      // Convert timestamp to mFrames[] index.
      frameIndex = mOldestFrameNum + audioTimestamp / mSamplesPerFrame;
      frameIndex = frameIndex % FRAMES_TO_STORE;
   }


   // Slice data to buffers.
   for (unsigned i = 0; i < decodedSamples; )
   {
      unsigned samplesNum; // Number of samples to copy.

      // Get new buffer.
      MpAudioBufPtr pBuf = MpMisc.RawAudioPool->getBuffer();
      if (!pBuf.isValid())
         return OS_FAILED;
      pBuf->setSamplesNumber(mSamplesPerFrame);
      pBuf->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);

      // Copy or resample audio data to buffer.
      if (codecSampleRate == mSampleRate)
      {
         samplesNum = sipx_min(mSamplesPerFrame, decodedSamples-i);
         samplesNum = sipx_min(samplesNum, pBuf->getSamplesNumber());
         memcpy(pBuf->getSamplesWritePtr(), mDecodedData+i,
                samplesNum*sizeof(MpAudioSample));
         i += samplesNum;
      }
      else
      {
         uint32_t inSamplesResampled;
         uint32_t outSamplesResampled;

         mpResampler->resample(0,
                               mDecodedData+i, decodedSamples-i, inSamplesResampled,
                               pBuf->getSamplesWritePtr(), pBuf->getSamplesNumber(),
                               outSamplesResampled);

         i += inSamplesResampled;
         samplesNum = outSamplesResampled;
      }
      // Update number of samples in buffer to actual number of samples.
      pBuf->setSamplesNumber(samplesNum);

      // Place buffer to queue.
      // Note: here we should check that we're not going to overwrite
      // old data by checking frameIndex pointer vs mOldestFrameIndex.
      // But we're doing simpler check instead and hope it will never
      // happen.
      assert(!mFrames[frameIndex].isValid());
      mFrames[frameIndex] = pBuf;
//      printf("decoded frame #%d\n", frameIndex);
      // Advance index for next buffer.
      frameIndex = (frameIndex+1) % FRAMES_TO_STORE;
   }

   return OS_SUCCESS;
}

MpAudioBufPtr MpJitterBuffer::getSamples()
{
   const MpAudioSample *plcSamplesIn = NULL;
   MpAudioSample *plcSamplesOut = NULL;
   UtlBoolean plcIsFrameModified;
   OsStatus plcResult;

   // Get frame from queue.
   MpAudioBufPtr pFrame;
   pFrame.swap(mFrames[mOldestFrameNum%FRAMES_TO_STORE]);
//   printf("returned frame #%d (index: %d)\n",
//          mOldestFrameNum, mOldestFrameNum%FRAMES_TO_STORE);

   RTL_EVENT("MpJitterBuffer_getSamples_loss_patern", !pFrame.isValid());

   // Prepare pointers to input and output frames for PLC.
   if (pFrame.isValid())
   {
      plcSamplesIn = pFrame->getSamplesPtr();
   }
   if (!mTempPlcFrame.isValid())
   {
      mTempPlcFrame = MpMisc.RawAudioPool->getBuffer();
      assert(mTempPlcFrame.isValid());
      mTempPlcFrame->setSamplesNumber(mSamplesPerFrame);
      mTempPlcFrame->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);
   }
   plcSamplesOut = mTempPlcFrame->getSamplesWritePtr();

   // Pass frame through PLC.
   plcResult = mpPlc->processFrame(mOldestFrameNum, mOldestFrameNum,
                                   plcSamplesIn, plcSamplesOut,
                                   &plcIsFrameModified);
   assert(plcResult == OS_SUCCESS);
   if (plcResult == OS_SUCCESS && plcIsFrameModified)
   {
      pFrame.release();
      pFrame.swap(mTempPlcFrame);
   }

#ifdef RTL_AUDIO_ENABLED
   UtlString outputLabel("MpJitterBuffer_getSamples");
   RTL_RAW_AUDIO(outputLabel,
                 mSampleRate,
                 pFrame->getSamplesNumber(),
                 pFrame->getSamplesPtr(),
                 0);
#endif

   // Advance queue pointer.
   mOldestFrameNum++;
   mOldestTimestamp += mSamplesPerFrame;

   return pFrame;
}


void MpJitterBuffer::setCodecList(MpDecoderPayloadMap *pPayloadMap)
{
   // Update list of decoders.
   mpPayloadMap = pPayloadMap;

   // Reset resampler preparing it to resample new audio stream.
   // Actually we should have MpJitterBuffer::resetStream() function for this.
   if (mpPayloadMap != NULL)
   {
      mpResampler->resetStream();
   }
}

void MpJitterBuffer::setPlc(const UtlString &plcName)
{
   // Free old PLC
   delete mpPlc;

   // Set PLC to new one and init it.
   mpPlc = MpPlcBase::createPlc(plcName);
   mpPlc->init(mSampleRate, mSamplesPerFrame);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
