//
// Copyright (C) 2007-2017 SIPez LLC. All rights reserved
//
// $$
///////////////////////////////////////////////////////////////////////////////


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
#include "mp/MpodAlsa.h"
#include "mp/MpOutputDeviceManager.h"
#include "os/OsTask.h"
#include "os/OsNotification.h"
#include "os/OsCallback.h"

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
char MpodAlsa::spDefaultDeviceName[MAX_DEVICE_NAME_SIZE];

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Default constructor
MpodAlsa::MpodAlsa(const UtlString& name)
: MpOutputDeviceDriver(name)
, mAudioFrame(NULL)
, mNotificationThreadEn(FALSE)
{
   mpCont = MpAlsaContainer::getContainer();
   if (mpCont != NULL)
   {
      pDevWrapper = mpCont->getALSADeviceWrapper(name);
   }
   else
   {
      pDevWrapper = NULL;
   }
}

MpodAlsa::~MpodAlsa()
{
   // ALSA Device must be freed
   assert (!isDeviceValid());

   if (mpCont != NULL)
   {
      MpAlsaContainer::releaseContainer(mpCont);
   }
}
/* ============================ MANIPULATORS ============================== */
OsStatus MpodAlsa::setNotificationMode(UtlBoolean bThreadNotification)
{
   if (isEnabled())
   {
      return OS_INVALID_STATE;
   }

   mNotificationThreadEn = bThreadNotification;
   return OS_SUCCESS;
}

OsStatus MpodAlsa::enableDevice(unsigned samplesPerFrame,
                               unsigned samplesPerSec,
                               MpFrameTime currentFrameTime,
                               OsCallback &frameTicker)
{
   OsStatus ret;
   if (isEnabled())
   {
       return OS_FAILED;
   }

   //Opening ALSA device
   if (pDevWrapper)
   {
       mSamplesPerFrame = samplesPerFrame;
       mSamplesPerSec = samplesPerSec;
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

OsStatus MpodAlsa::disableDevice()
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

OsStatus MpodAlsa::pushFrame(unsigned int numSamples,
                            const MpAudioSample* samples,
                            MpFrameTime frameTime)
{
   if (!isEnabled())
      return OS_FAILED;

   // Currently only full frame supported
   if(numSamples != mSamplesPerFrame)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
         "MpodAlsa::pushFrame given %d samples, expected full frame of: %d samples",
         numSamples, (int)mSamplesPerFrame);
      OsSysLog::flush();
      assert(numSamples == mSamplesPerFrame);
   }

   RTL_BLOCK("MpodAlsa::pushFrame");
   if (samples != NULL)
   {
      memcpy(mAudioFrame, samples, numSamples * sizeof(MpAudioSample));
   }

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

const char* MpodAlsa::getDefaultDeviceName()
{
    UtlSList deviceNames;

    // Get the list of available output devices
    getDeviceNames(deviceNames);

    // The first one is the default
    UtlString* firstDevice = (UtlString*) deviceNames.get();
    strncpy(spDefaultDeviceName, 
            firstDevice ? firstDevice->data() : "",
            MAX_DEVICE_NAME_SIZE);
    deviceNames.destroyAll();

    return(spDefaultDeviceName);
}

int MpodAlsa::getDeviceNames(UtlContainer& deviceNames)
{
    int deviceCount = 
        MpAlsa::getDeviceNames(deviceNames, 
                               false);  // output device names

    return(deviceCount);
}

/* ============================ INQUIRY =================================== */

OsStatus MpodAlsa::canEnable()
{
    OsStatus status = OS_FAILED;
    snd_pcm_t* pcmOut = NULL;

    int openStatus = 
        snd_pcm_open(&pcmOut, 
                     getDeviceName(),
		     SND_PCM_STREAM_PLAYBACK,
		     SND_PCM_NONBLOCK);
    switch(openStatus)
    {
    case 0:
        status = OS_SUCCESS;
        snd_pcm_close(pcmOut);
        break;

    case -2:
        status = OS_NOT_FOUND;
        break;

    case 16:
        status = OS_BUSY;
        break;

    default:
        status = OS_FAILED;
        break;
    }
    if(openStatus != 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "MposAlsa::canEnable snd_pcm_open(%p, \"%s\", %s) returned: %d %s",
                      pcmOut,
                      getDeviceName().data(),
                      "SND_PCM_STREAM_PLAYBACK",
                      openStatus,
                      snd_strerror(openStatus));
    }

    return(status);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsStatus MpodAlsa::signalForNextFrame()
{
   OsStatus ret = OS_FAILED;
   ret = mpTickerNotification->signal(mCurrentFrameTime);
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpodAlsa::signalForNextFrame %s",
                 data());
#endif
   return ret;
}

void MpodAlsa::skipFrame()
{
   mCurrentFrameTime += getFramePeriod();
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

