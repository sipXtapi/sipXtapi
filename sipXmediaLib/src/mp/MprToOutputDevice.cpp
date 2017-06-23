//  
// Copyright (C) 2007-2017 SIPez LLC. All rights reserved.
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
#include <mp/MpBufferMsg.h>
#include <mp/MpResampler.h>
#include <mp/MpIntResourceMsg.h>

#ifdef RTL_ENABLED
#include <rtl_macro.h>
#include <os/OsDateTime.h>
#else
#define RTL_BLOCK(x)
#define RTL_EVENT(x,y)
#define RTL_AUDIO_BUFFER(x,y,w,z)
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
static const UtlString classname="MprToOutputDevice";

// DEFINES
#define DEBUG_PRINT
#undef  DEBUG_PRINT

// MACROS
#ifdef DEBUG_PRINT // [
#  define debugPrintf    printf
#else  // DEBUG_PRINT ][
static void debugPrintf(...) {}
#endif // DEBUG_PRINT ]

//#define ENABLE_FILE_LOGGING
#ifdef ENABLE_FILE_LOGGING
static FILE *sgSpkrFile=NULL;
class OutFileInit
{
public:
   OutFileInit()
   {
      sgSpkrFile = fopen("/sdcard/spkr.raw", "w");
   }

   ~OutFileInit()
   {
      fclose(sgSpkrFile);
   }
};

static OutFileInit sgOutFileInit;
#endif // ENABLE_FILE_LOGGING

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
, mMixerBufferPosition(0)
, mDeviceId(deviceId)
, mpResampler(MpResamplerBase::createResampler(1, 8000, 8000))
, mCopyQPool(rName+"-copy-pool", MpBufferMsg(MpBufferMsg::AUD_PLAY),
             20, OsMsgQ::DEF_MAX_MSGS, OsMsgQ::DEF_MAX_MSGS, 1,
             OsMsgPool::SINGLE_CLIENT)
, mIsCopyQEnabled(FALSE)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MprToOutputDevice::MprToOutputDevice resource: %s using output device ID: %d",
        rName.data(), deviceId);
}

