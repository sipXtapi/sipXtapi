//
// Copyright (C) 2006-2017 SIPez LLC. All rights reserved.
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

// WIN32: Add libspeexdsp to linker input.
#ifdef WIN32 // [
#   pragma comment(lib, "libspeexdsp.lib")
#endif // WIN32 ]

// SYSTEM INCLUDES
#include <speex/speex_preprocess.h>
#include <speex/speex_types.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpBuf.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MprSpeexPreProcess.h"
#include "mp/MprSpeexEchoCancel.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
volatile MprSpeexPreprocess::GlobalEnableState MprSpeexPreprocess::smGlobalAgcEnableState
   = MprSpeexPreprocess::LOCAL_MODE;
volatile MprSpeexPreprocess::GlobalEnableState MprSpeexPreprocess::smGlobalNoiseReductionEnableState
   = MprSpeexPreprocess::LOCAL_MODE;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprSpeexPreprocess::MprSpeexPreprocess(const UtlString& rName,
                                       UtlBoolean isAgcEnabled,
                                       UtlBoolean isNoiseReductionEnabled)
:  MpAudioResource(rName, 1, 1, 1, 1)
, mpPreprocessState(NULL)
, mIsAgcEnabled(isAgcEnabled)
, mIsNoiseReductionEnabled(isNoiseReductionEnabled)
, mIsAgcEnabledReal(getRealAgcState())
, mIsNoiseReductionEnabledReal(getRealNoiseReductionState())
{
}

// Destructor
MprSpeexPreprocess::~MprSpeexPreprocess()
{
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean MprSpeexPreprocess::setAGC (UtlBoolean enable)
{
   MpFlowGraphMsg msg(SET_AGC, this, NULL, NULL, enable);
   return (postMessage(msg) == OS_SUCCESS);
}

UtlBoolean MprSpeexPreprocess::setNoiseReduction (UtlBoolean enable)
{
   MpFlowGraphMsg msg(SET_NOISE_REDUCTION, this, NULL, NULL, enable);
   return (postMessage(msg) == OS_SUCCESS);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprSpeexPreprocess::doProcessFrame(MpBufPtr inBufs[],
                                              MpBufPtr outBufs[],
                                              int inBufsSize,
                                              int outBufsSize,
                                              UtlBoolean isEnabled,
                                              int samplesPerFrame,
                                              int samplesPerSecond)
{
   MpAudioBufPtr   inputBuffer;
   bool res = false;

   // We don't need to do anything if we don't have an output.
   if (outBufsSize != 1)
      return FALSE;

   // Get incoming data
   inputBuffer.swap(inBufs[0]);

   // If the object is not enabled or we don't have valid input, pass input to output
   if (  isEnabled
      && inputBuffer.isValid()
      && ((int)(inputBuffer->getSamplesNumber()) == samplesPerFrame))
   {
      // This buffer will be modified in place. Make sure we're the only owner.
      res = inputBuffer.requestWrite();
      assert(res);

      // Turn on/off AGC and Noise Reduction if global flag has changed.
      if (mIsAgcEnabledReal != getRealAgcState())
      {
         mIsAgcEnabledReal = getRealAgcState();
         speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_AGC, &mIsAgcEnabledReal);
      }
      if (mIsNoiseReductionEnabledReal != getRealNoiseReductionState())
      {
         mIsNoiseReductionEnabledReal = getRealNoiseReductionState();
         speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_DENOISE, &mIsNoiseReductionEnabledReal);
      }

      // Get Echo residue if we have any
      speex_preprocess_run(mpPreprocessState, (spx_int16_t*)inputBuffer->getSamplesPtr());
   }

   outBufs[0].swap(inputBuffer);
   return TRUE;
}


// Handle messages for this resource.
UtlBoolean MprSpeexPreprocess::handleMessage(MpFlowGraphMsg& rMsg)
{
   int       msgType;

   msgType = rMsg.getMsg();
   switch (msgType)
   {
   case SET_AGC:
      return handleSetAGC(rMsg.getInt1());
      break;
   case SET_NOISE_REDUCTION:
      return handleSetNoiseReduction(rMsg.getInt1());
      break;
   default:
      return MpAudioResource::handleMessage(rMsg);
      break;
   }
}

// Handle the SET_AGC message.
UtlBoolean MprSpeexPreprocess::handleSetAGC(UtlBoolean enable)
{
   mIsAgcEnabled = enable;
   if (mpPreprocessState != NULL)
   {
      mIsAgcEnabledReal = getRealAgcState();
      speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_AGC, &mIsAgcEnabledReal);
   }
   return true;
}

// Handle the SET_NOISE_REDUCTION message.
UtlBoolean MprSpeexPreprocess::handleSetNoiseReduction(UtlBoolean enable)
{
   mIsNoiseReductionEnabled = enable;
   if (mpPreprocessState != NULL)
   {
      mIsNoiseReductionEnabledReal = getRealNoiseReductionState();
      speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_DENOISE, &mIsNoiseReductionEnabledReal);
   }
   return true;
}

OsStatus MprSpeexPreprocess::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   OsStatus res =  MpAudioResource::setFlowGraph(pFlowGraph);

   if (res == OS_SUCCESS)
   {
      // Check are we added to flowgraph or removed.
      if (pFlowGraph != NULL)
      {
         // Initialize Speex preprocessor state
         mpPreprocessState = speex_preprocess_state_init(mpFlowGraph->getSamplesPerFrame(),
                                                         mpFlowGraph->getSamplesPerSec());
         mIsAgcEnabledReal = getRealAgcState();
         speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_AGC, &mIsAgcEnabledReal);
         mIsNoiseReductionEnabledReal = getRealNoiseReductionState();
         speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_DENOISE, &mIsNoiseReductionEnabledReal);
      }
      else
      {
         if (mpPreprocessState != NULL)
         {
            speex_preprocess_state_destroy(mpPreprocessState);
            mpPreprocessState = NULL;
         }
      }
   }

   return res;
}

UtlBoolean MprSpeexPreprocess::connectInput(MpResource& rFrom,
                                            int fromPortIdx,
                                            int toPortIdx)
{
   UtlBoolean res = MpAudioResource::connectInput(rFrom, fromPortIdx, toPortIdx);
   if (res)
   {
      // Enable residual echo removal if we're connected to MprSpeexEchoCancel.
      if (rFrom.getContainableType() == MprSpeexEchoCancel::TYPE)
      {
         MprSpeexEchoCancel *pEchoCancel = (MprSpeexEchoCancel*)&rFrom;
         speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_ECHO_STATE,
                              pEchoCancel->getSpeexEchoState());
      }
   }
   return res;
}

UtlBoolean MprSpeexPreprocess::disconnectInput(int inPortIdx)
{
   UtlBoolean res = MpAudioResource::disconnectInput(inPortIdx);
   if (res)
   {
      // Disable residual echo processing.
      speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_ECHO_STATE, NULL);
   }
   return res;
}

/* ============================ FUNCTIONS ================================= */
#endif // HAVE_SPEEX ]
