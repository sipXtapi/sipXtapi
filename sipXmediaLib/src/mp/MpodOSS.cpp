//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>


// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#ifdef __linux__
#include <sys/types.h>
#include <sys/soundcard.h>
#endif // __linux__


// APPLICATION INCLUDES
#include "mp/MpodOSS.h"
#include "mp/MpOutputDeviceManager.h"
#include "mp/MpOSSDeviceWrapper.h"
#include "os/OsTask.h"
#include "os/OsNotification.h"

#ifdef RTL_ENABLED // [
#  include "rtl_macro.h"
#else  // RTL_ENABLED ][
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x, y)
#endif // RTL_ENABLED ]

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
#ifdef OSS_SINGLE_DEVICE
extern MpOSSDeviceWrapper ossSingleDriver;
#else
//extern MpOSSDeviceWrapperContainer mOSSContainer;
#endif

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Default constructor
MpodOSS::MpodOSS(const UtlString& name,
                 unsigned nInputBuffers)
: MpOutputDeviceDriver(name)
, mNumInBuffers(nInputBuffers)
, mpWaveBuffers(NULL)
, mCurBuff(-1)
, mLastReceived(-1)
, pNotificator(NULL)
, mNotificationThreadEn(FALSE)
, pDevWrapper(NULL)
{
   int result = sem_init(&mPushPopSem, 0, 1);
   assert(result != -1);

#ifdef OSS_SINGLE_DEVICE
   pDevWrapper = &ossSingleDriver;
#else
   //pDevWrapper = mOSSContainer.getOSSDeviceWrapper(name);
   mpCont = MpOSSDeviceWrapperContainer::getContainer();
   if (mpCont != NULL)
   {
      pDevWrapper = mpCont->getOSSDeviceWrapper(name);
   }
   else
   {
      pDevWrapper = NULL;
   }
#endif
}

MpodOSS::~MpodOSS()
{
    /*
   if (isDeviceValid())
   {
      pDevWrapper->freeOutputDevice();
   }*/

   // OSS Device must be freed
   assert (!isDeviceValid());

   if (mpCont != NULL)
   {
      MpOSSDeviceWrapperContainer::releaseContainer(mpCont);
   }

   sem_wait(&mPushPopSem);
   sem_destroy(&mPushPopSem);
}
/* ============================ MANIPULATORS ============================== */
OsStatus MpodOSS::setNotificationMode(UtlBoolean bThreadNotification)
{
   if (isEnabled())
   {
      return OS_INVALID_STATE;
   }

   mNotificationThreadEn = bThreadNotification;
   return OS_SUCCESS;
}

OsStatus MpodOSS::enableDevice(unsigned samplesPerFrame,
                               unsigned samplesPerSec,
                               MpFrameTime currentFrameTime)
{
   OsStatus ret;
   if (isEnabled())
   {
       return OS_FAILED;
   }

   //Opening OSS device
   if (pDevWrapper)
   {
       OsStatus res = pDevWrapper->setOutputDevice(this);
       if (res != OS_SUCCESS)
       {
           pDevWrapper = NULL;
       }
       else if (!pDevWrapper->mbWriteCap)
       {
           //Device dosen't support output
           pDevWrapper->freeOutputDevice();
           pDevWrapper = NULL;
       }
   }

   // If the device is not valid, let the user know it's bad.
   if (!isDeviceValid())
   {
      return OS_INVALID_STATE;
   }

   // Set some wave header stat information.
   mSamplesPerFrame = samplesPerFrame;
   mSamplesPerSec = samplesPerSec;
   mCurrentFrameTime = currentFrameTime;

   ret = initBuffers();
   if (ret != OS_SUCCESS)
   {
      return ret;
   }

   ret = pDevWrapper->attachWriter();
   if (ret != OS_SUCCESS)
   {
      return ret;
   }
   mIsEnabled = TRUE;
   mQueueLen = 0;
   return ret;
}

OsStatus MpodOSS::disableDevice()
{
   OsStatus ret;
   if (!isEnabled())
   {
       return OS_FAILED;
   }

   // If the device is not valid, let the user know it's bad.
   if (!isDeviceValid())
   {
      return OS_INVALID_STATE;
   }

   ret = pDevWrapper->detachWriter();
   if (ret != OS_SUCCESS)
   {
      return ret;
   }
   freeBuffers();
   mIsEnabled = FALSE;

   pDevWrapper->freeOutputDevice();

   return ret;
}

