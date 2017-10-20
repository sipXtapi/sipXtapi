//  
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
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
#ifdef ANDROID
   // Set to 1 to disable, 0 to enable verbose (LOGV) messages
#  define LOG_NDEBUG 1
#  define LOG_TAG "MpJitterBuffer"

#  include <utils/Log.h>
#endif


//#include "assert.h"
//#include "string.h"

// APPLICATION INCLUDES
#include <os/OsDefs.h> // for min macro
#include <os/OsSysLog.h>
#include <mp/MpJitterBuffer.h>
#include <mp/MpDecoderPayloadMap.h>
#include <mp/MpDecoderBase.h>
#include <mp/MpPlcBase.h>
#include <mp/MpVadBase.h>
#include <mp/MpAgcBase.h>
#include <mp/MpDspUtils.h>
// For debug use only
#include <mp/MpFlowGraphBase.h>

// MACROS
//#define RTL_ENABLED
#undef RTL_AUDIO_ENABLED
#ifndef EXTERNAL_PLC // [ 
#  define ENABLE_NON_PLC_ADJUSTMENT
#endif // !EXTERNAL_PLC ] 

#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#  ifdef RTL_AUDIO_ENABLED
#     include <SeScopeAudioBuffer.h>
#  endif
#else
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#endif

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT // [
#  define dprintf printf
#else // DEBUG_PRINT ][
static void dprintf(const char *, ...) {};
#endif // DEBUG_PRINT ]

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpJitterBuffer::MpJitterBuffer(MpDecoderPayloadMap *pPayloadMap, 
                               const UtlString& resourceName)
: mStreamSampleRate(0)
, mOutputSampleRate(0)
, mSamplesPerFrame(0)
, mpResampler(NULL)
, mCurFrameNum(0)
, mRemainingSamplesNum(0)
, mIsFirstPacket(TRUE)
, mStreamRtpPayload(-1)
, mpPayloadMap(pPayloadMap)
, mSamplesPerPacket(0)
, mpPlc(NULL)
, mpVad(NULL)
, mpAgc(NULL)
, mpFlowGraph(NULL)
{
   mpVad = MpVadBase::createVad();
   mpVad->setName(resourceName);
   mpAgc = MpAgcBase::createAgc();
}

void MpJitterBuffer::init(unsigned int samplesPerSec, unsigned int samplesPerFrame)
{
   mOutputSampleRate = samplesPerSec;
   mSamplesPerFrame = samplesPerFrame;

   assert(mpResampler == NULL);
   mpResampler = MpResamplerBase::createResampler(1, mOutputSampleRate, mOutputSampleRate);

}

MpJitterBuffer::~MpJitterBuffer()
{
   delete mpResampler;
   delete mpPlc;
   delete mpVad;
   delete mpAgc;
}

/* ============================ MANIPULATORS ============================== */

void MpJitterBuffer::reset()
{
   // Release audio buffers.
   for (int i=0; i<FRAMES_TO_STORE; i++)
   {
      if (mFrames[i].isValid())
      {
         mFrames[i].release();
      }
   }

   // Reset VAD, AGC, PLC and resampler
   mpVad->reset();
   mpAgc->reset();
   if (!mIsFirstPacket)
   {
      // We can't use simple reset() for PLC, because we do not know whether
      // next stream will have the same sample rate or not. Thus we have
      // to do a full reset.
      mpPlc->fullReset();
   }
   if (mpResampler != NULL)
   {
      mpResampler->resetStream();
   }

   // Reset variables to initial values.
   mCurFrameNum = 0;
   mRemainingSamplesNum = 0;
   mIsFirstPacket = TRUE;
   mStreamRtpPayload = -1;
   mSamplesPerPacket = 0;
   mStreamSampleRate = 0;
}

