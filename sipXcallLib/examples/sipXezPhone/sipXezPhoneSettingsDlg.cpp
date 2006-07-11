/////////////////////////////////////////////////////////////////////////////
// Name:        sipXezPhoneSettingsDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "sipXezPhoneSettingsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "sipXezPhoneSettingsDlg.h"
#include "sipXezPhoneSettings.h"
#include "sipXmgr.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// sipXezPhoneSettingsDlg
//----------------------------------------------------------------------------

// WDR: event table for sipXezPhoneSettingsDlg

BEGIN_EVENT_TABLE(sipXezPhoneSettingsDlg,wxDialog)
    EVT_BUTTON( wxID_OK, sipXezPhoneSettingsDlg::OnOk )
    EVT_BUTTON( wxID_CANCEL, sipXezPhoneSettingsDlg::OnCancel )
END_EVENT_TABLE()

sipXezPhoneSettingsDlg::sipXezPhoneSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    // WDR: dialog function sipXezPhoneSettingsDlgFunc for sipXezPhoneSettingsDlg
    sipXezPhoneSettingsDlgFunc(this, TRUE ); 

    wxTextCtrl* pCtrl;

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_IDENTITY_CTRL, this);
    pCtrl->SetValue( sipXezPhoneSettings::getInstance().getIdentity().data() );

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_USERNAME_CTRL, this);
    pCtrl->SetValue( sipXezPhoneSettings::getInstance().getUsername().data() );

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_PASSWORD_CTRL, this);
    pCtrl->SetValue( sipXezPhoneSettings::getInstance().getPassword().data() );

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_REALM_CTRL, this);
    pCtrl->SetValue( sipXezPhoneSettings::getInstance().getRealm().data() );

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_PROXY_SERVER_CTRL, this);
    pCtrl->SetValue( sipXezPhoneSettings::getInstance().getProxyServer().data() );
    
    wxCheckBox* pCheck = (wxCheckBox*)sipXezPhoneSettingsDlg::FindWindowById(ID_ENABLE_RPORT_CTRL, this);
    pCheck->SetValue( sipXezPhoneSettings::getInstance().getUseRport() );

    pCheck = (wxCheckBox*)sipXezPhoneSettingsDlg::FindWindowById(ID_ENABLE_AUTO_ANSWER, this);
    pCheck->SetValue( sipXezPhoneSettings::getInstance().getAutoAnswer() );

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_STUN_SERVER_CTRL, this);
    pCtrl->SetValue( sipXezPhoneSettings::getInstance().getStunServer().data() );
}

// WDR: handler implementations for sipXezPhoneSettingsDlg

void sipXezPhoneSettingsDlg::OnOk(wxCommandEvent &event)
{
    wxTextCtrl* pCtrl;
    wxString x;
      

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_IDENTITY_CTRL, this);
    x = pCtrl->GetValue();
    sipXezPhoneSettings::getInstance().setIdentity(x.c_str());

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_USERNAME_CTRL, this);
    x = pCtrl->GetValue();
    sipXezPhoneSettings::getInstance().setUsername(x.c_str());

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_PASSWORD_CTRL, this);
    x = pCtrl->GetValue();
    sipXezPhoneSettings::getInstance().setPassword(x.c_str());

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_REALM_CTRL, this);
    x = pCtrl->GetValue();
    sipXezPhoneSettings::getInstance().setRealm(x.c_str());

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_PROXY_SERVER_CTRL, this);
    x = pCtrl->GetValue();
    sipXezPhoneSettings::getInstance().setProxyServer(x.c_str());
    
    wxCheckBox* pCheck = (wxCheckBox*)sipXezPhoneSettingsDlg::FindWindowById(ID_ENABLE_RPORT_CTRL, this);
    sipXezPhoneSettings::getInstance().setUseRport(pCheck->GetValue());

    pCheck = (wxCheckBox*)sipXezPhoneSettingsDlg::FindWindowById(ID_ENABLE_AUTO_ANSWER, this);
    sipXezPhoneSettings::getInstance().setAutoAnswer(pCheck->GetValue());

    pCtrl = (wxTextCtrl*)sipXezPhoneSettingsDlg::FindWindowById(ID_STUN_SERVER_CTRL, this);
    x = pCtrl->GetValue();
    sipXezPhoneSettings::getInstance().setStunServer(x.c_str());


    sipXezPhoneSettings::getInstance().saveSettings();
    
    event.Skip();
}

void sipXezPhoneSettingsDlg::OnCancel(wxCommandEvent &event)
{
    event.Skip();
}




