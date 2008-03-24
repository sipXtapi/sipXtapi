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
#include "mp/MpDspUtils.h"

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

MpJitterBuffer::MpJitterBuffer(MpDecoderPayloadMap *pPayloadMap)
: mSampleRate(0)
, mSamplesPerFrame(0)
, mpResampler(NULL)
, mCurFrameNum(0)
, mRemainingSamplesNum(0)
, mpPayloadMap(pPayloadMap)
{
}

void MpJitterBuffer::init(unsigned int samplesPerSec, unsigned int samplesPerFrame)
{
   mSampleRate = samplesPerSec;
   mSamplesPerFrame = samplesPerFrame;

   assert(mpResampler == NULL);
   mpResampler = MpResamplerBase::createResampler(1, mSampleRate, mSampleRate);
}

MpJitterBuffer::~MpJitterBuffer()
{
   delete mpResampler;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpJitterBuffer::pushPacket(MpRtpBufPtr &rtpPacket)
{
   unsigned decodedSamples; // number of samples, returned from decoder
   MpDecoderBase* decoder;  // decoder for the packet
   unsigned codecSampleRate; // Codec sample rate

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

   assert(rtpPacket.isValid());

   RTL_EVENT("MpJitterBuffer_pushPacket_seq", rtpPacket->getRtpSequenceNumber());

//   printf("Got RTP #%u  TS %u  payload %d\n",
//          rtpPacket->getRtpSequenceNumber(),
//          rtpPacket->getRtpTimestamp(),
//          rtpPacket->getRtpPayloadType());

   // Decode packet to temporary resample and slice buffer.
   decodedSamples = decoder->decode(rtpPacket, DECODED_DATA_MAX_LENGTH,
                                    mDecodedData);
#ifdef RTL_AUDIO_ENABLED
   UtlString outputLabel("MpJitterBuffer_pushPacket");
   RTL_RAW_AUDIO(outputLabel,
                 codecSampleRate,
                 decodedSamples,
                 mDecodedData,
                 rtpPacket->getRtpSequenceNumber());
#endif

   // Get source sample rate and update sample rate if changed.
   if (  (codecSampleRate != mSampleRate)
      && (mpResampler->getInputRate() != codecSampleRate))
   {
      mpResampler->setInputRate(codecSampleRate);
   }

   // Reset total samples counter
   assert(mRemainingSamplesNum == 0);
   mRemainingSamplesNum = 0;

   // Slice data to buffers.
   for (unsigned i = 0, frame=0; i < decodedSamples; frame++)
   {
      uint32_t inSamplesNum; // Number of samples in frame before resampling
      uint32_t outSamplesNum; // Number of samples in frame after resampling

      // Get new buffer.
      MpAudioBufPtr pBuf = MpMisc.RawAudioPool->getBuffer();
      if (!pBuf.isValid())
         return OS_FAILED;
      pBuf->setSamplesNumber(mSamplesPerFrame);
      pBuf->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);

      // Copy or resample audio data to buffer.
      if (codecSampleRate == mSampleRate)
      {
         inSamplesNum = sipx_min(mSamplesPerFrame, decodedSamples-i);
         inSamplesNum = sipx_min(inSamplesNum, pBuf->getSamplesNumber());
         memcpy(pBuf->getSamplesWritePtr(), mDecodedData+i,
                inSamplesNum*sizeof(MpAudioSample));
         outSamplesNum = inSamplesNum;
      }
      else
      {
         mpResampler->resample(0,
                               mDecodedData+i, decodedSamples-i, inSamplesNum,
                               pBuf->getSamplesWritePtr(), pBuf->getSamplesNumber(),
                               outSamplesNum);
      }
      i += inSamplesNum;
      // Update number of samples in buffer to actual number of samples.
      pBuf->setSamplesNumber(outSamplesNum);

      // Place buffer to queue.
      if (mFrames[frame].isValid())
      {
         // Seems there is some unread data residing in buffer.
         // todo:: Check for residing data before doing resampling
         //        and append decoded data to it. It will allow us
         //        to support codecs with samples per frame not being
         //        multiple of our frame size.
         printf("Trying to overwrite non-empty frame #%d!\n", frame);
         assert(FALSE);
      }
      mFrames[frame] = pBuf;
      mOriginalSamples[frame] = inSamplesNum;
      mRemainingSamplesNum += outSamplesNum;
//      printf("decoded frame #%d\n", frame);
   }

   // Reset reading pointer.
   mCurFrameNum = 0;

   return OS_SUCCESS;
}

void MpJitterBuffer::getFrame(MpAudioBufPtr &pFrame, int &numOriginalSamples)
{
   // Return empty buffer, if no data is available.
   if (!mFrames[mCurFrameNum].isValid())
   {
//      printf("returning empty frame\n");
      numOriginalSamples = 0;
      return;
   }

//   printf("returning frame #%d\n", mCurFrameNum);

   // Return data
   pFrame.swap(mFrames[mCurFrameNum]);
   numOriginalSamples = mOriginalSamples[mCurFrameNum];

   // Reset pointer and statistic
   mCurFrameNum++;
   mRemainingSamplesNum -= pFrame->getSamplesNumber();
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

void MpJitterBuffer::flush()
{
   for (int i=mCurFrameNum; i<FRAMES_TO_STORE; i++)
   {
      mFrames[i].release();
   }
   mCurFrameNum = 0;
   mRemainingSamplesNum = 0;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
