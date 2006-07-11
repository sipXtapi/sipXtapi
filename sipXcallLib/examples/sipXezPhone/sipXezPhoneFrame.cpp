//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "stdwx.h"
#include "sipXezPhoneFrame.h"
#include "MainPanel.h"
#include "sipXezPhoneSettingsDlg.h"
#include "sipXAudioSettingsDlg.h"
#include "sipXezPhoneSettings.h"
#include "sipXVideoSettingsDlg.h"
#include "sipXSrtpSettingsDlg.h"
#include "EventLogDlg.h"
#include "sipXmgr.h"
#include "sipXezPhoneApp.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;

// CONSTANTS
enum
{
    ID_Quit = 1,
    ID_About,
    ID_Configuration,
    ID_Help,    
    ID_Minimal,
    ID_TestInfo,
    ID_TestTeardown,
    ID_TestLineRemove,
    ID_TestUnRegister,
    ID_AudioSettings,
    ID_TestPlayfile,
    ID_TestDNS,
    ID_TestPlayStop,
    ID_SipLog,
    ID_EventLog,
    ID_SysLog,
    ID_VideoSettings,
    ID_SrtpSettings,
    ID_AudioWizardTest
};

// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS
BEGIN_EVENT_TABLE(sipXezPhoneFrame, wxFrame)
    EVT_MENU(ID_Configuration,  sipXezPhoneFrame::OnConfiguration)
    EVT_MENU(ID_About, sipXezPhoneFrame::OnAbout)
    EVT_MENU(ID_Minimal, sipXezPhoneFrame::OnMinimalView)
    EVT_MENU(ID_TestInfo, sipXezPhoneFrame::OnTestInfo)
    EVT_MENU(ID_TestTeardown, sipXezPhoneFrame::OnTestTeardown)
    EVT_MENU(ID_TestLineRemove, sipXezPhoneFrame::OnTestLineRemove)
    EVT_MENU(ID_TestUnRegister, sipXezPhoneFrame::OnTestUnRegister)
    EVT_MENU(ID_TestPlayfile, sipXezPhoneFrame::OnPlayFile)
    EVT_MENU(ID_TestPlayStop, sipXezPhoneFrame::OnStopFile)
    EVT_MENU(ID_AudioSettings, sipXezPhoneFrame::OnAudioSettings)
#ifdef VOICE_ENGINE
    EVT_MENU(ID_AudioWizardTest, sipXezPhoneFrame::OnAudioWizardTest)
#endif    
    EVT_MENU(ID_VideoSettings, sipXezPhoneFrame::OnVideoSettings)
    EVT_MENU(ID_SrtpSettings, sipXezPhoneFrame::OnSrtpSettings)
    EVT_MENU(ID_TestDNS, sipXezPhoneFrame::OnTestDNS)
#ifdef VOICE_ENGINE
    EVT_MENU(ID_TestVoiceEngine, sipXezPhoneFrame::OnTestVoiceEngine)
#endif
    EVT_MENU(ID_SipLog, sipXezPhoneFrame::OnShowSipLog)
    EVT_MENU(ID_EventLog, sipXezPhoneFrame::OnShowEventLog)
    EVT_MENU(ID_SysLog, sipXezPhoneFrame::OnShowSysLog)
    EVT_CLOSE(sipXezPhoneFrame::OnClose)
    EVT_MOVE(sipXezPhoneFrame::OnMove)
END_EVENT_TABLE()


#ifdef _WIN32
#include <windows.h>
//LRESULT CALLBACK sipXezVideoProc(HWND, UINT, WPARAM, LPARAM); 
//LRESULT CALLBACK sipXezPreviewProc(HWND, UINT, WPARAM, LPARAM); 
#endif



// Constructor
sipXezPhoneFrame::sipXezPhoneFrame(const wxString& title, const wxPoint& pos, const wxSize& size) :
#ifdef _WIN32
   wxFrame((wxFrame *)NULL, -1, title, pos, size,  wxMINIMIZE_BOX | wxCAPTION | wxSIMPLE_BORDER | wxSYSTEM_MENU  | wxFRAME_FLOAT_ON_PARENT),
//   mhPreviewWnd(NULL),
//   mhVideoWnd(NULL),
#else
   wxFrame((wxFrame *)NULL, -1, title, pos, size,    wxCAPTION | wxFRAME_FLOAT_ON_PARENT),
#endif
   m_pAboutDlg(NULL),
   mpCallHistoryWnd(NULL),
   mpVideoPanel(NULL),
   mpConferencingWnd(NULL),
   mCallHistoryVisible(false),
   mConferencingVisible(false),
   mVideoVisible(false),
   mpEventLogDlg(NULL)
   
