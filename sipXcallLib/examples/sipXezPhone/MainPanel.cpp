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
#include "MainPanel.h"
#include "sipXmgr.h"
#include "sipXezPhoneSettings.h"
#include "sipXezPhoneSettingsDlg.h"
#include "states/PhoneStateMachine.h"
#include "states/PhoneStateConnected.h"
#include "states/PhoneStateCallHeldLocally.h"
#include "states/PhoneStateCallHeldRemotely.h"
#include "sipXezPhoneApp.h"

#define TIMER_ID    1000

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
    EVT_TIMER(TIMER_ID,                 MainPanel::OnTimer)
END_EVENT_TABLE()

// Constructor
MainPanel::MainPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size, bool bLogo) :
   wxPanel(parent, IDR_MAINPANEL, pos, size, wxTAB_TRAVERSAL, "MainPanel"),
   mpConferencingBtn(NULL),
   mpVideoBtn(NULL),
   m_timer(this, TIMER_ID) 
{
    wxColor* pPanelColor = & (sipXezPhoneSettings::getInstance().getBackgroundColor());
    SetBackgroundColour(*pPanelColor);

    // create and add the Volume Controls
    wxPoint origin(215,0);
    wxSize panelSize(190, 180);
    if (bLogo)
    {
        panelSize.SetHeight(180);
    }
    else
    {
        panelSize.SetHeight(150);
    }
    mpVolumeControlPanel = new VolumeControlPanel(this, origin, panelSize,
            sipXezPhoneSettings::getInstance().getSpeakerVolume(), sipXezPhoneSettings::getInstance().getMicGain());

    // create and add the Dial Pad Panel
    origin.x = 37;
    if (bLogo)
    {
        origin.y = 207;
    }
    else
    {
        origin.y = 134;
    }
    panelSize.SetWidth(174);
    panelSize.SetHeight(148);
    mpDialPadPanel = new DialPadPanel(this, origin, panelSize);

    // create and add the Dial Entry
    origin.x = 11;
    if (bLogo)
    {
        origin.y = 148;
    }
    else
    {
        origin.y = 74;
    }
    panelSize.SetWidth(201);
    panelSize.SetHeight(53);
    mpDialEntryPanel = new DialEntryPanel(this, origin, panelSize);

    // create and add the Button Panel
    if (bLogo)
    {
        origin.x = 110;
        origin.y = 52;
        panelSize.SetWidth(100);
        panelSize.SetHeight(80);
    }
    else
    {
        origin.x = 30;
        origin.y = 37;
        panelSize.SetWidth(175);
        panelSize.SetHeight(33);
    }
    mpButtonPanel = new ButtonPanel(this, origin, panelSize, bLogo);

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
    if (bLogo)
    {
        origin.y = 363;
    }
    else
    {
        origin.y = 285;
    }
    panelSize.SetWidth(224);
    panelSize.SetHeight(45);
    mpCallerID = new wxTextCtrl(this, IDR_CALLERID_BOX, "", origin, panelSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_CENTRE);
    mpCallerID->SetForegroundColour(*pColor);


    // add a call history button
    CreateCallHistoryButton(bLogo);

    // create conferencing button
    CreateConferencingButton(bLogo);
    
    // create the video panel button
    CreateVideoButton(bLogo);

    mpLogo = NULL;
    if (bLogo)
    {
        origin.x = 8;
        origin.y = 34;
        panelSize.SetHeight(108);
        panelSize.SetWidth(88);
        wxBitmap* pLogoBitmap = new wxBitmap("res/PhoneLogo.bmp", wxBITMAP_TYPE_BMP );
        pLogoBitmap->SetMask(new wxMask(*pLogoBitmap, * (wxTheColourDatabase->FindColour("RED"))));
        mpLogo = new wxStaticBitmap(this, -1, *pLogoBitmap, origin, panelSize);
        new wxStaticBox(this, -1, wxT(""), wxPoint(103, 36), wxSize(105, 102), wxALIGN_LEFT);
    }

    InitDialog();

    m_timer.Start(250) ;
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
   // save settings now
   sipXezPhoneSettings::getInstance().saveSettings();
   
   // uninitialize sipXtapi
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
   int iSipPort, iRtpPort;
   bool bRport;

   sipXezPhoneSettings::getInstance().getDefaultPorts(iSipPort, iRtpPort);
   bRport = sipXezPhoneSettings::getInstance().getUseRport();

   sipXmgr::getInstance().Initialize(iSipPort, iRtpPort, bRport);
}

