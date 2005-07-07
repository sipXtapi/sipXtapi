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
#include "sipXezPhoneApp.h"
#include "sipXezPhoneFrame.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// MACRO CALLS
IMPLEMENT_APP(sipXezPhoneApp)

// Constructor
sipXezPhoneApp::sipXezPhoneApp()
{

}

// Destructor
sipXezPhoneApp::~sipXezPhoneApp()
{

}

sipXezPhoneApp* thePhoneApp;

// Application initialization
bool sipXezPhoneApp::OnInit()
{
   thePhoneApp = this;
   // create a new frame
   mpFrame = new sipXezPhoneFrame( APPLICATION_TITLE, wxDefaultPosition, wxSize(255,378) );

   // set the icon
   wxIcon icon("res/sipXezPhone.ico", wxBITMAP_TYPE_ICO);
   mpFrame->SetIcon(icon);

   // show the frame and put it on top
   mpFrame->Show( TRUE );
   SetTopWindow( mpFrame );

   return TRUE;
}

void sipXezPhoneApp::addLogMessage(const UtlString message)
{
   wxWindow* pLogWindow = wxWindow::FindWindowById(IDR_CALLERID_BOX, GetTopWindow());
   if (pLogWindow)
   {
      ((wxTextCtrl*)pLogWindow)->AppendText(message.data());
   }
}

void sipXezPhoneApp::setStatusMessage(const wxString& message)
{
   wxWindow* pLogWindow = wxWindow::FindWindowById(IDR_STATUS_BOX, GetTopWindow());
   if (pLogWindow)
   {
      ((wxTextCtrl*)pLogWindow)->SetValue(message);
   }
}

const wxString sipXezPhoneApp::getEnteredText()
{
   wxString phoneNumber;

    wxComboBox* pCtrl = dynamic_cast<wxComboBox*>(wxWindow::FindWindowById(IDR_DIAL_ENTRY_TEXT, GetTopWindow()));
    if (pCtrl)
    {
        phoneNumber = pCtrl->GetValue();
    }
    return phoneNumber;
}

sipXezPhoneFrame& sipXezPhoneApp::getFrame() const
{
    return *mpFrame;
}

void sipXezPhoneApp::setTitleMessage(const char* const szMessage) const
{
    if (mpFrame)
    {
        UtlString sTitle;
        
        sTitle = APPLICATION_TITLE + UtlString("-") + UtlString(szMessage);
        mpFrame->SetTitle(sTitle.data());
    }
    return;
}
