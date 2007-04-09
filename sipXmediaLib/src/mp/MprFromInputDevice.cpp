//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpInputDeviceManager.h>
#include <mp/MprFromInputDevice.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromInputDevice::MprFromInputDevice(const UtlString& rName, 
                                      int samplesPerFrame, 
                                      int samplesPerSec,
                                      MpInputDeviceManager* deviceManager,
                                      MpInputDeviceHandle deviceId)
: MpAudioResource(rName,
                  1, 1, /* inputs */
                  0, 0, /* outputs */
                  samplesPerFrame, samplesPerSec)
, mpInputDeviceManager(deviceManager)
, mFrameTimeInitialized(FALSE)
, mPreviousFrameTime(0)
, mDeviceId(deviceId)
{
}

// Destructor
MprFromInputDevice::~MprFromInputDevice()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprFromInputDevice::doProcessFrame(MpBufPtr inBufs[],
                                             MpBufPtr outBufs[],
                                             int inBufsSize,
                                             int outBufsSize,
                                             UtlBoolean isEnabled,
                                             int samplesPerFrame,
                                             int samplesPerSecond)
{
   // NOTE: Logic to react to frequent starvation is missing.

   assert(mpInputDeviceManager);

   // Milliseconds per frame:
   int frameTimeInterval = samplesPerFrame * 1000 / samplesPerSecond;

   if (!mFrameTimeInitialized)
   {
      // Start with a frame behind.  Possible need smarter
      // decision for starting.
      mPreviousFrameTime = mpInputDeviceManager->getCurrentFrameTime();
      mPreviousFrameTime -= (2 * frameTimeInterval);
   }

   mPreviousFrameTime += frameTimeInterval;

   MpBufPtr buffer;
   unsigned int numFramesNotPlayed;
   unsigned int numFramedBufferedBehind;
   OsStatus getResult = mpInputDeviceManager->getFrame(mDeviceId,
                                                       mPreviousFrameTime,
                                                       buffer,
                                                       numFramesNotPlayed,
                                                       numFramedBufferedBehind);


   if (!mFrameTimeInitialized)
   {
      if (getResult == OS_SUCCESS)
      {
         mFrameTimeInitialized = TRUE;
      }

      if (numFramesNotPlayed > 1)
      {
         // TODO: now is a good time to adjust and get a newer
         // frame
         // could increment mPreviousFrameTime and getFrame again
      }
   }

   outBufs[0] = buffer;

   return getResult;
}

/* ============================ FUNCTIONS ================================= */

