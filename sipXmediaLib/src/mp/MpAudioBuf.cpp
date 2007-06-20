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

bool MpAudioBuf::isActiveAudio() const
{
    switch (getSpeechType()) {
        case MP_SPEECH_SILENT:
        case MP_SPEECH_COMFORT_NOISE:
        case MP_SPEECH_MUTED:
            return false;
        case MP_SPEECH_UNKNOWN:
        case MP_SPEECH_ACTIVE:
        case MP_SPEECH_TONE:
        default:
            return true;

    };
}

int MpAudioBuf::compareSamples(const MpAudioBuf& frame1, 
                               const MpAudioBuf& frame2, 
                               unsigned int tolerance)
{

    int difference = 0;
    int samplesInFrame1 = frame1.getSamplesNumber();
    int samplesInFrame2 = frame2.getSamplesNumber();

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
        const MpAudioSample* samples1 = frame1.getSamplesPtr();
        const MpAudioSample* samples2 = frame2.getSamplesPtr();

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
    mAttenDb = 0;
    mSpeechType = MP_SPEECH_UNKNOWN;
#ifdef MPBUF_DEBUG
    osPrintf(">>> MpAudioBuf::init()\n");
#endif
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
