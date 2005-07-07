// $Id$
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "../stdwx.h"
#include "../sipXmgr.h"
#include "../sipXezPhoneApp.h"
#include "PhoneStateIdle.h"
#include "PhoneStateDialing.h"
#include "PhoneStateRinging.h"
#include "PhoneStateAccepted.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateRinging::PhoneStateRinging(SIPX_CALL hCall)
{
   mhCall = hCall;
}

PhoneStateRinging::~PhoneStateRinging(void)
{
   sipxCallStopTone(mhCall);
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

    SIPX_RESULT result;
    result = sipxCallStartTone(sipXmgr::getInstance().getCurrentCall(), ID_TONE_RINGTONE, true, false); 

    return this;
}
