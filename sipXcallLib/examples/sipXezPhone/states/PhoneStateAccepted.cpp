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
#include "PhoneStateAccepted.h"
#include "PhoneStateConnected.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateAccepted::PhoneStateAccepted(SIPX_CALL hCall)
{
   mhCall = hCall;
}

PhoneStateAccepted::~PhoneStateAccepted(void)
{
}

PhoneState* PhoneStateAccepted::OnConnected()
{
   return (new PhoneStateConnected());
}

PhoneState* PhoneStateAccepted::Execute()
{
   thePhoneApp->setStatusMessage("Accepted.");   
   sipxCallAnswer(mhCall);

   return this;
}
