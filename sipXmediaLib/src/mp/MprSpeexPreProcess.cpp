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

// WIN32: Add libspeex to linker input.
#ifdef WIN32 // [
#   pragma comment(lib, "libspeex.lib")
#endif // WIN32 ]

// SYSTEM INCLUDES
#include <speex/speex_preprocess.h>
#include <speex/speex_types.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpBuf.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MprSpeexPreProcess.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprSpeexPreprocess::MprSpeexPreprocess(const UtlString& rName)
:  MpAudioResource(rName, 1, 1, 1, 1)
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
      && (inputBuffer->getSamplesNumber() == samplesPerFrame))
   {
      // This buffer will be modified in place. Make sure we're the only owner.
      res = inputBuffer.requestWrite();
      assert(res);

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
   speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_AGC, &enable);
   return true;
}

// Handle the SET_NOISE_REDUCTION message.
UtlBoolean MprSpeexPreprocess::handleSetNoiseReduction(UtlBoolean enable)
{
   speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_DENOISE, &enable);
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

/* ============================ FUNCTIONS ================================= */
#endif // HAVE_SPEEX ]