{
   // Create the help menu
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append( ID_About, "&About...");
    menuHelp->AppendSeparator() ;
    menuHelp->Append(ID_SipLog, "Show &SIP Log...") ;
    menuHelp->Append(ID_EventLog, "Show &Event Log...") ;
    menuHelp->Append(ID_SysLog, "Show S&ys Log...") ;

    // Create the Settings Menu
    mpMenuSettings = new wxMenu;
    mpMenuSettings->Append(ID_Configuration, "&Configuration");
    mpMenuSettings->Append(ID_AudioSettings, "&Audio Settings");
#ifdef VIDEO
    mpMenuSettings->Append(ID_VideoSettings, "&Video Settings");
#endif
    mpMenuSettings->Append(ID_SrtpSettings, "S&rtp Settings");
    mpMenuSettings->Append(ID_Minimal, "Minimal &View");
//    mpMenuSettings->Append(ID_Video, "Video Window");
//    mpMenuSettings->Append(ID_VideoPreview, "Preview Window");


    // Create the Test Menu (if in -test Mode)
    if (sipXezPhoneSettings::getInstance().getTestMode())
    {
        mpMenuTest = new wxMenu;
        mpMenuTest->Append(ID_TestInfo, "&sipXCallSendInfo Test");
        mpMenuTest->Append(ID_TestTeardown, "&Teardown Test");
        mpMenuTest->Append(ID_TestLineRemove, "sipxLineRemove");
        mpMenuTest->Append(ID_TestUnRegister, "sipxUnRegister");
        mpMenuTest->Append(ID_TestPlayfile, "Play test.wav");
        mpMenuTest->Append(ID_TestPlayStop, "Stop test.wav");
        mpMenuTest->Append(ID_TestDNS, "Toggle DNS SRV Lookups");
#ifdef VOICE_ENGINE
        mpMenuTest->Append(ID_TestVoiceEngine, "Voice Engine Test");
        mpMenuTest->Append(ID_AudioWizardTest, "Audio &Wizard Test");
#endif
    }
    
    // Set initial view state to normal
    mBoolMinimalView = false;

    // Create the Menu Bar, and append to it.
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( mpMenuSettings, "&Settings" );
    if (sipXezPhoneSettings::getInstance().getTestMode())
    {
        menuBar->Append(mpMenuTest, "&Test");
    }
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


    // create an 'attached' Video window
    mpVideoPanel = new wxDialog(this, -1, "", wxPoint(appOrigin.x - 330, appOrigin.y - 50),  wxSize(330, 495), wxNO_3D, "");

    // add a call history panel
    origin.x = 2;
    origin.y = 0;
    panelSize.SetWidth(330);
    panelSize.SetHeight(495);
    (new VideoPanel(mpVideoPanel, origin, panelSize))->Show();


    wxSize appSize = this->GetSize();
    
    mpConferencingWnd = new ConferencePanel(this, wxPoint(appOrigin.x + appSize.GetWidth(), appOrigin.y + 45),  wxSize(235, 300));
    if (sipXezPhoneSettings::getInstance().getTestMode())
    {
        setConferencingVisible(true);
    }  
#ifdef VIDEO
//    registerVideoWindowClasses();
#endif
}

// Event handler for the Configuration menu item
void sipXezPhoneFrame::OnConfiguration(wxCommandEvent& WXUNUSED(event))
{
    sipXezPhoneSettingsDlg *pDlg = new sipXezPhoneSettingsDlg(this, -1, "Configuration Settings");
    if(pDlg->ShowModal() == wxID_OK ){
        sipXmgr::getInstance().UnInitialize(); //unregister old proxy and delete old line
        wxCheckBox* pCheck = (wxCheckBox*)sipXezPhoneSettingsDlg::FindWindowById(ID_ENABLE_RPORT_CTRL, this);
        sipXmgr::getInstance().Initialize(DEFAULT_UDP_PORT, DEFAULT_RTP_START_PORT, pCheck->GetValue()); //register new proxy and add new line
   }
   delete pDlg;

}

// Event handler for the audio Settings menu item
void sipXezPhoneFrame::OnAudioSettings(wxCommandEvent& WXUNUSED(event))
{
    sipXAudioSettingsDlg *pDlg = new sipXAudioSettingsDlg(this, -1, "Audio Settings", wxDefaultPosition, wxSize(320, 330));
    if(pDlg->ShowModal() == wxID_OK )
    {
    
    }
    delete pDlg;
}

// Event handler for the video Settings menu item
void sipXezPhoneFrame::OnVideoSettings(wxCommandEvent& WXUNUSED(event))
{
    sipXVideoSettingsDlg *pDlg = new sipXVideoSettingsDlg(this, -1, "Video Settings", wxDefaultPosition, wxSize(320, 330));
    if(pDlg->ShowModal() == wxID_OK )
    {
    
    }
    delete pDlg;
}

// Event handler for the video Settings menu item
void sipXezPhoneFrame::OnSrtpSettings(wxCommandEvent& WXUNUSED(event))
{
    sipXSrtpSettingsDlg *pDlg = new sipXSrtpSettingsDlg(this, -1, "SRTP Settings", wxDefaultPosition, wxSize(320, 330));
    if(pDlg->ShowModal() == wxID_OK )
    {
    
    }
    delete pDlg;
}

