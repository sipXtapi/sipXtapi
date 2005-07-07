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
#include "PhoneStateCallHeld.h"
#include "PhoneStateConnected.h"
#include "PhoneStateIdle.h"
#include "PhoneStateDisconnectRequested.h"
#include "../sipXezPhoneApp.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateCallHeld::PhoneStateCallHeld()
{
}

PhoneStateCallHeld::~PhoneStateCallHeld(void)
{
}

PhoneState* PhoneStateCallHeld::OnFlashButton()
{
   return (new PhoneStateDisconnectRequested());
}

PhoneState* PhoneStateCallHeld::OnDisconnected()
{
   sipXmgr::getInstance().disconnect();
   return (new PhoneStateIdle());
}

PhoneState* PhoneStateCallHeld::OnHoldButton()
{
    sipXmgr::getInstance().unholdCurrentCall();
    return (new PhoneStateConnected());
}

PhoneState* PhoneStateCallHeld::Execute()
{
    sipXmgr::getInstance().holdCurrentCall();
    thePhoneApp->setStatusMessage("Call On Hold.");
   
   return this;
}
