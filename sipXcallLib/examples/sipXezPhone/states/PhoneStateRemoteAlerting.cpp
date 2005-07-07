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
#include "PhoneStateRemoteAlerting.h"
#include "PhoneStateConnected.h"
#include "../sipXezPhoneApp.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateRemoteAlerting::PhoneStateRemoteAlerting()
{
}

PhoneStateRemoteAlerting::~PhoneStateRemoteAlerting(void)
{
}

PhoneState* PhoneStateRemoteAlerting::OnConnected()
{
   return (new PhoneStateConnected());
}

PhoneState* PhoneStateRemoteAlerting::Execute()
{
   thePhoneApp->setStatusMessage("Remote Alerting.");
   return this;
}
