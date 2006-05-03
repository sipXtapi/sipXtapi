/////////////////////////////////////////////////////////////////////////////
// Name:        sipXSrtpSettingsDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "sipXSrtpSettingsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "utl/UtlRandom.h"

#include "sipXSrtpSettingsDlg.h"
#include "sipXezPhoneSettings.h"
#include "sipXmgr.h"

// WDR: class implementations

BEGIN_EVENT_TABLE(sipXSrtpSettingsDlg,wxDialog)
    EVT_BUTTON( wxID_OK, sipXSrtpSettingsDlg::OnOk )
    EVT_BUTTON( wxID_CANCEL, sipXSrtpSettingsDlg::OnCancel )
    EVT_CHECKBOX(ID_RTP_ENABLE_SEC, sipXSrtpSettingsDlg::OnSecurity)
    EVT_TEXT(ID_RTP_KEY, sipXSrtpSettingsDlg::OnKeyEntry)
    EVT_BUTTON(ID_RTP_RANDOM, sipXSrtpSettingsDlg::OnRandom)
END_EVENT_TABLE()

static wxCheckBox* checkEncryption;
static wxCheckBox* checkAuthentication;
static wxCheckBox* checkSecurity;
static wxTextCtrl* textSrtpKey;
static wxTextCtrl* certNickname;
static wxTextCtrl* certDbPassword;
static wxStaticText* keyLength;
static wxButton* randomButton;
static int charCount = 0;

#define SRTP_ORIGIN  185


wxSizer *sipXSrtpSettingsDlgFunc( wxWindow *parent, bool call_fit, bool set_sizer )
{

    wxGridSizer *grid0 = new wxGridSizer( 1, 0, 0 );

    new wxStaticBox(parent, -1, wxT("SRTP Settings"), wxPoint(1, SRTP_ORIGIN), wxSize(314, 165), wxALIGN_LEFT);

    new wxStaticBox(parent, -1, wxT("SMIME Settings"), wxPoint(1,1), wxSize(314, 173), wxALIGN_LEFT);

    new wxStaticText(parent, ID_TEXT, wxT("Cipher type:"), wxPoint(10,SRTP_ORIGIN+20), wxSize(70, 20), wxALIGN_LEFT );
    new wxStaticText(parent, ID_TEXT, wxT("AES_CM_128_HMAC_SAH1_80"), wxPoint(90, SRTP_ORIGIN+20), wxSize(-1,-1), wxALIGN_LEFT);

    new wxStaticText(parent, ID_TEXT, wxT("Authentication:"), wxPoint(10,SRTP_ORIGIN+40), wxSize(70, 20), wxALIGN_LEFT );
    new wxStaticText(parent, ID_TEXT, wxT("AUTH_HMAC_SHA1"), wxPoint(90, SRTP_ORIGIN+40), wxSize(-1,-1), wxALIGN_LEFT);

    new wxStaticText(parent, ID_TEXT, wxT("Security Level"), wxPoint(10, SRTP_ORIGIN+65), wxSize(70, 20), wxALIGN_LEFT );
    wxCheckBox *item3 = new wxCheckBox(parent, ID_RTP_ENABLE_AUTH, wxT("Authentication"), wxPoint(90, SRTP_ORIGIN+65), wxDefaultSize, 0 );
    wxCheckBox *item4 = new wxCheckBox(parent, ID_RTP_ENABLE_ENCR, wxT("Encryption"), wxPoint(90, SRTP_ORIGIN+85), wxDefaultSize, 0 );

    new wxStaticText(parent, ID_TEXT, wxT("SRTP Key"), wxPoint(10, SRTP_ORIGIN+110), wxSize(70, 20), wxALIGN_LEFT );
    wxTextCtrl *item5 = new wxTextCtrl(parent, ID_RTP_KEY, wxT(""), wxPoint(90, SRTP_ORIGIN+108), wxSize(200, 20), wxTE_LEFT);

    wxCheckBox *item8 = new wxCheckBox(parent, ID_RTP_ENABLE_SEC, wxT("Enable security"), wxPoint(90, 20), wxDefaultSize, 0 );

    new wxStaticText(parent, ID_TEXT, wxT("Cert Nickname"), wxPoint(10, 125), wxSize(70, 20), wxALIGN_LEFT);
    wxTextCtrl *item15 = new wxTextCtrl(parent, ID_CERT_NICKNAME, wxT(""), wxPoint(90, 125), wxSize(200, 20), wxTE_LEFT );

    new wxStaticText(parent, ID_TEXT, wxT("CertDb Psswd"), wxPoint(10, 145), wxSize(70, 20), wxALIGN_LEFT);
    wxTextCtrl *item16 = new wxTextCtrl(parent, ID_CERTDB_PASSWORD, wxT(""), wxPoint(90, 145), wxSize(200, 20), wxTE_LEFT | wxTE_PASSWORD);

    new wxStaticText(parent, ID_TEXT, wxT("Key length"), wxPoint(90, SRTP_ORIGIN+134), wxSize(50, 20), wxALIGN_LEFT);
    wxStaticText *item9 = new wxStaticText(parent, ID_RTP_KEYLEN, wxT("0"), wxPoint(145, SRTP_ORIGIN+134), wxSize(40, 20), wxALIGN_LEFT);

    wxButton* item10 = new wxButton(parent, ID_RTP_RANDOM, wxT("Random"), wxPoint(200, SRTP_ORIGIN+135), wxDefaultSize);

    new wxButton(parent, wxID_OK, wxT("OK"), wxPoint(200, SRTP_ORIGIN+173), wxDefaultSize);

    checkEncryption = item4;
    checkAuthentication = item3;
    checkSecurity = item8;
    textSrtpKey = item5;
    keyLength = item9;
    randomButton = item10;
    certNickname = item15;
    certDbPassword = item16;

    return grid0;
}

