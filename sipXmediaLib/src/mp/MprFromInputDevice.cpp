//  
// Copyright (C) 2007-2014 SIPez LLC.  All rights reserved.
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
#include "os/OsIntTypes.h"
#include <os/OsSysLog.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MprFromInputDevice.h>
#include <mp/MpMisc.h>
#include <mp/MpResampler.h>
#include <mp/MpIntResourceMsg.h>

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
#  ifdef ANDROID
#  include <mp/MpidAndroid.h> // pulls in Android LOG prototypes
#  define debugPrintf    LOGV
#  else
#  define debugPrintf    printf
#  endif
#else  // DEBUG_PRINT ][
static void debugPrintf(...) {}
#endif // DEBUG_PRINT ]

//#define ENABLE_FILE_LOGGING
#ifdef ENABLE_FILE_LOGGING
static FILE *sgInRawFile=NULL;
static FILE *sgInResampleFile=NULL;
class OutFileInit
{
public:
   OutFileInit()
   {
      sgInRawFile = fopen("/sdcard/fg_in_raw.raw", "w");
      sgInResampleFile = fopen("/sdcard/fg_in_resample.raw", "w");
//      LOGI("Openned file for audio debug: %x", sgOutFile);
   }

   ~OutFileInit()
   {
      fclose(sgInRawFile);
      fclose(sgInResampleFile);
   }
};

static OutFileInit sgOutFileInit;
#endif // ENABLE_FILE_LOGGING

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
, mpResampler(MpResamplerBase::createResampler(1, 8000, 8000))
, mGain(MP_BRIDGE_GAIN_PASSTHROUGH)
, mpGainBuffer(NULL)
{
#ifdef DEBUG_PRINT
   UtlString devName;
   mpInputDeviceManager->getDeviceName(mDeviceId, devName);
   OsSysLog::add(FAC_MP, PRI_DEBUG, "MprFromInputDevice::MprFromInputDevice deviceId: %d device name: \"%s\"",
                 mDeviceId, devName.data());
#endif
}

// Destructor
MprFromInputDevice::~MprFromInputDevice()
{
   delete mpResampler;
   delete[] mpGainBuffer;
}

/* ============================ MANIPULATORS ============================== */


OsStatus MprFromInputDevice::setDeviceId(const UtlString& resourceName,
                                       OsMsgQ& flowgraphMessageQueue,
                                       MpOutputDeviceHandle deviceId)
{
    MpIntResourceMsg message((MpResourceMsg::MpResourceMsgType)MPRM_SET_INPUT_DEVICE_ID,
                             resourceName,
                             deviceId);
    return(flowgraphMessageQueue.send(message, sOperationQueueTimeout));
}

OsStatus MprFromInputDevice::setGain(const UtlString& namedResource, OsMsgQ& fgQ,
                                     MpBridgeGain gain)
{
   MpIntResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_SET_GAIN,
                        namedResource, gain);
   return fgQ.send(msg, sOperationQueueTimeout);
}

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

   if(mpInputDeviceManager == NULL)
   {
       OsSysLog::add(FAC_MP, PRI_ERR, "MprFromInputDevice::doProcessFrame mpInputDeviceManager NULL");
       OsSysLog::flush();
   }
   assert(mpInputDeviceManager);

   if (!isEnabled)
   {
      return TRUE;
   }

   // Milliseconds per frame:
   int frameTimeInterval = samplesPerFrame * 1000 / samplesPerSecond;

   if (!mFrameTimeInitialized)
   {
      mPreviousFrameTime = mpInputDeviceManager->getCurrentFrameTime(mDeviceId);
//      mPreviousFrameTime -= (5 * frameTimeInterval);
   }
   else
   {
      mPreviousFrameTime += frameTimeInterval;
   }

   MpAudioBufPtr inAudioBuffer;
   unsigned int numFramesNotPlayed = 0;
   unsigned int numFramedBufferedBehind = 0;
   MpFrameTime  frameToFetch = 0;

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

   if(getResult != OS_SUCCESS)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MprFromInputDevice::doProcessFrame getFrame(mDeviceId=%d, frameToFetch=%d, inAudioBuffer=xx, numFramesNotPlayed=%d, numFramedBufferedBehind=%d) returned: %d",
                    mDeviceId, frameToFetch, numFramesNotPlayed, numFramedBufferedBehind, getResult);
   }
#ifdef ENABLE_FILE_LOGGING
   else
   {
      fwrite(inAudioBuffer->getSamplesPtr(), 1, inAudioBuffer->getSamplesNumber()*sizeof(short), sgInRawFile);
   }
#endif // ENABLE_FILE_LOGGING

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
   OsStatus stat = OS_SUCCESS;
