//
// Copyright (C) 2007-2018 SIPez LLC. All rights reserved.
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
#include <utl/UtlSListIterator.h>
#include <mp/MpidAlsa.h>
#include <mp/MpInputDeviceManager.h>

#ifdef RTL_ENABLED // [
#  include "rtl_macro.h"
#else  // RTL_ENABLED ][
#  define RTL_WRITE(x)
#  define RTL_BLOCK(x)
#  define RTL_START(x)
#endif // RTL_ENABLED ]

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
char MpidAlsa::spDefaultDeviceName[MAX_DEVICE_NAME_SIZE];

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Default constructor
MpidAlsa::MpidAlsa(const UtlString& name,
                 MpInputDeviceManager& deviceManager)
: MpInputDeviceDriver(name, deviceManager)
, mAudioFrame(NULL)
, pDevWrapper(NULL)
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

MpidAlsa::~MpidAlsa()
{
   if (mpCont != NULL)
   {
      MpAlsaContainer::releaseContainer(mpCont);
   }
}
/* ============================ MANIPULATORS ============================== */
OsStatus MpidAlsa::enableDevice(unsigned samplesPerFrame,
                               unsigned samplesPerSec,
                               MpFrameTime currentFrameTime)
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpidAlsa::enableDevice start");
#endif
   OsStatus ret;
   if (isEnabled())
   {
      return OS_FAILED;
   }

   if (pDevWrapper)
   {
      mSamplesPerFrame = samplesPerFrame;
      mSamplesPerSec = samplesPerSec;
      OsStatus res = pDevWrapper->setInputDevice(this);
      if (res != OS_SUCCESS) {
         pDevWrapper = NULL;
      }
      else if (!pDevWrapper->mbReadCap)
      {
         //Device dosen't support input
         pDevWrapper->freeInputDevice();
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

   // Get buffer and fill it with silence
   mAudioFrame = new MpAudioSample[samplesPerFrame];
   if (mAudioFrame == NULL)
   {
      return OS_LIMIT_REACHED;
   }
   memset(mAudioFrame, 0, samplesPerFrame * sizeof(MpAudioSample));

   ret = pDevWrapper->attachReader();
   if (ret != OS_SUCCESS)
   {
      return ret;
   }
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpidAlsa::enableDevice set");
#endif
   mIsEnabled = TRUE;

   return ret;
}

OsStatus MpidAlsa::disableDevice()
{
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpidAlsa::disableDevice start");
#endif
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

   ret = pDevWrapper->detachReader();
   if (ret != OS_SUCCESS)
   {
      return ret;
   }

   delete[] mAudioFrame;
#ifdef TEST_PRINT
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MpidAlsa::disableDevice set");
#endif
   mIsEnabled = FALSE;

   pDevWrapper->freeInputDevice();

   return ret;
}
/* ============================ ACCESSORS ================================= */

const char* MpidAlsa::getDefaultDeviceName()
{
    UtlSList deviceNames;

    // Get the list of available input devices
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

int MpidAlsa::getDeviceNames(UtlContainer& deviceNames)
{
    int deviceCount = 
        MpAlsa::getDeviceNames(deviceNames, 
                               true);  // input device names

    return(deviceCount);
}

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
void MpidAlsa::pushFrame()
{
   RTL_BLOCK("MpidAlsa::pushFrame");
   mpInputDeviceManager->pushFrame(mDeviceId,
                                   mSamplesPerFrame,
                                   mAudioFrame,
                                   mCurrentFrameTime);

   mCurrentFrameTime += getFramePeriod();
}

void MpidAlsa::skipFrame()
{
   mCurrentFrameTime += getFramePeriod();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

