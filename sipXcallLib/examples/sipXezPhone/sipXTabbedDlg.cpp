//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include "sipXezPhoneSettings.h"
#include "sipXmgr.h"
#include "sipXTabbedDlg.h"
#include "wx/notebook.h"
#include "wx/listbox.h"

BEGIN_EVENT_TABLE(sipXTabbedDlg,wxDialog)
    EVT_BUTTON( wxID_OK, sipXTabbedDlg::OnOk )
    EVT_BUTTON( wxID_CANCEL, sipXTabbedDlg::OnCancel )
    EVT_BUTTON( ID_TAB_REGISTER_CTRL, sipXTabbedDlg::OnRegister )
    EVT_NOTEBOOK_PAGE_CHANGING(ID_TAB_NOTEBOOK_CTRL, sipXTabbedDlg::OnChanging)
    EVT_NOTEBOOK_PAGE_CHANGED(ID_TAB_NOTEBOOK_CTRL, sipXTabbedDlg::OnChanged)
END_EVENT_TABLE()

wxSizer *sipXTabbedDlgFunc( wxWindow *parent, bool call_fit, bool set_sizer )
{

    wxGridSizer *grid0 = new wxGridSizer( 1, 0, 0 );

    return grid0;
}

//----------------------------------------------------------------------------
// sipXTabbedDlg
//----------------------------------------------------------------------------

sipXTabbedDlg::sipXTabbedDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style, long options ) :
    wxDialog( parent, id, title, position, size, style )
{
    sipXTabbedDlgFunc(this, TRUE, TRUE ); 

    mXpos = position.x;
    mYpos = position.y;
    mWidth = size.GetWidth();
    mHeight = size.GetHeight();
    mOptions = options;

    mLastPage = 0;
}

sipXTabbedDlg::~sipXTabbedDlg()
{
    if (mpNotebook)
    {
        delete mpNotebook;
        mpNotebook = NULL;
    }
    if (mpOkBtn)
    {
        delete mpOkBtn;
    }
    if(mpCancelBtn)
    {
        delete mpCancelBtn;
    }
}

void sipXTabbedDlg::InitDialog()
{
    mpNotebook = new wxNotebook(this, ID_TAB_NOTEBOOK_CTRL, wxPoint(5,5), wxSize(mWidth - 15, mHeight - 70));

    mpPage[0] = new sipXIdentityPage(mpNotebook, "Identity");
    mpPage[1] = new sipXNetworkPage(mpNotebook, "Network");
    mpPage[2] = new sipXAudioPage(mpNotebook, "Audio");
    mpPage[3] = new sipXVideoPage(mpNotebook, "Video");
    mpPage[4] = new sipXSecurityPage(mpNotebook, "Security");
    mPages = 5;

    mpNotebook->SetSelection(mLastPage);

    for (int i=0; i<mPages; i++)
    {
        mpNotebook->AddPage(mpPage[i], mpPage[i]->GetTitle(), false);
    }
 
    mpOkBtn = new wxButton( this, wxID_OK, wxT("OK"), wxPoint(mWidth - 180, mHeight - 55), wxDefaultSize, 0 );
    mpCancelBtn = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxPoint(mWidth - 100, mHeight - 55), wxDefaultSize, 0 );
    mpRegisterBtn = new wxButton( this, ID_TAB_REGISTER_CTRL, wxT("Register"), wxPoint(mWidth - 470, mHeight - 55), wxDefaultSize, 0 );
}

void sipXTabbedDlg::OnChanging( wxNotebookEvent &event )
{
    int page = event.GetSelection();

    if (mpPage[page]->validateData() == false)
    {
        event.Veto();
    }
}

void sipXTabbedDlg::OnChanged( wxNotebookEvent &event )
{
    mLastPage = event.GetSelection();
}

void sipXTabbedDlg::OnOk(wxCommandEvent &event)
{
    // Get all page data and store in settings
    for (int i=0; i<mPages; i++)
    {
        mpPage[i]->getData();
    }
 
    event.Skip();
}

