//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MprFromInputDevice.h>
#include <mp/MpMisc.h>
#include <mp/MpResampler.h>

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
                                      MpInputDeviceManager* deviceManager,
                                      MpInputDeviceHandle deviceId)
: MpAudioResource(rName,
                  0, 0, /* inputs */
                  1, 1  /* outputs */)
, mpInputDeviceManager(deviceManager)
, mFrameTimeInitialized(FALSE)
, mPreviousFrameTime(0)
, mDeviceId(deviceId)
, mResampler(1, 8000, 8000) // 8000 is just *some* initial value, it is expected
                            // that the real value will be something different
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

   MpAudioBufPtr inAudioBuffer;
   unsigned int numFramesNotPlayed;
   unsigned int numFramedBufferedBehind;
   MpFrameTime  frameToFetch;

   // Use temp variable for frame time to prevent frame time drift in case
   // of device hiccup.
   frameToFetch = mPreviousFrameTime;

   OsStatus getResult = mpInputDeviceManager->getFrame(mDeviceId,
                                                       frameToFetch,
                                                       inAudioBuffer,
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
                                           inAudioBuffer,
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
                                           inAudioBuffer,
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

   // Get the name of the device we're writing to.
   UtlString devName = "Unknown device";
   mpInputDeviceManager->getDeviceName(mDeviceId, devName);

   uint32_t devSampleRate = 0;
   OsStatus stat = mpInputDeviceManager->getDeviceSamplesPerSec(mDeviceId, devSampleRate);
   assert(stat == OS_SUCCESS);
   if(stat != OS_SUCCESS)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MprFromInputDevice::doProcessFrame "
         "- Couldn't get device sample rate from input device manager!  "
         "Device - \"%s\"", devName);
      return FALSE;
   }

   // Check to see if the resampler needs it's rate adjusted.
   if(mResampler.getInputRate() != devSampleRate)
      mResampler.setInputRate(devSampleRate);
   if(mResampler.getOutputRate() != samplesPerSecond)
      mResampler.setOutputRate(samplesPerSecond);

   MpAudioBufPtr resampledBuffer;
   // Try to resample and replace.
   // If the function determines resampling is unnecessary, then it will just
   // leave the buffer pointer unchanged, and return OS_SUCCESS, which is what
   // we want.
   if(mResampler.resampleBufPtr(inAudioBuffer, resampledBuffer,
         devSampleRate, samplesPerSecond, devName) != OS_SUCCESS)
   {
      // Error messages have already been logged. No need to do so here.
      return FALSE;
   }

   // Set the output buffer ptr to the device's audio (either resampled or not
   // if it was not needed)
   // To optimize for speed a bit, we use MpBufPtr's swap() method instead of 
   // assignment -- make sure we don't use resampledBuffer or inAudioBuffer
   // after this!
   outBufs[0].swap((resampledBuffer.isValid()) ? resampledBuffer : inAudioBuffer);

   return getResult;
}

/* ============================ FUNCTIONS ================================= */

