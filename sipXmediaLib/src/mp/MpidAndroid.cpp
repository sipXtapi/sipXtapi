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
#define LOG_TAG "MpidAndroid"
#define MPID_ANDROID_CLEAN_EXIT
//#define ENABLE_FRAME_TIME_LOGGING

// SIPX INCLUDES
#include <os/OsSysLog.h>
#include <mp/MpidAndroid.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MpResampler.h>

// SYSTEM INCLUDES
#include <utils/Log.h>
#include <media/AudioSystem.h>
#ifdef MPID_ANDROID_CLEAN_EXIT // [
#  include <signal.h>
#endif // MPID_ANDROID_CLEAN_EXIT ]

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const int MpidAndroid::mpSampleRatesList[] = 
   {8000, 16000, 24000, 32000, 48000, 96000, 11025, 22050, 44100};
const int MpidAndroid::mSampleRatesListLen =
   sizeof(MpidAndroid::mpSampleRatesList) / sizeof(MpidAndroid::mpSampleRatesList[0]);

#ifdef MPID_ANDROID_CLEAN_EXIT // [
static AudioRecord *sgAudioRecord = NULL;

static void sigabrt_handler(int signum)
{
   LOGE("Caught signal: %d", signum);
   if(sgAudioRecord != NULL)
   {
      LOGE("sigabrt_handler deleting sgAudioRecord: %p", sgAudioRecord);
      sgAudioRecord->stop();
      delete sgAudioRecord;
      sgAudioRecord = NULL;
   }
   exit(2);
}
#endif // MPID_ANDROID_CLEAN_EXIT ]

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpidAndroid::MpidAndroid(audio_source source,
                         MpInputDeviceManager& deviceManager)
: MpInputDeviceDriver("default", deviceManager)
, mStreamSource(source)
, mState(DRIVER_IDLE)
, mpAudioRecord(NULL)
, mpBufInternal(NULL)
, mBufInternalSamples(0)
, mpResampler(NULL)
, mpResampleBuf(NULL)
{
#ifdef MPID_ANDROID_CLEAN_EXIT // [
   // THIS IS A BIG HACK
   // We intercept SIGABRT signal here, which may be intercepted somewhere
   // else already. We shouldn't do this in a generic system, but for now
   // I think we don't intercept it anywhere else anyway.
   signal(SIGABRT, sigabrt_handler);
   // Current Android use SIGSEGV instead of SIGABRT.
   // See: bionic/libc/unistd/abort.c
   signal(SIGSEGV, sigabrt_handler);
#endif // MPID_ANDROID_CLEAN_EXIT ]
}

