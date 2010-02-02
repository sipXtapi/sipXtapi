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

// SIPX INCLUDES
#include "mp/MpidAndroid.h"
#include "mp/MpInputDeviceManager.h"
#include "mp/MpResampler.h"

// SYSTEM INCLUDES
#include <utils/Log.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const int MpidAndroid::mpSampleRatesList[] = 
   {8000, 16000, 24000, 32000, 48000, 96000, 11025, 22050, 44100};
const int MpidAndroid::mSampleRatesListLen =
   sizeof(MpidAndroid::mpSampleRatesList) / sizeof(MpidAndroid::mpSampleRatesList[0]);


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpidAndroid::MpidAndroid(audio_source source,
                         MpInputDeviceManager& deviceManager)
: MpInputDeviceDriver("", deviceManager)
, mStreamSource(source)
, mState(DRIVER_IDLE)
, mpAudioRecord(NULL)
, mpResampler(NULL)
, mpResampleBuf(NULL)
{
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
      return OS_INVALID_STATE;  // perhaps new OsState of OS_RESOURCE_INVALID?
   }

   if (isEnabled())
   {
      return OS_FAILED;
   }

   // Start accessing non-atomic member variables
   AutoMutex autoLock(mLock);

   if (mState != DRIVER_IDLE) {
      LOGV("MpidAndroid::enableDevice() wrong state %d\n", mState);
      return OS_FAILED;
   }

   // Set some wave header stat information.
   mSamplesPerFrame = samplesPerFrame;
   mSamplesPerSec = samplesPerSec;
   mCurrentFrameTime = currentFrameTime;

   if (probeSampleRate(mSamplesPerSec, mSamplesPerFrame, mpSampleRatesList, mSampleRatesListLen)) {
      LOGV("MpidAndroid::enableDevice() INIT OK, time: %"PRId64"\n", ns2ms(systemTime()));
   } else {
      LOGV("MpidAndroid::enableDevice() INIT FAILED!!!\n");
      return OS_FAILED;
   }

   if (mState != DRIVER_INIT) {
      LOGV("MpidAndroid::enableDevice() wrong state: %d\n", mState);
      return OS_FAILED;
   }

   // Create resampler
   if (mSamplesPerSecInternal != mSamplesPerSec)
   {
      mpResampler = MpResamplerBase::createResampler(1, mSamplesPerSecInternal, mSamplesPerSec);
      mpResampleBuf = new MpAudioSample[mSamplesPerFrameInternal];
      assert(mpResampler != NULL && mpResampleBuf != NULL);
   }

   mState = DRIVER_STARTING;
   mLock.unlock();
   mpAudioRecord->start();
   mLock.lock();
   if (mState == DRIVER_STARTING) {
      LOGV("MpidAndroid::enableDevice() waiting for start callback");
      status_t lStatus = mWaitCbkCond.waitRelative(mLock, seconds(3));
      if (lStatus != NO_ERROR) {
         LOGE("MpidAndroid::enableDevice() callback timed out, status %d", lStatus);
         mState = DRIVER_IDLE;
         mIsEnabled = FALSE;
         return OS_FAILED;
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
      }
   }

   // Clear out all the wave header information.
   mSamplesPerFrame = 0;
   mSamplesPerSec = 0;
   mCurrentFrameTime = 0;

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
   for (int i=0; i<ratesListLen; i++)
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

   AudioRecord::Buffer *buffer = static_cast<AudioRecord::Buffer *>(info);
   MpidAndroid *pDriver = static_cast<MpidAndroid *>(user);

   // Start accessing non-atomic member variables
   AutoMutex autoLock(pDriver->mLock);
   LOGV("MpidAndroid::audioCallback() frameCount=%d state=%d\n", buffer->frameCount, pDriver->mState);

   // Only process if we're enabled..
   if(pDriver->mIsEnabled)
   {
      if (buffer->frameCount != pDriver->mSamplesPerFrame)
      {
         LOGE("frameCount=%d mSamplesPerFrame=%d", buffer->frameCount, pDriver->mSamplesPerFrame);
         assert(buffer->frameCount == pDriver->mSamplesPerFrame);
      }
      else
      {
         MpAudioSample *pushSamples = buffer->i16;
         if (pDriver->mpResampler != NULL)
         {
            uint32_t samplesProcessed;
            uint32_t samplesWritten;
            pDriver->mpResampler->resample(0, buffer->i16,
                                           pDriver->mSamplesPerFrameInternal, samplesProcessed,
                                           pDriver->mpResampleBuf,
                                           pDriver->mSamplesPerFrame, samplesWritten);
            assert(pDriver->mSamplesPerFrameInternal == pDriver->samplesProcessed
                   && pDriver->mSamplesPerFrame == pDriver->samplesWritten);
            pushSamples = pDriver->mpResampleBuf;
         }
         pDriver->mpInputDeviceManager->pushFrame(pDriver->mDeviceId,
                                                  pDriver->mSamplesPerFrame,
                                                  pushSamples,
                                                  pDriver->mCurrentFrameTime);
      }
      // Ok, we have received and pushed a frame to the manager,
      // Now we advance the frame time.
      pDriver->mCurrentFrameTime += (pDriver->mSamplesPerFrame*1000)/pDriver->mSamplesPerSec;
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
