//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioBuf.h"
#include "mp/MpDspUtils.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
MpBufPool *MpAudioBuf::smpDefaultPool = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

int MpAudioBuf::compareSamples(const MpAudioBufPtr& frame,
                               unsigned int tolerance) const
{

    int difference = 0;
    int samplesInFrame1 = getSamplesNumber();
    int samplesInFrame2 = frame->getSamplesNumber();

    if(samplesInFrame1 > samplesInFrame2)
    {
        difference = 1;
    }

    else if(samplesInFrame1 < samplesInFrame2)
    {
        difference = -1;
    }

    else
    {
        const MpAudioSample* samples1 = getSamplesPtr();
        const MpAudioSample* samples2 = frame->getSamplesPtr();

        int sampleDiff;
        for(int sampleIndex = 0; sampleIndex < samplesInFrame1; sampleIndex++)
        {
            sampleDiff = *samples1 - *samples2;
            if((sampleDiff > 0) && (sampleDiff > (int)tolerance))
            {
                difference = 1;
                break;
            }
            else if((sampleDiff < 0) && ((sampleDiff + tolerance) < 0))
            {
                difference = -1;
                break;
            }
            samples1++;
            samples2++;
        }
    }

    return(difference);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpAudioBuf::init()
{
    // No VAD decision yet
    mParams.mSpeechType = MP_SPEECH_UNKNOWN;
    // No AGC decision yet
    mParams.mAmplitude = MpSpeechParams::MAX_AMPLITUDE;
    // We can't tell if it clipped or not
    mParams.mIsClipped = FALSE;
    // Energy is not set yet
    mParams.mFrameEnergy = -1;
    // Highest speaker rank - just some reasonable default
    mParams.mSpeakerRank = 0;
#ifdef MPBUF_DEBUG
    osPrintf(">>> MpAudioBuf::init()\n");
#endif
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