void sipXTabbedDlg::OnCancel(wxCommandEvent &event)
{
    // Make sure to give pages the opportunity to restore any modified
    // data on Cancel
    for (int i=0; i<mPages; i++)
    {
        mpPage[i]->restoreData();
    }

    event.Skip();
}

void sipXTabbedDlg::OnRegister(wxCommandEvent &event)
{
    wxCursor waitCurs(wxCURSOR_WAIT);
    wxCursor normalCurs(wxCURSOR_ARROW);

    this->SetCursor(waitCurs);

    // Get relevant data that may have changed first
    mpPage[0]->getData();  // Identity page
    mpPage[1]->getData();  // Network settings

    sipXmgr::getInstance().UnInitialize(); //unregister old proxy and delete old line
    
    int iSipPort;
    int iRtpPort;
    bool bRport;
    sipXezPhoneSettings::getInstance().getDefaultPorts(iSipPort, iRtpPort);

    bRport = sipXezPhoneSettings::getInstance().getUseRport();

    sipXmgr::getInstance().Initialize(iSipPort, iRtpPort, bRport);
    this->SetCursor(normalCurs);
}

#define SETTING_STR(x,y)  (x->SetValue(sipXezPhoneSettings::getInstance().y().data()))
#define SETTING_BOOL(x,y)  (x->SetValue(sipXezPhoneSettings::getInstance().y()))

///////////////////////////////////////////////////////////////////////////
// sipXIndentityPage

BEGIN_EVENT_TABLE(sipXIdentityPage, sipXTabbedPage)
END_EVENT_TABLE()

#define IDENT_ORIG_X   80
#define IDENT_ORIG_Y   40

sipXIdentityPage::sipXIdentityPage(wxWindow *parent,  const UtlString& title)
                                   : sipXTabbedPage(parent, title)
{
    new wxStaticText( this, ID_TEXT, wxT("Identity"), wxPoint(IDENT_ORIG_X+10,IDENT_ORIG_Y+20),
                      wxSize(50,-1), wxALIGN_LEFT );
    mpIdentity  = new wxTextCtrl( this, ID_TAB_IDENTITY_CTRL, wxT(""), wxPoint(IDENT_ORIG_X+80, IDENT_ORIG_Y+17), 
                                   wxSize(150,-1), 0 );

    new wxStaticText( this, ID_TEXT, wxT("Realm"), wxPoint(IDENT_ORIG_X+10,IDENT_ORIG_Y+50),
                      wxSize(50,-1), wxALIGN_LEFT );
    mpRealm = new wxTextCtrl( this, ID_TAB_REALM_CTRL, wxT(""), wxPoint(IDENT_ORIG_X+80, IDENT_ORIG_Y+47),
                              wxSize(150,-1), 0 );

    new wxStaticText( this, ID_TEXT, wxT("User name"), wxPoint(IDENT_ORIG_X+10,IDENT_ORIG_Y+80),
                      wxSize(50,-1), wxALIGN_LEFT );
    mpUser = new wxTextCtrl( this, ID_TAB_USERNAME_CTRL, wxT(""), wxPoint(IDENT_ORIG_X+80, IDENT_ORIG_Y+77),
                             wxSize(150,-1), 0 );

    new wxStaticText( this, ID_TEXT, wxT("Password"), wxPoint(IDENT_ORIG_X+10,IDENT_ORIG_Y+110),
                      wxSize(50,-1), wxALIGN_LEFT | wxTE_PASSWORD );
    mpPassword = new wxTextCtrl( this, ID_TAB_PASSWORD_CTRL, wxT(""), wxPoint(IDENT_ORIG_X+80, IDENT_ORIG_Y+107),
                                 wxSize(150,-1), 0 );

    new wxStaticText( this, ID_TEXT, wxT("Location"), wxPoint(IDENT_ORIG_X+10,IDENT_ORIG_Y+140),
                      wxSize(50,-1), wxALIGN_LEFT );
    mpLocation = new wxTextCtrl( this, ID_TAB_LOCATION_CTRL, wxT(""), wxPoint(IDENT_ORIG_X+80, IDENT_ORIG_Y+137),
                                 wxSize(150,-1), 0 );

    mpAnswer = new wxCheckBox( this, ID_TAB_ANSWER_CTRL, wxT(""),  wxPoint(IDENT_ORIG_X+80,IDENT_ORIG_Y+170),
                               wxDefaultSize, 0 );
    new wxStaticText( this, ID_TEXT, wxT("Enable AutoAnswer"), wxPoint(IDENT_ORIG_X+100,IDENT_ORIG_Y+170),
                      wxSize(150,-1), wxALIGN_LEFT );
    setData();
}

