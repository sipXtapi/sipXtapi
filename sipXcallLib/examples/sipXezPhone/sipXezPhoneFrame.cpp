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
#include <wx/colordlg.h>
#include "sipXezPhoneFrame.h"
#include "sipXezPhoneSettingsDlg.h"
#include "sipXAudioSettingsDlg.h"
#include "sipXezPhoneSettings.h"
#include "sipXVideoSettingsDlg.h"
#include "sipXSrtpSettingsDlg.h"
#include "sipXTabbedDlg.h"
#include "EventLogDlg.h"
#include "ImportCertificateDlg.h"
#include "CameraSettingsDlg.h"
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
    ID_ImportCertificate,
    ID_CameraSettings,
    ID_AudioWizardTest,
    ID_ContactSubMenu,
    ID_UseAutoContact,
    ID_UseLocalContact,
    ID_UseNatContact,
    ID_UseRelayContact,
    ID_DisplayContacts,
    ID_ColorDialog,
    ID_TestTabs,
    ID_AVWizard,
    ID_RenegotiateCodecs
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
    EVT_MENU(ID_TestTabs, sipXezPhoneFrame::OnTabs)
    EVT_MENU(ID_AudioSettings, sipXezPhoneFrame::OnAudioSettings)
    EVT_MENU(ID_AVWizard, sipXezPhoneFrame::OnAVWizard)
    EVT_MENU(ID_RenegotiateCodecs, sipXezPhoneFrame::OnRenegotiate)
#ifdef VOICE_ENGINE
    EVT_MENU(ID_AudioWizardTest, sipXezPhoneFrame::OnAudioWizardTest)
#endif    
    EVT_MENU(ID_VideoSettings, sipXezPhoneFrame::OnVideoSettings)
    EVT_MENU(ID_SrtpSettings, sipXezPhoneFrame::OnSrtpSettings)
    EVT_MENU(ID_ImportCertificate, sipXezPhoneFrame::OnImportCertificate)
    EVT_MENU(ID_CameraSettings, sipXezPhoneFrame::OnCameraSettings)
    EVT_MENU(ID_TestDNS, sipXezPhoneFrame::OnTestDNS)
#ifdef VOICE_ENGINE
    EVT_MENU(ID_TestVoiceEngine, sipXezPhoneFrame::OnTestVoiceEngine)
#endif
    EVT_MENU(ID_SipLog, sipXezPhoneFrame::OnShowSipLog)
    EVT_MENU(ID_EventLog, sipXezPhoneFrame::OnShowEventLog)
    EVT_MENU(ID_SysLog, sipXezPhoneFrame::OnShowSysLog)
    EVT_MENU(ID_UseAutoContact, sipXezPhoneFrame::OnSelectContact)
    EVT_MENU(ID_UseLocalContact, sipXezPhoneFrame::OnSelectContact)
    EVT_MENU(ID_UseNatContact, sipXezPhoneFrame::OnSelectContact)
    EVT_MENU(ID_UseRelayContact, sipXezPhoneFrame::OnSelectContact)
    EVT_UPDATE_UI(ID_UseAutoContact, sipXezPhoneFrame::OnUpdateContact)
    EVT_UPDATE_UI(ID_UseLocalContact, sipXezPhoneFrame::OnUpdateContact)
    EVT_UPDATE_UI(ID_UseNatContact, sipXezPhoneFrame::OnUpdateContact)
    EVT_UPDATE_UI(ID_UseRelayContact, sipXezPhoneFrame::OnUpdateContact)
    EVT_MENU(ID_DisplayContacts, sipXezPhoneFrame::OnDisplayContacts)
    EVT_MENU(ID_ColorDialog, sipXezPhoneFrame::OnColorDialog)
    
    EVT_CLOSE(sipXezPhoneFrame::OnClose)
    EVT_MOVE(sipXezPhoneFrame::OnMove)
END_EVENT_TABLE()


