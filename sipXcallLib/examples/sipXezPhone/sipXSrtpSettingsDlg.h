/////////////////////////////////////////////////////////////////////////////
// Name:        sipXSrtpSettingsDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __sipXSrtpSettingsDlg_H__
#define __sipXSrtpSettingsDlg_H__

#ifdef __GNUG__
    #pragma interface "sipXSrtpSettingsDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <utl/UtlString.h>

#include "sipXezPhone_wdr.h"

#define ID_SRTP_OK_BUTTON  8200
#define ID_RTP_CIPHER_TYPE 8201
#define ID_RTP_AUTH_TYPE   8202
#define ID_RTP_ENABLE_AUTH 8203
#define ID_RTP_ENABLE_ENCR 8204
#define ID_RTP_KEY         8205
#define ID_RTP_ENABLE_SEC  8208
#define ID_RTP_KEYLEN      8209
#define ID_RTP_RANDOM      8210
#define ID_DB_LOCATION     8211
#define ID_CERTDB_PASSWORD 8213
#define ID_CERT_NICKNAME   8214

// WDR: class declarations

//----------------------------------------------------------------------------
// sipXezPhoneSettingsDlg
//----------------------------------------------------------------------------

class sipXSrtpSettingsDlg: public wxDialog
{
public:
    // constructors and destructors
    sipXSrtpSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );

    void InitializeControls();

    // public members
public:    
   UtlString mSrtpKey;
   bool      mbEncryption;
   int       mSecurityLevel;
   UtlString mCertNickname;
   UtlString mCertDbPassword;

private:
    // WDR: member variable declarations for sipXVideoSettingsDlg
    
private:
    // WDR: handler declarations for sipXVideoSettingsDlg
    void OnOk( wxCommandEvent &event );
    void OnCancel( wxCommandEvent &event );
    void OnSecurity(wxCommandEvent &event);
    void OnKeyEntry(wxCommandEvent &event);
    void OnRandom(wxCommandEvent &event);
    void enable(bool bEnable);

private:
    DECLARE_EVENT_TABLE()
};

#endif