MpidAndroid::~MpidAndroid()
{
   LOGV("MpidAndroid destructor\n");

   if (mpAudioRecord) {
      disableDevice();
      LOGV("Delete Track Record: %p\n", mpAudioRecord);
      delete mpAudioRecord;
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpidAndroid::enableDevice(unsigned samplesPerFrame, 
                                   unsigned samplesPerSec, 
                                   MpFrameTime currentFrameTime)
{
   OsStatus status = OS_SUCCESS;

   // If the device is not valid, let the user know it's bad.
   if (!isDeviceValid())
   {
      return OS_INVALID;  // perhaps new OsState of OS_RESOURCE_INVALID?
   }

   if (isEnabled())
   {
      return OS_NAME_IN_USE;
   }

   // Start accessing non-atomic member variables
   AutoMutex autoLock(mLock);

   if (mState != DRIVER_IDLE) {
      LOGV("MpidAndroid::enableDevice() wrong state %d\n", mState);
      return OS_INVALID_STATE;
   }

   // Set some wave header stat information.
   mSamplesPerFrame = samplesPerFrame;
   mSamplesPerSec = samplesPerSec;
   mCurrentFrameTime = currentFrameTime;

   if (probeSampleRate(mSamplesPerSec, mSamplesPerFrame, mpSampleRatesList, mSampleRatesListLen)) {
      LOGV("MpidAndroid::enableDevice() INIT OK, time: %"PRId64"\n", ns2ms(systemTime()));
   } else {
      LOGV("MpidAndroid::enableDevice() INIT FAILED!!!\n");
      return OS_INVALID_ARGUMENT;
   }

   if (mState != DRIVER_INIT) {
      LOGV("MpidAndroid::enableDevice() wrong state: %d\n", mState);
      return OS_INVALID_STATE;
   }

   // Allocate internal buffer
   assert(mSamplesPerFrameInternal > 0);
   mpBufInternal = new MpAudioSample[mSamplesPerFrameInternal];
   assert(mpBufInternal != NULL);
   mBufInternalSamples = 0;

   // Create resampler
   if (mSamplesPerSecInternal != mSamplesPerSec)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "mSamplesPerSecInternal: %d mSamplesPerSec: %d\n", mSamplesPerSecInternal, mSamplesPerSec);
      LOGV("mSamplesPerSecInternal: %d mSamplesPerSec: %d\n", mSamplesPerSecInternal, mSamplesPerSec);
      mpResampler = MpResamplerBase::createResampler(1, mSamplesPerSecInternal, mSamplesPerSec);
      mpResampleBuf = new MpAudioSample[mSamplesPerFrame];
      assert(mpResampler != NULL && mpResampleBuf != NULL);
   }

   mState = DRIVER_STARTING;
   mLock.unlock();
   status_t startStatus = mpAudioRecord->start();
   if(startStatus != NO_ERROR)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MpidAndroid::enableDevice AudioRecord::start returned error: %d", startStatus);
      LOGE("MpidAndroid::enableDevice AudioRecord::start returned error: %d", startStatus);
      switch(startStatus)
      {
      case INVALID_OPERATION:
           status = OS_LIMIT_REACHED;
      break;
      default:
           status = OS_FAILED;
      break;
      }

      mState = DRIVER_IDLE;
      mIsEnabled = FALSE;
      return(status);
   }

   mLock.lock();
   if (mState == DRIVER_STARTING) {
      LOGV("MpidAndroid::enableDevice() waiting for start callback");
      status_t lStatus = mWaitCbkCond.waitRelative(mLock, seconds(3));
      if (lStatus != NO_ERROR) {
         LOGE("MpidAndroid::enableDevice() callback timed out, status %d", lStatus);
         mState = DRIVER_IDLE;
         mIsEnabled = FALSE;
         return OS_WAIT_TIMEOUT;
      }
   } else {
      LOGW("MpidAndroid::enableDevice() state %d\n", mState);
   }
   LOGV("MpidAndroid::enableDevice() started, time %"PRId64"\n", ns2ms(systemTime()));

   // Indicate driver has been started.
   mIsEnabled = TRUE;

   return status;
}

