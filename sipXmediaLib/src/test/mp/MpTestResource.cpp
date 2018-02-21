//  
// Copyright (C) 2006-2018 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <os/OsIntTypes.h>
#include <assert.h>

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
   // Under VC6, we need to define M_PI ourselves.
#  define M_PI 3.14159265358979323846264338327950288419716939937510
#else
   // Under Visual Studio (above VC6) we have to define _USE_MATH_DEFINES 
   // to get M_PI define.
#  define _USE_MATH_DEFINES
#endif

#include <math.h>

// APPLICATION INCLUDES
#include "mp/MpBuf.h"
#include "mp/MpMisc.h"
#include "mp/MpTestResource.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
static const int RESOURCE_MSG_TYPE = MpFlowGraphMsg::RESOURCE_SPECIFIC_START;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// Constructor
MpTestResource::MpTestResource(const UtlString& rName,
                               int minInputs,
                               int maxInputs,
                               int minOutputs,
                               int maxOutputs)
: MpAudioResource(rName, minInputs, maxInputs, minOutputs, maxOutputs)
, mGenOutBufMask(0)
, mProcessInBufMask(0)
, mLastMsg(0)
, mProcessedCnt(0)
, mMsgCnt(0)
, mSignalType(MP_TEST_SIGNAL_NULL)
, mpSignalPeriod(NULL)
, mpSignalAmplitude(NULL)
, mpSpeechType(NULL)
, mpBuffer(NULL)
, mBufferSize(0)
{
   mLastDoProcessArgs.inBufs = new MpBufPtr[mMaxInputs];
   mLastDoProcessArgs.outBufs = new MpBufPtr[mMaxOutputs];
   mpSignalPeriod = new float[maxOutputs];
   mpSignalAmplitude = new int[maxOutputs];
   mpSpeechType = new MpSpeechType[maxOutputs];
   int outIndex;
   for(outIndex = 0; outIndex < maxOutputs; outIndex++)
   {
      mpSignalPeriod[outIndex] = 0.0;
      mpSignalAmplitude[outIndex] = 0;
      mpSpeechType[outIndex] = MP_SPEECH_UNKNOWN;
   }
}

// Destructor
MpTestResource::~MpTestResource()
{
   if (mLastDoProcessArgs.inBufs != NULL)
      delete[] mLastDoProcessArgs.inBufs;

   if (mLastDoProcessArgs.outBufs != NULL)
      delete[] mLastDoProcessArgs.outBufs;

   if(mpBuffer)
   {
       delete[] mpBuffer;
       mpBuffer = NULL;
   }

}

/* ============================ MANIPULATORS ============================== */

// Sends a test message to this resource.
OsStatus MpTestResource::sendTestMessage(void* ptr1, void* ptr2,
                                     int int3, int int4)
{
   MpFlowGraphMsg msg(RESOURCE_MSG_TYPE, this, ptr1, ptr2, int3, int4);
   OsStatus       res;

   res = postMessage(msg);
   return(res);
}

// Specify the genOutBufMask.
// For each bit in the genOutBufMask that is set, if there is a
// resource connected to the corresponding output port, doProcessFrame()
// will create an output buffer on that output port.
void MpTestResource::setGenOutBufMask(int mask)
{
   mGenOutBufMask = mask;
}

// Specify the processInBufMask.
// For each bit in the processInBufMask that is set, doProcessFrame()
// will pass the input buffer from the corresponding input port,
// straight through to the corresponding output port.  If nothing is
// connected on the corresponding output port, the input buffer will
// be deleted.
void MpTestResource::setProcessInBufMask(int mask)
{
   mProcessInBufMask = mask;
}

void MpTestResource::setOutSignalType(MpTestOutSignal signalType)
{
   mSignalType = signalType;
}

void MpTestResource::setSignalPeriod(int outputIndex, float periodInSamples)
{
   assert(outputIndex < maxOutputs());
   mpSignalPeriod[outputIndex] = periodInSamples;
}

void MpTestResource::setSignalAmplitude(int outputIndex, int maxMinValue)
{
   assert(outputIndex < maxOutputs());
   mpSignalAmplitude[outputIndex] = maxMinValue;
}