#ifdef _WIN32
#include <windows.h>
//LRESULT CALLBACK sipXezVideoProc(HWND, UINT, WPARAM, LPARAM); 
//LRESULT CALLBACK sipXezPreviewProc(HWND, UINT, WPARAM, LPARAM); 
#endif



// Constructor
sipXezPhoneFrame::sipXezPhoneFrame(const wxString& title, const wxPoint& pos, const wxSize& size, bool bLogo) :
#ifdef _WIN32
   wxFrame((wxFrame *)NULL, -1, title, pos, size,  wxMINIMIZE_BOX | wxCAPTION | wxSIMPLE_BORDER | wxSYSTEM_MENU  | wxFRAME_FLOAT_ON_PARENT),
//   mhPreviewWnd(NULL),
//   mhVideoWnd(NULL),
#else
   wxFrame((wxFrame *)NULL, -1, title, pos, size,  wxCAPTION  | wxCLOSE_BOX | wxFRAME_FLOAT_ON_PARENT),
//   wxFrame((wxFrame *)NULL, -1, title, pos, size, wxCAPTION | wxFRAME_FLOAT_ON_PARENT),
#endif
   m_pAboutDlg(NULL),
   mpCallHistoryWnd(NULL),
   mpCallHistory(NULL),
   mpVideoPanel(NULL),
   mpMainPanel(NULL),
   mpVideoWindow(NULL),
   mpConferencingWnd(NULL),
   mCallHistoryVisible(false),
   mConferencingVisible(false),
   mVideoVisible(false),
   mpEventLogDlg(NULL),
   mOrigHeight(size.GetHeight()),
   mOrigWidth(size.GetWidth()),
   mbLogo(bLogo),
   mpMenuContactSelection(NULL)  
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
    mpMenuSettings->Append(ID_AVWizard, "Audio/Video &Wizard");
    mpMenuSettings->Append(ID_Configuration, "&Configuration");
    mpMenuSettings->Append(ID_AudioSettings, "&Audio Settings");
#ifdef VIDEO
    mpMenuSettings->Append(ID_VideoSettings, "&Video Settings");
#endif
    mpMenuSettings->Append(ID_SrtpSettings, "Secu&rity Settings");
    mpMenuSettings->Append(ID_ImportCertificate, "&Import Certificate");
    mpMenuSettings->Append(ID_CameraSettings, "Ca&mera Settings");
    mpMenuSettings->Append(ID_Minimal, "Minimal &View");
    mpMenuSettings->Append(ID_ColorDialog, "Set background color");
