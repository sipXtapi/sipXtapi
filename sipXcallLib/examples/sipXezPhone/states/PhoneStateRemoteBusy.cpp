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
#include "..\stdwx.h"
#include "..\sipXmgr.h"
#include "PhoneStateRemoteBusy.h"
#include "../sipXezPhoneApp.h"

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
}

PhoneState* PhoneStateRemoteBusy::Execute()
{
   thePhoneApp->setStatusMessage("Busy.");
   return this;
}
