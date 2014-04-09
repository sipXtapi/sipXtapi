//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#define LOG_NDEBUG 0
#define LOG_TAG "MpodAndroid"
//#define ENABLE_FRAME_TIME_LOGGING
//#define ENABLE_FILE_LOGGING

// SIPX INCLUDES
#include "mp/MpodAndroid.h"
#include <os/OsCallback.h>

// SYSTEM INCLUDES
#include <stdio.h>
#include <math.h>
#include <utils/Log.h>
#include <media/AudioSystem.h>

//#define RTL_ENABLED
//#define RTL_AUDIO_ENABLED
#ifdef RTL_ENABLED // [
#  include "rtl_macro.h"
#else  // RTL_ENABLED ][
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x, y)
#endif // RTL_ENABLED ]

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

#ifdef ENABLE_FILE_LOGGING
static FILE *sgOutFile=NULL;
class OutFileInit
{
public:
   OutFileInit()
   {
      sgOutFile = fopen("/sdcard/out.raw", "w");
      LOGI("Openned file for audio debug: %x", sgOutFile);
   }

   ~OutFileInit()
   {
      fclose(sgOutFile);
   }
};

static OutFileInit sgOutFileInit;
#endif // ENABLE_FILE_LOGGING

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpodAndroid::MpodAndroid(MpAndroidAudioBindingInterface::StreamType streamType)
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
   if (MpAndroidAudioBindingInterface::spGetAndroidAudioBinding()->getOutputLatency(outputLatency, streamType) != NO_ERROR) {
      LOGE("Unable to marshal AudioFlinger");
      return;
   } else {
      LOGV("AudioFlinger default output latency for the stream is %d\n", outputLatency);
   }
}