//    mpMenuSettings->Append(ID_Video, "Video Window");
//    mpMenuSettings->Append(ID_VideoPreview, "Preview Window");

    // Create the Test Menu (if in -test Mode)
    if (sipXezPhoneSettings::getInstance().getTestMode())
    {
        mpMenuTest = new wxMenu;

        mpMenuContactSelection = new wxMenu ;
        mpMenuContactSelection->AppendCheckItem(ID_UseAutoContact, "Auto") ;
        mpMenuContactSelection->AppendCheckItem(ID_UseLocalContact, "Local") ;
        mpMenuContactSelection->AppendCheckItem(ID_UseNatContact, "NAT-derived") ;
        mpMenuContactSelection->AppendCheckItem(ID_UseRelayContact, "TURN-derived") ;
        mpMenuTest->Append(ID_ContactSubMenu, "Select Contact", mpMenuContactSelection) ;

        mpMenuTest->Append(ID_DisplayContacts, "Display Contact List") ;

        mpMenuTest->Append(ID_TestInfo, "&sipXCallSendInfo Test");
        mpMenuTest->Append(ID_TestTeardown, "&Teardown Test");
        mpMenuTest->Append(ID_TestLineRemove, "sipxLineRemove");
        mpMenuTest->Append(ID_TestUnRegister, "sipxUnRegister");
        mpMenuTest->Append(ID_TestPlayfile, "Play test.wav");
        mpMenuTest->Append(ID_TestPlayStop, "Stop test.wav");
        mpMenuTest->Append(ID_TestDNS, "Toggle DNS SRV Lookups");
        mpMenuTest->Append(ID_TestTabs, "Test Tabs");
#ifdef VOICE_ENGINE
        mpMenuTest->Append(ID_TestVoiceEngine, "Voice Engine Test");
        mpMenuTest->Append(ID_AudioWizardTest, "Audio &Wizard Test");
#endif
        mpMenuTest->Append(ID_RenegotiateCodecs, "Renegotiate H263 video codec");
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
    mpMainPanel = new MainPanel(this, origin, size, bLogo);
    mpMainPanel->Show();

    wxPoint appOrigin = this->GetPosition();
    // create an 'attached' Call History window
    mpCallHistory = new wxDialog(this, -1, "", wxPoint(appOrigin.x - 325, appOrigin.y + 45),  wxSize(325, 300), wxNO_3D, "");

    // add a call history panel
    wxSize panelSize;
    origin.x = 2;
    origin.y = 0;
    panelSize.SetWidth(320);
    panelSize.SetHeight(295);
    mpCallHistoryWnd = new CallHistoryPanel(mpCallHistory, origin, panelSize);
    mpCallHistoryWnd->Show();


    // create an 'attached' Video window
    mpVideoPanel = new wxDialog(this, -1, "", wxPoint(appOrigin.x - 330, appOrigin.y - 50),  wxSize(330, 495), wxNO_3D, "");

    // add a video panel
    origin.x = 2;
    origin.y = 0;
    panelSize.SetWidth(330);
    panelSize.SetHeight(495);
    mpVideoWindow = new VideoPanel(mpVideoPanel, origin, panelSize);
    mpVideoWindow->Show();


    wxSize appSize = this->GetSize();
    
    if (sipXezPhoneSettings::getInstance().getTestMode())    
    {
        mpConferencingWnd = new ConferencePanel(this, wxPoint(appOrigin.x + appSize.GetWidth(), appOrigin.y + 45),  wxSize(235, 300));
        setConferencingVisible(true);
    }

    // load the settings
    autoPosition(POSITION_INIT);
    Raise();

#ifdef VIDEO
//    registerVideoWindowClasses();
#endif
}

// Event handler for the Configuration menu item
void sipXezPhoneFrame::OnConfiguration(wxCommandEvent& WXUNUSED(event))
{
    wxString x;
    wxTextCtrl* pText;
    long lSipPort, lRtpPort;

    sipXezPhoneSettingsDlg *pDlg = new sipXezPhoneSettingsDlg(this, -1, "Configuration Settings");
    if(pDlg->ShowModal() == wxID_OK ){
        sipXmgr::getInstance().UnInitialize(); //unregister old proxy and delete old line
        wxCheckBox* pCheck = (wxCheckBox*)sipXezPhoneSettingsDlg::FindWindowById(ID_ENABLE_RPORT_CTRL, this);
        pText = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_SIP_PORT, this);
        x = pText->GetValue();
        x.ToLong(&lSipPort);
        pText = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_RTP_PORT, this);
        x = pText->GetValue();
        x.ToLong(&lRtpPort);
        sipXmgr::getInstance().Initialize((int)lSipPort, (int)lRtpPort, pCheck->GetValue()); //register new proxy and add new line
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
void sipXezPhoneFrame::OnColorDialog(wxCommandEvent& WXUNUSED(event))
{
    wxColourDialog *pDlg = new wxColourDialog(this);
    wxColourData data;
    wxColour color;

    if(pDlg->ShowModal() == wxID_OK )
    {
        data = pDlg->GetColourData();
        color = data.GetColour();
        sipXezPhoneSettings::getInstance().setPhoneBackground(color.Red(), color.Green(), color.Blue());
        mpMainPanel->UpdateBackground();
        if (mpConferencingWnd)
        {
            mpConferencingWnd->UpdateBackground(color);
            mpConferencingWnd->Refresh();
        }
        mpVideoWindow->UpdateBackground(color);
        mpVideoWindow->Refresh();
        mpCallHistoryWnd->UpdateBackground(color);
        mpCallHistoryWnd->Refresh();
    }
    delete pDlg;
}