void sipXIdentityPage::setData()
{
    SETTING_STR(mpIdentity, getIdentity);
    SETTING_STR(mpRealm, getRealm);
    SETTING_STR(mpUser, getUsername);
    SETTING_STR(mpPassword, getPassword);
    SETTING_STR(mpLocation, getLocationHeader);
    SETTING_BOOL(mpAnswer, getAutoAnswer);

    mbInitialized = true;
}

void sipXIdentityPage::getData()
{
    UtlString x;

    x = mpIdentity->GetValue();
    sipXezPhoneSettings::getInstance().setIdentity(x);

    x = mpRealm->GetValue();
    sipXezPhoneSettings::getInstance().setRealm(x);

    x = mpUser->GetValue();
    sipXezPhoneSettings::getInstance().setUsername(x);

    x = mpPassword->GetValue();
    sipXezPhoneSettings::getInstance().setPassword(x);

    x = mpPassword->GetValue();
    sipXezPhoneSettings::getInstance().setPassword(x);

    x = mpLocation->GetValue();
    sipXezPhoneSettings::getInstance().setLocationHeader(x);

    sipXezPhoneSettings::getInstance().setAutoAnswer(mpAnswer->GetValue());
}

bool sipXIdentityPage::validateData()
{
    bool rc = true;

    if (mbInitialized)
    {
    }
    return rc;
}

void sipXIdentityPage::restoreData()
{
}

///////////////////////////////////////////////////////////////////////////
// sipXNetworkPage

#define NETW_ORIG_X   40
#define NETW_ORIG_Y   40

