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


// Constructor
TransferThread::TransferThread(SIPX_CALL hCall,
                               wxString phoneNumber) :
   wxThread(),
   mhCall(hCall),
   mPhoneNumber(phoneNumber)
{
}


// Entry point into the thread
void* TransferThread::Entry()
{
    sipxCallBlindTransfer(mhCall, mPhoneNumber);

    return NULL ;
}

// Routine called by the base when exiting the thread
void TransferThread::OnExit()
{

}
