//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "stdwx.h"
#include "MainPanel.h"
#include "sipXmgr.h"
#include "sipXezPhoneSettings.h"
#include "sipXezPhoneSettingsDlg.h"
#include "states/PhoneStateMachine.h"
#include "states/PhoneStateConnected.h"
#include "states/PhoneStateCallHeldLocally.h"
#include "states/PhoneStateCallHeldRemotely.h"
#include "sipXezPhoneApp.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS
BEGIN_EVENT_TABLE(MainPanel, wxPanel)
    EVT_INIT_DIALOG(MainPanel::OnInitDialog)
    EVT_BUTTON(IDR_CALL_HISTORY_BUTTON, MainPanel::OnCallHistoryButton)
    EVT_BUTTON(IDR_CONFERENCING_BUTTON, MainPanel::OnConferencingButton)
    EVT_BUTTON(IDR_VIDEO_BUTTON,        MainPanel::OnVideoButton)
END_EVENT_TABLE()

// Constructor
MainPanel::MainPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   wxPanel(parent, IDR_MAINPANEL, pos, size, wxTAB_TRAVERSAL, "MainPanel")
{
    // load the settings
    sipXezPhoneSettings::getInstance().loadSettings();

    wxColor* pPanelColor = & (sipXezPhoneSettings::getInstance().getBackgroundColor());
    SetBackgroundColour(*pPanelColor);

    // create and add the Volume Controls
    wxPoint origin(215,0);
    wxSize panelSize(190, 150);
    mpVolumeControlPanel = new VolumeControlPanel(this, origin, panelSize,
            sipXezPhoneSettings::getInstance().getSpeakerVolume(), sipXezPhoneSettings::getInstance().getMicGain());

    // create and add the Dial Pad Panel
    origin.x = 37;
    origin.y = 134;
    panelSize.SetWidth(174);
    panelSize.SetHeight(148);
    mpDialPadPanel = new DialPadPanel(this, origin, panelSize);

    // create and add the Dial Entry
    origin.x = 11;
    origin.y = 74;
    panelSize.SetWidth(201);
    panelSize.SetHeight(53);
    mpDialEntryPanel = new DialEntryPanel(this, origin, panelSize);

    // create and add the Button Panel
    origin.x = 30;
    origin.y = 37;
    panelSize.SetWidth(175);
    panelSize.SetHeight(33);
    mpButtonPanel = new ButtonPanel(this, origin, panelSize);

    // add the Call Status box
    origin.x = 3;
    origin.y = 10;
    panelSize.SetWidth(209);
    panelSize.SetHeight(19);
    mpCallStatus = new wxTextCtrl(this, IDR_STATUS_BOX, "Idle.", origin, panelSize, wxTE_READONLY | wxTE_CENTRE);
    wxColor* pColor = wxTheColourDatabase->FindColour("BLUE");
    mpCallStatus->SetForegroundColour(*pColor);


    // add the Caller ID box
    origin.x = 11;
    origin.y = 285;
    panelSize.SetWidth(224);
    panelSize.SetHeight(45);
    mpCallerID = new wxTextCtrl(this, IDR_CALLERID_BOX, "", origin, panelSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_CENTRE);
    mpCallerID->SetForegroundColour(*pColor);


    // add a call history button
    CreateCallHistoryButton();

    // create conferencing button
    CreateConferencingButton();
    
    // create the video panel button
    CreateVideoButton();

    InitDialog();
}

// Destructor
MainPanel::~MainPanel()
{
   if (dynamic_cast<PhoneStateConnected*>(PhoneStateMachine::getInstance().getState()) ||   // if its in the connected or held state
       dynamic_cast<PhoneStateCallHeldLocally* >(PhoneStateMachine::getInstance().getState()) ||
       dynamic_cast<PhoneStateCallHeldRemotely* >(PhoneStateMachine::getInstance().getState()))
   {
      PhoneStateMachine::getInstance().OnFlashButton();
   }
   sipXmgr::getInstance().UnInitialize();
   sipXmgr::release();
}

// Event handler for the wxInitDialog event
void MainPanel::OnInitDialog(wxInitDialogEvent& event)
{

   // if settings are missing, open the dialog box
   if ("" == sipXezPhoneSettings::getInstance().getIdentity() ||
       "" == sipXezPhoneSettings::getInstance().getRealm() )
   {
      sipXezPhoneSettingsDlg *pDlg = new sipXezPhoneSettingsDlg((wxWindow*)this, -1, "Configuration Settings");
      pDlg->ShowModal();
      delete pDlg;
   }

   // initialize the sipXtapi layer
   sipXmgr::getInstance().Initialize();
}

void MainPanel::CreateCallHistoryButton()
{
    wxBitmap bitmap("res/CallHistory.bmp",wxBITMAP_TYPE_BMP);
    bitmap.SetMask(new wxMask(bitmap, * (wxTheColourDatabase->FindColour("RED"))));

    mpCallHistoryBtn = new wxBitmapButton(this, IDR_CALL_HISTORY_BUTTON, bitmap, wxPoint(0, 190), wxSize(bitmap.GetWidth(),bitmap.GetHeight()), 0);

    wxColor btnColor = sipXezPhoneSettings::getInstance().getBackgroundColor();
    mpCallHistoryBtn->SetBackgroundColour(btnColor);
}

void MainPanel::CreateVideoButton()
{
    wxBitmap bitmap("res/video.bmp",wxBITMAP_TYPE_BMP);
    bitmap.SetMask(new wxMask(bitmap, * (wxTheColourDatabase->FindColour("RED"))));

    mpVideoBtn = new wxBitmapButton(this, IDR_VIDEO_BUTTON, bitmap, wxPoint(0, 130), wxSize(bitmap.GetWidth(),bitmap.GetHeight()), 0);

    wxColor btnColor = sipXezPhoneSettings::getInstance().getBackgroundColor();
    mpVideoBtn->SetBackgroundColour(btnColor);
}

void MainPanel::CreateConferencingButton()
{
    wxBitmap bitmap("res/Conferencing.bmp",wxBITMAP_TYPE_BMP);
    bitmap.SetMask(new wxMask(bitmap, * (wxTheColourDatabase->FindColour("RED"))));

    mpConferencingBtn = new wxBitmapButton(this, IDR_CONFERENCING_BUTTON, bitmap, wxPoint(219, 150), wxSize(bitmap.GetWidth(),bitmap.GetHeight()), 0);

    wxColor btnColor = sipXezPhoneSettings::getInstance().getBackgroundColor();
    mpConferencingBtn->SetBackgroundColour(btnColor);
}

void MainPanel::OnConferencingButton(wxEvent& event)
{
    thePhoneApp->getFrame().setConferencingVisible(! thePhoneApp->getFrame().getConferencingVisible());
}

void MainPanel::OnCallHistoryButton(wxEvent& event)
{
    thePhoneApp->getFrame().setCallHistoryVisible(! thePhoneApp->getFrame().getCallHistoryVisible());
}

void MainPanel::OnVideoButton(wxEvent& event)
{
    if (!thePhoneApp->getFrame().getVideoVisible())
    {
        if (thePhoneApp->getFrame().getCallHistoryVisible())
        {
            thePhoneApp->getFrame().setCallHistoryVisible(false);
        }
    }
    thePhoneApp->getFrame().setVideoVisible(! thePhoneApp->getFrame().getVideoVisible());
}