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
// DEFINES
#define DEBUG_PRINT
#undef  DEBUG_PRINT

// MACROS
#ifdef DEBUG_PRINT // [
#  define debugPrintf    printf
#else  // DEBUG_PRINT ][
static void debugPrintf(...) {}
#endif // DEBUG_PRINT ]


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromInputDevice::MprFromInputDevice(const UtlString& rName, 
                                      int samplesPerFrame, 
                                      int samplesPerSec,
                                      MpInputDeviceManager* deviceManager,
                                      MpInputDeviceHandle deviceId)
: MpAudioResource(rName,
                  0, 0, /* inputs */
                  1, 1, /* outputs */
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
   RTL_BLOCK("MprFromInputDevice::doProcessFrame");

   assert(mpInputDeviceManager);

   if (!isEnabled)
   {
      return TRUE;
   }

   // Milliseconds per frame:
   int frameTimeInterval = samplesPerFrame * 1000 / samplesPerSecond;

   if (!mFrameTimeInitialized)
   {
      // Start with a frame behind.  Possible need smarter
      // decision for starting.
      mPreviousFrameTime = mpInputDeviceManager->getCurrentFrameTime(mDeviceId);
//      mPreviousFrameTime -= (5 * frameTimeInterval);
   }

   mPreviousFrameTime += frameTimeInterval;

   MpBufPtr buffer;
   unsigned int numFramesNotPlayed;
   unsigned int numFramedBufferedBehind;
   MpFrameTime  frameToFetch;

   // Use temp variable for frame time to prevent frame time drift in case
   // of device hiccup.
   frameToFetch = mPreviousFrameTime;

   OsStatus getResult = mpInputDeviceManager->getFrame(mDeviceId,
                                                       frameToFetch,
                                                       buffer,
                                                       numFramesNotPlayed,
                                                       numFramedBufferedBehind);


   debugPrintf("MprFromInputDevice::doProcessFrame()"
               " frameToFetch=%d, getResult=%d, numFramesNotPlayed=%d, numFramedBufferedBehind=%d\n",
               frameToFetch, getResult, numFramesNotPlayed, numFramedBufferedBehind);

   if(getResult != OS_SUCCESS)
   {
      int numAdvances = 0;
      while (getResult == OS_NOT_FOUND && numFramedBufferedBehind == 0 && numFramesNotPlayed > 0)
      {
         debugPrintf("+ %d numFramesNotPlayed=%d, numFramedBufferedBehind=%d\n",
                     mPreviousFrameTime, numFramesNotPlayed, numFramedBufferedBehind);
         mPreviousFrameTime += frameTimeInterval;
         frameToFetch = mPreviousFrameTime;
         getResult = 
            mpInputDeviceManager->getFrame(mDeviceId,
                                           frameToFetch,
                                           buffer,
                                           numFramesNotPlayed,
                                           numFramedBufferedBehind);
         numAdvances++;
         RTL_EVENT("MprFromInputDevice::advance", numAdvances);
      }
      numAdvances = 0;
      RTL_EVENT("MprFromInputDevice::advance", numAdvances);
      while (getResult == OS_NOT_FOUND && numFramedBufferedBehind > 0 && numFramesNotPlayed == 0)
      {
         debugPrintf("- %d numFramesNotPlayed=%d, numFramedBufferedBehind=%d\n",
                     mPreviousFrameTime, numFramesNotPlayed, numFramedBufferedBehind);
         mPreviousFrameTime -= frameTimeInterval;
         frameToFetch = mPreviousFrameTime;
         getResult = 
            mpInputDeviceManager->getFrame(mDeviceId,
                                           frameToFetch,
                                           buffer,
                                           numFramesNotPlayed,
                                           numFramedBufferedBehind);
         numAdvances--;
         RTL_EVENT("MprFromInputDevice::advance", numAdvances);
      }
      numAdvances = 0;
      RTL_EVENT("MprFromInputDevice::advance", numAdvances);
   }

   if (!mFrameTimeInitialized)
   {
      if (getResult == OS_SUCCESS)
      {
         mPreviousFrameTime = frameToFetch;
         mFrameTimeInitialized = TRUE;
      }
   }

   outBufs[0] = buffer;

   return getResult;
}

/* ============================ FUNCTIONS ================================= */