sipXNetworkPage::sipXNetworkPage(wxWindow *parent, const UtlString& title) 
                                 : sipXTabbedPage(parent, title)
{
//sipXezPhoneSettings::getInstance().getTurnServer
    new wxStaticText( this, ID_TEXT, wxT("Outgoing proxy"), wxPoint(NETW_ORIG_X+10,NETW_ORIG_Y+20),
                      wxSize(80,-1), wxALIGN_LEFT );
    mpProxy = new wxTextCtrl( this, ID_TAB_PROXY_CTRL, wxT(""), wxPoint(NETW_ORIG_X+100, NETW_ORIG_Y+17), 
                              wxSize(150,-1), 0 );

    new wxStaticText( this, ID_TEXT, wxT("Stun server"), wxPoint(NETW_ORIG_X+10,NETW_ORIG_Y+50),
                      wxSize(80,-1), wxALIGN_LEFT );
    mpStunServer = new wxTextCtrl( this, ID_TAB_STUNSERVER_CTRL, wxT(""), wxPoint(NETW_ORIG_X+100, NETW_ORIG_Y+47), 
                                   wxSize(150,-1), 0 );

    new wxStaticText( this, ID_TEXT, wxT("Stun port"), wxPoint(NETW_ORIG_X+260,NETW_ORIG_Y+50),
                      wxSize(50,-1), wxALIGN_LEFT );
    mpStunPort = new wxTextCtrl( this, ID_TAB_STUNPORT_CTRL, wxT(""), wxPoint(NETW_ORIG_X+320, NETW_ORIG_Y+47), 
                                 wxSize(40,-1), 0 );

    new wxStaticText( this, ID_TEXT, wxT("Turn server"), wxPoint(NETW_ORIG_X+10,NETW_ORIG_Y+80),
                      wxSize(80,-1), wxALIGN_LEFT );
    mpTurnServer = new wxTextCtrl( this, ID_TAB_TURNSERVER_CTRL, wxT(""), wxPoint(NETW_ORIG_X+100, NETW_ORIG_Y+77), 
                                   wxSize(150,-1), 0 );

    new wxStaticText( this, ID_TEXT, wxT("Turn port"), wxPoint(NETW_ORIG_X+260,NETW_ORIG_Y+80),
                      wxSize(50,-1), wxALIGN_LEFT );
    mpTurnPort = new wxTextCtrl( this, ID_TAB_TURNPORT_CTRL, wxT(""), wxPoint(NETW_ORIG_X+320, NETW_ORIG_Y+77), 
                                 wxSize(40,-1), 0 );

    new wxStaticText( this, ID_TEXT, wxT("Default SIP port"), wxPoint(NETW_ORIG_X+10,NETW_ORIG_Y+110),
                      wxSize(80,-1), wxALIGN_LEFT );
    mpSipPort = new wxTextCtrl( this, ID_TAB_SIPPORT_CTRL, wxT(""), wxPoint(NETW_ORIG_X+100, NETW_ORIG_Y+107), 
                                wxSize(40,-1), 0 );

    new wxStaticText( this, ID_TEXT, wxT("Default RTP port"), wxPoint(NETW_ORIG_X+10,NETW_ORIG_Y+140),
                      wxSize(80,-1), wxALIGN_LEFT );
    mpRtpPort = new wxTextCtrl( this, ID_TAB_RTPPORT_CTRL, wxT(""), wxPoint(NETW_ORIG_X+100, NETW_ORIG_Y+137), 
                                wxSize(40,-1), 0 );

    mpIce = new wxCheckBox( this, ID_TAB_ICE_CTRL, wxT(""),  wxPoint(NETW_ORIG_X+200,IDENT_ORIG_Y+110),
                            wxDefaultSize, 0 );
    new wxStaticText( this, ID_TEXT, wxT("Enable ICE"), wxPoint(NETW_ORIG_X+220,IDENT_ORIG_Y+110),
                      wxSize(80,-1), wxALIGN_LEFT );

    mpRport = new wxCheckBox( this, ID_TAB_RPORT_CTRL, wxT(""),  wxPoint(NETW_ORIG_X+200,IDENT_ORIG_Y+140),
                              wxDefaultSize, 0 );
    new wxStaticText( this, ID_TEXT, wxT("Enable Rport"), wxPoint(NETW_ORIG_X+220,IDENT_ORIG_Y+140),
                      wxSize(80,-1), wxALIGN_LEFT );

    new wxStaticText( this, ID_TEXT, wxT("Idle timeout"), wxPoint(NETW_ORIG_X+10,NETW_ORIG_Y+170),
                      wxSize(80,-1), wxALIGN_LEFT );
    mpIdle = new wxTextCtrl( this, ID_TAB_IDLE_CTRL, wxT(""), wxPoint(NETW_ORIG_X+100, NETW_ORIG_Y+167), 
                             wxSize(40,-1), 0 );

    mpShortNames = new wxCheckBox( this, ID_TAB_SHORT_CTRL, wxT(""),  wxPoint(NETW_ORIG_X+200,IDENT_ORIG_Y+170),
                                   wxDefaultSize, 0 );
    new wxStaticText( this, ID_TEXT, wxT("Use short headers"), wxPoint(NETW_ORIG_X+220,IDENT_ORIG_Y+170),
                      wxSize(100,-1), wxALIGN_LEFT );

    setData();
}

