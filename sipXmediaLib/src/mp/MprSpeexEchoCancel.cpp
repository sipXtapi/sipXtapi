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

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprSpeexEchoCancel::MprSpeexEchoCancel(const UtlString& rName,
                                       int filterLength,
                                       int echoResiduePoolSize)
: MpAudioResource(rName, 1, 1, 1, 2)
, mEchoResiduePool( MpArrayBuf::getHeaderSize() + sizeof(spx_int32_t)*(mpFlowGraph->getSamplesPerFrame() + 1),
                   echoResiduePoolSize)
, mFilterLength(filterLength)
{
}

// Destructor
MprSpeexEchoCancel::~MprSpeexEchoCancel()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

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
   MpArrayBufPtr   echoResidueBuffer;
   MpBufferMsg*    bufferMsg;
   spx_int32_t*    pEchoResidue;
   bool            res = false;

   // We don't need to do anything if we don't have an output.
   if (!isOutputConnected(0) && !isOutputConnected(1))
      return FALSE;

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

      // Try to get a reference frame for echo cancelation.  21 = MAX_SPKR_BUFFERS(12) +
      if (MpMisc.pEchoQ->numMsgs() > MAX_ECHO_QUEUE) {

         // Flush queue
         while ( (MpMisc.pEchoQ->receive((OsMsg*&) bufferMsg, OsTime::NO_WAIT_TIME) == OS_SUCCESS)
               && MpMisc.pEchoQ->numMsgs() > MAX_ECHO_QUEUE)
         {
            bufferMsg->releaseMsg();
         }

         // Get buffer from the message and free message
         echoRefBuffer = bufferMsg->getBuffer();
         assert(echoRefBuffer.isValid());
         bufferMsg->releaseMsg();

         if (echoRefBuffer->getSamplesNumber() == samplesPerFrame) {
            mStartedCanceling = true;

            // Get new buffer
            outBuffer = MpMisc.RawAudioPool->getBuffer();
            assert(outBuffer.isValid());
            outBuffer->setSamplesNumber(samplesPerFrame);
            outBuffer->setSpeechType(inputBuffer->getSpeechType());

            if (isOutputConnected(1)) {
               echoResidueBuffer = mEchoResiduePool.getBuffer();
               assert(outBuffer.isValid());
            }

            if (outBuffer.isValid()) {
               pEchoResidue = (spx_int32_t*)echoResidueBuffer->getDataWritePtr();
            }

            // Do echo cancelation
            speex_echo_cancel(mpEchoState,
                              (spx_int16_t*)inputBuffer->getSamplesPtr(),
                              (spx_int16_t*)echoRefBuffer->getSamplesPtr(),
                              (spx_int16_t*)outBuffer->getSamplesPtr(),
                              pEchoResidue);
         } else {
            //The sample count didn't match so we can't echo cancel.  Pass the frame.
            outBuffer = inputBuffer;
            assert(!mStartedCanceling);
         }
      } else {
         //There was no speaker data to match.  Pass the frame.
         outBuffer = inputBuffer;
//         osPrintf("SpeexEchoCancel: No frame to match...\n");
//         assert (!mStartedCanceling);
      }
   }

   outBufs[0].swap(outBuffer);
   outBufs[1].swap(echoResidueBuffer);
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

