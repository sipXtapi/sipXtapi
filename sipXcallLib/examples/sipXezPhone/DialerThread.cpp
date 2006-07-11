//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "stdwx.h"
#include "DialerThread.h"
#include "sipXmgr.h"
#include "states/PhoneStateMachine.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Constructor
DialerThread::DialerThread(wxString phoneNumber) :
   wxThread(),
   mPhoneNumber(phoneNumber)
{
}


// Entry point into the thread
void* DialerThread::Entry()
{
   sipXmgr::getInstance().placeCall(mPhoneNumber);
        return NULL;
}

// Routine called by the base when exiting the thread
void DialerThread::OnExit()
{

}
