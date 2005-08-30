//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "stdwx.h"
#include "sipXezPhoneApp.h"
#include "sipXezPhoneFrame.h"
#include "sipXezPhoneSettings.h"



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
    
    // create a new frame
    mpFrame = new sipXezPhoneFrame( "sipXezPhone", wxDefaultPosition, wxSize(255,378) );

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

void sipXezPhoneApp::addLogMessage(const UtlString message)
{
#ifdef _WIN32 // appending to a scrollable window causes
              // a crash on Linux, if the call originates
              // from a thread other than the main
              // UI thread
   wxWindow* pLogWindow = wxWindow::FindWindowById(IDR_CALLERID_BOX, GetTopWindow());
   if (pLogWindow)
   {
      ((wxTextCtrl*)pLogWindow)->AppendText(message.data());
   }
#endif
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


#if !defined(_WIN32)
// Dummy definition of JNI_LightButton() to prevent the reference in
// sipXcallLib from producing an error.
void JNI_LightButton(long)
{

}

#endif /* !defined(_WIN32) */

