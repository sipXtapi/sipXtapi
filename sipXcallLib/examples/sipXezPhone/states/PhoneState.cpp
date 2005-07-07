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
#include "PhoneState.h"
#include "PhoneStateIdle.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneState::PhoneState(void) : mhCall(NULL)
{
}

PhoneState::~PhoneState(void)
{
}

PhoneState* PhoneState::OnDisconnected()
{
   return (new PhoneStateIdle);
}