// Destructor
MprToOutputDevice::~MprToOutputDevice()
{
   delete mpResampler;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprToOutputDevice::setDeviceId(const UtlString& resourceName,
                                        OsMsgQ& flowgraphMessageQueue,
                                        MpOutputDeviceHandle deviceId)
{
    MpIntResourceMsg message((MpResourceMsg::MpResourceMsgType)MPRM_SET_OUTPUT_DEVICE_ID,
                             resourceName,
                             deviceId);
    return(flowgraphMessageQueue.send(message, sOperationQueueTimeout));
}

OsStatus MprToOutputDevice::enableCopyQ(const UtlString& namedResource,
                                        OsMsgQ& fgQ,
                                        UtlBoolean enable)
{
   MpResourceMsg msg(enable?(MpResourceMsg::MpResourceMsgType)MPRM_ENABLE_COPY_QUEUE:
                            (MpResourceMsg::MpResourceMsgType)MPRM_DISABLE_COPY_QUEUE,
                     namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprToOutputDevice::enableCopyQ(UtlBoolean enable)
{
   MpResourceMsg msg(enable?(MpResourceMsg::MpResourceMsgType)MPRM_ENABLE_COPY_QUEUE:
                            (MpResourceMsg::MpResourceMsgType)MPRM_DISABLE_COPY_QUEUE,
                     getName());
   return postMessage(msg);
}


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
   MpAudioBufPtr inAudioBuffer;
   MpBufferMsg *pCopyMsg = NULL;
   UtlBoolean isCopyMsgSet = FALSE;

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

   inAudioBuffer.swap(inBufs[0]);
   RTL_AUDIO_BUFFER("MprToOutputDevice::doProcessFrame", samplesPerSecond, inAudioBuffer, 0);

   if (mIsCopyQEnabled)
   {
      pCopyMsg = (MpBufferMsg*)mCopyQPool.findFreeMsg();
   }

   // Milliseconds per frame:
   frameTimeInterval = samplesPerFrame * 1000 / samplesPerSecond;

   uint32_t devSampleRate = 0;
   OsStatus stat = mpOutputDeviceManager->getDeviceSamplesPerSec(mDeviceId, devSampleRate);
   if(stat != OS_SUCCESS)
   {
      // Get the name of the device we're writing to.
      UtlString devName = "Unknown device";
      mpOutputDeviceManager->getDeviceName(mDeviceId, devName);

      OsSysLog::add(FAC_MP, PRI_ERR, "MprToOutputDevice::doProcessFrame "
         "- Couldn't get device sample rate from output device manager!  "
         "Device - \"%s\"", devName.data());
      return FALSE;
   }

   // If the output device is disabled, don't resample to save the CPU cycles
   UtlBoolean deviceDisabled = FALSE;
   if(!mpOutputDeviceManager->isDeviceEnabled(mDeviceId))
   {
       deviceDisabled = TRUE;
       devSampleRate = samplesPerSecond;
   }

   // Check to see if the resampler needs it's rate adjusted.
   if((int)(mpResampler->getInputRate()) != samplesPerSecond)
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
            samplesPerSecond, devSampleRate, classname) != OS_SUCCESS)
      {
         // Error messages have already been logged. No need to log it again here.
         return FALSE;
      }

      // If the resampled buffer is valid, then use it.
      if(resampledBuffer.isValid())
      {
         if (pCopyMsg != NULL)
         {
            // If copy queue is activated save original buffer to it.
            pCopyMsg->ownBuffer(inAudioBuffer);
            isCopyMsgSet = TRUE;
         }
         // To optimize for speed a bit, we use MpBufPtr's swap() method 
         // instead of assignment -- make sure we don't use resampledBuffer
         // after this!
         inAudioBuffer.swap(resampledBuffer);
      }
   }

   if(deviceDisabled)
   {
       // we do nothing until it becomes enabled
   }

   // We push buffer to output device even if buffer is NULL. With NULL buffer we
   // notify output device that we will not push more frames during this time
   // interval.
   else if (!mFrameTimeInitialized)
   {
      status = mpOutputDeviceManager->pushFrameFirst(mDeviceId,
                                                     mFrameTime,
                                                     inAudioBuffer,
                                                     !mFrameTimeInitialized);

      RTL_EVENT(mpFlowGraph->getFlowgraphName()+"_"+getName()+"_pushFrame_result", status);
      debugPrintf("MprToOutputDevice::doProcessFrame(): frameTime+mixerBufferPosition=%d+%d=%d pushResult=%d %s\n",
                  mFrameTime, mMixerBufferPosition, mFrameTime+mMixerBufferPosition,
                  status, inAudioBuffer.isValid()?"":"[NULL BUFFER]");
      if (status == OS_SUCCESS)
      {
         // Frame time should be initialized at this point.
         mFrameTimeInitialized = TRUE;
         mLastPushedTime = mFrameTime;
         // Advance frame time by one step.
         RTL_EVENT(mpFlowGraph->getFlowgraphName()+"_"+getName()+"_mFrameTime", mFrameTime);
         mFrameTime += frameTimeInterval;
      }
   }
   else
   {
      status = mpOutputDeviceManager->pushFrame(mDeviceId,
                                                mFrameTime+mMixerBufferPosition,
                                                inAudioBuffer);
      if (status == OS_SUCCESS)
      {
         mLastPushedTime = mFrameTime+mMixerBufferPosition;
      }
      else
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG, "MprToOutputDevice::doProcessFrame() returned %d %s\n",
                       status, inAudioBuffer.isValid()?"":"[NULL BUFFER]");
      }

      RTL_EVENT(mpFlowGraph->getFlowgraphName()+"_"+getName()+"_pushFrame_result", status);
      debugPrintf("MprToOutputDevice::doProcessFrame(): frameTime+mixerBufferPosition=%d+%d=%d pushResult=%d %s\n",
                  mFrameTime, mMixerBufferPosition, mFrameTime+mMixerBufferPosition,
                  status, inAudioBuffer.isValid()?"":"[NULL BUFFER]");
      while (status == OS_INVALID_STATE)
      {
         // Ouch.. Frame is reported to be too late. Likely this means that device
         // processed several frames in a burst. This rarely happens alone,
         // usually driver burst all the time and we need to move slightly
         // forward in mixer buffer to iron out this bursts. This will increase
         // latency, so we adjust only as small as needed.
         // This can also happen when we generate data more slowly then it is
         // consumed. E.g. this happens when we're running out of CPU and can't
         // keep up with required rate.
         MpFrameTime mixerBufferLength;
         mpOutputDeviceManager->getMixerBufferLength(mDeviceId, mixerBufferLength);
         if (mMixerBufferPosition + frameTimeInterval <= mixerBufferLength - frameTimeInterval)
         {
            // Move further in a mixer buffer if we have more room in it.
            mMixerBufferPosition += frameTimeInterval;
            OsSysLog::add(FAC_MP, PRI_DEBUG, "MprToOutputDevice::doProcessFrame(): mMixerBufferPosition=%d %s\n",
                          mMixerBufferPosition, inAudioBuffer.isValid()?"":"[NULL BUFFER]");
            RTL_EVENT(mpFlowGraph->getFlowgraphName()+"_"+getName()+"_mixerBufferPosition", mMixerBufferPosition);
         }
         else
         {
            // We've run out of mixer buffer space, but still need to advance
            // some more. Looks like we either can't keep abreast with sound
            // card rate because of lack of CPU power, or it's a sound card
            // clock temporary deviation. Anyway, react                                 with increasing mFrameTime
            // to continue playing.
            mFrameTime += frameTimeInterval;
            OsSysLog::add(FAC_MP, PRI_WARNING, "MprToOutputDevice::doProcessFrame(): frameTime=%d %s\n",
                          mFrameTime, inAudioBuffer.isValid()?"":"[NULL BUFFER]");
         }
         // Push data for playback
         status = mpOutputDeviceManager->pushFrame(mDeviceId,
                                                   mFrameTime+mMixerBufferPosition,
                                                   inAudioBuffer);
         // Save frame time of the last successful push
         if (status == OS_SUCCESS)
         {
            mLastPushedTime = mFrameTime+mMixerBufferPosition;
         }
         else
         {
            OsSysLog::add(FAC_MP, PRI_DEBUG, "MprToOutputDevice::doProcessFrame() returned %d after retry %s\n",
                          status, inAudioBuffer.isValid()?"":"[NULL BUFFER]");
         }
         RTL_EVENT(mpFlowGraph->getFlowgraphName()+"_"+getName()+"_pushFrame_result", status);
      }

      while (status == OS_LIMIT_REACHED)
      {
         // And now we're too early to push the frame. So either CPU utilization
         // have got better or sound card clocks were put on line.
         // In both cases we ought to react by moving backward in time.
         mFrameTime -= frameTimeInterval;
         OsSysLog::add(FAC_MP, PRI_DEBUG, "MprToOutputDevice::doProcessFrame(): frameTime=%d %s\n",
                        mFrameTime, inAudioBuffer.isValid()?"":"[NULL BUFFER]");

         // But we don't want to push frame over the frame we've already pushed.
         // We'll get a very well audible click if we did. Click was caused by
         // that we mix next frame over last frame and it leads to double volume
         // of that specific frame. It's far less audible to just drop this
         // frame.
         if (mFrameTime+mMixerBufferPosition <= mLastPushedTime)
         {
            break;
         }

         // Push data for playback
         status = mpOutputDeviceManager->pushFrame(mDeviceId,
                                                   mFrameTime+mMixerBufferPosition,
                                                   inAudioBuffer);
         // Save frame time of the last successful push
         if (status == OS_SUCCESS)
         {
            mLastPushedTime = mFrameTime+mMixerBufferPosition;
         }
         else
         {
            OsSysLog::add(FAC_MP, PRI_DEBUG, "MprToOutputDevice::doProcessFrame() returned %d after retry %s\n",
                          status, inAudioBuffer.isValid()?"":"[NULL BUFFER]");
         }
         RTL_EVENT(mpFlowGraph->getFlowgraphName()+"_"+getName()+"_pushFrame_result", status);
      }

      // Advance frame time by one step.
      RTL_EVENT(mpFlowGraph->getFlowgraphName()+"_"+getName()+"_mFrameTime", mFrameTime);
      mFrameTime += frameTimeInterval;
   }

