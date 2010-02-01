//  
// Copyright (C) 2010 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2010 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#define LOG_NDEBUG 0
#define LOG_TAG "AudioOut"

// SIPX INCLUDES
#include "mp/MpodAndroid.h"
#include <os/OsCallback.h>

// SYSTEM INCLUDES
#include <stdio.h>
#include <math.h>
#include <utils/Log.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpodAndroid::MpodAndroid(StreamType streamType)
: MpOutputDeviceDriver("")
, mStreamType(streamType)
, mState(DRIVER_IDLE)
, mpAudioTrack(NULL)
, mpNotifier(NULL)
, mpSampleBuffer(NULL)
, mSampleBufferIndex(0)
{
   int frameCount;
   uint32_t outputLatency;
   int samplingRate;

   LOGV("MpodAndroid constructor: streamType=%d\n", streamType);

   if (AudioSystem::getOutputSamplingRate(&samplingRate, streamType) != NO_ERROR) {
      LOGE("Unable to marshal AudioFlinger");
      return;
   } else {
      LOGV("AudioFlinger default sampling rate for the stream is %d\n", samplingRate);
   }
   if (AudioSystem::getOutputFrameCount(&frameCount, streamType) != NO_ERROR) {
      LOGE("Unable to marshal AudioFlinger");
      return;
   } else {
      LOGV("AudioFlinger default frame count for the stream is %d\n", frameCount);
   }
   if (AudioSystem::getOutputLatency(&outputLatency, streamType) != NO_ERROR) {
      LOGE("Unable to marshal AudioFlinger");
      return;
   } else {
      LOGV("AudioFlinger default output latency for the stream is %d\n", outputLatency);
   }
}