OsStatus MpJitterBuffer::pushPacket(const MpRtpBufPtr &rtpPacket,
                                    int minBufferSamples,
                                    int wantedBufferSamples,
                                    int &decodedSamples,
                                    int &adjustment,
                                    UtlBoolean &played)
{
   unsigned outSamplesNum = 0;  // Number of samples in output buffer.
   MpDecoderBase* decoder;      // Decoder for the packet.
   MpSpeechParams packetSpeechParams;
   RtpSeq rtpSeq;

   // If decoders and/or MpJitterBuffer are not initialized.
   if (mpPayloadMap == NULL || mOutputSampleRate == 0)
   {
      return OS_FAILED;
   }

   // Initialize variables we have to return.
   adjustment = 0;
   decodedSamples = 0;
   played = FALSE;

   // Initialize stream if not initialized yet, otherwise update variables
   // before decoding next packet.
   if (mIsFirstPacket)
   {
      assert(rtpPacket.isValid());
      mStreamSeq = rtpPacket->getRtpSequenceNumber();
      mStreamTimestamp = rtpPacket->getRtpTimestamp();
      // mIsFirstPacket will be set to FALSE after all checks.
   }

   // STEP 0. Consistency checks.
   if (rtpPacket.isValid())
   {
      rtpSeq = rtpPacket->getRtpSequenceNumber();
      RTL_EVENT("MpJitterBuffer_pushPacket_seq", rtpSeq);

      // Silently drop packets which are too late and can;t be used for
      // updating PLC history.
      if (  MpDspUtils::compareSerials(rtpSeq, mStreamSeq) < 0
         && (int)(mStreamSeq - rtpSeq) > mpPlc->getMaxDelayedFramesNum())
      {
         dprintf(" drop");
         RTL_EVENT("MpJitterBuffer_packet_vad", -1);
         return OS_SUCCESS;
      }

      if (MpDspUtils::compareSerials(rtpSeq, mStreamSeq) >= 0)
      {
         mStreamRtpPayload = rtpPacket->getRtpPayloadType();
         mStreamTimestamp = rtpPacket->getRtpTimestamp();
         mStreamSeq = rtpPacket->getRtpSequenceNumber();
         played = TRUE;
      }
      decoder = mpPayloadMap->mapPayloadType(rtpPacket->getRtpPayloadType());
   }
   else
   {
      // If we haven't got a packet, we have to proceed with PLC.
      // So, set sequence number of this "virtual" packet to our local
      // sequence number.
      decoder = mpPayloadMap->mapPayloadType(mStreamRtpPayload);
   }

   // STEP 1. Decoding
   if (decoder == NULL)
   {
      // Ignore it, if we can't decode it.
      dprintf(" can't decode");
      return OS_FAILED;
   }
   RTL_EVENT("MpJitterBuffer_loss_pattern", !rtpPacket.isValid());
   if (mIsFirstPacket)
   {
      // Now is a good time to drop mIsFirstPacket flag and perform
      // remaining initialization.
      mIsFirstPacket = FALSE;
      mStreamSampleRate = decoder->getInfo()->getSampleRate();
      mpPlc->init(mStreamSampleRate);

      // Have to wait for the first packet to initialize AGC and VAD
      // as we need the specific codec sample rate to do so.
      if (mpVad)
      {
         OsStatus status = mpVad->init(mStreamSampleRate);
         assert(status == OS_SUCCESS);
      }

      if (mpAgc)
      {
         OsStatus status = mpAgc->init(mStreamSampleRate);
         assert(status == OS_SUCCESS);
      }

   }
   if (rtpPacket.isValid() || decoder->getInfo()->haveInternalPLC())
   {
      // Decode packet to temporary resample and slice buffer.
      decodedSamples = decoder->decode(rtpPacket, DECODED_DATA_MAX_LENGTH,
                                       mDecodedData);
      if (decodedSamples > 0)
      {
         // TODO:: For now we do not support samplerate change in the middle
         //        of a stream.
         assert(mStreamSampleRate == (int)decoder->getInfo()->getSampleRate());

         // Usual audio packet have been decoded, life is fine
         mSamplesPerPacket = decodedSamples;
#ifdef RTL_AUDIO_ENABLED
         UtlString outputLabel("MpJitterBuffer_pushPacket");
         RTL_RAW_AUDIO(outputLabel,
                       mStreamSampleRate,
                       decodedSamples,
                       mDecodedData,
                       rtpPacket->getRtpSequenceNumber());
#endif

         // STEP 2. Automatic Gain Calculation and Voice Activity Detection
         mpAgc->processFrame(mDecodedData, decodedSamples);
         mpAgc->getAmplitude(packetSpeechParams.mAmplitude,
                             packetSpeechParams.mIsClipped);
#ifdef TEST_PRINT
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpJitterBuffer VAD processFrame(timeStamp: %d,",
                 mStreamTimestamp);
#endif
         packetSpeechParams.mSpeechType = mpVad->processFrame(mStreamTimestamp,
                                                              mDecodedData, decodedSamples,
                                                              packetSpeechParams);
         packetSpeechParams.mFrameEnergy = mpVad->getEnergy();
      }
      else if (decoder->getInfo()->isSignalingCodec())
      {
         // Signaling (e.g. RFC2833/4733) packet have been decoded,
         // well, we have nothing else to do here. Actual signaling
         // data processing will be done in MprDecode.
         RTL_EVENT("MpJitterBuffer_packet_vad", -2);
         dprintf(" signaling");
         return OS_SUCCESS;
      }
      else
      {
         // Something is definitely wrong here.
         UtlString socketIpAddress;
         OsSocket::inet_ntoa_pt(rtpPacket->getIP(), socketIpAddress);
         OsSysLog::add(FAC_MP, PRI_ERR,
                 "MpJitterBuffer::pushPacket payload ID: %d decoder: %s returned zero samples for playload size: %d from: %s:%d in flowgraph: %p frame num: %d",
                 rtpPacket->getRtpPayloadType(), decoder->getInfo()->getCodecName(), rtpPacket->getPayloadSize(), socketIpAddress.data(), rtpPacket->getUdpPort(), mpFlowGraph, 
                 mpFlowGraph ? mpFlowGraph->numFramesProcessed() : -2);

         // Dump the packet so we can see why decode failed
         // Should not do this in normal cases as this does a new/malloc in the middle of media task runtime sensaive code
         if(OsSysLog::willLog(FAC_MP, PRI_DEBUG))
         {
             UtlString hexEncodedPacket;
             hexEncodedPacket.appendBinaryToString(rtpPacket->getDataPtr(), rtpPacket->getPayloadSize());
             OsSysLog::add(FAC_MP, PRI_ERR,
                     "MpJitterBuffer::pushPacket decode failed on RTP packet: \n%s",
                     hexEncodedPacket.data());

             OsSysLog::flush();
         }
         // But, in release mode we could continue with PLC.
         //assert(!"Decoder returned 0 samples for non-signaling packet!");
      }
   }
   RTL_EVENT("MpJitterBuffer_packet_vad", packetSpeechParams.mSpeechType);
   dprintf(" %d", packetSpeechParams.mSpeechType);

   int wantedAdjustment = wantedBufferSamples + mSamplesPerPacket - getSamplesNum();
#ifdef ANDROID
   int wantedAdjustmentOrig = wantedAdjustment;
#endif

#define N_POS  2
#define N_NEG  3
   switch (packetSpeechParams.mSpeechType)
   {
   case MP_SPEECH_TONE:
      break;
   case MP_SPEECH_ACTIVE:
      // In case of active speech allow only big adjustments.
      // TODO:: Make this heuristic better!
      if (  wantedAdjustment < N_POS*(int)mSamplesPerPacket
         && wantedAdjustment > -N_NEG*(int)mSamplesPerPacket)
      {
         wantedAdjustment = 0;
      }
      break;
   case MP_SPEECH_UNKNOWN:
      if (decodedSamples > 0)
      {
         // Do nothing if VAD haven't determined speech type.
         break;
      }
      // But in case of lost packet, we need to round down wantedAdjustment.
   case MP_SPEECH_SILENT:
   case MP_SPEECH_COMFORT_NOISE:
      // Round wanted adjustment to be integer multiple of frame size.
/*      if (wantedAdjustment>0)
      {
         wantedAdjustment =
            ((wantedAdjustment+getSamplesNum()+mSamplesPerFrame/2)/mSamplesPerFrame)
               * mSamplesPerFrame
            - (getSamplesNum()+mSamplesPerFrame/2);
      }
      else
      {
         wantedAdjustment =
            ((wantedAdjustment+getSamplesNum()-mSamplesPerFrame/2)/mSamplesPerFrame)
               * mSamplesPerFrame
            - (getSamplesNum()-mSamplesPerFrame/2);
      }
*/
      wantedAdjustment = mSamplesPerFrame
                           * ((wantedAdjustment+getSamplesNum())/mSamplesPerFrame)
                        - getSamplesNum();
      break;
   case MP_SPEECH_MUTED:
      assert(!"Muted audio right after decode VAD? Something is wrong here..");
      break;
   }

   int maxAdjustment = 0;
   // Bound wantedAdjustment to maintain minimum number of samples in buffer.
   if (minBufferSamples > 0 && wantedAdjustment < 0)
   {
      maxAdjustment = getSamplesNum()+decodedSamples-minBufferSamples;
      wantedAdjustment = sipx_min(sipx_max(wantedAdjustment, -maxAdjustment), 0);
   }

   // STEP 3. Packet Loss Concealment & Timescale adjustment
   if (!decoder->getInfo()->haveInternalPLC())
   {
      if (decodedSamples > 0 && MpDspUtils::compareSerials(rtpSeq, mStreamSeq) < 0)
      {
         // Late packet. Push it to PLC history and return.
         mpPlc->insertToHistory(rtpSeq-mStreamSeq, packetSpeechParams,
                                mDecodedData, decodedSamples);

         // Nothing else to do, just return.
         return OS_SUCCESS;
      }
      else
      {
         // In time packet. Decode packet if present, or PLC it.
         mpPlc->processFrame(packetSpeechParams, mDecodedData,
                             DECODED_DATA_MAX_LENGTH, decodedSamples,
                             mSamplesPerPacket, wantedAdjustment, adjustment);
         // Calculate number of samples in output buffer.
         outSamplesNum = mSamplesPerPacket + adjustment;
         decodedSamples = mSamplesPerPacket;
      }
#ifdef ANDROID
      LOGV("pushPacket wantedAdjustmentOrig: %d wantedBufferSamples: %d mSamplesPerPacket: %d getSamplesNum()=%d maxAdjustment: %d decodedSamples: %d",
          wantedAdjustmentOrig, wantedBufferSamples, mSamplesPerPacket, getSamplesNum(), maxAdjustment, decodedSamples);
#endif

   }
   else
   {
      adjustment = 0;
      outSamplesNum = decodedSamples;
   }
#ifdef ENABLE_NON_PLC_ADJUSTMENT // [
   if (adjustment == 0)
   {
      // Second try to adjust stream delay
      int tempAdjustment = adjustStream(mDecodedData, DECODED_DATA_MAX_LENGTH,
                                        outSamplesNum, wantedAdjustment);
      outSamplesNum += tempAdjustment;
      adjustment += tempAdjustment;
   }
#endif // ENABLE_NON_PLC_ADJUSTMENT ]

   // STEP 4. Slice data to standard audio buffers.

   // Move all remaining frames to queue start
   if (mRemainingSamplesNum > 0)
   {
      for (int i=0; mFrames[mCurFrameNum+i].isValid(); i++)
      {
         mFrames[i].swap(mFrames[mCurFrameNum+i]);
      }
   }

   // Slice data to buffers.
   if (sliceToFrames(outSamplesNum, mStreamSampleRate, packetSpeechParams) != OS_SUCCESS)
   {
      return OS_FAILED;
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
      numOriginalSamples = 0;
      return;
   }

   // Return data
   pFrame.swap(mFrames[mCurFrameNum]);
   numOriginalSamples = mOriginalSamples[mCurFrameNum];

#ifdef RTL_AUDIO_ENABLED
   UtlString outputLabel("MpJitterBuffer_getFrame");
   RTL_RAW_AUDIO(outputLabel,
                 mOutputSampleRate,
                 pFrame->getSamplesNumber(),
                 pFrame->getSamplesPtr(),
                 0);
#endif

   // Update pointer and statistic
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

void MpJitterBuffer::setPlc(const UtlString &plcName)
{
   // Free old PLC
   delete mpPlc;

   // Set PLC to a new one
   mPlcName = plcName;
   mpPlc = MpPlcBase::createPlc(mPlcName);
   if (mStreamSampleRate != 0)
   {
      // If (mStreamSampleRate > 0), then we've got first RTP packet
      // and know stream sample rate. In other case PLC will be initialized
      // when first packet will be received.
      mpPlc->init(mOutputSampleRate);
   }
}

/* ============================ ACCESSORS ================================= */

void MpJitterBuffer::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
    // Just for debugging
    mpFlowGraph = pFlowGraph;
}