void sipXNetworkPage::setData()
{
    char szBuffer[32];

    SETTING_STR(mpProxy, getProxyServer);
    SETTING_STR(mpStunServer, getStunServer);
    SETTING_STR(mpTurnServer, getTurnServer);
    SETTING_BOOL(mpIce, getIceEnabled);
    SETTING_BOOL(mpRport, getUseRport);
    SETTING_BOOL(mpShortNames, getShortNamesEnabled);

    sprintf(szBuffer, "%d", sipXezPhoneSettings::getInstance().getStunServerPort());
    mpStunPort->SetValue(szBuffer);

    sprintf(szBuffer, "%d", sipXezPhoneSettings::getInstance().getTurnServerPort());
    mpTurnPort->SetValue(szBuffer);

    int iSipPort, iRtpPort;
    sipXezPhoneSettings::getInstance().getDefaultPorts(iSipPort, iRtpPort);

    sprintf(szBuffer, "%d", iSipPort);
    mpSipPort->SetValue(szBuffer);
    sprintf(szBuffer, "%d", iRtpPort);
    mpRtpPort->SetValue(szBuffer);
    sprintf(szBuffer, "%d", sipXezPhoneSettings::getInstance().getConnectionIdleTimeout());
    mpIdle->SetValue(szBuffer);

    mbInitialized = true;
}

void sipXNetworkPage::getData()
{
    UtlString x;

    x = mpProxy->GetValue();
    sipXezPhoneSettings::getInstance().setProxyServer(x);

    x = mpStunServer->GetValue();
    sipXezPhoneSettings::getInstance().setStunServer(x);

    x = mpTurnServer->GetValue();
    sipXezPhoneSettings::getInstance().setTurnServer(x);

    sipXezPhoneSettings::getInstance().setEnableIce(mpIce->GetValue());
    sipXezPhoneSettings::getInstance().setUseRport(mpRport->GetValue());
    sipXezPhoneSettings::getInstance().setShortNamesEnabled(mpShortNames->GetValue());

    x = mpTurnPort->GetValue();
    sipXezPhoneSettings::getInstance().setTurnServerPort(atoi(x));

    x = mpStunPort->GetValue();
    sipXezPhoneSettings::getInstance().setStunServerPort(atoi(x));

    x = mpIdle->GetValue();
    sipXezPhoneSettings::getInstance().setConnectionIdleTimeout(atoi(x));
}

bool sipXNetworkPage::validateData()
{
    bool rc = true;

    if (mbInitialized)
    {
    }
    return rc;
}

void sipXNetworkPage::restoreData()
{
}

///////////////////////////////////////////////////////////////////////////
// sipXAudioPage

#define AUDIO_ORIG_X   10
#define AUDIO_ORIG_Y   20

static wxString bwChoices[] = {
    "Low Bandwidth", "Normal Bandwidth", "High Bandwidth"
};

sipXAudioPage::sipXAudioPage(wxWindow *parent, const UtlString& title)
                             : sipXTabbedCodecPage(parent, title)
{
    mpEcho = new wxCheckBox( this, ID_TAB_ECHO_CTRL, wxT(""),  wxPoint(AUDIO_ORIG_X+310, AUDIO_ORIG_Y+50),
                             wxDefaultSize, 0 );
    new wxStaticText( this, ID_TEXT, wxT("Enable echo cancellation"), wxPoint(AUDIO_ORIG_X+327,AUDIO_ORIG_Y+50),
                      wxSize(120,-1), wxALIGN_LEFT );

    mpDtmf = new wxCheckBox( this, ID_TAB_DTMF_CTRL, wxT(""),  wxPoint(AUDIO_ORIG_X+310, AUDIO_ORIG_Y+80),
                             wxDefaultSize, 0 );
    new wxStaticText( this, ID_TEXT, wxT("Enable out-of-band DTMF"), wxPoint(AUDIO_ORIG_X+327,AUDIO_ORIG_Y+80),
                      wxSize(130,-1), wxALIGN_LEFT );
    setData();
}

void sipXAudioPage::setData()
{
    UtlString mCodecName = sipXezPhoneSettings::getInstance().getSelectedAudioCodecs();
    mCodecPref = sipXezPhoneSettings::getInstance().getCodecPref() - 1;

    if (mCodecPref == 3)
    {
        mbCodecByName = true;
        mpCodecPref->Append("By name:");
    }

    mpCodecPref->SetSelection(mCodecPref);
    mOrigCodecPref = mCodecPref;
    mOrigCodecName = mCodecName;

    rebuildCodecList(mCodecPref);

    mpDtmf->SetValue(sipXezPhoneSettings::getInstance().getEnableOOBDTMF());
    mpEcho->SetValue(sipXezPhoneSettings::getInstance().getEnableAEC());

    mbInitialized = true;
}

