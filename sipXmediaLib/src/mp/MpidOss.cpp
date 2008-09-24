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

// APPLICATION INCLUDES
#include "mp/MpidOss.h"
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
MpidOss::MpidOss(const UtlString& name,
                 MpInputDeviceManager& deviceManager)
: MpInputDeviceDriver(name, deviceManager)
, mAudioFrame(NULL)
, pDevWrapper(NULL)
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

MpidOss::~MpidOss()
{
   if (mpCont != NULL)
   {
      MpOssContainer::releaseContainer(mpCont);
   }
}
/* ============================ MANIPULATORS ============================== */
OsStatus MpidOss::enableDevice(unsigned samplesPerFrame,
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

   mAudioFrame = new MpAudioSample[samplesPerFrame];
   if (mAudioFrame == NULL)
   {
      return OS_LIMIT_REACHED;
   }

   ret = pDevWrapper->attachReader();
   if (ret != OS_SUCCESS)
   {
      return ret;
   }
   mIsEnabled = TRUE;

   return ret;
}

OsStatus MpidOss::disableDevice()
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
void MpidOss::pushFrame()
{
   RTL_BLOCK("MpidOss::pushFrame");
   mpInputDeviceManager->pushFrame(mDeviceId,
                                   mSamplesPerFrame,
                                   mAudioFrame,
                                   mCurrentFrameTime);

   mCurrentFrameTime += getFramePeriod();
}

void MpidOss::skipFrame()
{
   mCurrentFrameTime += getFramePeriod();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

