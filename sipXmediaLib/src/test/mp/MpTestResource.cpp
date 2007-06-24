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

// SYSTEM INCLUDES
#include <assert.h>

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
MpTestResource::MpTestResource(const UtlString& rName
                              , int minInputs, int maxInputs
                              , int minOutputs, int maxOutputs
                              , int samplesPerFrame, int samplesPerSec
                              )
:  MpAudioResource(rName, minInputs, maxInputs, minOutputs, maxOutputs,
                   samplesPerFrame, samplesPerSec),
   mGenOutBufMask(0),
   mProcessInBufMask(0),
   mLastMsg(0),
   mProcessedCnt(0),
   mMsgCnt(0),
   mSignalType(MP_TEST_SIGNAL_NULL),
   mpSignalPeriod(NULL),
   mpSignalAmplitude(NULL)
{
   mLastDoProcessArgs.inBufs = new MpBufPtr[mMaxInputs];
   mLastDoProcessArgs.outBufs = new MpBufPtr[mMaxOutputs];
   mpSignalPeriod = new int[maxOutputs];
   mpSignalAmplitude = new int[maxOutputs];
   int outIndex;
   for(outIndex = 0; outIndex < maxOutputs; outIndex++)
   {
      mpSignalPeriod[outIndex] = 0;
      mpSignalAmplitude[outIndex] = 0;
   }
}

// Destructor
MpTestResource::~MpTestResource()
{
   if (mLastDoProcessArgs.inBufs != NULL)
      delete[] mLastDoProcessArgs.inBufs;

   if (mLastDoProcessArgs.outBufs != NULL)
      delete[] mLastDoProcessArgs.outBufs;
}

/* ============================ MANIPULATORS ============================== */

// Sends a test message to this resource.
void MpTestResource::sendTestMessage(void* ptr1, void* ptr2,
                                     int int3, int int4)
{
   MpFlowGraphMsg msg(RESOURCE_MSG_TYPE, this, ptr1, ptr2, int3, int4);
   OsStatus       res;

   res = postMessage(msg);
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

void MpTestResource::setSignalPeriod(int outputIndex, int periodInSamples)
{
   assert(outputIndex < maxOutputs());
   mpSignalPeriod[outputIndex] = periodInSamples;
}

void MpTestResource::setSignalAmplitude(int outputIndex, int maxMinValue)
{
   assert(outputIndex < maxOutputs());
   mpSignalAmplitude[outputIndex] = maxMinValue;
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
   return squareSampleValue(mpSignalPeriod[outputIndex],
                            mpSignalAmplitude[outputIndex],
                            sampleIndex);
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
            if(mSignalType == MP_TEST_SIGNAL_NULL)
            {
               memset(pBuf->getSamplesWritePtr(), 0,
                      pBuf->getSamplesNumber()*sizeof(MpAudioSample));
            }

            else if(mSignalType == MP_TEST_SIGNAL_SQUARE)
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

