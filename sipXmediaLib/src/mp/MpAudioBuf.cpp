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
