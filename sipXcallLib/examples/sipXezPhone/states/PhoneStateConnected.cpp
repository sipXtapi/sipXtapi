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
#include "../stdwx.h"
#include "../sipXmgr.h"
#include "../MainPanel.h"
#include "PhoneStateConnected.h"
#include "PhoneStateCallHeld.h"
#include "PhoneStateIdle.h"
#include "PhoneStateDisconnectRequested.h"
#include "PhoneStateTransferRequested.h"
#include "../sipXezPhoneApp.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateConnected::PhoneStateConnected()
{
}

PhoneStateConnected::~PhoneStateConnected(void)
{
}

PhoneState* PhoneStateConnected::OnFlashButton()
{
   return (new PhoneStateDisconnectRequested());
}

PhoneState* PhoneStateConnected::OnDisconnected(SIPX_CALL hCall)
{
   if (hCall == sipXmgr::getInstance().getCurrentCall())
   {
      sipXmgr::getInstance().disconnect();
      return (new PhoneStateIdle());
   }
   else
   {
      return this;
   }
}

PhoneState* PhoneStateConnected::OnHoldButton()
{
    return (new PhoneStateCallHeld());
}

PhoneState* PhoneStateConnected::OnTransferRequested(const wxString phoneNumber)
{
    return (new PhoneStateTransferRequested(phoneNumber));
}

PhoneState* PhoneStateConnected::Execute()
{
   MainPanel* pMainPanel = dynamic_cast<MainPanel*>(wxWindow::FindWindowById(IDR_MAINPANEL, thePhoneApp->GetTopWindow()));
   if (pMainPanel)
   {
       pMainPanel->StartTimer(true);
   }
   thePhoneApp->setStatusMessage("Connected.");
   
   return this;
}