MpodAndroid::~MpodAndroid()
{
   if (isEnabled()) 
   {
      LOGV("MpodAndroid::~MpodAndroid calling disableDevice\n");
      disableDevice();
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpodAndroid::enableDevice(unsigned samplesPerFrame,
                                   unsigned samplesPerSec,
                                   MpFrameTime currentFrameTime,
                                   OsCallback &frameTicker)
{
   LOGV("MpodAndroid::enableDevice(samplesPerFrame=%d, samplesPerSec=%d, currentFrameTime=%d, frameTicker=%p) mState=%d, mIsEnabled=%s\n",
        samplesPerFrame, samplesPerSec, currentFrameTime, (void*)&frameTicker, (int)mState, ((mIsEnabled != FALSE) ? "true" : "false"));

   if (isEnabled())
   {
      LOGE("MpodAndroid::enableDevice() already enabled");
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
         LOGE("MpodAndroid::enableDevice() init failed!");
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
   mpAudioTrack->dumpAudioTrack("MpodAndroid::enableDevice");
   mLock.unlock();
   mpAudioTrack->start();
   mpAudioTrack->dumpAudioTrack("MpodAndroid::enableDevice after start");
   mLock.lock();
   if (mState == DRIVER_STARTING) {
      LOGV("MpodAndroid::enableDevice() waiting for start callback");
      int /*status_t*/ lStatus = mWaitCbkCond.waitRelative(mLock, seconds(3));
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
      LOGV("Device already disabled\n");
      return OS_FAILED;
   }

   // Start accessing non-atomic member variables
   AutoMutex autoLock(mLock);

   if (mState == DRIVER_PLAYING || mState == DRIVER_STARTING) {
      mState = DRIVER_STOPPING;
      LOGV("MpodAndroid::disableDevice() waiting cond");
      int /*status_t*/ lStatus = mWaitCbkCond.waitRelative(mLock, seconds(3));
      if (lStatus == NO_ERROR) {
         LOGV("MpodAndroid::disableDevice() track stop complete, time %d", (unsigned int)(systemTime()/1000000));
      } else {
         LOGE("MpodAndroid::disableDevice() Stop timed out");
         mState = DRIVER_IDLE;
         mpAudioTrack->stop();
      }
   }

   mpAudioTrack->dumpAudioTrack("MpodAndroid::disableDevice");
   LOGV("MpodAndroid::disableDevice() Delete Track: %p\n", mpAudioTrack);
   delete mpAudioTrack;
   mpAudioTrack = NULL;
   LOGV("MpodAndroid::disableDevice() Deleted Track\n");

   // Clear out stream information.
   mSamplesPerFrame = 0;
   mSamplesPerSec = 0;
   mCurFrameTime = 0;
   mpTickerNotification = NULL;
   delete[] mpSampleBuffer;
   LOGV("MpodAndroid::disableDevice() Deleted mpSampleBuffer\n");

   // Indicate we are no longer enabled
   mIsEnabled = FALSE;

   return OS_SUCCESS;
}

OsStatus MpodAndroid::pushFrame(unsigned int numSamples,
                                const MpAudioSample* samples,
                                MpFrameTime frameTime)
{
   RTL_BLOCK("MpodAndroid::pushFrame");

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
   LOGV("MpodAndroid::initAudioTrack");
   int /*status_t*/ initRes;

   if (mpAudioTrack) {
      LOGV("MpodAndroid::initAudioTrack deleting mpAudioTrack");
      delete mpAudioTrack;
      LOGV("MpodAndroid::initAudioTrack deleted mpAudioTrack");
      mpAudioTrack = NULL;
   }

   // Open audio track
   mpAudioTrack = MpAndroidAudioBindingInterface::spGetAndroidAudioBinding()->createAudioTrack(mStreamType,  // streamType
                                 mSamplesPerSec,  // sampleRate
                                 AudioSystem::PCM_16_BIT,  // format
#ifdef ANDROID_1_6
                                 1,  // channels
#else
                                 AudioSystem::CHANNEL_OUT_MONO,
#endif
                                 0,  // frameCount
                                 0,  // flags
                                 audioCallback,  // cbf
                                 this,  // user
                                 mSamplesPerFrame);  // notificationFrames
   if (mpAudioTrack == NULL) {
      LOGE("MpodAndroid::initAudioTrack() AudioTrack allocation failed\n");
      goto initAudioTrack_exit;
   }
   mpAudioTrack->dumpAudioTrack("MpodAndroid::initAudioTrack");
   LOGV("MpodAndroid::initAudioTrack() Create Track: %p\n", mpAudioTrack);

   initRes = mpAudioTrack->initCheck();
   if (initRes != NO_ERROR) {
      LOGE("MpodAndroid::initAudioTrack() AudioTrack->initCheck() returned %d\n", initRes);
      goto initAudioTrack_exit;
   }

   LOGV("MpodAndroid::initAudioTrack() AudioTrack->initCheck() returned %d, NO_ERROR=%p\n", initRes, NO_ERROR);
   mpAudioTrack->dumpAudioTrack("MpodAndroid::initAudioTrack after initCheck");
   LOGD("initAudioTrack AudioTrack sampleRate: %d frameSize: %d frameCount: %d latency: %d",
        (int)mpAudioTrack->getSampleRate(), mpAudioTrack->frameSize(), (int)mpAudioTrack->frameCount(), (int)mpAudioTrack->latency());

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
   //LOGV("MpodAndroid::audioCallback(event=%d)\n", event);
   RTL_BLOCK("MpodAndroid::audioCallback");
   bool lSignal = false;
   if (event != MpAndroidAudioTrack::EVENT_MORE_DATA)
   {
      LOGV("MpodAndroid::audioCallback(event=%d)\n", event);
      return;
   }

   MpAndroidAudioTrack::Buffer *buffer = static_cast<MpAndroidAudioTrack::Buffer *>(info);
   MpodAndroid *pDriver = static_cast<MpodAndroid *>(user);

   // Start accessing non-atomic member variables
   AutoMutex autoLock(pDriver->mLock);

   int samplesToCopy = sipx_min(buffer->frameCount,
                                pDriver->mSamplesPerFrame-pDriver->mSampleBufferIndex);
#ifdef ENABLE_FRAME_TIME_LOGGING
   LOGV("MpodAndroid::audioCallback() buffer=%p samples=%d size=%d toCopy=%d\n",
        buffer->i16, buffer->frameCount, buffer->size, samplesToCopy);
#endif
   RTL_EVENT("MpodAndroid::audioCallback_bufsize", samplesToCopy);
   // Copy data to buffer
   memcpy(buffer->i16, pDriver->mpSampleBuffer+pDriver->mSampleBufferIndex, samplesToCopy*sizeof(short));
   buffer->frameCount = samplesToCopy;
   buffer->size = samplesToCopy*sizeof(short);
   pDriver->mSampleBufferIndex += samplesToCopy;

#ifdef ENABLE_FILE_LOGGING
   fwrite(buffer->i16, 1, buffer->frameCount*sizeof(short), sgOutFile);
#endif // ENABLE_FILE_LOGGING

   if (pDriver->mSampleBufferIndex >= pDriver->mSamplesPerFrame)
   {
      RTL_BLOCK("MpodAndroid::audioCallback_tick");
      if(pDriver->mSampleBufferIndex > pDriver->mSamplesPerFrame)
      {
         LOGE("MpodAndroid::audioCallback() sample index (%d) > samples/frame (%d)\n", (int)pDriver->mSampleBufferIndex, (int)pDriver->mSamplesPerFrame);
      }

      // Return index to the beginning
      pDriver->mSampleBufferIndex = 0;

      // Fire callback. It will call our pushFrame() in turn.
#ifdef ENABLE_FRAME_TIME_LOGGING
      LOGV("MpodAndroid::audioCallback() signal ticker, time %"PRIi64"ns\n", systemTime(SYSTEM_TIME_REALTIME));
#endif
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
       LOGV("MpodAndroid::audioCallback() stopping Track\n");
       pDriver->mpAudioTrack->stop();
       LOGV("MpodAndroid::audioCallback() stopped Track\n");
       buffer->size = 0;
       pDriver->mState = DRIVER_IDLE;
       lSignal = true;
       break;
    default:
       break;
   }

   if (lSignal)
   {
      LOGV("MpodAndroid::audioCallback signalling\n");
      pDriver->mWaitCbkCond.signal();
      LOGV("MpodAndroid::audioCallback signalled\n");
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