//----------------------------------------------------------------------------
// sipXSrtpSettingsDlg
//----------------------------------------------------------------------------

sipXSrtpSettingsDlg::sipXSrtpSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    // WDR: dialog function sipXSrtpSettingsDlgFunc for sipXSrtpSettingsDlg
    sipXSrtpSettingsDlgFunc(this, TRUE, TRUE ); 

    mSrtpKey = "";
    mCertNickname = "";
    mCertDbPassword = "";
    mbEncryption = false;
    mSecurityLevel = 0;
}

// WDR: handler implementations for sipXSrtpSettingsDlg

void sipXSrtpSettingsDlg::InitializeControls()
{
    char buffer[10];

    checkSecurity->SetValue(mbEncryption);
    checkEncryption->SetValue(mSecurityLevel&1);
    checkAuthentication->SetValue(mSecurityLevel&2);
    textSrtpKey->SetValue(mSrtpKey.data());
    certNickname->SetValue(mCertNickname.data());    
    certDbPassword->SetValue(mCertDbPassword.data());

    sprintf(buffer, "%d", mSrtpKey.length());
    keyLength->SetLabel(buffer);

    if (mbEncryption)
    {
        enable(true);
    }
    else
    {
        enable(false);
    }
}

void sipXSrtpSettingsDlg::enable(bool bEnable)
{
    if (bEnable)
    {
        checkEncryption->Enable();
        checkAuthentication->Enable();
        textSrtpKey->Enable();
        keyLength->Enable();
        certDbPassword->Enable();
        certNickname->Enable();
    }
    else
    {
        checkEncryption->Disable();
        checkAuthentication->Disable();
        textSrtpKey->Disable();
        keyLength->Disable();
        certNickname->Disable();
        certDbPassword->Disable();
    }

}
void sipXSrtpSettingsDlg::OnOk(wxCommandEvent &event)
{
    wxString x;
 
    x = textSrtpKey->GetValue();
    if (x.Length() != 30 && x.Length() != 0)
    {
        wxMessageDialog* pMsg = new wxMessageDialog(this, "SRTP key must be empty\nor exactly 30 chars long!", "Error", wxOK);
        if(pMsg->ShowModal() == wxID_OK )
        {
    
        }
        delete pMsg;                        
        return;
    }
    mbEncryption = checkSecurity->GetValue();

    mSecurityLevel = 0;
    if (checkEncryption->GetValue())
    {
        mSecurityLevel |= 1;
    }
    if (checkAuthentication->GetValue())
    {
        mSecurityLevel |= 2;
    }
    mSrtpKey = textSrtpKey->GetValue();
    mCertNickname = certNickname->GetValue();
    mCertDbPassword = certDbPassword->GetValue();
    event.Skip();
}

void sipXSrtpSettingsDlg::OnCancel(wxCommandEvent &event)
{
    event.Skip();
}

void sipXSrtpSettingsDlg::OnKeyEntry(wxCommandEvent &event)
{
    char buffer[10];

    sprintf(buffer, "%d", wxString(event.m_commandString).Length());
    keyLength->SetLabel(buffer);
}

void sipXSrtpSettingsDlg::OnRandom(wxCommandEvent &event)
{
    char buffer[32];
    UtlRandom r;
    long t;

    for (int i=0; i<30; ++i)
    {
        t = 91;
        while (t >=91 && t <=96)
        {
            t = (r.rand() % 58) + 65;
        }
        buffer[i] = t;
    }
    buffer[30] = 0;
    textSrtpKey->SetValue(buffer);
}

void sipXSrtpSettingsDlg::OnSecurity(wxCommandEvent &event)
{
    int i = event.m_commandInt; // Boolean value of check box

    if (event.m_commandInt)
    {
        enable(true);
    }
    else
    {
        enable(false);
    }
}
