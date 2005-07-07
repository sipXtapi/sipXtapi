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
#include "sipXezPhoneFrame.h"
#include "MainPanel.h"
#include "sipXezPhoneAboutDlg.h"
#include "sipXezPhoneSettingsDlg.h"
#include "sipXezPhoneSettings.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
enum
{
    ID_Quit = 1,
    ID_About = 2,
    ID_Configuration = 3,
    ID_Help = 4,
    ID_Minimal = 5,
};

// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS
BEGIN_EVENT_TABLE(sipXezPhoneFrame, wxFrame)
    EVT_MENU(ID_Configuration,  sipXezPhoneFrame::OnConfiguration)
    EVT_MENU(ID_About, sipXezPhoneFrame::OnAbout)
    EVT_MENU(ID_Minimal, sipXezPhoneFrame::OnMinimalView)
    EVT_CLOSE(sipXezPhoneFrame::OnClose)
END_EVENT_TABLE()

// Constructor
sipXezPhoneFrame::sipXezPhoneFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame((wxFrame *)NULL, -1, title, pos, size,  wxMINIMIZE_BOX | wxCAPTION | wxSIMPLE_BORDER | wxSYSTEM_MENU  )
{
   // Create the help menu
	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append( ID_About, "&About...");

    // Create the Settings Menu
    mpMenuSettings = new wxMenu;
    mpMenuSettings->Append(ID_Configuration, "&Configuration");
    mpMenuSettings->Append(ID_Minimal, "Minimal &View");

    // Set initial view state to normal
    mBoolMinimalView = false;

    // Create the Menu Bar, and append to it.
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( mpMenuSettings, "&Settings" );
    menuBar->Append( menuHelp, "&Help");
    SetMenuBar(menuBar);

    // Create the Main Panel
    wxPoint origin(0, 50);
    MainPanel* mainPanel = new MainPanel(this, origin, size);
    mainPanel->Show();
}

// Event handler for the Configuration menu item
void sipXezPhoneFrame::OnConfiguration(wxCommandEvent& WXUNUSED(event))
{
   sipXezPhoneSettingsDlg *pDlg = new sipXezPhoneSettingsDlg(this, -1, "Configuration Settings");
   pDlg->ShowModal();
   delete pDlg;
}

// Event handler for the Minimal/Normal View menu item
void sipXezPhoneFrame::OnMinimalView(wxCommandEvent& WXUNUSED(event))
{
    mBoolMinimalView = ! mBoolMinimalView;
    if ( mBoolMinimalView )
    {
        mpMenuSettings->SetLabel(ID_Minimal, "Normal &View");
        SetSize(wxSize(228,175));
    }
    else
    {
        mpMenuSettings->SetLabel(ID_Minimal, "Minimal &View");
        SetSize(wxSize(270,378));
    }

}

// Event hanlder for the About menu item
void sipXezPhoneFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
   sipXezPhoneAboutDlg *pDlg = new sipXezPhoneAboutDlg(this, -1, "About sipXezPhone");
   pDlg->ShowModal();
   delete pDlg;
}

void sipXezPhoneFrame::OnClose(wxCloseEvent& event)
{
   //sipXezPhoneSettings::getInstance().saveSettings();
   wxWindow::Destroy();
}