void MpTestResource::setSpeechType(int outputIndex, MpSpeechType speech)
{
   assert(outputIndex < maxOutputs());
   mpSpeechType[outputIndex] = speech;
}

void MpTestResource::setBuffer(const MpAudioSample samples[], int sampleCount)
{
    if(mpBuffer)
    {
        delete[] mpBuffer;
    }
    mpBuffer = new MpAudioSample[sampleCount];

    memcpy(mpBuffer, samples, sampleCount * sizeof(MpAudioSample));
    mBufferSize = sampleCount;
}

/* ============================ ACCESSORS ================================= */

// Returns the count of the number of frames processed by this resource.
int MpTestResource::numFramesProcessed(void)
{
   return mProcessedCnt;
}

// Returns the count of the number of messages successfully processed by 
// this resource.
int MpTestResource::numMsgsProcessed(void)
{
   return mMsgCnt;
}

MpAudioSample MpTestResource::getSquareSampleValue(int outputIndex,
                                                   int sampleIndex)
{
   return squareSampleValue((int)mpSignalPeriod[outputIndex],
                            mpSignalAmplitude[outputIndex],
                            sampleIndex);
}

int MpTestResource::getLastInputFrameMagnitude(int inputIndex)
{
    double sumSquares = 0.0;
    if(mLastDoProcessArgs.inBufs[inputIndex].isValid())
    {
        MpAudioBufPtr pBuf = mLastDoProcessArgs.inBufs[inputIndex];
        int numSamples = pBuf->getSamplesNumber();
        const MpAudioSample* samples = pBuf->getSamplesPtr();

        for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
        {
            sumSquares += samples[sampleIndex] * samples[sampleIndex];
        }

        sumSquares /= (double) numSamples;
    }

    return((int)sqrt(sumSquares));
}

/* ============================ INQUIRY =================================== */

/* ============================ UTILITY =================================== */

