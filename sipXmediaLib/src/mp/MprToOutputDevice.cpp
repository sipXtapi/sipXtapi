//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpOutputDeviceManager.h>
#include <mp/MprToOutputDevice.h>

#ifdef RTL_ENABLED
#include <rtl_macro.h>
#else
#define RTL_BLOCK(x)
#define RTL_EVENT(x,y)
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprToOutputDevice::MprToOutputDevice(const UtlString& rName, 
                                     int samplesPerFrame, 
                                     int samplesPerSec,
                                     MpOutputDeviceManager* deviceManager,
                                     MpOutputDeviceHandle deviceId)
: MpAudioResource(rName,
                  1, 1, /* inputs */
                  0, 0, /* outputs */
                  samplesPerFrame, samplesPerSec)
, mpOutputDeviceManager(deviceManager)
, mFrameTimeInitialized(FALSE)
, mFrameTime(0)
, mDeviceId(deviceId)
{
}

// Destructor
MprToOutputDevice::~MprToOutputDevice()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprToOutputDevice::doProcessFrame(MpBufPtr inBufs[],
                                             MpBufPtr outBufs[],
                                             int inBufsSize,
                                             int outBufsSize,
                                             UtlBoolean isEnabled,
                                             int samplesPerFrame,
                                             int samplesPerSecond)
{
   int frameTimeInterval;
   OsStatus status = OS_SUCCESS;

   if (!isEnabled)
   {
      return TRUE;
   }

   RTL_BLOCK("MprToOutputDevice::doProcessFrame");

   assert(mpOutputDeviceManager != NULL);

   // MprToOutputDevice need one input.
   if (inBufsSize < 1)
   {
      return FALSE;
   }

   // Milliseconds per frame:
   frameTimeInterval = samplesPerFrame * 1000 / samplesPerSecond;

   // If current frame time is not initialized, suggest putting this frame
   // in the middle of mixer buffer. In direct-write mode mixer buffer
   // length is zero, so current frame time will be used without additional
   // work.
   // If frame time initialized already - advance one step.
   if (!mFrameTimeInitialized)
   {
      MpFrameTime mixerBufferLength;
      if (mpOutputDeviceManager->getMixerBufferLength(mDeviceId, mixerBufferLength) != OS_SUCCESS)
      {
         return FALSE;
      }

      mFrameTime = mpOutputDeviceManager->getCurrentFrameTime();
      mFrameTime += mixerBufferLength / 2;
      
      mFrameTimeInitialized = TRUE;
   }
   else
   {
      mFrameTime += frameTimeInterval;
   }

   // Push buffer to output device even if buffer is NULL. WIth NULL buffer we
   // notify output device that we will not push more frames this time interval.
   status = mpOutputDeviceManager->pushFrame(mDeviceId, mFrameTime, inBufs[0]);
   osPrintf("MprToOutputDevice::doProcessFrame(): frameToPush=%d, pushResult=%d %s\n",
            mFrameTime, status, inBufs[0].isValid()?"":"[NULL BUFFER]");

   // Do processing only if data is really present.
   if (inBufs[0].isValid())
   {
      // If push frame fail and we're in mixer mode, advance our current frame
      // time to fit into mixer buffer.
      MpFrameTime mixerBufferLength;
      if (  (status != OS_SUCCESS)
         && mpOutputDeviceManager->getMixerBufferLength(mDeviceId, mixerBufferLength) == OS_SUCCESS
         && (mixerBufferLength > 0) )
      {
         // TODO:: This should be changed, to be done only once on startup,
         // because it may cause several consecutive frames to be written
         // with one timestamp and so, mixed together.
         while (status == OS_LIMIT_REACHED)
         {
            RTL_EVENT("MprToOutputDevice::overflow",1);
            mFrameTime -= frameTimeInterval;
            status = mpOutputDeviceManager->pushFrame(mDeviceId, mFrameTime, inBufs[0]);
            osPrintf("MprToOutputDevice::doProcessFrame(): frameToPush=%d, pushResult=%d ---\n", mFrameTime, status);
         }
         while (status == OS_INVALID_STATE)
         {
            RTL_EVENT("MprToOutputDevice::overflow",-1);
            mFrameTime += frameTimeInterval;
            status = mpOutputDeviceManager->pushFrame(mDeviceId, mFrameTime, inBufs[0]);
            osPrintf("MprToOutputDevice::doProcessFrame(): frameToPush=%d, pushResult=%d +++\n", mFrameTime, status);
         }
         RTL_EVENT("MprToOutputDevice::overflow",0);
      }
   }

   return (status == OS_SUCCESS);
}

/* ============================ FUNCTIONS ================================= */

