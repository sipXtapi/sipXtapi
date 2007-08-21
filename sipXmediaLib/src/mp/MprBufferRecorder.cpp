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

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprBufferRecorder::MprBufferRecorder(const UtlString& rName,
                                     int samplesPerFrame, 
                                     int samplesPerSec)
:  MpAudioResource(rName, 1, 1, 0, 1, samplesPerFrame, samplesPerSec),
   mpBuffer(NULL),
   mnBufferSamplesUsed(0),
   mStatus(RECORD_IDLE)
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

OsStatus MprBufferRecorder::startRecording(
            const UtlString& namedResource, 
            OsMsgQ& fgQ,
            UtlString* pBuffer)
{
   MpBufRecStartResourceMsg msg(namedResource, pBuffer);
   return fgQ.send(msg, sOperationQueueTimeout);
}

OsStatus MprBufferRecorder::stopRecording(
            const UtlString& namedResource,
            OsMsgQ& fgQ)
{
   MpResourceMsg msg(MpResourceMsg::MPRM_BUF_RECORDER_STOP, namedResource);
   return fgQ.send(msg, sOperationQueueTimeout);
}

UtlBoolean MprBufferRecorder::enable(void)
{
   UtlBoolean retVal = FALSE;

   if (mpBuffer != NULL)
   {
      mStatus = RECORDING;
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MprBufferRecorder::enable");
      retVal = MpResource::enable();
   } else 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, 
                    "MprBufferRecorder::enable (No buffer specified!)");
   }

   return retVal;
}

UtlBoolean MprBufferRecorder::disable(Completion code)
{
   UtlBoolean res = FALSE;

   // Set the status...
   mStatus = code;

   // Set the buffer pointer to null..
   mpBuffer = NULL;

   res = (MpResource::disable());

   // If the recording was stopped or finished,
   if(code == RECORD_FINISHED || code == RECORD_STOPPED)
   {
      // Send resource notification message to indicate recording complete.
      sendNotification(MpResNotificationMsg::MPRNM_BUFRECORDER_STOPPED);
   }
   else if(code == NO_INPUT_DATA)
   {
      // Send resource notification message to indicate error - 
      // no input data buffer provided.
      sendNotification(MpResNotificationMsg::MPRNM_BUFRECORDER_NOINPUTDATA);
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
   if (!isEnabled) 
   {
      return TRUE;
   }

   if (mpBuffer == NULL || mpBuffer->isNull())
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

   unsigned int nRecordBufSamples = 
      mpBuffer->length() / sizeof(MpAudioSample);

   // We only use buffer 0 in this resource
   MpAudioBufPtr inBuf = inBufs[0];

   if (inBufsSize == 0)
   {
      // no input buffers, indicate config error
      disable(NO_INPUT_DATA);
      return TRUE;
   }

   // maximum record time reached
   if (mnBufferSamplesUsed >= nRecordBufSamples) 
   {
      OsSysLog::add(FAC_MP, PRI_INFO,
                    "MprRecorder::doProcessFrame disabling recording -- finished:"
                    " mnBufferSamplesUsed=%d, numberSamples=%d, mStatus=%d", 
                    mnBufferSamplesUsed, nRecordBufSamples, mStatus);
      disable(RECORD_FINISHED);
   } 
   else 
   {
      // Write silence if no input
      if (!inBuf.isValid())
      {
         // Replace the invalid in with the silence frame.
         inBuf = MpMisc.mpFgSilence;
      }

      // Our destination to copy samples is after the samples we've
      // already written (mnBufferSamplesUsed samples in from the beginning)
      MpAudioSample* destSamples = 
         ((MpAudioSample*)mpBuffer->data()) + mnBufferSamplesUsed;

      const MpAudioSample* inputSamples = inBuf->getSamplesPtr();

      unsigned int numInSamples = inBuf->getSamplesNumber();

      // If the input audio is bigger than the amount of room
      // we have left, then set our perceived notion of the input 
      // size to the amount of room we have left.
      unsigned int nSamplesToWrite = 
         min(numInSamples, (nRecordBufSamples-mnBufferSamplesUsed));

      // Copy the audio data from the input to our buffer.
      memcpy(destSamples, 
             inputSamples, 
             nSamplesToWrite * sizeof(MpAudioSample));

      mnBufferSamplesUsed += nSamplesToWrite;
   }
   return TRUE;
}

// Handle start recording message for this resource.
UtlBoolean MprBufferRecorder::handleStartRecording(UtlString* pBuffer)
{
   if (isEnabled()) 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, 
         "MprRecorder::handleStartRecording -- "
         "attempt to start recording while already enabled!!");
      return TRUE;
   }

   assert(pBuffer->length() >= sizeof(MpAudioSample));
   if (pBuffer->length() < sizeof(MpAudioSample))
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MprRecorder::handleStartRecording -- "
                    "length of buffer is less too short!");
      return TRUE;
   }

   mpBuffer = pBuffer;
   // Reset number of samples used, since we have a new buffer
   mnBufferSamplesUsed = 0;
   mStatus = RECORD_IDLE;

   // Now we finally enable the resource to start the recording.
   enable();

   OsSysLog::add(FAC_MP, PRI_DEBUG, 
                 "MprRecorder::handleStartRecording"
                 "(0x%08X, %d)... #samplesUsed=%d",
                 mpBuffer->data(), mpBuffer->length(), 
                 mnBufferSamplesUsed);
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
         return handleStartRecording(rsRMsg->getAudioBuffer());
      }
      break;

   case MpResourceMsg::MPRM_BUF_RECORDER_STOP:
      return handleStopRecording();
      break;
   }
   return MpAudioResource::handleMessage(rMsg);
}

/* ============================ FUNCTIONS ================================= */

