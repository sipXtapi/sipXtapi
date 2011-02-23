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

// This wraper class is used to create some extra memory padding
// at the end of the AudioTrack class.  on Droid X an assert was
// firing because memory was getting trashed in the footer of the
// malloc memory chunk for the AudioTrack.  It is not clear why
// this was happening.  However when we got lucky and used a chunk
// with an extra 8 bytes at the end, everything was happy.  Hense
// this wrapper class with 2 int of bufer space at the end.
class SipxAudioTrack : public AudioTrack
{
public:
   SipxAudioTrack(int streamType,
                  uint32_t sampleRate  = 0,
                  int format           = 0,
                  int channels         = 0,
                  int frameCount       = 0,
                  uint32_t flags       = 0,
                  callback_t cbf       = 0,
                  void* user           = 0,
                  int notificationFrames = 0) :
   AudioTrack(streamType, sampleRate, format, channels, frameCount, flags, cbf, user, notificationFrames)
   {
      dummy1 = 11;
      dummy2 = 7;
   };

private:
   SipxAudioTrack(const SipxAudioTrack&); // no copy
   SipxAudioTrack& operator=(const SipxAudioTrack&); // no copy

   int dummy1;  // Padding to prevent overwrite on Droid X
   int dummy2;
};

void dumpAudioTrack(const char* label, AudioTrack* atPtr)
{
   LOGV("sizeof AudioTrack: %d sizeof size_t: %d\n", sizeof(AudioTrack), sizeof(size_t));
   LOGV("%s AudioTrack[-2] = %p\n", label, *(((int*)(atPtr))-2));
   LOGV("%s AudioTrack[-1] = %p\n", label, *(((int*)(atPtr))-1));
   LOGV("%s AudioTrack[%d] = %p\n", label, (sizeof(SipxAudioTrack)/4)+2, *(((int*)(atPtr))-(sizeof(SipxAudioTrack)/4)+2));
   LOGV("%s AudioTrack[%d] = %p\n", label, (sizeof(SipxAudioTrack)/4)+1, *(((int*)(atPtr))-(sizeof(SipxAudioTrack)/4)+1));
#if 0
   for(int ptrIndex = 0; ptrIndex < (sizeof(SipxAudioTrack) / 4); ptrIndex ++)
   {
      LOGV("%s AudioTrack[%d] = %p\n", label, ptrIndex, *(((int*)(atPtr))+ptrIndex));
   }
#endif
}

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
   dumpAudioTrack("MpodAndroid::enableDevice", mpAudioTrack);
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
      LOGV("Device already disabled\n");
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

   dumpAudioTrack("MpodAndroid::disableDevice", mpAudioTrack);
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
   status_t initRes;

   if (mpAudioTrack) {
      LOGV("MpodAndroid::initAudioTrack deleting mpAudioTrack");
      delete mpAudioTrack;
      LOGV("MpodAndroid::initAudioTrack deleted mpAudioTrack");
      mpAudioTrack = NULL;
   }

   // Open audio track
   mpAudioTrack = new SipxAudioTrack(mStreamType,  // streamType
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
   dumpAudioTrack("MpodAndroid::initAudioTrack", mpAudioTrack);
   LOGV("MpodAndroid::initAudioTrack() Create Track: %p\n", mpAudioTrack);

   initRes = mpAudioTrack->initCheck();
   if (initRes != NO_ERROR) {
      LOGE("MpodAndroid::initAudioTrack() AudioTrack->initCheck() returned %d\n", initRes);
      goto initAudioTrack_exit;
   }

   LOGV("MpodAndroid::initAudioTrack() AudioTrack->initCheck() returned %d, NO_ERROR=%p\n", initRes, NO_ERROR);
   dumpAudioTrack("MpodAndroid::initAudioTrack after initCheck", mpAudioTrack);
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
   RTL_BLOCK("MpodAndroid::audioCallback");
   bool lSignal = false;
   if (event != AudioTrack::EVENT_MORE_DATA)
   {
      LOGV("MpodAndroid::audioCallback(event=%d)\n", event);
      return;
   }

   AudioTrack::Buffer *buffer = static_cast<AudioTrack::Buffer *>(info);
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