OsStatus MpJitterBuffer::setVadParam(const UtlString& parameterName,
                                       int value)
{
    OsStatus status = OS_INVALID_STATE;
    assert(mpVad);
    if(mpVad)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpJitterBuffer setting VAD parameter: %s value: %d",
                parameterName.data(), value);

        status = mpVad->setParam(parameterName, &value);
    }
    else
    {
        OsSysLog::add(FAC_MP, PRI_WARNING,
            "MpJitterBuffer NULL VAD, unable to set parameter: %s value: %d",
            parameterName.data(), value);
    }

    return(status);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

enum {
   E_FRAME_WINDOW = 8,
   E_THRESH = 1200
};

int32_t calcE(MpAudioSample *pBuf1, MpAudioSample *pBuf2)
{
   int32_t accumulator = 0;
   for (int i=0; i<E_FRAME_WINDOW; i++)
   {
      accumulator += abs(pBuf1[i] - pBuf2[i]);
   }

   return accumulator/E_FRAME_WINDOW;
}

int getMinEPosition(MpAudioSample *pattern, MpAudioSample *pBuffer, int bufferLength)
{
   int32_t minE = INT32_MAX;
   int minPosition = 0;
   for (int i=0; i<bufferLength-E_FRAME_WINDOW; i++)
   {
      int32_t E = calcE(pattern, pBuffer+i);
      if (E <= minE)
      {
         minE = E;
         minPosition = i;
      }
   }

   if (minE < E_THRESH)
   {
      return minPosition;
   }
   else
   {
      return -1;
   }
}

