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
#include "sipXmgr.h"

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
    EVT_MOVE(sipXezPhoneFrame::OnMove)
END_EVENT_TABLE()

// Constructor
sipXezPhoneFrame::sipXezPhoneFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame((wxFrame *)NULL, -1, title, pos, size,  wxMINIMIZE_BOX | wxCAPTION | wxSIMPLE_BORDER | wxSYSTEM_MENU  ),
         mpCallHistoryWnd(NULL),
         mpConferencingWnd(NULL),
         mConferencingVisible(false),
         mCallHistoryVisible(false)
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

    wxPoint appOrigin = this->GetPosition();
    // create an 'attached' Call History window
    mpCallHistoryWnd = new wxDialog(this, -1, "", wxPoint(appOrigin.x - 250, appOrigin.y + 45),  wxSize(250, 300), wxNO_3D, "");


    // add a call history panel
    wxSize panelSize;
    origin.x = 2;
    origin.y = 0;
    panelSize.SetWidth(245);
    panelSize.SetHeight(295);
    (new CallHistoryPanel(mpCallHistoryWnd, origin, panelSize))->Show();

    wxSize appSize = this->GetSize();
    mpConferencingWnd = new wxDialog(this, -1, "", wxPoint(appOrigin.x + appSize.GetWidth(), appOrigin.y + 45),  wxSize(250, 300), wxNO_3D, "");
}

// Event handler for the Configuration menu item
void sipXezPhoneFrame::OnConfiguration(wxCommandEvent& WXUNUSED(event))
{
    sipXezPhoneSettingsDlg *pDlg = new sipXezPhoneSettingsDlg(this, -1, "Configuration Settings");
    if(pDlg->ShowModal() == wxID_OK ){
        sipXmgr::getInstance().release(); //unregister old proxy and delete old line
        wxCheckBox* pCheck = (wxCheckBox*)sipXezPhoneSettingsDlg::FindWindowById(ID_ENABLE_RPORT_CTRL, this);
        sipXmgr::getInstance().Initialize(DEFAULT_UDP_PORT, DEFAULT_RTP_START_PORT, pCheck->GetValue()); //register new proxy and add new line
   }
   delete pDlg;

}

// Event handler for the Minimal/Normal View menu item
void sipXezPhoneFrame::OnMinimalView(wxCommandEvent& WXUNUSED(event))
{
    mBoolMinimalView = ! mBoolMinimalView;
    if ( mBoolMinimalView )
    {
        mpMenuSettings->SetLabel(ID_Minimal, "Normal &View");
        SetSize(wxSize(220,174));
    }
    else
    {
        mpMenuSettings->SetLabel(ID_Minimal, "Minimal &View");
        SetSize(wxSize(255,378));
    }
    positionPanels();

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

void sipXezPhoneFrame::OnMove(wxMoveEvent& moveEvent)
{
    positionPanels();
    moveEvent.Skip();
}

void sipXezPhoneFrame::positionPanels()
{
    if (mpCallHistoryWnd)
    {
        wxPoint appOrigin = this->GetPosition();
        // create an 'attached' Call History window
        mpCallHistoryWnd->Move(wxPoint(appOrigin.x - 250, appOrigin.y + 45));
    }
    if (mpConferencingWnd)
    {
        wxPoint appOrigin = this->GetPosition();
        wxSize  appSize = this->GetSize();
        // create an 'attached' Call History window
        mpConferencingWnd->Move(wxPoint(appOrigin.x + appSize.GetWidth(), appOrigin.y + 45));
    }
}

const bool sipXezPhoneFrame::getCallHistoryVisible() const
{
    return mCallHistoryVisible;
}

void sipXezPhoneFrame::setCallHistoryVisible(const bool bVisible)
{
    mCallHistoryVisible = bVisible;

    if (bVisible)
    {
        this->mpCallHistoryWnd->Show();
    }
    else
    {
        this->mpCallHistoryWnd->Hide();
    }
}

const bool sipXezPhoneFrame::getConferencingVisible() const
{
    return mConferencingVisible;
}

void sipXezPhoneFrame::setConferencingVisible(const bool bVisible)
{
    mConferencingVisible = bVisible;

    if (bVisible)
    {
        positionPanels();
        this->mpConferencingWnd->Show();
    }
    else
    {
        this->mpConferencingWnd->Hide();
    }
}
