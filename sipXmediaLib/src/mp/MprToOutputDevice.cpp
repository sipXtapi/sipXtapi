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
   }
   else
   {
      mFrameTime += frameTimeInterval;
   }

   // Do processing only if data is really present.
   if (inBufs[0].isValid())
   {
      status = mpOutputDeviceManager->pushFrame(mDeviceId, mFrameTime, inBufs[0]);

      if (status == OS_LIMIT_REACHED)
      {
         // TODO:: Put code to workaround mixer buffer overflow. 
      }
      else if (status == OS_INVALID_STATE)
      {
         // TODO:: Put code to workaround mixer buffer underflow.
      }
   }

   return (status == OS_SUCCESS);
}

/* ============================ FUNCTIONS ================================= */

