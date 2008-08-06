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

void MpAudioBuf::scale(const MpAudioSample* src,
                       MpAudioSample* dst,
                       int sampleCount,
                       MpAudioSample resultingAmplitude,
                       MpAudioSample sourceAmplitudeStart,
                       MpAudioSample sourceAmplitudeEnd)
{
   int scaleStart = ((int32_t)resultingAmplitude << MP_AUDIO_SAMPLE_SIZE) / sourceAmplitudeStart;
   int scaleEnd   = ((int32_t)resultingAmplitude << MP_AUDIO_SAMPLE_SIZE) / sourceAmplitudeEnd;

   int step = (scaleEnd - scaleStart) / sampleCount;

   for (int i = 0, scale = scaleStart; i < sampleCount; i++, scale += step)
   {
      int tmp = (int)(((int64_t)src[i] * scale) >> MP_AUDIO_SAMPLE_SIZE);
      dst[i] = (MpAudioSample)MPF_SATURATE16(tmp);
   }
}


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
    mParams.mSpeechType = MP_SPEECH_UNKNOWN;
    mParams.mAmplitude = MpSpeechParams::MAX_AMPLITUDE;
    mParams.mIsClipped = FALSE;
#ifdef MPBUF_DEBUG
    osPrintf(">>> MpAudioBuf::init()\n");
#endif
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
