//  
// Copyright (C) 2007-2016 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpAudioOutputConnection.h>
#include <mp/MpOutputDeviceDriver.h>
#include <mp/MpDspUtils.h>
#include <os/OsLock.h>
#include <os/OsDefs.h>    // for min macro
#include <os/OsSysLog.h>

// milliseconds
//#define LOG_HEART_BEAT_PERIOD 100

//#define RTL_ENABLED
//#define RTL_AUDIO_ENABLED

#ifdef RTL_ENABLED
#include <rtl_macro.h>
#else
#define RTL_BLOCK(x)
#define RTL_EVENT(x,y)
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// PRIVATE CLASSES
// DEFINES
// MACROS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpAudioOutputConnection::MpAudioOutputConnection(MpOutputDeviceHandle deviceId,
                                                 MpOutputDeviceDriver *deviceDriver)
: UtlInt(deviceId)
, mMutex(OsMutexBase::Q_PRIORITY)
, mUseCount(0)
, mpDeviceDriver(deviceDriver)
, mCurrentFrameTime(0)
, mMixerBufferLength(0)
, mpMixerBuffer(NULL)
, mMixerBufferBegin(0)
, readyForDataCallbackNotf((intptr_t)this, readyForDataCallback)
, mpFlowgraphTicker(NULL)
{
   assert(mpDeviceDriver != NULL);
};

MpAudioOutputConnection::~MpAudioOutputConnection()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpAudioOutputConnection::enableDevice(unsigned samplesPerFrame, 
                                               unsigned samplesPerSec,
                                               MpFrameTime currentFrameTime,
                                               MpFrameTime mixerBufferLength)
{
   OsStatus result = OS_FAILED;
   OsLock lock(mMutex);

   assert(samplesPerFrame > 0);
   assert(samplesPerSec > 0);

   if (mpDeviceDriver->isEnabled())
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
              "MpAudioOutputConnection::enableDevice() device (%s id: %d) already enabled status: %d",
              mpDeviceDriver->getDeviceName().data(),
              getValue(),
              OS_INVALID_STATE);
      return(OS_INVALID_STATE);
   }

   // Mixer buffer length can't be zero.
   if (mixerBufferLength == 0)
   {
      return OS_NOT_SUPPORTED;
   }

   // Set current frame time for this connection.
   mCurrentFrameTime = currentFrameTime;

   // Calculate number of samples in mixer buffer.
   // I.e. convert from milliseconds to samples and round to frame boundary.
   // Note: this calculation may overflow (e.g. 10000msec*44100Hz > 4294967296smp).
   unsigned bufferSamples = mixerBufferLength*samplesPerSec/1000;
   bufferSamples = ((bufferSamples + samplesPerFrame - 1) / samplesPerFrame)
                 * samplesPerFrame;

   // Initialize mixer buffer. All mixer buffer related variables will be set here.
   if (initMixerBuffer(bufferSamples) != OS_SUCCESS)
   {
      return result;
   }

   // Enable device driver
   result = mpDeviceDriver->enableDevice(samplesPerFrame,
                                         samplesPerSec,
                                         mCurrentFrameTime,
                                         readyForDataCallbackNotf);

   return result;
}

OsStatus MpAudioOutputConnection::disableDevice()
{
   OsStatus result = OS_FAILED;
   OsLock lock(mMutex);

   // Disable device and set result code.
   result = mpDeviceDriver->disableDevice();

   // Free mixer buffer if device was set to disabled state.
   if (!mpDeviceDriver->isEnabled())
   {
      // Do not check return code, as it is not critical if it fails.
      freeMixerBuffer();
   }

   return result;
}

OsStatus MpAudioOutputConnection::enableFlowgraphTicker(OsNotification *pFlowgraphTicker)
{
   OsStatus result = OS_SUCCESS;
   OsLock lock(mMutex);

   assert(mpFlowgraphTicker == NULL);

   // Set flowgraph ticker
   mpFlowgraphTicker = pFlowgraphTicker;

   return result;
}

OsStatus MpAudioOutputConnection::disableFlowgraphTicker()
{
   OsStatus result = OS_SUCCESS;
   OsLock lock(mMutex);

   assert(mpFlowgraphTicker != NULL);

   // Clear flowgraph ticker notification
   mpFlowgraphTicker = NULL;

   return result;
}

OsStatus MpAudioOutputConnection::pushFrameBeginning(unsigned int numSamples,
                                                     const MpAudioSample* samples,
                                                     MpFrameTime &frameTime)
{
   OsStatus result = OS_SUCCESS;
   RTL_BLOCK("MpAudioOutputConnection::pushFrame");

   assert(numSamples > 0);

   // From now we access internal data. Take lock.
   OsLock lock(mMutex);

   RTL_EVENT("MpAudioOutputConnection::pushFrame_frameTime", frameTime);
   RTL_EVENT("MpAudioOutputConnection::pushFrame_currentTime", mCurrentFrameTime);
   RTL_EVENT("MpAudioOutputConnection::pushFrame", this->getValue());

   // Do nothing if no audio was pushed. Mixer buffer will be filled with
   // silence or data from other sources.
   if (samples != NULL)
   {
      // Mix frame to the very beginning of the mixer buffer.
      result = mixFrame(mMixerBufferBegin, samples, numSamples);
   }

   frameTime = mCurrentFrameTime;

   return result;
};

