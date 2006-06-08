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
#include "PhoneStateRemoteAlerting.h"
#include "PhoneStateConnected.h"
#include "PhoneStateIdle.h"
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
#ifdef VOICE_ENGINE
        sipxCallAudioPlayFileStop(sipXmgr::getInstance().getCurrentCall());
#else
        sipxCallStopTone(mhCall);
#endif
}

PhoneState* PhoneStateRemoteAlerting::OnConnected()
{
   return (new PhoneStateConnected());
}

PhoneState* PhoneStateRemoteAlerting::OnFlashButton()
{
   sipxCallAudioPlayFileStop(sipXmgr::getInstance().getCurrentCall());
   sipXmgr::getInstance().disconnect(0, true);
   return (new PhoneStateIdle());
}

PhoneState* PhoneStateRemoteAlerting::OnDisconnected(const SIPX_CALL hCall)
{
   if (hCall == sipXmgr::getInstance().getCurrentCall())
   {
      sipXmgr::getInstance().disconnect(hCall, false);
   }
   return (new PhoneStateIdle());
}

PhoneState* PhoneStateRemoteAlerting::Execute()
{
   thePhoneApp->setStatusMessage("Remote Alerting.");
   sipxCallAudioPlayFileStart(sipXmgr::getInstance().getCurrentCall(), "res/ringTone.raw", true, true, false, true, 0.05);
   return this;
}
