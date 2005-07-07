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
#include "stdwx.h"
#include "DialerThread.h"
#include "sipXmgr.h"
#include "states\PhoneStateMachine.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Constructor
DialerThread::DialerThread(wxString phoneNumber) :
        mPhoneNumber(phoneNumber),
        wxThread()
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
