//
// Copyright (C) 2006-2017 SIPez LLC.  All right reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 ProfitFuel Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// $$
///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_SPEEX // [

// SYSTEM INCLUDES
#include <speex/speex_echo.h>
#include <speex/speex_types.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpBufPool.h"
#include "mp/MpBufferMsg.h"
#include "mp/MpPackedResourceMsg.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MprSpeexEchoCancel.h"
#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#  ifdef RTL_AUDIO_ENABLED
#     include <SeScopeAudioBuffer.h>
#  endif
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType MprSpeexEchoCancel::TYPE = "MprSpeexEchoCancel";
volatile MprSpeexEchoCancel::GlobalEnableState MprSpeexEchoCancel::smGlobalEnableState
   = MprSpeexEchoCancel::LOCAL_MODE;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprSpeexEchoCancel::MprSpeexEchoCancel(const UtlString& rName,
                                       OsMsgQ* pSpkrQ,
                                       int spkrQDelayMs,
                                       int filterLength)
: MpAudioResource(rName, 1, 1, 1, 1)
, mFilterLength(filterLength)
, mpSpkrQ(pSpkrQ)
, mSpkrQDelayMs(spkrQDelayMs)
, mSpkrQDelayFrames(0)
{
}

// Destructor
MprSpeexEchoCancel::~MprSpeexEchoCancel()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprSpeexEchoCancel::setSpkrQ(const UtlString& namedResource,
                                      OsMsgQ& fgQ,
                                      OsMsgQ *pSpkrQ)
{
   OsStatus stat;
   MpPackedResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_SET_SPEAKER_QUEUE,
                           namedResource);
   UtlSerialized &msgData = msg.getData();

   stat = msgData.serialize((void*)pSpkrQ);
   assert(stat == OS_SUCCESS);
   msgData.finishSerialize();
   return fgQ.send(msg, sOperationQueueTimeout);
}

/* ============================ ACCESSORS ================================= */

