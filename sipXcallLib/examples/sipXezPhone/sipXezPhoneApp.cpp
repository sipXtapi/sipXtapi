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
#include <sys/types.h>
#include <sys/stat.h>

// APPLICATION INCLUDES
#include "stdwx.h"
BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(ezEVT_STATUS_MESSAGE_COMMAND, 8301)
    DECLARE_EVENT_TYPE(ezEVT_LOG_MESSAGE_COMMAND, 8302)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(ezEVT_STATUS_MESSAGE_COMMAND)
DEFINE_EVENT_TYPE(ezEVT_LOG_MESSAGE_COMMAND)

#include "sipXezPhoneApp.h"
#include "sipXezPhoneFrame.h"
#include "sipXezPhoneSettings.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// MACRO CALLS
IMPLEMENT_APP(sipXezPhoneApp)

BEGIN_EVENT_TABLE(sipXezPhoneApp, wxApp)
    EVT_STATUS_MESSAGE_COMMAND(-1, sipXezPhoneApp::OnProcessStatusMessage)
    EVT_LOG_MESSAGE_COMMAND(-1, sipXezPhoneApp::OnProcessLogMessage)
END_EVENT_TABLE()


// Constructor
sipXezPhoneApp::sipXezPhoneApp() :
    mStatusMessage(""),
    mLogMessage("")
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
        if (!sipXezPhoneSettings::getInstance().loadSettings())
        {
#        ifdef _WIN32
            PROCESS_INFORMATION pi ;
            STARTUPINFO si ;
            memset(&si, 0, sizeof(si)) ;
            si.cb = sizeof(si) ;

            CreateProcess("ConfigWizard.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) ;

            // Wait until child process exits.
            WaitForSingleObject( pi.hProcess, INFINITE );

            // Close process and thread handles. 
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );

            memset(&si, 0, sizeof(si)) ;
            si.cb = sizeof(si) ;

            CreateProcess("AVWizard.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) ;

            // Wait until child process exits.
            WaitForSingleObject( pi.hProcess, INFINITE );

            // Close process and thread handles. 
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );


            sipXezPhoneSettings::getInstance().loadSettings() ;
#       endif
        }


    // check the command line arguements
    if (argc > 0)
    {
        // loop through command line args
        for (int i = 0; i < argc; i++)
        {
            if (strcmp(argv[i], "-test") == 0)
            {
                sipXezPhoneSettings::getInstance().setTestMode(true);
            }
        }
    }
    
    thePhoneApp = this;

    bool bLogo = false;
    int height = 378;

#ifdef _WIN32
    struct _stat buf;

    if (_stat("res/PhoneLogo.bmp", &buf) == 0)
    {
        bLogo = true;
        height = 460;
    }
#endif
    // create a new frame
    mpFrame = new sipXezPhoneFrame( "sipXezPhone", wxDefaultPosition, wxSize(255, height), bLogo );

    // set the icon
    // this icon type doesn't seem to work in Linux
    #ifdef _WIN32
        wxIcon icon("res/sipXezPhone.ico", wxBITMAP_TYPE_ICO);
        mpFrame->SetIcon(icon);
    #endif

    // show the frame and put it on top
    mpFrame->Show( TRUE );
    SetTopWindow( mpFrame );

    return TRUE;
}

void sipXezPhoneApp::addLogMessage(UtlString message)
{
   wxCommandEvent logMessageEvent(ezEVT_LOG_MESSAGE_COMMAND);

   message.append("\n");
   // TODO - probably need to make the string copy thread-safe
   mLogMessage = message;

   wxPostEvent(this, logMessageEvent);
}

void sipXezPhoneApp::setStatusMessage(const wxString& message)
{
   wxCommandEvent statusMessageEvent(ezEVT_STATUS_MESSAGE_COMMAND);

   // TODO - should probably make the setting of the message thread-safe
   mStatusMessage = message;

   wxPostEvent(this, statusMessageEvent);
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

void sipXezPhoneApp::OnProcessStatusMessage(wxCommandEvent& event)
{
   wxWindow* pLogWindow = wxWindow::FindWindowById(IDR_STATUS_BOX, GetTopWindow());
   if (pLogWindow)
   {
      ((wxTextCtrl*)pLogWindow)->SetValue(mStatusMessage);
   }
    return;
}

void sipXezPhoneApp::OnProcessLogMessage(wxCommandEvent& event)
{
   wxWindow* pLogWindow = wxWindow::FindWindowById(IDR_CALLERID_BOX, GetTopWindow());
   if (pLogWindow)
   {
      ((wxTextCtrl*)pLogWindow)->AppendText(mLogMessage);
   }

}
#if !defined(_WIN32)
// Dummy definition of JNI_LightButton() to prevent the reference in
// sipXcallLib from producing an error.
void JNI_LightButton(long)
{

}

#endif /* !defined(_WIN32) */

