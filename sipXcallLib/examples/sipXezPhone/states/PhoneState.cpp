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
#include "PhoneState.h"
#include "PhoneStateIdle.h"
#include "PhoneStateRemoteBusy.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneState::PhoneState(void) :
   mhCall(SIPX_CALL_NULL)
{
}

PhoneState::~PhoneState(void)
{
}

PhoneState* PhoneState::OnRemoteBusy()
{
    return (new PhoneStateRemoteBusy());
}

PhoneState* PhoneState::OnDisconnected(const SIPX_CALL hCall)
{
   return (new PhoneStateIdle);
}

PhoneState* PhoneState::OnOffer(SIPX_CALL hCall)
{
    sipxCallReject(hCall);  // default behavior is to just reject the call
    return this;
}
