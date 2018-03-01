//
// Copyright (C) 2007-2018 SIPez LLC. All rights reserved
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
#include <os/OsTask.h>
#include <os/OsNotification.h>
#include <os/OsCallback.h>
#include <utl/UtlSListIterator.h>
#include <mp/MpodAlsa.h>
#include <mp/MpOutputDeviceManager.h>

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
, mNullTickers(0)
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
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpodAlsa::enableDevice start");
#endif
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
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpodAlsa::enableDevice set");
#endif
   mIsEnabled = TRUE;

   return ret;
}

OsStatus MpodAlsa::disableDevice()
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpodAlsa::disableDevice start");
#endif
   OsStatus ret = OS_SUCCESS;
   if (!isEnabled())
   {
#ifdef TEST_PRINT
       OsSysLog::add(FAC_MP, PRI_DEBUG,
                     "MpodAlsa::disableDevice not enabled");
#endif
       return OS_FAILED;
   }

   // If the device is not valid, let the user know it's bad.
   if (!isDeviceValid())
   {
#ifdef TEST_PRINT
       OsSysLog::add(FAC_MP, PRI_DEBUG,
                     "MpodAlsa::disableDevice not valid");
#endif
      ret = OS_INVALID_STATE;
   }

   if(ret != OS_SUCCESS)
   {
       // Attempt the detach despite the prior error, but
       // don't hide the prior error.
       pDevWrapper->detachWriter();
#ifdef TEST_PRINT
       OsSysLog::add(FAC_MP, PRI_DEBUG,
                     "MpodAlsa::disableDevice detach no ret check");
#endif
   }
   else
   { 
       ret = pDevWrapper->detachWriter();
       if (ret != OS_SUCCESS)
       {
#ifdef TEST_PRINT
           OsSysLog::add(FAC_MP, PRI_DEBUG,
                         "MpodAlsa::disableDevice detach ret: %d",
                         ret);
#endif
       }
   }
   delete[] mAudioFrame;
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpodAlsa::disableDevice set");
#endif
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

    // The first one is the default if no plughw* device names are found
    UtlString* defaultDevice = (UtlString*) deviceNames.get();
    UtlSListIterator nameIterator(deviceNames);
    UtlString* deviceName = NULL;
    while((deviceName = (UtlString*) nameIterator()))
    {
        if(deviceName->index("plughw") == 0)
        {
            defaultDevice = deviceName;
            // The first device name that begins with plughw is the default
            break;
        }
    }

    strncpy(spDefaultDeviceName, 
            defaultDevice ? defaultDevice->data() : "",
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
    int ret;

    int openStatus = 
        snd_pcm_open(&pcmOut, 
                     getDeviceName(),
		     SND_PCM_STREAM_PLAYBACK,
		     SND_PCM_NONBLOCK);
    OsSysLog::add(FAC_MP, PRI_DEBUG, "ALSA::canEnable snd_pcm_open(%s) returned: %p %d", getDeviceName(), pcmOut, ret);
    switch(openStatus)
    {
    case 0:
        status = OS_SUCCESS;
        ret = snd_pcm_close(pcmOut);
        OsSysLog::add(FAC_MP, PRI_DEBUG, "ALSA::canEnable snd_pcm_close(pPcmOut=%p) returned: %d", pcmOut, ret);
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
   OsCallback *ticker = mpTickerNotification;

   if (NULL != ticker) {
      ret = ticker->signal(mCurrentFrameTime);
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MpodAlsa::signalForNextFrame %s", data());
      if (mNullTickers) {
         OsSysLog::add(FAC_MP, PRI_WARNING, "MpodAlsa::signalForNextFrame: called with NULL %d times", mNullTickers);
         mNullTickers = 0;
      }
   } else {
      if (0 == mNullTickers) OsSysLog::add(FAC_MP, PRI_WARNING, "MpodAlsa::signalForNextFrame: called with NULL (first time this series)");
      mNullTickers++;
   }
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

