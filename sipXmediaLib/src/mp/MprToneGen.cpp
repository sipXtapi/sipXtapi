//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsConfigDb.h"
#include "os/OsUtil.h"
/* #include "pinger/Pinger.h" */
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
// TODO: Remove reference to MpCallFlowGraph
#include "mp/MpCallFlowGraph.h"
#include "mp/MprToneGen.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
const int MprToneGen::MIN_SAMPLE_RATE = 8000;
const int MprToneGen::MAX_SAMPLE_RATE = 48000;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprToneGen::MprToneGen(const UtlString& rName,
                       int samplesPerFrame,
                       int samplesPerSec,
                       const char* locale)
:  MpAudioResource(rName, 0, 1, 1, 1, samplesPerFrame, samplesPerSec),
   mpToneGenState(NULL)
{
   mpToneGenState = MpToneGen_MpToneGen(samplesPerSec, locale);
}

// Destructor
MprToneGen::~MprToneGen()
{
   MpToneGen_delete(mpToneGenState);
}

/* ============================ MANIPULATORS ============================== */

#ifdef LATER
/* Later (soon) this will be incorporated, but this is not quite the right
implementation.  At least these changes are needed:
(1) this should be an overriding virtual function, named
    handleSetSamplesPerSec.
(2) MpResource (the base class) needs to be enhanced so that the base
    virtual function exists to be overridden.
*/
// Sets the number of samples expected per second.
// Returns FALSE if the specified rate is not supported, TRUE otherwise.
UtlBoolean MprToneGen::setSamplesPerSec(int samplesPerSec)
{
   if (MprToneGen::MIN_SAMPLE_RATE > samplesPerSec)
      return FALSE;
   if (samplesPerSec > MprToneGen::MAX_SAMPLE_RATE)
      return FALSE;
   MpToneGen_delete(mpToneGenState);
   mpToneGenState = MpToneGen_MpToneGen(samplesPerSec);
   return TRUE;
}
#endif

// Sends a START_TONE message to this resource to begin generating 
// an audio tone.
// Returns the result of attempting to queue the message to this
// resource.
OsStatus MprToneGen::startTone(int toneId)
{
   MpFlowGraphMsg msg(START_TONE, this, NULL, NULL, toneId);
   OsStatus       res;

   res = postMessage(msg);
   return res;
}

// Sends a STOP_TONE message to this resource to stop generating 
// an audio tone.
// Returns the result of attempting to queue the message to this
// resource.
OsStatus MprToneGen::stopTone(void)
{
   MpFlowGraphMsg msg(STOP_TONE, this);
   OsStatus       res;

   res = postMessage(msg);
   return res;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprToneGen::doProcessFrame(MpBufPtr inBufs[],
                                      MpBufPtr outBufs[],
                                      int inBufsSize,
                                      int outBufsSize,
                                      UtlBoolean isEnabled,
                                      int samplesPerFrame,
                                      int samplesPerSecond)
{
   MpAudioBufPtr out;
   MpAudioSample *outbuf;
   int count;
   OsStatus ret;

   // We have one output
   if (outBufsSize != 1)
       return FALSE;

   // Don't waste the time if output is not connected
   if (!isOutputConnected(0))
       return TRUE;

   // Avoid division by zero
   if (samplesPerFrame == 0)
       return FALSE;

   if (isEnabled) {
      // Get new buffer
      out = MpMisc.RawAudioPool->getBuffer();
      if (!out.isValid())
         return FALSE;
      out->setSamplesNumber(samplesPerFrame);
      count = out->getSamplesNumber();

      // Generate new portion of tone
      outbuf = out->getSamples();
      ret = MpToneGen_getNextBuff(mpToneGenState, outbuf, count);

      // See what we get...
      switch (ret) {
      case OS_WAIT_TIMEOUT: /* one-shot tone completed */
          // TODO: remove referene to MpCallFlowGraph
          // MprToneGen::stopTone(mName, getFloGraph()->getMsgQ());
         ((MpCallFlowGraph*)getFlowGraph())->stopTone();
         out->setSpeechType(MpAudioBuf::MP_SPEECH_TONE);
         break;

      case OS_NO_MORE_DATA: /* silent */
         out.release();
         break;

      case OS_SUCCESS:
      default:
         out->setSpeechType(MpAudioBuf::MP_SPEECH_TONE);
         break;
      }
   } else {
      // If disabled just push input buffer downstream
      if (inBufsSize > 0)
          out = inBufs[0];
   }

   // Comment out this: Be honest, we failed.
//   if (!out.isValid()) {
//      out = MpMisc.mpFgSilence;
//   }

   // Set output
   outBufs[0] = out;
   
   return (mpToneGenState != NULL);
}

// Handle messages for this resource.
UtlBoolean MprToneGen::handleMessage(MpFlowGraphMsg& rMsg)
{
   int msgType;

   msgType = rMsg.getMsg();
   switch (msgType)
   {
   case START_TONE:
      MpToneGen_startTone(mpToneGenState, rMsg.getInt1());
      enable();
      break;
   case STOP_TONE:
      MpToneGen_stopTone(mpToneGenState);
      disable();
      break;
   default:
      return MpAudioResource::handleMessage(rMsg);
      break;
   }
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */

