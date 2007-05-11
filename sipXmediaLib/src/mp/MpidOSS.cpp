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
#include "mp/MpidOSS.h"
#include "mp/MpInputDeviceManager.h"
#include "mp/MpOSSDeviceWrapper.h"
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
MpidOSS::MpidOSS(const UtlString& name,
                 MpInputDeviceManager& deviceManager,
                 unsigned nInputBuffers)
: MpInputDeviceDriver(name, deviceManager)
, mNumInBuffers(nInputBuffers)
, mpWaveBuffers(NULL)
, mCurBuff(-1)
, pDevWrapper(NULL)
{
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

MpidOSS::~MpidOSS()
{/*
   if (isDeviceValid())
   {
      pDevWrapper->freeInputDevice();
   }
*/
   if (mpCont != NULL)
   {
      MpOSSDeviceWrapperContainer::releaseContainer(mpCont);
   }
}
/* ============================ MANIPULATORS ============================== */
OsStatus MpidOSS::enableDevice(unsigned samplesPerFrame,
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

   ret = initBuffers();
   if (ret != OS_SUCCESS)
   {
      return ret;
   }

   ret = pDevWrapper->attachReader();
   if (ret != OS_SUCCESS)
   {
      return ret;
   }
   mIsEnabled = TRUE;

   return ret;
}

OsStatus MpidOSS::disableDevice()
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

   freeBuffers();
   mIsEnabled = FALSE;

   pDevWrapper->freeInputDevice();

   return ret;
}
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
OsStatus MpidOSS::initBuffers()
{
   int total_size = mNumInBuffers * mSamplesPerFrame * sizeof(MpAudioSample) *
                    (OSS_SOUND_STEREO ? 2 : 1);

   mpWaveBuffers = new char[total_size];
   if (mpWaveBuffers != NULL)
   {
      mCurBuff = 0;
      return OS_SUCCESS;
   }
   else
   {
      mCurBuff = -1;
   }
   return OS_FAILED;
}

void MpidOSS::freeBuffers()
{
   delete[] mpWaveBuffers;
   mCurBuff = -1;
}

MpAudioSample* MpidOSS::getBuffer()
{
   assert (mCurBuff != -1);
   MpAudioSample* ret = (MpAudioSample*)(mpWaveBuffers +
                        mCurBuff * mSamplesPerFrame * sizeof(MpAudioSample) *
                        (OSS_SOUND_STEREO ? 2 : 1));
   mCurBuff ++;
   if (mCurBuff == mNumInBuffers)
   {
      mCurBuff = 0;
   }
   return ret;
}


void MpidOSS::pushFrame(MpAudioSample* frm)
{
   RTL_BLOCK("MpidOSS::pushFrame");
   mpInputDeviceManager->pushFrame(mDeviceId,
                                   mSamplesPerFrame,
                                   frm,
                                   mCurrentFrameTime);

   mCurrentFrameTime += getFramePeriod();
}

void MpidOSS::skipFrame()
{
   mCurrentFrameTime += getFramePeriod();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

