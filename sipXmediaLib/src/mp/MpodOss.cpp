//
// Copyright (C) 2007-2017 SIPez LLC. All rights reserved
//
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

// APPLICATION INCLUDES
#include <mp/MpodOss.h>
#include <mp/MpOutputDeviceManager.h>
#include <os/OsTask.h>
#include <os/OsNotification.h>
#include <os/OsCallback.h>
#include <os/OsFS.h>

#ifdef RTL_ENABLED // [
#  include <rtl_macro.h>
#else  // RTL_ENABLED ][
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x, y)
#endif // RTL_ENABLED ]

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Default constructor
MpodOss::MpodOss(const UtlString& name)
: MpOutputDeviceDriver(name)
, mAudioFrame(NULL)
, mNotificationThreadEn(FALSE)
{
   mpCont = MpOssContainer::getContainer();
   if (mpCont != NULL)
   {
      pDevWrapper = mpCont->getOSSDeviceWrapper(name);
   }
   else
   {
      pDevWrapper = NULL;
   }
}

MpodOss::~MpodOss()
{
   // OSS Device must be freed
   assert (!isDeviceValid());

   if (mpCont != NULL)
   {
      MpOssContainer::releaseContainer(mpCont);
   }
}
/* ============================ MANIPULATORS ============================== */
OsStatus MpodOss::setNotificationMode(UtlBoolean bThreadNotification)
{
   if (isEnabled())
   {
      return OS_INVALID_STATE;
   }

   mNotificationThreadEn = bThreadNotification;
   return OS_SUCCESS;
}

OsStatus MpodOss::enableDevice(unsigned samplesPerFrame,
                               unsigned samplesPerSec,
                               MpFrameTime currentFrameTime,
                               OsCallback &frameTicker)
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
   mpTickerNotification = &frameTicker;

   // Get buffer for audio data and fill it with silence
   mAudioFrame = new MpAudioSample[samplesPerFrame];
   if (mAudioFrame == NULL)
   {
      return OS_LIMIT_REACHED;
   }
   memset(mAudioFrame, 0, samplesPerFrame * sizeof(MpAudioSample));

   ret = pDevWrapper->attachWriter();
   if (ret != OS_SUCCESS)
   {
      return ret;
   }
   mIsEnabled = TRUE;

   return ret;
}

OsStatus MpodOss::disableDevice()
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
   delete[] mAudioFrame;
   mIsEnabled = FALSE;

   pDevWrapper->freeOutputDevice();

   return ret;
}

OsStatus MpodOss::pushFrame(unsigned int numSamples,
                            const MpAudioSample* samples,
                            MpFrameTime frameTime)
{
   if (!isEnabled())
      return OS_FAILED;

   // Currently only full frame supported
   if(numSamples != mSamplesPerFrame)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
         "MpodOss::pushFrame (%p) given %d samples, expected full frame of: %d samples",
         this, numSamples, (int)mSamplesPerFrame);
      OsSysLog::flush();
      assert(numSamples == mSamplesPerFrame);
   }

   RTL_BLOCK("MpodOss::pushFrame");
   if (samples != NULL)
   {
      memcpy(mAudioFrame, samples, numSamples * sizeof(MpAudioSample));
   }

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

OsStatus MpodOss::canEnable()
{
    OsStatus status = OS_FAILED;

    OsFile deviceFile(getDeviceName());
    if(deviceFile.exists())
    {
        status = deviceFile.open();
        deviceFile.close();
    }
    else
    {
        status = OS_NOT_FOUND;
    }

    return(status);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsStatus MpodOss::signalForNextFrame()
{
   OsStatus ret = OS_FAILED;

   ret = mpTickerNotification->signal(mCurrentFrameTime);
   return ret;
}

void MpodOss::skipFrame()
{
   mCurrentFrameTime += getFramePeriod();
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

