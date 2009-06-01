//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 ProfitFuel Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// Copyright (C) 2006 SIPez LLC. 
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
#include "mp/MpFlowGraphBase.h"
#include "mp/MprSpeexEchoCancel.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType MprSpeexEchoCancel::TYPE = "MprSpeexEchoCancel";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprSpeexEchoCancel::MprSpeexEchoCancel(const UtlString& rName,
                                       OsMsgQ* pSpkrQ,
                                       int filterLength)
: MpAudioResource(rName, 1, 1, 1, 1)
, mFilterLength(filterLength)
, mpSpkrQ(pSpkrQ)
{
}

// Destructor
MprSpeexEchoCancel::~MprSpeexEchoCancel()
{
}

/* ============================ MANIPULATORS ============================== */

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
   bool            res = false;

   // If disabled pass buffer through
   if (!isEnabled) {
      outBufs[0].swap(inBufs[0]);
      return TRUE;
   }

   // Get incoming data
   inputBuffer.swap(inBufs[0]);

   // If the object is not enabled or we don't have valid input,
   // pass input to output
   if (  inputBuffer.isValid()
      && (inputBuffer->getSamplesNumber() == samplesPerFrame))
   {
      // This buffer will be modified in place. Make sure we're the only owner.
      res = inputBuffer.requestWrite();
      assert(res);

      // Try to get a reference frame for echo cancellation.  21 = MAX_SPKR_BUFFERS(12) +
      if (mpSpkrQ->numMsgs() > MAX_ECHO_QUEUE)
      {
         // Flush speaker queue
         while ( (mpSpkrQ->receive((OsMsg*&) bufferMsg, OsTime::NO_WAIT_TIME) == OS_SUCCESS)
               && mpSpkrQ->numMsgs() > MAX_ECHO_QUEUE)
         {
            bufferMsg->releaseMsg();
            OsSysLog::add(FAC_MP, PRI_WARNING, "Flushing speaker queue in %s",
                          getName().data());
         }

         // Get the buffer from the message and free the message
         echoRefBuffer = bufferMsg->getBuffer();
         assert(echoRefBuffer.isValid());
         bufferMsg->releaseMsg();

         if (echoRefBuffer->getSamplesNumber() == samplesPerFrame)
         {
            mStartedCanceling = true;

            // Get new buffer
            outBuffer = MpMisc.RawAudioPool->getBuffer();
            assert(outBuffer.isValid());
            outBuffer->setSamplesNumber(samplesPerFrame);
            assert(outBuffer->getSamplesNumber() == samplesPerFrame);
            outBuffer->setSpeechType(inputBuffer->getSpeechType());

            // Do echo cancellation
            speex_echo_cancellation(mpEchoState,
                                    (spx_int16_t*)inputBuffer->getSamplesPtr(),
                                    (spx_int16_t*)echoRefBuffer->getSamplesPtr(),
                                    (spx_int16_t*)outBuffer->getSamplesPtr());
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

   outBufs[0].swap(outBuffer);
   return TRUE;
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

         //mpEchoResidue = (spx_int32_t*)malloc(sizeof(spx_int32_t) * (samplesPerFrame + 1));
         mStartedCanceling = false; // Debug Use only
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

