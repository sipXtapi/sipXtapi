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
#include "../DialerThread.h"
#include "../sipXezPhoneApp.h"
#include "PhoneStateDialing.h"
#include "PhoneStateRemoteAlerting.h"
#include "PhoneStateConnected.h"
#include "PhoneStateIdle.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS



PhoneStateDialing::PhoneStateDialing(const wxString phoneNumber) :
   mPhoneNumber(phoneNumber)
{
}

PhoneStateDialing::~PhoneStateDialing(void)
{

}

PhoneState* PhoneStateDialing::OnConnected()
{
   return (new PhoneStateConnected());
}

PhoneState* PhoneStateDialing::OnDisconnected(const SIPX_CALL hCall)
{
   if (hCall == sipXmgr::getInstance().getCurrentCall())
   {
      sipXmgr::getInstance().disconnect(hCall, false);
   }
   return (new PhoneStateIdle());
}

PhoneState* PhoneStateDialing::OnFlashButton()
{
   sipXmgr::getInstance().disconnect(0, true);
   return (new PhoneStateIdle());
}

PhoneState* PhoneStateDialing::OnRemoteAlerting()
{
    return (new PhoneStateRemoteAlerting());
}

PhoneState* PhoneStateDialing::Execute()
{

   thePhoneApp->setStatusMessage("Dialing.");
        DialerThread* dt = new DialerThread(mPhoneNumber);
        dt->Create();
        dt->Run();

   return this;
}