void MainPanel::CreateCallHistoryButton(bool bLogo)
{
    wxBitmap bitmap("res/CallHistory.bmp",wxBITMAP_TYPE_BMP);
    bitmap.SetMask(new wxMask(bitmap, * (wxTheColourDatabase->FindColour("RED"))));

    wxBitmap bitmapDown("res/CallHistory_down.bmp",wxBITMAP_TYPE_BMP);
    bitmapDown.SetMask(new wxMask(bitmapDown, * (wxTheColourDatabase->FindColour("RED"))));

    if (bLogo)
    {
        mpCallHistoryBtn = new wxBitmapButton(this, IDR_CALL_HISTORY_BUTTON, bitmap, wxPoint(0, 270), wxSize(bitmap.GetWidth(),bitmap.GetHeight()), 0);
    }
    else
    {
        mpCallHistoryBtn = new wxBitmapButton(this, IDR_CALL_HISTORY_BUTTON, bitmap, wxPoint(0, 190), wxSize(bitmap.GetWidth(),bitmap.GetHeight()), 0);
    }
    mpCallHistoryBtn->SetBitmapSelected(bitmapDown);

    wxColor btnColor = sipXezPhoneSettings::getInstance().getBackgroundColor();
    mpCallHistoryBtn->SetBackgroundColour(btnColor);
}

void MainPanel::CreateVideoButton(bool bLogo)
{
    wxBitmap bitmap("res/video.bmp",wxBITMAP_TYPE_BMP);
    bitmap.SetMask(new wxMask(bitmap, * (wxTheColourDatabase->FindColour("RED"))));
    wxBitmap bitmapDown("res/video_down.bmp",wxBITMAP_TYPE_BMP);
    bitmapDown.SetMask(new wxMask(bitmapDown, * (wxTheColourDatabase->FindColour("RED"))));

    if (bLogo)
    {
        mpVideoBtn = new wxBitmapButton(this, IDR_VIDEO_BUTTON, bitmap, wxPoint(0, 210), wxSize(bitmap.GetWidth(),bitmap.GetHeight()), 0);
    }
    else
    {
        mpVideoBtn = new wxBitmapButton(this, IDR_VIDEO_BUTTON, bitmap, wxPoint(0, 130), wxSize(bitmap.GetWidth(),bitmap.GetHeight()), 0);
    }
    mpVideoBtn->SetBitmapSelected(bitmapDown);

    wxColor btnColor = sipXezPhoneSettings::getInstance().getBackgroundColor();
    mpVideoBtn->SetBackgroundColour(btnColor);
}

void MainPanel::CreateConferencingButton(bool bLogo)
{
    wxBitmap bitmap("res/Conferencing.bmp",wxBITMAP_TYPE_BMP);
    bitmap.SetMask(new wxMask(bitmap, * (wxTheColourDatabase->FindColour("RED"))));

    if (sipXezPhoneSettings::getInstance().getTestMode())
    {
        if (bLogo)
        {
            mpConferencingBtn = new wxBitmapButton(this, IDR_CONFERENCING_BUTTON, bitmap, wxPoint(219, 230), wxSize(bitmap.GetWidth(),bitmap.GetHeight()), 0);
        }
        else
        {
            mpConferencingBtn = new wxBitmapButton(this, IDR_CONFERENCING_BUTTON, bitmap, wxPoint(219, 150), wxSize(bitmap.GetWidth(),bitmap.GetHeight()), 0);
        }

        wxColor btnColor = sipXezPhoneSettings::getInstance().getBackgroundColor();
        mpConferencingBtn->SetBackgroundColour(btnColor);
    }
}