#ifdef ENABLE_FILE_LOGGING
   if (status == OS_SUCCESS && inAudioBuffer.isValid())
   {
      fwrite(inAudioBuffer->getSamplesPtr(), 1, inAudioBuffer->getSamplesNumber()*sizeof(short), sgSpkrFile);
   }
   else
   {
      MpAudioSample silence = 0;
      for (int i=0; i<samplesPerFrame; i++)
      {
         fwrite(&silence, 1, sizeof(short), sgSpkrFile);
      }
   }
#endif // ENABLE_FILE_LOGGING

   // Send buffer to copy queue if copy queue is enabled
   if (pCopyMsg != NULL)
   {
      if (status != OS_SUCCESS)
      {
         // Push failed - send empty buffer
         inAudioBuffer.release();
         pCopyMsg->ownBuffer(inAudioBuffer);
      }
      else if (!isCopyMsgSet)
      {
         // Push was successful and no resampling occurred.
         // If there was resampling pCopyMsg already has original buffer in it.
         pCopyMsg->ownBuffer(inAudioBuffer);
      }
      if (mCopyQ.send(*pCopyMsg, OsTime::NO_WAIT_TIME) != OS_SUCCESS)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "Sending message to copy queue in %s failed!", getName().data());
         pCopyMsg->releaseMsg();
      }
   }

   return (status == OS_SUCCESS);
}