OsStatus MpAudioOutputConnection::pushFrame(unsigned int numSamples,
                                            const MpAudioSample* samples,
                                            MpFrameTime frameTime)
{
   OsStatus result = OS_SUCCESS;
   RTL_EVENT("MpAudioOutputConnection::pushFrame", -1);

   assert(numSamples > 0);

   // From now we access internal data. Take lock.
   OsLock lock(mMutex);

   RTL_EVENT("MpAudioOutputConnection::pushFrame_frameTime", frameTime);
   RTL_EVENT("MpAudioOutputConnection::pushFrame_currentTime", mCurrentFrameTime);
   RTL_EVENT("MpAudioOutputConnection::pushFrame", this->getValue());

   // Check for late frame. Check for early frame is done inside mixFrame().
   if (MpDspUtils::compareSerials(frameTime, mCurrentFrameTime) < 0)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpAudioOutputConnection::pushFrame()"
                    " OS_INVALID_STATE frameTime=%d, currentTime=%d\n",
                    frameTime, mCurrentFrameTime);
      result = OS_INVALID_STATE;
      RTL_EVENT("MpAudioOutputConnection::pushFrame", result);
      return result;
   }

   if(mpDeviceDriver->isEnabled())
   {
       // Convert frameTime to offset in mixer buffer.
       // Note: frameTime >= mCurrentFrameTime.
       unsigned mixerBufferOffsetFrames =
                (frameTime-mCurrentFrameTime) / mpDeviceDriver->getFramePeriod();
       unsigned mixerBufferOffsetSamples = 
                mixerBufferOffsetFrames * mpDeviceDriver->getSamplesPerFrame();

       // Don't touch mix buffer if no audio was pushed. Mixer buffer will be filled
       // with silence or data from other sources.
       if (samples != NULL)
       {
          // Mix this data with other sources.
          result = mixFrame(mixerBufferOffsetSamples, samples, numSamples);
          if(result != OS_SUCCESS)
          {
              OsSysLog::add(FAC_MP, PRI_WARNING,
                            "MpAudioOutputConnection::pushFrame mixFrame(%d, %p, %d) returned: %d"
                            " frameTime=%d, currentTime=%d\n",
                            (int)mixerBufferOffsetSamples, samples, numSamples, result,
                            frameTime, mCurrentFrameTime);
          }
       }
       else
       {
          // Just check for late frame.
          result = isLateToMix(mixerBufferOffsetSamples, numSamples);
       }
   }
   else
   {
       result = OS_INVALID_STATE;
   }

   RTL_EVENT("MpAudioOutputConnection::pushFrame", result);
   return result;
};

/* ============================ ACCESSORS ================================= */

MpFrameTime MpAudioOutputConnection::getCurrentFrameTime() const
{
   OsLock lock(mMutex);
   return mCurrentFrameTime;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsStatus MpAudioOutputConnection::initMixerBuffer(unsigned mixerBufferLength)
{
   // We cannot allocate buffer of zero length.
   if (mixerBufferLength == 0)
   {
      return OS_FAILED;
   }

   // Deallocate mixer buffer if it is allocated.
   // Do not check return value. Nothing fatal may happen inside.
   freeMixerBuffer();

   // Initialize variables.
   mMixerBufferLength = mixerBufferLength;
   mpMixerBuffer = new MpAudioSample[mMixerBufferLength];
   mMixerBufferBegin = 0;

   // Clear buffer data.
   memset(mpMixerBuffer, 0, mMixerBufferLength*sizeof(MpAudioSample));

   return OS_SUCCESS;
}

OsStatus MpAudioOutputConnection::freeMixerBuffer()
{
   mMixerBufferLength = 0;
   if (mpMixerBuffer != NULL)
   {
      delete[] mpMixerBuffer;
      mpMixerBuffer = NULL;
   }
   mMixerBufferBegin = 0;

   return OS_SUCCESS;
}

OsStatus MpAudioOutputConnection::mixFrame(unsigned frameOffset,
                                           const MpAudioSample* samples,
                                           unsigned numSamples)
{
   assert(numSamples > 0);
   assert(samples != NULL);

   // Check for late frame.
   if (isLateToMix(frameOffset, numSamples) == OS_LIMIT_REACHED)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                  "MpAudioOutputConnection::mixFrame()"
                  " OS_LIMIT_REACHED offset=%d, samples=%d, bufferLength=%d\n",
                  frameOffset, numSamples, mMixerBufferLength);
      return OS_LIMIT_REACHED;
   }

   // Calculate frame start as if buffer is linear
   unsigned frameBegin = mMixerBufferBegin+frameOffset;
   // and wrap it because it is circular actually.
   if (frameBegin >= mMixerBufferLength)
   {
      frameBegin -= mMixerBufferLength;
   }

   // Calculate size of first chunk to mix.
   unsigned firstChunkSize = sipx_min(numSamples, mMixerBufferLength-frameBegin);

   // Counter variables for next two loops
   unsigned srcIndex, dstIndex;

   // from frame begin to buffer wrap
   for (srcIndex=0, dstIndex=frameBegin;
        srcIndex<firstChunkSize;
        srcIndex++, dstIndex++)
   {
      mpMixerBuffer[dstIndex] += samples[srcIndex];
   }

   // from buffer wrap to frame end
   for (dstIndex=0;
        srcIndex<numSamples;
        srcIndex++, dstIndex++)
   {
      mpMixerBuffer[dstIndex] += samples[srcIndex];
   }

   return OS_SUCCESS;
}