void sipXezPhoneFrame::OnTestInfo(wxCommandEvent& WXUNUSED(event))
{
    // for testing
    SIPX_INFO hInfo = 0;
    void* pContent;
    pContent = malloc(256);
    memset(pContent, 42, 256);
    sipxCallSendInfo(&hInfo, sipXmgr::getInstance().getCurrentCall(), "text/plain", (char*)pContent, 256); 
}

void sipXezPhoneFrame::OnTestDNS(wxCommandEvent& WXUNUSED(event))
{
    static bool bEnable = false;
    sipxConfigEnableDnsSrv(bEnable);
    bEnable = !bEnable;
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
    if (m_pAboutDlg == NULL)
    {
        m_pAboutDlg = new sipXezPhoneAboutDlg(this, -1, "About sipXezPhone");
    }
    m_pAboutDlg->Show();
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
    if (mpVideoPanel)
    {
        wxPoint appOrigin = this->GetPosition();
        mpVideoPanel->Move(wxPoint(appOrigin.x - 330, appOrigin.y - 50));
    }
    if (mpCallHistoryWnd)
    {
        wxPoint appOrigin = this->GetPosition();
        mpCallHistoryWnd->Move(wxPoint(appOrigin.x - 250, appOrigin.y + 45));
    }
    if (mpConferencingWnd)
    {
        wxPoint appOrigin = this->GetPosition();
        wxSize  appSize = this->GetSize();
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


const bool sipXezPhoneFrame::getVideoVisible() const
{
    return mVideoVisible;
}

void sipXezPhoneFrame::setVideoVisible(const bool bVisible)
{
    mVideoVisible = bVisible;

    if (bVisible)
    {
        this->mpVideoPanel->Show();
    }
    else
    {
        this->mpVideoPanel->Hide();
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

void sipXezPhoneFrame::OnTestTeardown(wxCommandEvent& WXUNUSED(event))
{
    // for testing
    sipXmgr::getInstance().UnInitialize();

    sipXmgr::getInstance().Initialize();
}

void sipXezPhoneFrame::OnPlayFile(wxCommandEvent& WXUNUSED(event))
{
    SIPX_CALL hCall = sipXmgr::getInstance().getCurrentCall();

    sipxCallPlayFileStart(hCall, "res/ringTone.raw", true, true, true) ;
}

void sipXezPhoneFrame::OnStopFile(wxCommandEvent& WXUNUSED(event))
{
    SIPX_CALL hCall = sipXmgr::getInstance().getCurrentCall();

    sipxCallPlayFileStop(hCall) ;
}

void sipXezPhoneFrame::OnTestLineRemove(wxCommandEvent& WXUNUSED(event))
{
    // for testing
    sipXmgr::getInstance().removeCurrentLine();
}

void sipXezPhoneFrame::OnTestUnRegister(wxCommandEvent& WXUNUSED(event))
{
    // for testing
    sipXmgr::getInstance().UnRegister();
}

#ifdef VOICE_ENGINE
#include "../../../sipXbuild/vendors/gips/VoiceEngine/interface/GipsVoiceEngineLib.h"
#include "tapi/sipXtapiInternal.h"
void sipXezPhoneFrame::OnTestVoiceEngine(wxCommandEvent& WXUNUSED(event))
{

    GipsVoiceEngineLib* pLib = NULL;
    
    pLib = sipxCallGetVoiceEnginePtr(sipXmgr::getInstance().getCurrentCall());
    pLib->GIPSVE_SetSpeakerVolume(0);
}

void sipXezPhoneFrame::OnAudioWizardTest(wxCommandEvent& WXUNUSED(event))
{
    GIPSAECTuningWizard* pWizard = sipxConfigGetVoiceEngineAudioWizard();
    int rc = pWizard->GIPS_AEC_TW_Init(NULL, NULL);
    assert(rc == 0);
    rc = pWizard->GIPS_AEC_TW_Terminate();
    assert(rc == 0);
}

#endif


// EventHandler for the Show SIP Log menu item.
void sipXezPhoneFrame::OnShowSipLog(wxCommandEvent& event)
{
    wxMessageBox("Not yet implemented.", "Sorry", wxOK, this) ;
}


// EventHandler for the Show Event Log menu item.
void sipXezPhoneFrame::OnShowEventLog(wxCommandEvent& event)
{
    if (mpEventLogDlg == NULL)
    {
        mpEventLogDlg = new EventLogDlg(this, -1, "Event Log") ;
    }

    mpEventLogDlg->Show() ;
}

// EventHandler for the Show Sys Log menu item.
void sipXezPhoneFrame::OnShowSysLog(wxCommandEvent& event) 
{
    wxMessageBox("Not yet implemented.", "Sorry", wxOK, this) ;
}

