//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "../stdwx.h"
#include "../sipXmgr.h"
#include "PhoneStateRemoteBusy.h"
#include "../sipXezPhoneApp.h"
#include "PhoneStateIdle.h"
#include "../sipXmgr.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateRemoteBusy::PhoneStateRemoteBusy(void)
{
}

PhoneStateRemoteBusy::~PhoneStateRemoteBusy(void)
{
    sipXmgr::getInstance().stopTone();
}

PhoneState* PhoneStateRemoteBusy::OnFlashButton()
{
   return (new PhoneStateIdle());
}

PhoneState* PhoneStateRemoteBusy::Execute()
{
    thePhoneApp->setStatusMessage("Busy.");
    
    sipxCallStartTone(sipXmgr::getInstance().getCurrentCall(), ID_TONE_BUSY, true, false); 
   
    return this;
}