void sipXAudioPage::getData()
{
    if (!mbCodecByName)
    {
        mCodecName = "";
    }
    sipXezPhoneSettings::getInstance().setCodecPref(mCodecPref+1);
    sipXezPhoneSettings::getInstance().setSelectedAudioCodecs(mCodecName);

    sipXezPhoneSettings::getInstance().setEnableOOBDTMF(mpDtmf->GetValue());
    sipXmgr::getInstance().enableOutOfBandDTMF(mpDtmf->GetValue());

    sipXezPhoneSettings::getInstance().setEnableAEC(mpEcho->GetValue());
    sipXmgr::getInstance().enableAEC(mpEcho->GetValue());
}

bool sipXAudioPage::validateData()
{
    bool rc = true;

    if (mbInitialized)
    {
    }
    return rc;
}

void sipXAudioPage::restoreData()
{
    // Restore original settings on Cancel. These settings cause actual 
    // calls to sipXtapi to change codec preferences during the operation
    // of the page and must be explicitely reverted.
    if (mOrigCodecPref == 3)
    {
        sipXmgr::getInstance().setAudioCodecByName(mOrigCodecName);
    }
    else
    {
        sipXmgr::getInstance().setCodecPreferences(mOrigCodecPref+1);
    }
}

///////////////////////////////////////////////////////////////////////////
// sipXVideoPage

sipXVideoPage::sipXVideoPage(wxWindow *parent, const UtlString& title)
                             : sipXTabbedCodecPage(parent, title)
{
}

void sipXVideoPage::setData()
{
    mbInitialized = true;
}

void sipXVideoPage::getData()
{
}

bool sipXVideoPage::validateData()
{
    bool rc = true;

    if (mbInitialized)
    {
    }
    return rc;
}

void sipXVideoPage::restoreData()
{
}

///////////////////////////////////////////////////////////////////////////
// sipXSecurityPage

sipXSecurityPage::sipXSecurityPage(wxWindow *parent, const UtlString& title)
                                   : sipXTabbedPage(parent, title)
{
}

void sipXSecurityPage::setData()
{
    mbInitialized = true;
}

void sipXSecurityPage::getData()
{
}

bool sipXSecurityPage::validateData()
{
    bool rc = true;

    if (mbInitialized)
    {
    }
    return rc;
}

void sipXSecurityPage::restoreData()
{
}

///////////////////////////////////////////////////////////////////////////
// sipXTabbedPage

sipXTabbedPage::sipXTabbedPage(wxWindow *parent, const UtlString& sTitle, wxWindowID id,
                               const wxPoint &position, const wxSize& size)
                                   : wxPanel(parent, id, position, size)
{
    mbInitialized = false;
    mTitle = sTitle;
}

///////////////////////////////////////////////////////////////////////////
// sipXTabbedCodecPage

BEGIN_EVENT_TABLE(sipXTabbedCodecPage, sipXTabbedPage)
    EVT_BUTTON( ID_TAB_CODEC_SELECT, sipXTabbedCodecPage::OnSelect )
    EVT_CHOICE( ID_TAB_BANDWIDTH_CHOICE, sipXTabbedCodecPage::OnBandwidthChange )
    EVT_LISTBOX_DCLICK( ID_TAB_CODEC_LIST, sipXTabbedCodecPage::OnDblClick )
END_EVENT_TABLE()