void crossJoin8(MpAudioSample *pSrc1Dst, MpAudioSample *pSrc2)
{
   pSrc1Dst[0] = pSrc1Dst[0];
   pSrc1Dst[1] = (((int32_t)pSrc1Dst[1])*7 + ((int32_t)pSrc2[1])*1)/8;
   pSrc1Dst[2] = (((int32_t)pSrc1Dst[2])*6 + ((int32_t)pSrc2[2])*2)/8;
   pSrc1Dst[3] = (((int32_t)pSrc1Dst[3])*5 + ((int32_t)pSrc2[3])*3)/8;
   pSrc1Dst[4] = (((int32_t)pSrc1Dst[4])*4 + ((int32_t)pSrc2[4])*4)/8;
   pSrc1Dst[5] = (((int32_t)pSrc1Dst[5])*3 + ((int32_t)pSrc2[5])*5)/8;
   pSrc1Dst[6] = (((int32_t)pSrc1Dst[6])*2 + ((int32_t)pSrc2[6])*6)/8;
   pSrc1Dst[7] = (((int32_t)pSrc1Dst[6])*1 + ((int32_t)pSrc2[7])*7)/8;
}

int MpJitterBuffer::adjustStream(MpAudioSample *pBuffer,
                                 int bufferSize,
                                 unsigned numSamples,
                                 int wantedAdjustment)
{
   // Don't do anything if wanted adjustment is too small.
//   if (abs(wantedAdjustment) < numSamples/2)
   if (abs(wantedAdjustment) < 20)
   {
      return 0;
   }

   int pos = 0;
   if (wantedAdjustment > 0)
   {
      numSamples = sipx_min(numSamples, (unsigned)(wantedAdjustment+wantedAdjustment/8));
      // Cannot expand past the maximum size of buffer, so only look for a splice point
      // within the window of the maximum possible shift.
      // Find a good place to glue
      int maxExpand = sipx_min(numSamples-E_FRAME_WINDOW, bufferSize - numSamples-E_FRAME_WINDOW);
      if(maxExpand > 0)
      {
         pos = getMinEPosition(pBuffer, pBuffer+E_FRAME_WINDOW,
                               maxExpand);
      }
   }
   else
   {
      numSamples = sipx_min(numSamples, (unsigned)(-wantedAdjustment+wantedAdjustment/8));
      // Find a good place to glue
      pos = getMinEPosition(pBuffer, pBuffer+E_FRAME_WINDOW,
                            numSamples-E_FRAME_WINDOW);
   }

   RTL_EVENT("MpJitterBuffer_adjustStream_wantedAdjustment", wantedAdjustment);
   RTL_EVENT("MpJitterBuffer_adjustStream_numSamples", numSamples);
   RTL_EVENT("MpJitterBuffer_adjustStream_pos", pos);
   // Go further only if we've got a good place to glue.
   if (pos > 0)
   {
      // Do we want to expand or to shrink?
      if (wantedAdjustment > 0)
      {
         if(numSamples <= E_FRAME_WINDOW)
         {
             // Not enough samples to work with
             return(0);
         }

         else
         {
             // Expand
             memmove(pBuffer+2*E_FRAME_WINDOW+pos, pBuffer+E_FRAME_WINDOW,
                     (numSamples-E_FRAME_WINDOW)*sizeof(MpAudioSample));
             crossJoin8(pBuffer+E_FRAME_WINDOW+pos, pBuffer);
             RTL_EVENT("MpJitterBuffer_adjustStream_adjustment", pos + E_FRAME_WINDOW);
             return pos + E_FRAME_WINDOW;
         }
      }
      else //if (wantedAdjustment > 0) // wantedAdjustment can't be 0
      {
         if(numSamples <= (pos + 2 * E_FRAME_WINDOW))
         {
             // Not enough samples to work with
             return(0);
         }
         else
         {
             // Shrink
             crossJoin8(pBuffer, pBuffer+E_FRAME_WINDOW+pos);
             memmove(pBuffer+E_FRAME_WINDOW, pBuffer+2*E_FRAME_WINDOW+pos,
                     (numSamples-pos-2*E_FRAME_WINDOW)*sizeof(MpAudioSample));
             RTL_EVENT("MpJitterBuffer_adjustStream_adjustment", -(pos + E_FRAME_WINDOW));
             return -(pos + E_FRAME_WINDOW);
          }
      }
   }

   RTL_EVENT("MpJitterBuffer_adjustStream_adjustment", 0);
   return 0;
}