OsStatus MpAudioOutputConnection::advanceMixerBuffer(unsigned numSamples)
{
   // If buffer could be copied in one pass
   if(numSamples <= 0)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpAudioOutputConnection::advanceMixerBuffer invoked with: %d samples\n", numSamples);
      assert(numSamples > 0);
   }
   else if(numSamples > mMixerBufferLength)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpAudioOutputConnection::advanceMixerBuffer invoked with numSamples: %d > mMixerBufferLength: %d\n",
             numSamples, mMixerBufferLength);
      assert(numSamples <= mMixerBufferLength);
   }
   else if (mMixerBufferBegin+numSamples <= mMixerBufferLength)
   {
      memset(&mpMixerBuffer[mMixerBufferBegin],
             0,
             numSamples*sizeof(MpAudioSample));
      mMixerBufferBegin += numSamples;
      mMixerBufferBegin %= mMixerBufferLength;
   }
   else
   {
      unsigned firstChunkSize = mMixerBufferLength-mMixerBufferBegin;
      memset(&mpMixerBuffer[mMixerBufferBegin],
             0,
             firstChunkSize*sizeof(MpAudioSample));
      memset(&mpMixerBuffer[0],
             0,
             (numSamples-firstChunkSize)*sizeof(MpAudioSample));

      mMixerBufferBegin = numSamples - firstChunkSize;
   }

   return OS_SUCCESS;
}

void MpAudioOutputConnection::readyForDataCallback(const intptr_t userData,
                                                   const intptr_t eventData)
{
   OsStatus result;
   MpAudioOutputConnection *pConnection = (MpAudioOutputConnection*)userData;
   RTL_BLOCK("MpAudioOutputConnection::tickerCallBack");

#ifdef LOG_HEART_BEAT_PERIOD
   if(pConnection->mCurrentFrameTime % LOG_HEART_BEAT_PERIOD == 0)
   {
       OsSysLog::add(FAC_MP, PRI_DEBUG,
                     "MpAudioOutputConnection::readyForDataCallback aquiring mutex frame time: %d",
                     pConnection->mCurrentFrameTime);
      
   }
#endif

   if (pConnection->mMutex.acquire(OsTime(5)) == OS_SUCCESS)
   {
      // Push data to device driver and forget.
      result = pConnection->mpDeviceDriver->pushFrame(
                     pConnection->mpDeviceDriver->getSamplesPerFrame(),
                     pConnection->mpMixerBuffer+pConnection->mMixerBufferBegin,
                     pConnection->mCurrentFrameTime);
#ifdef LOG_HEART_BEAT_PERIOD
      if(pConnection->mCurrentFrameTime % LOG_HEART_BEAT_PERIOD == 0)
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "MpAudioOutputConnection::readyForDataCallback()"
                      " frame=%d, pushFrame result=%d\n",
                      pConnection->mCurrentFrameTime, result);
      }
#else
      SIPX_UNUSED(result);
#endif
//      assert(result == OS_SUCCESS);

      // Advance mixer buffer and frame time.
      pConnection->advanceMixerBuffer(pConnection->mpDeviceDriver->getSamplesPerFrame());
      pConnection->mCurrentFrameTime +=
                           pConnection->mpDeviceDriver->getSamplesPerFrame() * 1000
                           / pConnection->mpDeviceDriver->getSamplesPerSec();
      RTL_EVENT("MpAudioOutputConnection::tickerCallBack_currentFrameTime",
                pConnection->mCurrentFrameTime);

      pConnection->mMutex.release();
   }

   // Signal frame processing interval start if requested.
   if (pConnection->mpFlowgraphTicker)
   {
      pConnection->mpFlowgraphTicker->signal(0);
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

