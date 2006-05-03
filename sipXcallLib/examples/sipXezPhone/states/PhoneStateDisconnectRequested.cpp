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
#include "PhoneStateDisconnectRequested.h"
#include "PhoneStateIdle.h"
#include "PhoneStateMachine.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateDisconnectRequested::PhoneStateDisconnectRequested()
{
}

PhoneStateDisconnectRequested::~PhoneStateDisconnectRequested(void)
{
}

PhoneState* PhoneStateDisconnectRequested::OnFlashButton()
{
   // try to disconnect again, then force the state to IDLE
   sipXmgr::getInstance().disconnect(0, true);  // SHOULD cause the DISCONNECT sipXtapiEvent to occur
   return (new PhoneStateIdle());   
}

PhoneState* PhoneStateDisconnectRequested::Execute()
{
   sipXmgr::getInstance().disconnect(0, true);  // should cause the DISCONNECT sipXtapiEvent to occur
   
   
   // now, force a transition to the Idle state
   PhoneStateMachine::getInstance().setState(new PhoneStateIdle());
   return this;
}