#ifdef DISABLE_LOCAL_AUDIO
   devSampleRate = samplesPerSecond;
#else
   stat = mpInputDeviceManager->getDeviceSamplesPerSec(mDeviceId, devSampleRate);
#endif
   if(stat != OS_SUCCESS)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MprFromInputDevice::doProcessFrame "
         "- Couldn't get device sample rate from input device manager!  "
         "Device - \"%s\" deviceId: %d", devName.data(), mDeviceId);
      OsSysLog::flush();
      assert(stat == OS_SUCCESS);
#ifdef ANDROID
      disable();
#endif
      return FALSE;
   }

   // If the input device is disabled, don't resample to save the CPU cycles
   UtlBoolean deviceDisabled = FALSE;
   if(!mpInputDeviceManager->isDeviceEnabled(mDeviceId))
   {
       deviceDisabled = TRUE;
       devSampleRate = samplesPerSecond;
   }

   if(mpResampler == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "MprFromInputDevice::doProcessFrame mpResampler NULL");
      OsSysLog::flush();
      assert(mpResampler);
   }
   // Check to see if the resampler needs it's rate adjusted.
   if(mpResampler->getInputRate() != devSampleRate)
      mpResampler->setInputRate(devSampleRate);
   if(mpResampler->getOutputRate() != (unsigned int)samplesPerSecond)
      mpResampler->setOutputRate(samplesPerSecond);

   MpAudioBufPtr resampledBuffer;
   // Try to resample and replace.
   // If the function determines resampling is unnecessary, then it will just
   // leave the buffer pointer unchanged, and return OS_SUCCESS, which is what
   // we want.
   if(mpResampler->resampleBufPtr(inAudioBuffer, resampledBuffer,
         devSampleRate, samplesPerSecond, devName) != OS_SUCCESS)
   {
      // Error messages have already been logged. No need to do so here.
      return FALSE;
   }

#ifdef ENABLE_FILE_LOGGING
   if (resampledBuffer.isValid())
   {
      fwrite(resampledBuffer->getSamplesPtr(), 1, resampledBuffer->getSamplesNumber()*sizeof(short), sgInResampleFile);
   }
#endif // ENABLE_FILE_LOGGING

   // Set the output buffer ptr to the device's audio (either resampled or not
   // if it was not needed)
   // To optimize for speed a bit, we use MpBufPtr's swap() method instead of 
   // assignment -- make sure we don't use resampledBuffer or inAudioBuffer
   // after this!
   outBufs[0].swap((resampledBuffer.isValid()) ? resampledBuffer : inAudioBuffer);

   if (outBufs[0].isValid() && mGain != MP_BRIDGE_GAIN_PASSTHROUGH)
   {
      // Allocate temp buffer if not yet allocated.
      if (mpGainBuffer == NULL)
      {
         mpGainBuffer = new MpBridgeAccum[samplesPerFrame];
      }
      
      // We're going to modify buffer inline.
      outBufs[0].requestWrite();

      MpAudioBufPtr audio(outBufs[0]);

      // Multiply
      MpDspUtils::mul(audio->getSamplesPtr(), mGain, mpGainBuffer, samplesPerFrame);
      // Shift back
      MpDspUtils::convert_Att(mpGainBuffer, audio->getSamplesWritePtr(), samplesPerFrame, MP_BRIDGE_FRAC_LENGTH);
   }

   return getResult;
}

UtlBoolean MprFromInputDevice::handleMessage(MpResourceMsg& rMsg)
{
    UtlBoolean msgHandled = FALSE;

    switch (rMsg.getMsg()) 
    {
    case MPRM_SET_GAIN:
    {
        MpIntResourceMsg *pMsg = (MpIntResourceMsg*)&rMsg;
        msgHandled = handleSetGain((int16_t)pMsg->getData());
    }
    break;

    case MPRM_SET_INPUT_DEVICE_ID:
        msgHandled = handleSetInputDeviceId(*((MpIntResourceMsg*)&rMsg));
    break;

    default:
        // If we don't handle the message here, let our parent try.
        msgHandled = MpResource::handleMessage(rMsg); 
    break;
    }

    return msgHandled;
}

UtlBoolean MprFromInputDevice::handleSetGain(MpBridgeGain gain)
{
   mGain = gain;
   return TRUE;
}

UtlBoolean MprFromInputDevice::handleSetInputDeviceId(const MpIntResourceMsg& message)
{
    MpOutputDeviceHandle newDeviceId = message.getData();
    if(newDeviceId != mDeviceId)
    {
        mFrameTimeInitialized = FALSE;
        mDeviceId = newDeviceId;
    }

    return(TRUE);
}

/* ============================ FUNCTIONS ================================= */