UtlContainableType MprSpeexEchoCancel::getContainableType() const
{
   return TYPE;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprSpeexEchoCancel::doProcessFrame(MpBufPtr inBufs[],
                                              MpBufPtr outBufs[],
                                              int inBufsSize,
                                              int outBufsSize,
                                              UtlBoolean isEnabled,
                                              int samplesPerFrame,
                                              int samplesPerSecond)
{
   MpAudioBufPtr   outBuffer;
   MpAudioBufPtr   inputBuffer;
   MpAudioBufPtr   echoRefBuffer;
   MpBufferMsg*    bufferMsg;

   // If disabled pass buffer through
   if (  smGlobalEnableState == GLOBAL_DISABLE
      || (smGlobalEnableState == LOCAL_MODE && !isEnabled))
   {
      outBufs[0].swap(inBufs[0]);
      return TRUE;
   }

   // Get incoming data
   inputBuffer.swap(inBufs[0]);

   // If we already started cancelling, but received empty buffer from mic
   // just replace it with silence to keep AEC in sync
   if (mStartedCanceling && !inputBuffer.isValid())
   {
      inputBuffer = mpSilenceBuf;
   }

         // Check for valid input
   if (  inputBuffer.isValid()
      && ((int)(inputBuffer->getSamplesNumber()) == samplesPerFrame)
         // Speaker data should be delayed by mSpkrQDelayFrames frames
      && mpSpkrQ->numMsgs() > mSpkrQDelayFrames)
   {
      // Try to get a reference frame for echo cancellation.
      if (mpSpkrQ->receive((OsMsg*&) bufferMsg, OsTime::NO_WAIT_TIME) == OS_SUCCESS)
      {
         // Flush speaker queue
         while (  mpSpkrQ->numMsgs() > MAX_ECHO_QUEUE
               && (mpSpkrQ->receive((OsMsg*&) bufferMsg, OsTime::NO_WAIT_TIME) == OS_SUCCESS))
         {
            bufferMsg->releaseMsg();
            OsSysLog::add(FAC_MP, PRI_WARNING, "Flushing speaker queue in %s",
                          getName().data());
            assert(!"Flushing speaker queue");
         }

         // Get the buffer from the message and free the message
         echoRefBuffer = bufferMsg->getBuffer();
         bufferMsg->releaseMsg();
         // Use silence buffer if we received empty message.
         if (!echoRefBuffer.isValid())
         {
            echoRefBuffer = mpSilenceBuf;
         }
         if ((int)(echoRefBuffer->getSamplesNumber()) == samplesPerFrame)
         {
            mStartedCanceling = true;

            // Get new buffer
            outBuffer = MpMisc.RawAudioPool->getBuffer();
            assert(outBuffer.isValid());
            outBuffer->setSamplesNumber(samplesPerFrame);
            assert((int)(outBuffer->getSamplesNumber()) == samplesPerFrame);
            outBuffer->setSpeechType(inputBuffer->getSpeechType());

            // Do echo cancellation
            speex_echo_cancellation(mpEchoState,
                                    (const spx_int16_t*)inputBuffer->getSamplesPtr(),
                                    (const spx_int16_t*)echoRefBuffer->getSamplesPtr(),
                                    (spx_int16_t*)outBuffer->getSamplesWritePtr());

#ifdef RTL_AUDIO_ENABLED
            UtlString rtlOutputLabel(mpFlowGraph->getFlowgraphName());
            rtlOutputLabel.append("_");
            rtlOutputLabel.append(getName());
            RTL_AUDIO_BUFFER(rtlOutputLabel+"_rec",
                             mpFlowGraph->getSamplesPerSec(),
                             inputBuffer,
                             0);
            RTL_AUDIO_BUFFER(rtlOutputLabel+"_play",
                             mpFlowGraph->getSamplesPerSec(),
                             echoRefBuffer,
                             0);
            RTL_AUDIO_BUFFER(rtlOutputLabel+"_out",
                             mpFlowGraph->getSamplesPerSec(),
                             outBuffer,
                             0);
#endif
         }
         else
         {
            //The sample count didn't match so we can't echo cancel.  Pass the frame.
            outBuffer.swap(inputBuffer);
            assert(!mStartedCanceling);
         }
      }
      else
      {
         //There was no speaker data to match.  Pass the frame.
         outBuffer.swap(inputBuffer);
//         osPrintf("SpeexEchoCancel: No frame to match...\n");
//         assert (!mStartedCanceling);
      }
   }
   else
   {
      // There was no correct data on the input.  Pass the frame as is.
      outBuffer.swap(inputBuffer);
   }

   outBufs[0].swap(outBuffer);
   return TRUE;
}

UtlBoolean MprSpeexEchoCancel::handleMessage(MpResourceMsg& rMsg)
{
   OsSysLog::add(FAC_MP, PRI_DEBUG,
      "MprSpeexEchoCancel::handleMessage(%d)", rMsg.getMsg());
   switch (rMsg.getMsg())
   {
   case MPRM_SET_SPEAKER_QUEUE:
      {
         OsStatus stat;
         OsMsgQ *pSpkrQ;

         UtlSerialized &msgData = ((MpPackedResourceMsg*)(&rMsg))->getData();
         stat = msgData.deserialize((void*&)pSpkrQ);
         assert(stat == OS_SUCCESS);

         mpSpkrQ = pSpkrQ;
         return TRUE;
      }
      break;
   }
   return MpAudioResource::handleMessage(rMsg);
}

OsStatus MprSpeexEchoCancel::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   OsStatus res =  MpAudioResource::setFlowGraph(pFlowGraph);

   if (res == OS_SUCCESS)
   {
      // Check are we added to flowgraph or removed.
      if (pFlowGraph != NULL)
      {
         //Initialize Speex Echo state with frame size and number of frames for length of buffer
         mpEchoState = speex_echo_state_init(mpFlowGraph->getSamplesPerFrame(), 
                                             mpFlowGraph->getSamplesPerSec()*mFilterLength/1000);
         mSpkrQDelayFrames = mSpkrQDelayMs*mpFlowGraph->getSamplesPerSec()/mpFlowGraph->getSamplesPerFrame()/1000;
         // Allocate buffer for silence and fill it with 0s
         if (!mpSilenceBuf.isValid())
         {
            mpSilenceBuf = MpMisc.RawAudioPool->getBuffer();
         }
         mpSilenceBuf->setSamplesNumber(mpFlowGraph->getSamplesPerFrame());
         memset(mpSilenceBuf->getSamplesWritePtr(), 0,
                mpSilenceBuf->getSamplesNumber()*sizeof(MpAudioSample));

         mStartedCanceling = false;
      }
      else
      {
         if (mpEchoState != NULL)
         {
            speex_echo_state_destroy(mpEchoState);
            mpEchoState = NULL;
         }
      }
   }

   return res;
}

/* ============================ FUNCTIONS ================================= */

#endif // HAVE_SPEEX ]

