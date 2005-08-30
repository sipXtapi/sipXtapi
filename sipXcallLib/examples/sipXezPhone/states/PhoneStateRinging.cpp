//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "../stdwx.h"
#include "../sipXmgr.h"
#include "../sipXezPhoneApp.h"
#include "PhoneStateIdle.h"
#include "PhoneStateDialing.h"
#include "PhoneStateRinging.h"
#include "PhoneStateAccepted.h"
#include "../sipXezPhoneSettings.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateRinging::PhoneStateRinging(SIPX_CALL hCall) : 
    mbPlayingTone(false)
{
   mhCall = hCall;
}

PhoneStateRinging::~PhoneStateRinging(void)
{
#ifdef VOICE_ENGINE
    if (mbPlayingTone)
        sipxCallPlayFileStop(sipXmgr::getInstance().getCurrentCall());
#else
    if (mbPlayingTone)
        sipxCallStopTone(mhCall);
#endif
}

PhoneState* PhoneStateRinging::OnFlashButton()
{
    return (new PhoneStateAccepted(mhCall));
}

PhoneState* PhoneStateRinging::Execute()
{
    sipXmgr::getInstance().setCurrentCall(mhCall); 
    
    char szIncomingNumber[256];
    sipxCallGetRemoteID(mhCall, szIncomingNumber, 256);
    wxString incomingNumber(szIncomingNumber);

    thePhoneApp->setStatusMessage(incomingNumber);

#ifdef VOICE_ENGINE
    if (SIPX_RESULT_SUCCESS == sipxCallPlayFileStart(sipXmgr::getInstance().getCurrentCall(), "res/ringTone.raw", true, true, false))
    {
        mbPlayingTone  = true;
    }
#else
    SIPX_RESULT result;
    result = sipxCallStartTone(sipXmgr::getInstance().getCurrentCall(), ID_TONE_RINGTONE, true, false); 
    if (SIPX_RESULT_SUCCESS == result)
    {
        mbPlayingTone = true;
    }
    else
    {
        mbPlayingTone = false;
    }
#endif


    if (sipXezPhoneSettings::getInstance().getAutoAnswer() == true)
    {
        // if in Auto Answer mode, go ahead and answer
        sipxCallAnswer(sipXmgr::getInstance().getCurrentCall());
    }
    return this;
}