MpAudioSample MpTestResource::squareSampleValue(int squareWavePeriod,
                                                int squareWaveAmplitude,
                                                int sampleIndex)
{
   MpAudioSample sample;
   int halfPeriod = squareWavePeriod / 2;
   int periodMod = sampleIndex % squareWavePeriod;
   if(periodMod >= halfPeriod)
   {
      sample = 0 - squareWaveAmplitude;
   }
   else
   {
      sample = squareWaveAmplitude;
   }
   return(sample);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Processes the next frame interval's worth of media.
UtlBoolean MpTestResource::doProcessFrame(MpBufPtr inBufs[],
                                          MpBufPtr outBufs[],
                                          int inBufsSize,
                                          int outBufsSize,
                                          UtlBoolean isEnabled,
                                          int samplesPerFrame,
                                          int samplesPerSecond)
{
   int i=0;
   // keep a copy of the input buffers
   for (i=0; i<mMaxInputs; i++) {
      mLastDoProcessArgs.inBufs[i]     = inBufs[i];
   }
   // keep a copy of the arguments passed to this method
   mLastDoProcessArgs.inBufsSize       = inBufsSize;
   mLastDoProcessArgs.outBufsSize      = outBufsSize;
   mLastDoProcessArgs.isEnabled        = isEnabled;
   mLastDoProcessArgs.samplesPerFrame  = samplesPerFrame;
   mLastDoProcessArgs.samplesPerSecond = samplesPerSecond;

   for (i=0; i < outBufsSize; i++)
   {
      outBufs[i].release();
      if (isOutputConnected(i))
      {
         if ((mProcessInBufMask & (1 << i)) &&
             (inBufsSize > i))
         {
            // if the corresponding bit in the mProcessInBufMask is set for
            // the input port then pass the input buffer straight through
            outBufs[i] = inBufs[i];
            inBufs[i].release();
         }

         if ( isEnabled &&
             (mGenOutBufMask & (1 << i)) &&
             (!outBufs[i].isValid()))
         {
            // if the output buffer is presently NULL and the corresponding
            // bit in the mGenOutBufMask is set then allocate a new buffer
            // for the output port
            assert(MpMisc.RawAudioPool != NULL);
            MpAudioBufPtr pBuf = MpMisc.RawAudioPool->getBuffer();
            if (mSignalType == MP_TEST_SIGNAL_NULL)
            {
               memset(pBuf->getSamplesWritePtr(), 0,
                      pBuf->getSamplesNumber()*sizeof(MpAudioSample));
            }

            else if (mSignalType == MP_TEST_SIGNAL_SQUARE)
            {
               unsigned sampleIndex;
               MpAudioSample* squareSamples = pBuf->getSamplesWritePtr();
               assert(squareSamples);
               for(sampleIndex = 0; sampleIndex < pBuf->getSamplesNumber(); sampleIndex++)
               {
                  squareSamples[sampleIndex] =
                     getSquareSampleValue(i, sampleIndex);
               }
            }

            else if (mSignalType == MP_SINE_SAW)
            {
               unsigned sampleIndex;
               MpAudioSample* samples = pBuf->getSamplesWritePtr();
               int samplesNum = pBuf->getSamplesNumber();
               assert(samples);
               for(sampleIndex = 0; sampleIndex < (unsigned)samplesNum; sampleIndex++)
               {
                  samples[sampleIndex] = (MpAudioSample) 
                                         ( mpSignalAmplitude[i]
                                         * (samplesPerFrame-sampleIndex)
                                         * cos(10*2*M_PI*sampleIndex/samplesPerFrame)
                                         / samplesPerFrame);
               }
            }

            else if (mSignalType == MP_SINE)
            {
                unsigned sampleIndex;
                MpAudioSample* samples = pBuf->getSamplesWritePtr();
                int samplesNum = pBuf->getSamplesNumber();
                assert(samples);
                for(sampleIndex = 0; sampleIndex < (unsigned)samplesNum; sampleIndex++)
                {
                    samples[sampleIndex] = (MpAudioSample)
                                           (((float) mpSignalAmplitude[i]) *
                                            sin((2.0 * M_PI * (float)(sampleIndex + mProcessedCnt * samplesPerFrame)) /
                                            mpSignalPeriod[i]));
#if 0
                    printf("sample[%d]=%d amp: %d period: %f t: %d\n",
                           sampleIndex,
                           samples[sampleIndex],
                           mpSignalAmplitude[i],
                           mpSignalPeriod[i],
                           (sampleIndex + mProcessedCnt * samplesPerFrame));
#endif

                }
            }

            else if(mSignalType == MP_BUFFER)
            {
                assert(mpBuffer);
                assert(mBufferSize > 0);
                int sampleIndex = (mProcessedCnt * samplesPerFrame) % mBufferSize;
                // For now just truncate partial frame at end of buffer
                if(mBufferSize - sampleIndex < samplesPerFrame)
                {
                    sampleIndex = 0;
                }
                memcpy( pBuf->getSamplesWritePtr(),
                        &mpBuffer[sampleIndex],
                        samplesPerFrame * sizeof(MpAudioSample));
            }

            pBuf->setSpeechType(mpSpeechType[i]);
            outBufs[i] = pBuf;            
         }
      }
   }

   for (i=0; i < inBufsSize; i++)
   {
      // if the corresponding bit in the mProcessInBufMask is set and we
      // haven't processed the input buffer then free it.
      if ((mProcessInBufMask & (1 << i)) &&
          (inBufs[i].isValid()))
      {
         inBufs[i].release();
      }
   }

   mProcessedCnt++;

   // keep a copy of the generated buffers
   for (i=0; i<mMaxOutputs; i++) {
      mLastDoProcessArgs.outBufs[i]    = outBufs[i];
   }

   return TRUE;
}

// Handles messages for this resource.
UtlBoolean MpTestResource::handleMessage(MpFlowGraphMsg& rMsg)
{
   mLastMsg = rMsg;
   mMsgCnt++;

   if (rMsg.getMsg() == RESOURCE_MSG_TYPE)
      return TRUE;
   else
      return MpAudioResource::handleMessage(rMsg);
}

/* ============================ FUNCTIONS ================================= */

