//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsIntTypes.h"
#include <os/OsSysLog.h>
#include <mp/MpOutputDeviceManager.h>
#include <mp/MprToOutputDevice.h>
#include <mp/MpFlowGraphBase.h>
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
MprToOutputDevice::MprToOutputDevice(const UtlString& rName, 
                                     MpOutputDeviceManager* deviceManager,
                                     MpOutputDeviceHandle deviceId)
: MpAudioResource(rName,
                  1, 1, /* inputs */
                  0, 0  /* outputs */)
, mpOutputDeviceManager(deviceManager)
, mFrameTimeInitialized(FALSE)
, mFrameTime(0)
, mDeviceId(deviceId)
, mpResampler(MpResamplerBase::createResampler(1, 8000, 8000))
{
}

// Destructor
MprToOutputDevice::~MprToOutputDevice()
{
   delete mpResampler;
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
   // Create a bufPtr that points to the data we want to push to the device.
   MpAudioBufPtr inAudioBuffer = inBufs[0];

   if (!isEnabled)
   {
      return TRUE;
   }

   assert(mpOutputDeviceManager != NULL);

   // MprToOutputDevice need one input.
   if (inBufsSize < 1)
   {
      return FALSE;
   }

   // Milliseconds per frame:
   frameTimeInterval = samplesPerFrame * 1000 / samplesPerSecond;

   // Get the name of the device we're writing to.
   UtlString devName = "Unknown device";
   mpOutputDeviceManager->getDeviceName(mDeviceId, devName);

   uint32_t devSampleRate = 0;
   OsStatus stat = mpOutputDeviceManager->getDeviceSamplesPerSec(mDeviceId, devSampleRate);
   assert(stat == OS_SUCCESS);
   if(stat != OS_SUCCESS)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MprToOutputDevice::doProcessFrame "
         "- Couldn't get device sample rate from output device manager!  "
         "Device - \"%s\"", devName.data());
      return FALSE;
   }

   // Check to see if the resampler needs it's rate adjusted.
   if(mpResampler->getInputRate() != samplesPerSecond)
      mpResampler->setInputRate(samplesPerSecond);
   if(mpResampler->getOutputRate() != devSampleRate)
      mpResampler->setOutputRate(devSampleRate);

   {
      MpAudioBufPtr resampledBuffer;
      // Try to resample and replace.
      // If the function determines resampling is unnecessary, then it will just
      // leave the buffer pointer unchanged, and return OS_SUCCESS, which is what
      // we want.
      if(mpResampler->resampleBufPtr(inAudioBuffer, resampledBuffer, 
            samplesPerSecond, devSampleRate, devName) != OS_SUCCESS)
      {
         // Error messages have already been logged. No need to do so here.
         return FALSE;
      }

      // If the resampled buffer is valid, then use it.
      if(resampledBuffer.isValid())
      {
         // To optimize for speed a bit, we use MpBufPtr's swap() method 
         // instead of assignment -- make sure we don't use resampledBuffer
         // after this!
         inAudioBuffer.swap(resampledBuffer);
      }
   }

   // Push buffer to output device even if buffer is NULL. With NULL buffer we
   // notify output device that we will not push more frames during this time
   // interval.
   status = mpOutputDeviceManager->pushFrame(mDeviceId, mFrameTime, inAudioBuffer,
                                             !mFrameTimeInitialized);

   RTL_EVENT(mpFlowGraph->getFlowgraphName()+getName()+"_pushFrame_result", status);
   debugPrintf("MprToOutputDevice::doProcessFrame(): frameToPush=%d, pushResult=%d %s\n",
               mFrameTime, status, inAudioBuffer.isValid()?"":"[NULL BUFFER]");

   if (!mFrameTimeInitialized)
   {
      if (status == OS_SUCCESS)
      {
         // Frame time should be initialized at this point.
         mFrameTimeInitialized = TRUE;
         // Advance frame time by one step.
         mFrameTime += frameTimeInterval;
      }
   }
   else
   {
      // Advance frame time by one step.
      mFrameTime += frameTimeInterval;
   }

   return (status == OS_SUCCESS);
}

/* ============================ FUNCTIONS ================================= */

