//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>
#ifdef __pingtel_on_posix__
#include <unistd.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "os/OsTask.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprBufferRecorder.h"
#include "mp/MpBufRecStartResourceMsg.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MpResNotificationMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprBufferRecorder::MprBufferRecorder(const UtlString& rName)
: MpAudioResource(rName, 1, 1, 0, 1)
, mpBuffer(NULL)
, mSamplesToRecord(0)
, mBufferSamplesUsed(0)
, mStatus(RECORD_IDLE)
{
}

// Destructor
MprBufferRecorder::~MprBufferRecorder()
{
   if (isEnabled())
   {
      handleStopRecording();
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprBufferRecorder::startRecording(const UtlString& namedResource, 
                                           OsMsgQ& fgQ,
                                           int ms,
                                           UtlString* pBuffer)
{
   MpBufRecStartResourceMsg msg(namedResource, ms, pBuffer);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprBufferRecorder::stopRecording(const UtlString& namedResource,
                                          OsMsgQ& fgQ)
{
   MpResourceMsg msg(MpResourceMsg::MPRM_BUF_RECORDER_STOP, namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

UtlBoolean MprBufferRecorder::disable(Completion code)
{
   // This implementation is broken by design. All this should be done
   // in handleDisable() acually.

   UtlBoolean res = FALSE;

   // Set the status...
   mStatus = code;

   // Set the buffer pointer to null..
   mpBuffer = NULL;

   res = (MpResource::disable());

   // If the recording was stopped or finished,
   MpResNotificationMsg::RNMsgType noteMsgType = 
      MpResNotificationMsg::MPRNM_MESSAGE_INVALID;
   switch(code)
   {
      case RECORD_FINISHED:
         // Send resource notification message to indicate recording complete.
         noteMsgType = MpResNotificationMsg::MPRNM_BUFRECORDER_FINISHED;
         break;
      case RECORD_STOPPED:
         // Send resource notification message to indicate recording stopped prematurely.
         noteMsgType = MpResNotificationMsg::MPRNM_BUFRECORDER_STOPPED;
         break;
      case NO_INPUT_DATA:
         // Send resource notification message to indicate error - 
         // no input data buffer provided.
         noteMsgType = MpResNotificationMsg::MPRNM_BUFRECORDER_NOINPUTDATA;
         break;
      default:
         break;
   }

   if(noteMsgType != MpResNotificationMsg::MPRNM_MESSAGE_INVALID)
   {
      sendNotification(noteMsgType);
   }

   return res;
}

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprBufferRecorder::doProcessFrame(MpBufPtr inBufs[],
                                             MpBufPtr outBufs[],
                                             int inBufsSize,
                                             int outBufsSize,
                                             UtlBoolean isEnabled,
                                             int samplesPerFrame,
                                             int samplesPerSecond)
{
   // Pass input to output. We should do this all cases, so do it here.
   outBufs[0] = inBufs[0];

   if (!isEnabled) 
   {
      return TRUE;
   }

   if (mpBuffer == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, 
                    "MprBufferRecorder::doProcessFrame to disable "
                    "recording because mBuffer is NULL");
      // If we're in the idle state here then we should be disabled, but
      // there isn't any reason to send notification, so disable keeping idle state.
      if(mStatus == RECORD_IDLE)
      {
         disable(RECORD_IDLE);
      }
      else
      {
         // Otherwise, if it's some other state we're in, then we should stop.
         disable(RECORD_STOPPED); // just in case...
      }
      return TRUE;
   }

   // We only use input 0 in this resource
   MpAudioBufPtr inBuf = inBufs[0];

   if (inBufsSize == 0)
   {
      // no input buffers, indicate config error
      disable(NO_INPUT_DATA);
      return TRUE;
   }

   // Our destination to copy samples is after the samples we've
   // already written (mnBufferSamplesUsed samples in from the beginning)
   MpAudioSample* destSamples = 
      ((MpAudioSample*)mpBuffer->data()) + mBufferSamplesUsed;
   unsigned int samplesToWrite = mSamplesToRecord-mBufferSamplesUsed;

   // Copy input data to buffer or write silence if no input.
   if (inBuf.isValid())
   {
      // Be sure to not overflow buffer.
      samplesToWrite = sipx_min(samplesToWrite, inBuf->getSamplesNumber());

      // Copy the audio data from the input to our buffer.
      memcpy(destSamples, 
             inBuf->getSamplesPtr(), 
             samplesToWrite * sizeof(MpAudioSample));
   }
   else
   {
      // Be sure to not overflow buffer.
      samplesToWrite = sipx_min(samplesToWrite,
                                mpFlowGraph->getSamplesPerFrame());

      // Copy the audio data from the input to our buffer.
      memset(destSamples, 0,
             samplesToWrite * sizeof(MpAudioSample));
   }

   mBufferSamplesUsed += samplesToWrite;
   mpBuffer->resize(mBufferSamplesUsed*sizeof(MpAudioSample), FALSE);

   if (mBufferSamplesUsed == mSamplesToRecord)
   {
      // We've finished recording.
      OsSysLog::add(FAC_MP, PRI_INFO,
                    "MprRecorder::doProcessFrame disabling recording -- finished:"
                    " mnBufferSamplesUsed=%d, numberSamples=%d, mStatus=%d", 
                    mBufferSamplesUsed, mSamplesToRecord, mStatus);
      disable(RECORD_FINISHED);
   }

   return TRUE;
}

// Handle start recording message for this resource.
UtlBoolean MprBufferRecorder::handleStartRecording(int ms, UtlString* pBuffer)
{
   if (isEnabled()) 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, 
         "MprRecorder::handleStartRecording -- "
         "attempt to start recording while already enabled!!");
      return TRUE;
   }

   assert(ms >= sizeof(MpAudioSample));
   if (ms < sizeof(MpAudioSample))
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MprRecorder::handleStartRecording -- "
                    "length of buffer is less too short!");
      return TRUE;
   }

   mpBuffer = pBuffer;
   // Reset number of samples used, since we have a new buffer
   mBufferSamplesUsed = 0;
   mStatus = RECORD_IDLE;
   if (mpBuffer != NULL)
   {
      mSamplesToRecord = mpFlowGraph->getSamplesPerSec()/1000*ms;
      // We have to add 1 to capacity, because UtlString think we need
      // place for a leading zero.
      mpBuffer->capacity(mSamplesToRecord*sizeof(MpAudioSample)+1);
      mpBuffer->resize(0);
   }

   // Now we finally enable the resource to start the recording.
   handleEnable();

   OsSysLog::add(FAC_MP, PRI_DEBUG, 
                 "MprRecorder::handleStartRecording"
                 "(0x%08X, %d)... #samplesUsed=%d",
                 mpBuffer->data(), mpBuffer->length(), 
                 mBufferSamplesUsed);
   return TRUE;
}

