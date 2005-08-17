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
#include "../DialerThread.h"
#include "../sipXezPhoneApp.h"
#include "PhoneStateDialing.h"
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

PhoneState* PhoneStateDialing::OnFlashButton()
{
   sipXmgr::getInstance().disconnect();
   return (new PhoneStateIdle());
}

PhoneState* PhoneStateDialing::Execute()
{

   thePhoneApp->setStatusMessage("Dialing.");
        DialerThread* dt = new DialerThread(mPhoneNumber);
        dt->Create();
        dt->Run();

   return this;
}
