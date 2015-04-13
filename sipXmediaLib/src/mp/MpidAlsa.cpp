//
// Copyright (C) 2007-2015 SIPez LLC. All rights reserved.
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
#include "mp/MpidAlsa.h"
#include "mp/MpInputDeviceManager.h"
#include "os/OsTask.h"

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
   OsStatus ret;
   if (isEnabled())
   {
      return OS_FAILED;
   }

   if (pDevWrapper)
   {
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
   mSamplesPerFrame = samplesPerFrame;
   mSamplesPerSec = samplesPerSec;
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
   mIsEnabled = TRUE;

   return ret;
}

OsStatus MpidAlsa::disableDevice()
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

   ret = pDevWrapper->detachReader();
   if (ret != OS_SUCCESS)
   {
      return ret;
   }

   delete[] mAudioFrame;
   mIsEnabled = FALSE;

   pDevWrapper->freeInputDevice();

   return ret;
}
/* ============================ ACCESSORS ================================= */
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