UtlBoolean MprBufferRecorder::handleStopRecording()
{
   if (!isEnabled()) 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MprBufferRecorder::stopRecording"
         " -- attempt to stop while disabled!!");
   }
   else
   {
      disable(RECORD_STOPPED);
   }
   return TRUE;
}

UtlBoolean MprBufferRecorder::handleEnable()
{
   if (mpBuffer != NULL)
   {
      mStatus = RECORDING;
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MprBufferRecorder::enable");
      MpResource::handleEnable();
   } else 
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
         "MprBufferRecorder::enable (No buffer specified!)");
   }

   return TRUE;
}

UtlBoolean MprBufferRecorder::handleMessage(MpResourceMsg& rMsg)
{
   OsSysLog::add(FAC_MP, PRI_DEBUG, 
                 "MprBufferRecorder::handleMessage(%d)", 
                 rMsg.getMsg());

   MpBufRecStartResourceMsg* rsRMsg = NULL;
   switch (rMsg.getMsg())
   {
   case MpResourceMsg::MPRM_BUF_RECORDER_START:
      {
         rsRMsg = (MpBufRecStartResourceMsg*)&rMsg;
         return handleStartRecording(rsRMsg->getRecordingLength(),
                                     rsRMsg->getAudioBuffer());
      }
      break;

   case MpResourceMsg::MPRM_BUF_RECORDER_STOP:
      return handleStopRecording();
      break;
   }
   return MpAudioResource::handleMessage(rMsg);
}

/* ============================ FUNCTIONS ================================= */