void MainPanel::OnConferencingButton(wxCommandEvent& event)
{
    thePhoneApp->getFrame().setConferencingVisible(! thePhoneApp->getFrame().getConferencingVisible());
}

void MainPanel::OnCallHistoryButton(wxCommandEvent& event)
{
    thePhoneApp->getFrame().setCallHistoryVisible(! thePhoneApp->getFrame().getCallHistoryVisible());
}

void MainPanel::OnVideoButton(wxCommandEvent& event)
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

void MainPanel::UpdateBackground()
{
    wxColor btnColor = sipXezPhoneSettings::getInstance().getBackgroundColor();

    mpVideoBtn->SetBackgroundColour(btnColor);
    if (mpConferencingBtn)
    {
        mpConferencingBtn->SetBackgroundColour(btnColor);
    }
    mpCallHistoryBtn->SetBackgroundColour(btnColor);

    mpVolumeControlPanel->UpdateBackground(btnColor);
    mpDialPadPanel->UpdateBackground(btnColor);
    mpDialEntryPanel->UpdateBackground(btnColor);
    mpButtonPanel->UpdateBackground(btnColor);
    if (mpLogo)
    {
        mpLogo->SetBackgroundColour(btnColor);
    }
    SetBackgroundColour(btnColor);
    Refresh();
    mpDialEntryPanel->SetFocus();
}


void MainPanel::OnTimer(wxTimerEvent& event)
{
    /*
    SIPX_CALL hCall = sipXmgr::getInstance().getCurrentCall() ;
    if (hCall != SIPX_CALL_NULL)
    {
        int inputEL = 0 ;
        int outputEL = 0 ;
        size_t nMaxContributors = 8 ;
        unsigned int CCSRCs[8] ;
        int iEnergyLevels[8] ;
        size_t nActualContributors ;


        char cTemp[1024] ;

        if (sipxCallGetEnergyLevels(hCall, inputEL, outputEL, nMaxContributors, 
                CCSRCs, iEnergyLevels, nActualContributors) == SIPX_RESULT_SUCCESS)
        {
            sprintf(cTemp, "Call: IEL: %d OEL: %d\n", inputEL, outputEL) ;
            OutputDebugString(cTemp) ;
        }

        unsigned int sendSSRC, receiveSSRC ;

        if (sipxCallGetAudioRtpSourceIds(hCall, sendSSRC, receiveSSRC) == SIPX_RESULT_SUCCESS)
        {
            sprintf(cTemp, "Sending SSRC: 0x%08X Receiving SSRC: 0x%08X\n", sendSSRC, receiveSSRC) ;
            OutputDebugString(cTemp) ;
        }


        SIPX_RTCP_STATS stats ;
        stats.cbSize = sizeof(SIPX_RTCP_STATS) ;
        if (sipxCallGetAudioRtcpStats(hCall, &stats) == SIPX_RESULT_SUCCESS)
        {            
            sprintf(cTemp, "br: %d bs: %d cl: %d j: %d RTT: %d\n",
                    stats.bytesReceived,
                    stats.bytesSent,
                    stats.cum_lost,
                    stats.jitter,
                    stats.RTT) ;
            OutputDebugString(cTemp) ;
        }
    }

    SIPX_CONF hConf = sipXmgr::getInstance().getCurrentConference() ;
    if (hConf)
    {
        char cTemp[1024] ;
        int inputEL = 0 ;
        int outputEL = 0 ;


        if (sipxConferenceGetEnergyLevels(hConf, inputEL, outputEL) == SIPX_RESULT_SUCCESS)
        {
            sprintf(cTemp, "Conf: IEL: %d OEL: %d\n", inputEL, outputEL) ;
            OutputDebugString(cTemp) ;
        }
    }
*/
}
