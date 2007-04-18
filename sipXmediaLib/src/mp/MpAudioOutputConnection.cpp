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
#include <os/OsLock.h>
#include <os/OsCallback.h>

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

   if (!mpDeviceDriver->isFrameTickerSupported())
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

      // Create callback and register it with device driver
      mpTickerCallback = new OsCallback((int)this, tickerCallback);
      assert(mpTickerCallback != NULL);
      if (mpDeviceDriver->setTickerNotification(mpTickerCallback) != OS_SUCCESS)
      {
         freeMixerBuffer();
         return OS_NOT_SUPPORTED;
      }
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
   if (mpTickerCallback != NULL)
   {
      mpDeviceDriver->setTickerNotification(NULL);
      delete mpTickerCallback;
      mpTickerCallback = NULL;
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

OsStatus MpAudioOutputConnection::pushFrame(unsigned int numSamples,
                                            MpAudioSample* samples,
                                            MpFrameTime frameTime)
{
   OsStatus result = OS_FAILED;

   assert(samples != NULL);
   assert(numSamples > 0);

   // Check for late frame.
   // TODO:: This check should be fixed to support frame time wrap around.
   if (frameTime < mCurrentFrameTime)
   {
      result = OS_INVALID_STATE;
      return result;
   }

   // From now we access internal data. Take lock.
   OsLock lock(mMutex);

   // Do we have mixer buffer, i.e. are we in direct write mode or not?
   if (isMixerBufferAvailable())
   {
      // We're in mixer mode.

      // Convert frameTime to offset in mixer buffer.
      // Note: frameTime >= mCurrentFrameTime.
      // Note: this calculation may overflow (e.g. 10000msec*44100Hz > 4294967296smp).
      unsigned mixerBufferOffset = 
             ( (frameTime-mCurrentFrameTime) * mpDeviceDriver->getSamplesPerSec()) / 1000;

      // Mix this data with other sources.
      result = mixFrame(mixerBufferOffset, samples, numSamples);
   }
   else
   {
      // We're in direct write mode.
      // In this mode pushed frame should have same size as device driver frame.
      // Later we may write code which enable pushing frames containing multiple
      // device driver frames.
      assert(numSamples == mpDeviceDriver->getSamplesPerFrame());

      // So, push data to device driver and forget.
      result = mpDeviceDriver->pushFrame(numSamples, samples);
   }

   return result;
};

OsStatus MpAudioOutputConnection::pushToDevice()
{
   OsStatus result = OS_FAILED;

   OsLock lock(mMutex);

   // Return error if connection is configured in direct write mode.
   if (!isMixerBufferAvailable())
   {
      return OS_INVALID_STATE;
   }

   // Do not check does buffer wrap in the middle of the frame, because buffer
   // was allocated to be multiple of frame size (see enableDevice()).

   // Push frame to device.
   result = mpDeviceDriver->pushFrame(mpDeviceDriver->getSamplesPerFrame(),
                             mpMixerBuffer);

   // Advance mixer buffer even if push was not successful.
   // This may look strange, but really we do not want outdated frames to lay
   // here, new frames are coming.
   advanceMixerBuffer(mpDeviceDriver->getSamplesPerFrame());

   return result;
}

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
                                           MpAudioSample* samples,
                                           unsigned numSamples)
{
   assert(numSamples > 0);
   assert(samples != NULL);

   // Check for late frame. Whole frame should fit into buffer to be accepted.
   if (frameOffset+numSamples >= mMixerBufferLength)
   {
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
   assert(numSamples > 0 && numSamples < mMixerBufferLength);

   // If buffer could be copied in one pass
   if (mMixerBufferBegin+numSamples < mMixerBufferLength)
   {
      memset(&mpMixerBuffer[0],
             0,
             numSamples);
      mMixerBufferBegin += numSamples;
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

void MpAudioOutputConnection::tickerCallback(const int userData, const int eventData)
{
   OsStatus result;
   MpAudioOutputConnection *pConnection = (MpAudioOutputConnection*)userData;

   if (pConnection->mMutex.acquire(OsTime(5)) == OS_SUCCESS)
   {
      // So, push data to device driver and forget.
      result = pConnection->mpDeviceDriver->pushFrame(
                     pConnection->mpDeviceDriver->getSamplesPerFrame(),
                     pConnection->mpMixerBuffer+pConnection->mMixerBufferBegin);
      assert(result == OS_SUCCESS);

      pConnection->advanceMixerBuffer(pConnection->mpDeviceDriver->getSamplesPerFrame());

      pConnection->mMutex.release();
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

