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
#include "../sipXezPhoneApp.h"
#include "PhoneStateTransfering.h"
#include "PhoneStateIdle.h"
#include "PhoneStateMachine.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateTransfering::PhoneStateTransfering()
{
}

PhoneStateTransfering::~PhoneStateTransfering(void)
{
}


PhoneState* PhoneStateTransfering::Execute()
{
   thePhoneApp->setStatusMessage("Transferring.");  

   return this;
}