sipXTabbedCodecPage::sipXTabbedCodecPage(wxWindow *parent, const UtlString& sTitle)
                                          : sipXTabbedPage(parent, sTitle)
{
    mCodecPref = 0;
    mbCodecByName = false;
    mCodecName = "";

    new wxStaticText(this, ID_TEXT, wxT("Codec preferences"), wxPoint(AUDIO_ORIG_X+10,AUDIO_ORIG_Y+20),
                     wxSize(100, 20), wxALIGN_LEFT );
    mpCodecPref = new wxChoice(this, ID_TAB_BANDWIDTH_CHOICE, wxPoint(AUDIO_ORIG_X+120,AUDIO_ORIG_Y+17), wxSize(170,-1), 3, bwChoices);

    new wxStaticText(this, ID_TEXT, wxT("Supported codecs\nfor this bandwidth"), wxPoint(AUDIO_ORIG_X+10,AUDIO_ORIG_Y+50),
                     wxSize(100, 40), wxALIGN_LEFT );
    mpCodecList = new wxListBox(this, ID_TAB_CODEC_LIST, wxPoint(AUDIO_ORIG_X+120,AUDIO_ORIG_Y+50), wxSize(170,150), 0, NULL, 
                                wxLB_ALWAYS_SB | wxLB_MULTIPLE);

    mpSelectBtn = new wxButton( this, ID_TAB_CODEC_SELECT, wxT("Select"), wxPoint(AUDIO_ORIG_X+18,AUDIO_ORIG_Y+170), wxDefaultSize, 0 );

    new wxStaticText(this, ID_TEXT, wxT("Highlight one or more entries in the list and click the 'Select' button to select ")
                                    wxT("those codecs as active codecs. Changing the ")
                                    wxT("preferred bandwidth will deselect the codecs again."),
                                    wxPoint(AUDIO_ORIG_X+10,AUDIO_ORIG_Y+220),
                                    wxSize(300, 50), wxALIGN_LEFT );
}

void sipXTabbedCodecPage::OnBandwidthChange(wxCommandEvent &event)
{
    int sel = event.GetSelection();

    if (mbCodecByName && sel < 3)
    {
        // Delete 'By name:' entry in drop-down list
        mpCodecPref->Delete(3);
        mbCodecByName = false;
    }

    if (mpCodecList && sel < 3)
    {
        sipXmgr::getInstance().setCodecPreferences(sel + 1);
        mCodecPref = sel;
        rebuildCodecList(sel);
    }
}

void sipXTabbedCodecPage::OnDblClick(wxCommandEvent &event)
{
    if (!mbCodecByName)
    {
        int sel = event.GetSelection();

        mbCodecByName = true;
        mCodecName = mpCodecList->GetStringSelection();
        mpCodecPref->Append("By name:");
        mpCodecPref->SetSelection(3);
        mCodecPref = 3;

        sipXmgr::getInstance().setAudioCodecByName((const char *)mCodecName.data());

        rebuildCodecList(0);
    }
}

void sipXTabbedCodecPage::OnSelect(wxCommandEvent &event)
{
    wxArrayInt selections;
    UtlString s;
    int index;

    if (!mbCodecByName)
    {
        int numSels = mpCodecList->GetSelections(selections);

        if (numSels)
        {
            mCodecName = "";

            for (int i=0; i<numSels; i++)
            {
                s = mpCodecList->GetString(selections[i]);
                if ((index = s.index(" ")) != UTL_NOT_FOUND)
                {
                    // Only get name up to first space
                    s = s.remove(index, s.length() - index);
                }
                mCodecName = mCodecName + s + " ";
            }
            mbCodecByName = true;
            mpCodecPref->Append("By name:");
            mpCodecPref->SetSelection(3);
            mCodecPref = 3;

            sipXmgr::getInstance().setAudioCodecByName((const char *)mCodecName.data());

            rebuildCodecList(0);
        }
    }
}

void sipXTabbedCodecPage::rebuildCodecList(int sel)
{
    mpCodecList->Clear();

    UtlString sData = "";
    sipXmgr::getInstance().getCodecList(sData);

    char *tokTmp;
    char *str = (char*)sData.data();

    tokTmp = strtok(str, "\n");

    while (tokTmp != NULL)
    {
        mpCodecList->Append(wxT(tokTmp));
        tokTmp = strtok(NULL, "\n");
    }
}


