//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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
#include <mp/MpMisc.h>    // for min macro
#include <mp/MpMediaTask.h> // for MpMediaTask::signalFrameStart()
#include <os/OsLock.h>
#include <os/OsCallback.h>

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
, mpTickerCallback(NULL)
, mDoFlowgraphTicker(FALSE)
{
   assert(mpDeviceDriver != NULL);
};

MpAudioOutputConnection::~MpAudioOutputConnection()
{
   assert(mpTickerCallback == NULL);
   if (mpTickerCallback != NULL)
   {
      if (mpDeviceDriver != NULL)
      {
         mpDeviceDriver->setTickerNotification(NULL);
      }

      delete mpTickerCallback;
      mpTickerCallback = NULL;
   }
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
      return OS_INVALID_STATE;
   }

   // Driver should be able to request next frame in mixer mode.
   if (mixerBufferLength > 0 && !mpDeviceDriver->isFrameTickerSupported())
   {
      return OS_NOT_SUPPORTED;
   }

   // Set current frame time for this connection.
   mCurrentFrameTime = currentFrameTime;

   // Allocate mixer buffer if requested.
   // There is nothing to do if it is not requested, as we're sure that we was
   // in disabled state and mixer buffer was not allocated.
   if (mixerBufferLength > 0)
   {
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
   }

   // Enable ticker if needed.
   if (isTickerNeeded())
   {
      setDeviceTicker();
   }

   // Enable device driver
   result = mpDeviceDriver->enableDevice(samplesPerFrame, samplesPerSec,
                                         mCurrentFrameTime);

   return result;
}

OsStatus MpAudioOutputConnection::disableDevice()
{
   OsStatus result = OS_FAILED;
   OsLock lock(mMutex);

   // Disable ticker notification and delete it if any.
   if (isTickerNeeded())
   {
      clearDeviceTicker();
   }

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

OsStatus MpAudioOutputConnection::enableFlowgraphTicker()
{
   OsStatus result = OS_NOT_SUPPORTED;
   OsLock lock(mMutex);

   assert(mDoFlowgraphTicker == FALSE);

   if (mpDeviceDriver->isFrameTickerSupported())
   {
      result = OS_SUCCESS;

      // Set flowgraph ticker flag
      mDoFlowgraphTicker = TRUE;

      // Enable ticker if not enabled already
      if (mpTickerCallback == NULL)
      {
         result = setDeviceTicker();
      }
   }

   return result;
}

OsStatus MpAudioOutputConnection::disableFlowgraphTicker()
{
   OsStatus result = OS_NOT_SUPPORTED;
   OsLock lock(mMutex);

   assert(mDoFlowgraphTicker == TRUE);

   if (mpDeviceDriver->isFrameTickerSupported())
   {
      result = OS_SUCCESS;

      // Set flowgraph ticker flag
      mDoFlowgraphTicker = FALSE;

      // Disable ticker if we're not in mixer mode.
      if (!isTickerNeeded())
      {
         clearDeviceTicker();
      }
   }

   return result;
}

OsStatus MpAudioOutputConnection::pushFrame(unsigned int numSamples,
                                            const MpAudioSample* samples,
                                            MpFrameTime frameTime)
{
   OsStatus result = OS_FAILED;
   RTL_BLOCK("MpAudioOutputConnection::pushFrame");

   assert(numSamples > 0);

   // Check for late frame. Check for too early frame is done inside mixFrame()
   // function (if mixer mode is used) and will not be done if direct write
   // mode is used.
   // TODO:: This check should be fixed to support frame time wrap around.
   if (frameTime < mCurrentFrameTime)
   {
      osPrintf("MpAudioOutputConnection::pushFrame()"
               " OS_INVALID_STATE frameTime=%d, currentTime=%d\n",
               frameTime, mCurrentFrameTime);
      result = OS_INVALID_STATE;
      return result;
   }

   // From now we access internal data. Take lock.
   OsLock lock(mMutex);
   RTL_EVENT("MpAudioOutputConnection::pushFrame", this->getValue());

   // Do we have mixer buffer, i.e. are we in direct write mode or not?
   if (isMixerBufferAvailable())
   {
      // We're in mixer mode.

      // Do nothing if no audio was pushed. Mixer buffer will be filled with
      // silence or data from other sources.
      if (samples != NULL)
      {
         // Convert frameTime to offset in mixer buffer.
         // Note: frameTime >= mCurrentFrameTime.
         // Note: this calculation may overflow (e.g. 10000msec*44100Hz > 4294967296smp).
         unsigned mixerBufferOffset = 
                  ( (frameTime-mCurrentFrameTime) * mpDeviceDriver->getSamplesPerSec()) / 1000;

         // Mix this data with other sources.
         result = mixFrame(mixerBufferOffset, samples, numSamples);
      }
   }
   else
   {
      // We're in direct write mode.

      // In this mode pushed frame should have same size as device driver frame.
      // Later we may write code which enable pushing frames containing multiple
      // device driver frames.
      assert(numSamples == mpDeviceDriver->getSamplesPerFrame());

      // So, push data to device driver and forget.
      result = mpDeviceDriver->pushFrame(numSamples, samples, frameTime);

      // But do not forget to advance our frame time.
      mCurrentFrameTime += numSamples * 1000 / mpDeviceDriver->getSamplesPerSec();
   }

   return result;
};

/* ============================ ACCESSORS ================================= */

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
   if (isMixerBufferAvailable())
   {
      // Do not check return value. Nothing fatal may happen inside.
      freeMixerBuffer();
   }

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
   OsStatus result = OS_FAILED;

   // Free only if there is something to free.
   if (isMixerBufferAvailable())
   {
      mMixerBufferLength = 0;
      if (mpMixerBuffer != NULL)
      {
         delete[] mpMixerBuffer;
         mpMixerBuffer = NULL;
      }
      mMixerBufferBegin = 0;

      result = OS_SUCCESS;
   }

   return result;
}