UtlBoolean MprToOutputDevice::handleMessage(MpResourceMsg& rMsg)
{
    UtlBoolean msgHandled = FALSE;

    switch (rMsg.getMsg()) 
    {
    case MPRM_ENABLE_COPY_QUEUE:
        mIsCopyQEnabled = TRUE;
        msgHandled = TRUE;
        break;

    case MPRM_DISABLE_COPY_QUEUE:
        mIsCopyQEnabled = FALSE;
        msgHandled = TRUE;
        break;

    case MPRM_SET_OUTPUT_DEVICE_ID:
       msgHandled = handleSetOutputDeviceId((const MpIntResourceMsg&)rMsg);
       break;

    default:
        // If we don't handle the message here, let our parent try.
        msgHandled = MpResource::handleMessage(rMsg); 
        break;
    }
    return msgHandled;
}


UtlBoolean MprToOutputDevice::handleEnable()
{
   mFrameTimeInitialized = FALSE;
   mFrameTime = 0;
   mMixerBufferPosition = 0;
   return MpResource::handleEnable();
}

UtlBoolean MprToOutputDevice::handleSetOutputDeviceId(const MpIntResourceMsg& message)
{
    MpOutputDeviceHandle newDeviceId = message.getData();
    OsSysLog::add(FAC_MP, PRI_DEBUG,
        "MprToOutputDevice::handleSetOutputDeviceId setting deviceId: %d",
        newDeviceId);
    if(newDeviceId != mDeviceId)
    {
        mFrameTimeInitialized = FALSE;
    }
    mDeviceId = newDeviceId;

    return(TRUE);
}

/* ============================ FUNCTIONS ================================= */