OsStatus MpidAndroid::disableDevice()
{
   OsStatus status = OS_SUCCESS;

   if (!isDeviceValid() || !isEnabled())
   {
      return OS_FAILED;
   }

   // Start accessing non-atomic member variables
   AutoMutex autoLock(mLock);

   if (mState == DRIVER_RECORDING || mState == DRIVER_STARTING) {
      mState = DRIVER_STOPPING;
      LOGV("MpidAndroid::disableDevice() waiting cond");
      status_t lStatus = mWaitCbkCond.waitRelative(mLock, seconds(3));
      if (lStatus == NO_ERROR) {
         LOGV("MpidAndroid::disableDevice() track stop complete, time %d", (unsigned int)(systemTime()/1000000));
      } else {
         LOGE("MpidAndroid::disableDevice() Stop timed out");
         mState = DRIVER_IDLE;
         mpAudioRecord->stop();
         delete mpAudioRecord;
         mpAudioRecord = NULL;
#ifdef MPID_ANDROID_CLEAN_EXIT // [
         sgAudioRecord = NULL;
#endif // MPID_ANDROID_CLEAN_EXIT ]
      }
   }

   // Clear out all the audio stream information.
   mSamplesPerFrame = 0;
   mSamplesPerSec = 0;
   mCurrentFrameTime = 0;

   // Free internal buffer
   delete[] mpBufInternal;
   mBufInternalSamples = 0;

   // Free resampler
   delete mpResampler;
   mpResampler = NULL;
   delete[] mpResampleBuf;
   mpResampleBuf = NULL;

   // Indicate driver is no longer enabled
   mIsEnabled = FALSE;

   return status;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean MpidAndroid::isDeviceValid()
{
   return mStreamSource < AUDIO_SOURCE_LIST_END;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

bool MpidAndroid::initAudioRecord()
{
   status_t initRes;

   if (mpAudioRecord) {
      delete mpAudioRecord;
      mpAudioRecord = NULL;
   }

   // Open audio track
   mpAudioRecord = new AudioRecord();
   if (mpAudioRecord == NULL) {
      LOGE("MpidAndroid::initAudioRecord() AudioRecord allocation failed\n");
      goto initAudioTrack_exit;
   }
   LOGV("MpidAndroid::initAudioRecord() Create Record: %p\n", mpAudioRecord);

#ifdef MPID_ANDROID_CLEAN_EXIT // [
   sgAudioRecord = mpAudioRecord;
#endif // MPID_ANDROID_CLEAN_EXIT ]

//   initRes = mpAudioTrack->initCheck();
   initRes = mpAudioRecord->set(mStreamSource,  // inputSource
                                mSamplesPerSecInternal,  // sampleRate
                                AudioSystem::PCM_16_BIT,  // format
                                AudioSystem::CHANNEL_IN_MONO,  // channels
                                0,  // frameCount
                                AudioRecord::RECORD_AGC_ENABLE | AudioRecord::RECORD_NS_ENABLE,  // flags
                                audioCallback,  // cbf
                                (void*)this,  // user
                                mSamplesPerFrameInternal,  // notificationFrames
                                false);  // threadCanCallJava
   if (initRes != NO_ERROR) {
      LOGE("MpidAndroid::initAudioRecord() AudioRecord->initCheck() returned %d\n", initRes);
      goto initAudioTrack_exit;
   }

   mState = DRIVER_INIT;

   return true;

initAudioTrack_exit:

   // Cleanup
   if (mpAudioRecord) {
      LOGV("MpidAndroid::initAudioRecord() Delete AudioRecord: %p\n", mpAudioRecord);
      delete mpAudioRecord;
      mpAudioRecord = NULL;
   }

   return false;
}

bool MpidAndroid::probeSampleRate(int targetRate, int targetFrameSize,
                                  const int *ratesList, int ratesListLen,
                                  bool probeDefault)
{
   // Test target frequency first. If this works, we don't need anything else.
   mSamplesPerSecInternal = targetRate;
   mSamplesPerFrameInternal = targetFrameSize;
   LOGI("MpidAndroid::probeSampleRate() testing frequency %d with frame size %d (target)\n",
        targetRate, targetFrameSize);
   if (initAudioRecord())
   {
      LOGI("MpidAndroid::probeSampleRate() success!\n");
      return true;
   }

   // Now test frequencies which are higher then target one.
   for (int i=0; i<ratesListLen; i++)
   {
      if (ratesList[i] > targetRate)
      {
         int remainingSamplesNum;
         mSamplesPerSecInternal = ratesList[i];
         mSamplesPerFrameInternal =
            MpResamplerBase::getNumSamplesOriginal(mSamplesPerSecInternal,
                                                   targetRate, targetFrameSize,
                                                   remainingSamplesNum);
         // We only want frequencies where frames can be converted 1-to-1 to
         // target ones. Elsewise we'll have troubles with callback rate.
         if (remainingSamplesNum != 0)
         {
            LOGI("MpidAndroid::probeSampleRate() skipping frequency %d because frame size is fractional (%d%%%d)\n",
                 mSamplesPerSecInternal, mSamplesPerFrameInternal, remainingSamplesNum);
         }
         else
         {
            LOGI("MpidAndroid::probeSampleRate() testing frequency %d with frame size %d\n",
                 mSamplesPerSecInternal, mSamplesPerFrameInternal);
            if (initAudioRecord())
            {
               LOGI("MpidAndroid::probeSampleRate() success!\n");
               return true;
            }
         }
      }
   }

   // And now test frequencies which are lower then target one.
   for (int i=ratesListLen-1; i>=0; i--)
   {
      if (ratesList[i] < targetRate)
      {
         int remainingSamplesNum;
         mSamplesPerSecInternal = ratesList[i];
         mSamplesPerFrameInternal =
            MpResamplerBase::getNumSamplesOriginal(mSamplesPerSecInternal,
                                                   targetRate, targetFrameSize,
                                                   remainingSamplesNum);
         // We only want frequencies where frames can be converted 1-to-1 to
         // target ones. Elsewise we'll have troubles with callback rate.
         if (remainingSamplesNum != 0)
         {
            LOGI("MpidAndroid::probeSampleRate() skipping frequency %d because frame size is fractional (%d%%%d)\n",
                 mSamplesPerSecInternal, mSamplesPerFrameInternal, remainingSamplesNum);
            continue;
         }
         else
         {
            LOGI("MpidAndroid::probeSampleRate() testing frequency %d with frame size %d\n",
                 mSamplesPerSecInternal, mSamplesPerFrameInternal);
            if (initAudioRecord())
            {
               LOGI("MpidAndroid::probeSampleRate() success!\n");
               return true;
            }
         }
      }
   }

   // If nothing helps - try default frequency.
   int remainingSamplesNum;
   mSamplesPerSecInternal = AudioRecord::DEFAULT_SAMPLE_RATE;
   mSamplesPerFrameInternal =
      MpResamplerBase::getNumSamplesOriginal(mSamplesPerSecInternal,
                                             targetRate, targetFrameSize,
                                             remainingSamplesNum);
   // We only want frequencies where frames can be converted 1-to-1 to
   // target ones. Elsewise we'll have troubles with callback rate.
   if (remainingSamplesNum != 0)
   {
      LOGI("MpidAndroid::probeSampleRate() skipping frequency %d because frame size is fractional (%d%%%d)\n",
            mSamplesPerSecInternal, mSamplesPerFrameInternal, remainingSamplesNum);
   }
   else
   {
      LOGI("MpidAndroid::probeSampleRate() testing frequency %d with frame size %d\n",
           mSamplesPerSecInternal, mSamplesPerFrameInternal);
      if (initAudioRecord())
      {
         LOGI("MpidAndroid::probeSampleRate() success!\n");
         return true;
      }
   }

   return false;
}

void MpidAndroid::audioCallback(int event, void* user, void *info)
{
   bool lSignal = false;
   if (event != AudioTrack::EVENT_MORE_DATA) {
      LOGV("MpidAndroid::audioCallback(event=%d)\n", event);
      return;
   }

#ifdef ENABLE_FRAME_TIME_LOGGING
   LOGV("MpidAndroid::audioCallback() time %"PRIi64"ns\n", systemTime(SYSTEM_TIME_REALTIME));
#endif

   AudioRecord::Buffer *buffer = static_cast<AudioRecord::Buffer *>(info);
   MpidAndroid *pDriver = static_cast<MpidAndroid *>(user);

   // Start accessing non-atomic member variables
   AutoMutex autoLock(pDriver->mLock);
#ifdef ENABLE_FRAME_TIME_LOGGING
   LOGV("MpidAndroid::audioCallback() frameCount=%d state=%d\n", buffer->frameCount, pDriver->mState);
#endif

   // Only process if we're enabled..
   if(pDriver->mIsEnabled)
   {
      if (buffer->frameCount + pDriver->mBufInternalSamples < pDriver->mSamplesPerFrameInternal)
      {
         LOGV("frameCount=%d mBufInternalSamples=%d (sum=%d) mSamplesPerFrameInternal=%d",
              buffer->frameCount, pDriver->mBufInternalSamples,
              buffer->frameCount + pDriver->mBufInternalSamples,
              pDriver->mSamplesPerFrameInternal);

         memcpy(pDriver->mpBufInternal, buffer->i16, buffer->frameCount*sizeof(short));
         pDriver->mBufInternalSamples += buffer->frameCount;
      }
      else
      {
         // Copy samples to the temp buffer if needed.
         MpAudioSample *origSamples;
         int origSamplesConsumed;
         if (pDriver->mBufInternalSamples > 0)
         {
            origSamplesConsumed = sipx_min(pDriver->mSamplesPerFrameInternal-pDriver->mBufInternalSamples,
                                           buffer->frameCount);
            memcpy(pDriver->mpBufInternal, buffer->i16, origSamplesConsumed*sizeof(short));
            pDriver->mBufInternalSamples += origSamplesConsumed;
            origSamples = pDriver->mpBufInternal;
         }
         else
         {
            origSamples = buffer->i16;
            origSamplesConsumed = pDriver->mSamplesPerFrameInternal;
         }
         
         // Resample is needed.
         MpAudioSample *pushSamples = origSamples;
         if (pDriver->mpResampler != NULL)
         {
            uint32_t samplesProcessed;
            uint32_t samplesWritten;
            LOGV("origSamples: %d mSamplesPerFrameInternal: %d samplesProcessed: %d mpResampleBuf: %d mSamplesPerFrame: %d samplesWritten: %d\n",
                 pDriver->mBufInternalSamples, pDriver->mSamplesPerFrameInternal, samplesProcessed, pDriver->mpResampleBuf, pDriver->mSamplesPerFrame, samplesWritten);
            LOGV("pDriver->mpResampler->getInputRate(): %d pDriver->mpResampler->getOutputRate(): %d\n",
                 pDriver->mpResampler->getInputRate(), pDriver->mpResampler->getOutputRate());
            OsStatus status =
               pDriver->mpResampler->resample(0, pDriver->mpBufInternal,
                                              pDriver->mSamplesPerFrameInternal, samplesProcessed,
                                              pDriver->mpResampleBuf,
                                              pDriver->mSamplesPerFrame, samplesWritten);
            assert(status == OS_SUCCESS);
            if(pDriver->mSamplesPerFrameInternal != samplesProcessed ||
               pDriver->mSamplesPerFrame != samplesWritten)
            {
               LOGE("mSamplesPerFrameInternal: %d samplesProcessed: %d mSamplesPerFrame: %d samplesWritten: %d\n", 
                      pDriver->mSamplesPerFrameInternal, samplesProcessed, pDriver->mSamplesPerFrame, samplesWritten);
               printf("mSamplesPerFrameInternal: %d samplesProcessed: %d mSamplesPerFrame: %d samplesWritten: %d\n", 
                      pDriver->mSamplesPerFrameInternal, samplesProcessed, pDriver->mSamplesPerFrame, samplesWritten);
            }
            assert(pDriver->mSamplesPerFrameInternal == samplesProcessed
                   && pDriver->mSamplesPerFrame == samplesWritten);
            pushSamples = pDriver->mpResampleBuf;
         }
         pDriver->mpInputDeviceManager->pushFrame(pDriver->mDeviceId,
                                                  pDriver->mSamplesPerFrame,
                                                  pushSamples,
                                                  pDriver->mCurrentFrameTime);

         // Copy remaining samples to temp buffer if anything left.
         pDriver->mBufInternalSamples = sipx_min(buffer->frameCount-origSamplesConsumed,
                                                 pDriver->mSamplesPerFrameInternal);
         if (pDriver->mBufInternalSamples > 0)
         {
            memcpy(pDriver->mpBufInternal, buffer->i16+origSamplesConsumed,
                   pDriver->mBufInternalSamples*sizeof(short));
            if (buffer->frameCount-origSamplesConsumed >= pDriver->mSamplesPerFrameInternal)
            {
               LOGW("TOO BIG FRAMES FROM MIC: %d", buffer->frameCount);
            }
            
         }

         // Ok, we have received and pushed a frame to the manager,
         // Now we advance the frame time.
         pDriver->mCurrentFrameTime += (pDriver->mSamplesPerFrame*1000)/pDriver->mSamplesPerSec;
      }
   }

   switch (pDriver->mState) {
    case DRIVER_STARTING:
       pDriver->mState = DRIVER_RECORDING;
       lSignal = true;
       break;
    case DRIVER_STOPPING:
//      pDriver->mState = DRIVER_STOPPED;
//      break;
    case DRIVER_STOPPED:
       pDriver->mpAudioRecord->stop();
       buffer->size = 0;
       pDriver->mState = DRIVER_IDLE;
       lSignal = true;
       break;
    default:
       break;
   }

   if (lSignal)
   {
      LOGV("MpidAndroid::audioCallback() signaling condition state=%d\n", pDriver->mState);
      pDriver->mWaitCbkCond.signal();
   }

//   LOGV("MpidAndroid::audioCallback() done.\n");
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