OsStatus MpJitterBuffer::sliceToFrames(int decodedSamples,
                                       int codecSampleRate,
                                       const MpSpeechParams &speechParams)
{
   // Get source sample rate and update sample rate if changed.
   if (  (codecSampleRate != mOutputSampleRate)
      && (((int)mpResampler->getInputRate()) != codecSampleRate))
   {
      mpResampler->setInputRate(codecSampleRate);
   }

   // Slice data to frames.
   for (int i = 0, frame=0; i < decodedSamples; frame++)
   {
      uint32_t inSamplesNum; // Number of samples in the frame before resampling
      uint32_t outSamplesNum; // Number of samples in the frame after resampling
      MpAudioSample *pWritePtr;
      unsigned numSamplesToWrite;

      if (mFrames[frame].isValid())
      {
         pWritePtr = mFrames[frame]->getSamplesWritePtr()
                   + mFrames[frame]->getSamplesNumber();
         numSamplesToWrite = mSamplesPerFrame - mFrames[frame]->getSamplesNumber();

         //TODO:: Upgrade this to support all possible speech types!
         if (numSamplesToWrite > 0 && speechParams.mSpeechType == MP_SPEECH_ACTIVE)
         {
            // If new data contain active speech, frame should be marked accordingly.
            mFrames[frame]->setSpeechParams(speechParams);
         }
      }
      else
      {
         // Get new buffer.
         mFrames[frame] = MpMisc.RawAudioPool->getBuffer();
         if (!mFrames[frame].isValid())
            return OS_FAILED;
         mFrames[frame]->setSpeechParams(speechParams);
         mFrames[frame]->setSamplesNumber(0);

         pWritePtr = mFrames[frame]->getSamplesWritePtr();
         numSamplesToWrite = mSamplesPerFrame;
         mOriginalSamples[frame] = 0;
      }

      // This frame is already full, skip to next one.
      if (numSamplesToWrite == 0)
      {
         continue;
      }

      // Copy or resample audio data to buffer.
      if (codecSampleRate == mOutputSampleRate)
      {
         inSamplesNum = sipx_min(mSamplesPerFrame, decodedSamples-i);
         inSamplesNum = sipx_min(inSamplesNum, numSamplesToWrite);
         memcpy(pWritePtr, mDecodedData+i, inSamplesNum*sizeof(MpAudioSample));
         outSamplesNum = inSamplesNum;
      }
      else
      {
         mpResampler->resample(0,
                               mDecodedData+i, decodedSamples-i, inSamplesNum,
                               pWritePtr, numSamplesToWrite,
                               outSamplesNum);
      }
      i += inSamplesNum;
      // Update number of samples in buffer to actual number of samples.
      mFrames[frame]->setSamplesNumber(mFrames[frame]->getSamplesNumber()+outSamplesNum);

      // Place buffer to queue.
      mOriginalSamples[frame] += inSamplesNum;
      mRemainingSamplesNum += outSamplesNum;
   }

   return OS_SUCCESS;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