OsStatus MpAudioOutputConnection::mixFrame(unsigned frameOffset,
                                           const MpAudioSample* samples,
                                           unsigned numSamples)
{
   assert(numSamples > 0);
   assert(samples != NULL);

   // Check for late frame. Whole frame should fit into buffer to be accepted.
   if (frameOffset+numSamples >= mMixerBufferLength)
   {
      osPrintf("MpAudioOutputConnection::mixFrame()"
               " OS_LIMIT_REACHED offset=%d, samples=%d, bufferLength=%d\n",
               frameOffset, numSamples, mMixerBufferLength);
      return OS_LIMIT_REACHED;
   }

   // Calculate frame start as if buffer is linear.
   unsigned frameBegin = mMixerBufferBegin+frameOffset;
   if (frameBegin >= mMixerBufferLength)
   {
      frameBegin -= mMixerBufferLength;
   }

   // Calculate size of first chunk to mix.
   unsigned firstChunkSize = min(numSamples, mMixerBufferLength-frameBegin);

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
   assert(numSamples > 0 && numSamples <= mMixerBufferLength);

   // If buffer could be copied in one pass
   if (mMixerBufferBegin+numSamples <= mMixerBufferLength)
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

OsStatus MpAudioOutputConnection::setDeviceTicker()
{
   // Do driver support ticker callback?
   if (!mpDeviceDriver->isFrameTickerSupported())
   {
      return OS_NOT_SUPPORTED;
   }

   // Create callback and register it with device driver
   mpTickerCallback = new OsCallback((int)this, tickerCallback);
   assert(mpTickerCallback != NULL);
   if (mpDeviceDriver->setTickerNotification(mpTickerCallback) != OS_SUCCESS)
   {
      delete mpTickerCallback;
      mpTickerCallback = NULL;
      freeMixerBuffer();
      return OS_FAILED;
   }

   return OS_SUCCESS;
}

OsStatus MpAudioOutputConnection::clearDeviceTicker()
{
   assert(mpTickerCallback != NULL);

   mpDeviceDriver->setTickerNotification(NULL);
   delete mpTickerCallback;
   mpTickerCallback = NULL;

   return OS_SUCCESS;
}

void MpAudioOutputConnection::tickerCallback(const int userData, const int eventData)
{
   OsStatus result;
   MpAudioOutputConnection *pConnection = (MpAudioOutputConnection*)userData;
   RTL_BLOCK("MpAudioOutputConnection::tickerCallBack");


   if (pConnection->mMutex.acquire(OsTime(5)) == OS_SUCCESS)
   {

      // If we're in mixer mode - push frame to device.
      if (pConnection->isMixerBufferAvailable())
      {
         // So, push data to device driver and forget.
         result = pConnection->mpDeviceDriver->pushFrame(
                        pConnection->mpDeviceDriver->getSamplesPerFrame(),
                        pConnection->mpMixerBuffer+pConnection->mMixerBufferBegin,
                        pConnection->mCurrentFrameTime);
         osPrintf("MpAudioOutputConnection::tickerCallback()"
                  " frame=%d, pushFrame result=%d\n",
                  pConnection->mCurrentFrameTime, result);
//       assert(result == OS_SUCCESS);

         // Advance mixer buffer and frame time.
         pConnection->advanceMixerBuffer(pConnection->mpDeviceDriver->getSamplesPerFrame());
         pConnection->mCurrentFrameTime +=
                              pConnection->mpDeviceDriver->getSamplesPerFrame() * 1000
                              / pConnection->mpDeviceDriver->getSamplesPerSec();
      }

      // Signal frame processing interval start if we were asked about.
      if (pConnection->mDoFlowgraphTicker)
      {
         MpMediaTask::signalFrameStart();
      }

      pConnection->mMutex.release();
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