OsStatus MpodOSS::pushFrame(unsigned int numSamples,
                            const MpAudioSample* samples,
                            MpFrameTime frameTime)
{
   if (!isEnabled())
      return OS_FAILED;

   OsStatus status = OS_FAILED;
   // Currently only full frame supported
   assert(numSamples == mSamplesPerFrame);

   RTL_BLOCK("MpodOSS::pushFrame");
   
   int res = sem_wait(&mPushPopSem);
   assert (res != -1);

   UtlBoolean doSignal = FALSE;
   
   do
   {
      if (((mCurBuff + 1 ) == mLastReceived) ||
         ((mCurBuff + 1  == mNumInBuffers) && (mLastReceived == 0)))
      {
         //Overwriting existing buffer
         //Need more buffs
         OsSysLog::add(FAC_MP, PRI_DEBUG,
            "OSS: MpodOSS out buffer is overflowing\n");

         status = OS_LIMIT_REACHED;
         break;
      }
      MpAudioSample* buff = (MpAudioSample*)(mpWaveBuffers +
                            mCurBuff * mSamplesPerFrame * sizeof(MpAudioSample) *
                            (OSS_SOUND_STEREO ? 2 : 1));
      mCurBuff++;
      if (mCurBuff == mNumInBuffers)
      {
         mCurBuff = 0;
      }
      if (samples)
      {
           memcpy(buff, samples,
                  numSamples * sizeof(MpAudioSample) * (OSS_SOUND_STEREO ? 2 : 1));
      }
      else
      {
           memset(buff, 0,
                  numSamples * sizeof(MpAudioSample) * (OSS_SOUND_STEREO ? 2 : 1));
      }
      mCurrentFrameTime += getFramePeriod();

      doSignal = TRUE;
      mQueueLen ++;
      RTL_EVENT("MpodOSS::queue", mQueueLen);

      status = OS_SUCCESS;
   } while (FALSE);
   res = sem_post(&mPushPopSem);
   assert (res != -1);

   if (doSignal)
   {
      res = pthread_cond_signal(&pDevWrapper->mNewQueueFrame);
      assert (res != -1);
   }

   return status;
}

OsStatus MpodOSS::setTickerNotification(OsNotification *pFrameTicker)
{/*
   if (!isDeviceValid())
   {
      return OS_FAILED;
   }*/

   pNotificator = pFrameTicker;
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */
UtlBoolean MpodOSS::isFrameTickerSupported() const
{
   return TRUE;
}

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
OsStatus MpodOSS::initBuffers()
{
   int total_size = mNumInBuffers * mSamplesPerFrame * sizeof(MpAudioSample) *
                    (OSS_SOUND_STEREO ? 2 : 1);

   mpWaveBuffers = new char[total_size];
   if (mpWaveBuffers != NULL)
   {
      mCurBuff = 0;
      mLastReceived = 0;
      return OS_SUCCESS;
   }
   else
   {
      mCurBuff = -1;
      mLastReceived = -1;
   }
   return OS_FAILED;
}

void MpodOSS::freeBuffers()
{
   int res = sem_wait(&mPushPopSem);
   assert (res != -1);

   delete[] mpWaveBuffers;
   mCurBuff = -1;
   mpWaveBuffers = NULL;

   res = sem_post(&mPushPopSem);
   assert (res != -1);
}

MpAudioSample* MpodOSS::popFrame(unsigned& size)
{
   MpAudioSample* ret = NULL;
   if (!isEnabled())
      return ret;

   int res = sem_wait(&mPushPopSem);
   assert (res != -1);

   do
   {
      if (mLastReceived == mCurBuff)
      {
         //No data stored in buffer
         break;
      }
      unsigned sampleSize = mSamplesPerFrame * sizeof(MpAudioSample) *
                            (OSS_SOUND_STEREO ? 2 : 1);
      ret = (MpAudioSample*)(mpWaveBuffers +
         mLastReceived * sampleSize);

      mLastReceived++;
      if (mLastReceived == mNumInBuffers)
      {
         mLastReceived = 0;
      }
      size = sampleSize;
      mQueueLen --;
      RTL_EVENT("MpodOSS::queue", mQueueLen);
   } while (FALSE);

   res = sem_post(&mPushPopSem);
   assert (res != -1);

   return ret;
}

OsStatus MpodOSS::signalForNextFrame()
{
   OsStatus ret = OS_FAILED;

   if (!isNotificationNeeded())
      return ret;

   ret = pNotificator->signal(mCurrentFrameTime);
   return ret;
}

void MpodOSS::skipFrame()
{
   mCurrentFrameTime += getFramePeriod();
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