// Event handler for the tabbed Settings menu item
void sipXezPhoneFrame::OnTabs(wxCommandEvent& WXUNUSED(event))
{
    int pwidth = 500;
    int pheight = 400;
    int x, y, width, height, px, py;

    ::wxClientDisplayRect(&x, &y, &width, &height);
    px = (width/2) - (pwidth/2);
    py = (height/2) - (pheight/2);

    sipXTabbedDlg *pDlg = new sipXTabbedDlg(this, -1, "Settings", wxPoint(px, py), wxSize(pwidth, pheight));

    if(pDlg->ShowModal() == wxID_OK )
    {
    }
    delete pDlg;
}

// Event handler for the video Settings menu item
void sipXezPhoneFrame::OnSrtpSettings(wxCommandEvent& WXUNUSED(event))
{
    sipXSrtpSettingsDlg *pDlg = new sipXSrtpSettingsDlg(this, -1, "Security Settings", wxDefaultPosition, wxSize(320, 420));

    sipXezPhoneSettings::getInstance().getSecurityEnabled(pDlg->mbEncryption);
    sipXezPhoneSettings::getInstance().getSrtpParameters(pDlg->mSecurityLevel);
    UtlString dummy;
    sipXezPhoneSettings::getInstance().getSmimeParameters(dummy, pDlg->mCertNickname, pDlg->mCertDbPassword);

    sipXmgr::getInstance().getSrtpKey(pDlg->mSrtpKey);
    pDlg->InitializeControls();

    if(pDlg->ShowModal() == wxID_OK )
    {
        sipXezPhoneSettings::getInstance().setSecurityEnabled(pDlg->mbEncryption);
        sipXezPhoneSettings::getInstance().setSrtpParameters(pDlg->mSecurityLevel);
        UtlString dbLocation = "." ;
        sipXezPhoneSettings::getInstance().setSmimeParameters(dbLocation, pDlg->mCertNickname, pDlg->mCertDbPassword);
        sipXmgr::getInstance().setSrtpKey(pDlg->mSrtpKey);
    }
    delete pDlg;
}

void sipXezPhoneFrame::OnImportCertificate(wxCommandEvent& WXUNUSED(event))
{
    ImportCertificateDlg *pDlg = new ImportCertificateDlg(this, -1, "Import Certificate");
    pDlg->ShowModal();
}

