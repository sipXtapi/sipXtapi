//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

// SYSTEM INCLUDES
#include <string.h>

// APPLICATION INCLUDES
#include "mp/MpPlcBase.h"
#include "mp/MpPlcSilence.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char *MpPlcSilence::name = "Silence substitution";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsStatus MpPlcSilence::init(int samplesPerSec, int samplesPerFrame)
{
   mSamplesPerFrame = samplesPerFrame;
   return OS_SUCCESS;
}

OsStatus MpPlcSilence::processFrame(int inFrameNum,
                                    int outFrameNum,
                                    const MpAudioSample* in,
                                    MpAudioSample* out,
                                    UtlBoolean* signalModified)
{
   int delta = outFrameNum - inFrameNum;
   UtlBoolean doPlc = ((in == NULL) || (delta != 0)) && (out != NULL);
   UtlBoolean doUpdateHistory = ((delta == 0) && (out != NULL) && (in != NULL));

   if (doPlc) 
   {
      memset(out, 0, mSamplesPerFrame*2);
      *signalModified = TRUE;
   }
   else if (doUpdateHistory)
   {
      *signalModified = FALSE;
   }
   else
   {
      return OS_FAILED;
   }

   return OS_SUCCESS;
}

MpPlcSilence::~MpPlcSilence()
{

}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

int MpPlcSilence::getMaxDelayedFramesNum() const
{
   return 0;
}

int MpPlcSilence::getMaxFutureFramesNum() const
{
   return 0;
}

int MpPlcSilence::getAlgorithmicDelay() const
{
   return 0;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
