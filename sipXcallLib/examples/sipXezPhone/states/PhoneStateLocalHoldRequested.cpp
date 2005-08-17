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
#include "PhoneStateLocalHoldRequested.h"
#include "PhoneStateCallHeldLocally.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateLocalHoldRequested::PhoneStateLocalHoldRequested(SIPX_CALL hCall)
{
}

PhoneStateLocalHoldRequested::~PhoneStateLocalHoldRequested(void)
{
}

PhoneState* PhoneStateLocalHoldRequested::OnConnectedInactive()
{
    return (new PhoneStateCallHeldLocally());
}

PhoneState* PhoneStateLocalHoldRequested::Execute()
{

   sipXmgr::getInstance().holdCurrentCall();
   return this;
}
