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

// MACROS
//#define PLC_ALLOW_UNBOUNDED_SILENCE_INJECTION

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char *MpPlcSilence::name = "Silence substitution";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsStatus MpPlcSilence::init(int samplesPerSec)
{
   return OS_SUCCESS;
}

MpPlcSilence::~MpPlcSilence()
{

}

void MpPlcSilence::reset()
{

}

void MpPlcSilence::fullReset()
{

}


/* ============================ MANIPULATORS ============================== */

OsStatus MpPlcSilence::insertToHistory(int frameNum,
                                       const MpSpeechParams &speechParams,
                                       MpAudioSample* pBuf,
                                       unsigned inSamplesNum)
{
   return OS_NOT_SUPPORTED;
}

OsStatus MpPlcSilence::processFrame(MpSpeechParams &speechParams,
                                    MpAudioSample* pBuf,
                                    unsigned bufferSize,
                                    unsigned inSamplesNum,
                                    unsigned outSamplesNum,
                                    int wantedAdjustment,
                                    int &madeAdjustment)
{
#ifdef PLC_ALLOW_UNBOUNDED_SILENCE_INJECTION // [
   unsigned wantedOutSamplesNum = sipx_max(80, (int)outSamplesNum+wantedAdjustment);
#else // PLC_ALLOW_UNBOUNDED_SILENCE_INJECTION ][
   unsigned wantedOutSamplesNum = outSamplesNum;
#endif // PLC_ALLOW_UNBOUNDED_SILENCE_INJECTION ]
   wantedOutSamplesNum = sipx_min(wantedOutSamplesNum, bufferSize);

   if (inSamplesNum < wantedOutSamplesNum)
   {
      // Add silence if requested
      memset(pBuf+inSamplesNum,
             0,
            (wantedOutSamplesNum-inSamplesNum)*sizeof(MpAudioSample));

      if (inSamplesNum == 0)
      {
         // This is pure PLC, i.e. frame is entirely lost.
         speechParams.mSpeechType = MP_SPEECH_SILENT;
         speechParams.mAmplitude = 0;
         speechParams.mIsClipped = FALSE;
         speechParams.mFrameEnergy = 0;
      }
   }

   madeAdjustment = wantedOutSamplesNum - outSamplesNum;

   return OS_SUCCESS;
}

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