void sipXezPhoneFrame::OnCameraSettings(wxCommandEvent& WXUNUSED(event))
{
    CameraSettingsDlg *pDlg = new CameraSettingsDlg(this, -1, "Camera Selection");
    pDlg->ShowModal();
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


void sipXezPhoneFrame::OnAVWizard(wxCommandEvent& WXUNUSED(event))
{
#ifdef WIN32
    PROCESS_INFORMATION pi ;
    STARTUPINFO si ;
    memset(&si, 0, sizeof(si)) ;
    si.cb = sizeof(si) ;

    CreateProcess("AVWizard.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) ;

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess ) ;
    CloseHandle( pi.hThread ) ; 

    sipXezPhoneSettings::getInstance().loadSettings() ;

    wxMessageBox("Please restart sipXezPhone to apply any changes from the Wizard.", "sipXezPhone", wxOK, this) ;

    sipXezPhoneSettings::getInstance().loadSettings() ;

/*
    int iSipPort, iRtpPort ;
    bool bUseRport ;

    sipXmgr::getInstance().UnInitialize() ;

    sipXezPhoneSettings::getInstance().getDefaultPorts(iSipPort, iRtpPort);
    bUseRport = sipXezPhoneSettings::getInstance().getUseRport() ;

    sipXmgr::getInstance().Initialize(iSipPort, iRtpPort, bUseRport);
*/
                
#endif
}

// Event handler for the Minimal/Normal View menu item
void sipXezPhoneFrame::OnMinimalView(wxCommandEvent& WXUNUSED(event))
{
    mBoolMinimalView = ! mBoolMinimalView;
    if ( mBoolMinimalView )
    {
        mpMenuSettings->SetLabel(ID_Minimal, "Normal &View");
        if (mbLogo)
        {
            SetSize(wxSize(220, 250));
        }
        else
        {
            SetSize(wxSize(220,174));
        }
    }
    else
    {
        mpMenuSettings->SetLabel(ID_Minimal, "Minimal &View");
        SetSize(wxSize(mOrigWidth, mOrigHeight));
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
   autoPosition(POSITION_SAVE);
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
    if (mpCallHistory)
    {
        wxPoint appOrigin = this->GetPosition();
        mpCallHistory->Move(wxPoint(appOrigin.x - 325, appOrigin.y + 45));
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
        this->mpCallHistory->Show();
    }
    else
    {
        this->mpCallHistory->Hide();
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

    if (mpConferencingWnd)
    {
        if (bVisible)
        {
            positionPanels();        
            mpConferencingWnd->Show();
        }
        else
        {
            mpConferencingWnd->Hide();
        }
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
    sipxCallAudioPlayFileStart(hCall, "res/ringTone.raw", true, true, true) ;

    // SIPX_CONF hConf = sipXmgr::getInstance().getCurrentConference();
    // sipxConferenceLimitCodecPreferences(hConf, AUDIO_CODEC_BW_DEFAULT, VIDEO_CODEC_BW_DEFAULT, "H263-CIF") ;
}

void sipXezPhoneFrame::OnRenegotiate(wxCommandEvent& WXUNUSED(event))
{
    SIPX_CONF hConf = sipXmgr::getInstance().getCurrentConference();
    sipxConferenceLimitCodecPreferences(hConf, AUDIO_CODEC_BW_DEFAULT, VIDEO_CODEC_BW_DEFAULT, "H263-CIF") ;
}

void sipXezPhoneFrame::OnStopFile(wxCommandEvent& WXUNUSED(event))
{
    SIPX_CALL hCall = sipXmgr::getInstance().getCurrentCall();

    sipxCallAudioPlayFileStop(hCall) ;
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
//#include "tapi/sipXtapiInternal.h"
void sipXezPhoneFrame::OnTestVoiceEngine(wxCommandEvent& WXUNUSED(event))
{
/*
    GipsVoiceEngineLib* pLib = NULL;
    
    pLib = sipxCallGetVoiceEnginePtr(sipXmgr::getInstance().getCurrentCall());
    pLib->GIPSVE_SetSpeakerVolume(0);
*/
}

void sipXezPhoneFrame::OnAudioWizardTest(wxCommandEvent& WXUNUSED(event))
{
    /*GIPSAECTuningWizard* pWizard = sipxConfigGetVoiceEngineAudioWizard();
    int rc = pWizard->GIPS_AEC_TW_Init(NULL, NULL);
    assert(rc == 0);
    rc = pWizard->GIPS_AEC_TW_Terminate();
    assert(rc == 0);
    */
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

// EventHandler for selecting a contact
void sipXezPhoneFrame::OnSelectContact(wxCommandEvent& event) 
{
    switch (event.GetId())
    {
        case ID_UseAutoContact:
            sipXezPhoneSettings::getInstance().setContactType(CONTACT_AUTO) ;
            break ;
        case ID_UseLocalContact:
            sipXezPhoneSettings::getInstance().setContactType(CONTACT_LOCAL) ;
            break ;
        case ID_UseNatContact:
            sipXezPhoneSettings::getInstance().setContactType(CONTACT_NAT_MAPPED) ;
            break ;
        case ID_UseRelayContact:
            sipXezPhoneSettings::getInstance().setContactType(CONTACT_RELAY) ;
            break ;
        default:
            break; 
    }       
}


void sipXezPhoneFrame::OnUpdateContact(wxUpdateUIEvent& event) 
{
    UtlString stunServer = sipXezPhoneSettings::getInstance().getStunServer() ;
    UtlString turnServer = sipXezPhoneSettings::getInstance().getTurnServer() ;
    SIPX_CONTACT_TYPE eContactType = sipXezPhoneSettings::getInstance().getContactType() ;

    switch (event.GetId())
    {
        case ID_UseAutoContact:
            event.Check(eContactType == CONTACT_AUTO) ;            
            break ;
        case ID_UseLocalContact:
            event.Check(eContactType == CONTACT_LOCAL) ;
            break ;
        case ID_UseNatContact:
            event.Check(eContactType == CONTACT_NAT_MAPPED) ;            
            event.Enable(stunServer.length() > 0) ;
            break ;
        case ID_UseRelayContact:            
            event.Check(eContactType == CONTACT_RELAY) ;
            event.Enable(turnServer.length() > 0) ;
            break ;
        default:
            break; 
    }   
}


void sipXezPhoneFrame::OnDisplayContacts(wxCommandEvent& event) 
{
    SIPX_CONTACT_ADDRESS addresses[32] ;
    size_t numAddresses ; 

    sipXmgr::getInstance().getLocalContacts(32, addresses, numAddresses) ;

    UtlString results ;
    for (unsigned int i=0; i<numAddresses; i++)
    {
        char cType[32] ;
        switch (addresses[i].eContactType)
        {
            case CONTACT_LOCAL:
                strcpy(cType, "LOCAL") ;
                break ;
            case CONTACT_RELAY:
                strcpy(cType, "RELAY") ;
                break ;
            case CONTACT_NAT_MAPPED:
                strcpy(cType, "NAT_MAPPED") ;
                break ;
            default:
                strcpy(cType, "UNKNOWN") ;
                break ;
        }

        char cTemp[128] ;

        sprintf(cTemp, "%d: %s %s:%d (%s)\n",
            addresses[i].id,
            addresses[i].cInterface,
            addresses[i].cIpAddress,
            addresses[i].iPort,
            cType) ;

        results.append(cTemp) ;
    }

    wxMessageBox(results.data(), "Contacts", wxOK, this) ;
}

void sipXezPhoneFrame::autoPosition(enum AUTO_POSITION evt)
{
    int width, height, x, y;
    int px, py, ps, pwidth, pheight;
    int state = 0;

    ::wxClientDisplayRect(&x, &y, &width, &height);
    GetSize(&pwidth, &pheight);

    switch (evt)
    {
    case POSITION_INIT:
        sipXezPhoneSettings::getInstance().getPhoneParameters(px, py, ps);

        if (px == -1 || py == -1)
        {
            px = (width/2) - (pwidth/2);
            py = (height/2) - (pheight/2);
        }
        Move(px, py);
        if (ps & VIDEO_VISIBLE)
        {
            setVideoVisible(true);
        }
        if (ps & CONFERENCE_VISIBLE)
        {
            setConferencingVisible(true);
        }
        if (ps & HISTORY_VISIBLE)
        {
            setCallHistoryVisible(true);
        }
        break;

    case POSITION_SAVE:
        GetPosition(&px, &py);
        if (getVideoVisible())
        {
            state |= VIDEO_VISIBLE;
        }
        if (getConferencingVisible())
        {
            state |= CONFERENCE_VISIBLE;
        }
        if (getCallHistoryVisible())
        {
            state |= HISTORY_VISIBLE;
        }
        sipXezPhoneSettings::getInstance().setPhoneParameters(px, py, state);
        break;
    }
}