MpodAndroid::~MpodAndroid()
{
   if (mpAudioTrack) {
      disableDevice();
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpodAndroid::enableDevice(unsigned samplesPerFrame,
                                   unsigned samplesPerSec,
                                   MpFrameTime currentFrameTime,
                                   OsCallback &frameTicker)
{
   LOGV("MpodAndroid::enableDevice()\n");

   if (isEnabled())
   {
      return OS_FAILED;
   }

   // Start accessing non-atomic member variables
   AutoMutex autoLock(mLock);

   mSamplesPerFrame = samplesPerFrame;
   mSamplesPerSec = samplesPerSec;
   mCurFrameTime = currentFrameTime;
   mpTickerNotification = &frameTicker;
   mpSampleBuffer = new MpAudioSample[mSamplesPerFrame];
   memset(mpSampleBuffer, 0, mSamplesPerFrame*sizeof(MpAudioSample));
   mSampleBufferIndex = 0;

   if (mState == DRIVER_IDLE) {
      LOGV("MpodAndroid::enableDevice() trying to init AudioTrack");
      if (!initAudioTrack()) {
         return OS_FAILED;
      }
   }

   if (mState != DRIVER_INIT) {
      LOGV("MpodAndroid::enableDevice() Wrong MpodAndroid state: %d\n", mState.load());
      return OS_FAILED;
   }

   mIsEnabled = TRUE;

   LOGV("MpodAndroid::enableDevice() starting, time %d\n", (unsigned int)(systemTime()/1000000));
   mState = DRIVER_STARTING;
   mLock.unlock();
   mpAudioTrack->start();
   mLock.lock();
   if (mState == DRIVER_STARTING) {
      LOGV("MpodAndroid::enableDevice() waiting for start callback");
      status_t lStatus = mWaitCbkCond.waitRelative(mLock, seconds(3));
      if (lStatus != NO_ERROR) {
         LOGE("MpodAndroid::enableDevice() callback timed out, status %d", lStatus);
         mState = DRIVER_IDLE;
         mIsEnabled = FALSE;
         return OS_FAILED;
      }
   } else {
      LOGW("MpodAndroid::enableDevice() state %d\n", mState.load());
   }
   LOGV("MpodAndroid::enableDevice() started, time %d\n", (unsigned int)(systemTime()/1000000));

   return OS_SUCCESS;
}

OsStatus MpodAndroid::disableDevice()
{
   LOGV("MpodAndroid::disableDevice()");

   // If the device is not enabled,
   // then don't do anything and return failure.
   if ( !isEnabled() )
   {
      return OS_FAILED;
   }

   // Start accessing non-atomic member variables
   AutoMutex autoLock(mLock);

   if (mState == DRIVER_PLAYING || mState == DRIVER_STARTING) {
      mState = DRIVER_STOPPING;
      LOGV("MpodAndroid::disableDevice() waiting cond");
      status_t lStatus = mWaitCbkCond.waitRelative(mLock, seconds(3));
      if (lStatus == NO_ERROR) {
         LOGV("MpodAndroid::disableDevice() track stop complete, time %d", (unsigned int)(systemTime()/1000000));
      } else {
         LOGE("MpodAndroid::disableDevice() Stop timed out");
         mState = DRIVER_IDLE;
         mpAudioTrack->stop();
      }
   }

   LOGV("MpodAndroid::disableDevice() Delete Track: %p\n", mpAudioTrack);
   delete mpAudioTrack;
   mpAudioTrack = NULL;

   // Clear out stream information.
   mSamplesPerFrame = 0;
   mSamplesPerSec = 0;
   mCurFrameTime = 0;
   mpTickerNotification = NULL;
   delete[] mpSampleBuffer;

   // Indicate we are no longer enabled
   mIsEnabled = FALSE;
}

OsStatus MpodAndroid::pushFrame(unsigned int numSamples,
                                const MpAudioSample* samples,
                                MpFrameTime frameTime)
{
   if (!isEnabled())
   {
      return OS_FAILED;
   }

   // Only full frames are supported right now.
//   LOGV("MpodAndroid::pushFrame(numSamples=%d, samples=%p)", numSamples, samples);
   assert((mSamplesPerFrame == numSamples) ||
          (numSamples == 0 && samples== NULL));

   // This pushFrame() is actually just a hack to adopt Android's AudioTrack
   // for use with our driver framework. We know that this function will be
   // called from ticker callback, so we assume that mpSampleBuffer points
   // to a buffer, passed to us by AudioTrack and we can simply memcpy to it.

   // If samples == NULL, then silent (full) frame should be inserted.
   if (samples != NULL)
   {
      // We have data
#ifdef RTL_AUDIO_ENABLED
      RTL_RAW_AUDIO("MpodAndroid_pushFrame", mSamplesPerSec, numSamples, samples,
                    frameTime*(mSamplesPerSec/1000)/mSamplesPerFrame);
#endif

      if (mpSampleBuffer != NULL)
      {
         memcpy(mpSampleBuffer, samples, mSamplesPerFrame*sizeof(MpAudioSample));
      }
   } else {
      // No data, fill with silence
      if (mpSampleBuffer != NULL)
      {
         memset(mpSampleBuffer, 0, mSamplesPerFrame*sizeof(MpAudioSample));
#ifdef RTL_AUDIO_ENABLED
         RTL_RAW_AUDIO("MpodAndroid_pushFrame", mSamplesPerSec, mSamplesPerFrame, mpSampleBuffer,
                       frameTime*(mSamplesPerSec/1000)/mSamplesPerFrame);
#endif
      }
   }

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MpodAndroid::initAudioTrack()
{
   status_t initRes;

   if (mpAudioTrack) {
      delete mpAudioTrack;
      mpAudioTrack = NULL;
   }

   // Open audio track
   mpAudioTrack = new AudioTrack(mStreamType,  // streamType
//                               44100,  // sampleRate
                                 mSamplesPerSec,  // sampleRate
                                 AudioSystem::PCM_16_BIT,  // format
#ifdef ANDROID_1_6
                                 1,  // channels
#else
                                 AudioSystem::CHANNEL_OUT_MONO,
#endif
//                               6*441,  // frameCount
                                 0,  // frameCount
                                 0,  // flags
                                 audioCallback,  // cbf
                                 this,  // user
                                 mSamplesPerFrame);  // notificationFrames
   if (mpAudioTrack == NULL) {
      LOGE("MpidAndroid::initAudioRecord() AudioTrack allocation failed\n");
      goto initAudioTrack_exit;
   }
   LOGV("MpidAndroid::initAudioRecord() Create Track: %p\n", mpAudioTrack);

   initRes = mpAudioTrack->initCheck();
   if (initRes != NO_ERROR) {
      LOGE("MpidAndroid::initAudioRecord() AudioTrack->initCheck() returned %d\n", initRes);
      goto initAudioTrack_exit;
   }

   mpAudioTrack->setVolume(1.0, 1.0);

   mState = DRIVER_INIT;

   return true;

initAudioTrack_exit:

   // Cleanup
   if (mpAudioTrack) {
      LOGV("Delete Track: %p\n", mpAudioTrack);
      delete mpAudioTrack;
      mpAudioTrack = NULL;
   }

   return false;
}

void MpodAndroid::audioCallback(int event, void* user, void *info)
{
   bool lSignal = false;
   if (event != AudioTrack::EVENT_MORE_DATA)
   {
      LOGV("AudioOut::audioCallback(event=%d)\n", event);
      return;
   }

   AudioTrack::Buffer *buffer = static_cast<AudioTrack::Buffer *>(info);
   MpodAndroid *pDriver = static_cast<MpodAndroid *>(user);

   // Start accessing non-atomic member variables
   AutoMutex autoLock(pDriver->mLock);

   int samplesToCopy = sipx_min(buffer->frameCount,
                                pDriver->mSamplesPerFrame-pDriver->mSampleBufferIndex);
//   LOGV("AudioOut::audioCallback() buffer=%p samples=%d toCopy=%d\n",
//        buffer->i16, buffer->frameCount, samplesToCopy);

   // Copy data to buffer
   memcpy(buffer->i16, pDriver->mpSampleBuffer+pDriver->mSampleBufferIndex, samplesToCopy*sizeof(short));
   buffer->frameCount = samplesToCopy;
   buffer->size = samplesToCopy*sizeof(short);
   pDriver->mSampleBufferIndex += samplesToCopy;

   if (pDriver->mSampleBufferIndex >= pDriver->mSamplesPerFrame)
   {
      // Return index to the beginning
      pDriver->mSampleBufferIndex = 0;

      // Fire callback. It will call our pushFrame() in turn.
      LOGV("AudioOut::audioCallback() signal ticker, time %"PRIi64"ns\n", systemTime(SYSTEM_TIME_REALTIME));
      pDriver->mpTickerNotification->signal(pDriver->mSamplesPerFrame);

      // Update frame time.
      pDriver->mCurFrameTime += pDriver->mSamplesPerFrame;
   }

   // Step forward state
   switch (pDriver->mState) {
    case DRIVER_STARTING:
       pDriver->mState = DRIVER_PLAYING;
       lSignal = true;
       break;
    case DRIVER_STOPPING:
//       pDriver->mState = DRIVER_STOPPED;
//       break;
    case DRIVER_STOPPED:
       pDriver->mpAudioTrack->stop();
       buffer->size = 0;
       pDriver->mState = DRIVER_IDLE;
       lSignal = true;
       break;
    default:
       break;
   }

   if (lSignal)
   {
      pDriver->mWaitCbkCond.signal();
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
