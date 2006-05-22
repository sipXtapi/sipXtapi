//
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////


#ifndef __sipXSrtpSettingsDlg_H__
#define __sipXSrtpSettingsDlg_H__

#ifdef __GNUG__
    #pragma interface "sipXSrtpSettingsDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "sipXezPhone_wdr.h"

#define ID_SRTP_OK_BUTTON  8200
#define ID_RTP_CIPHER_TYPE 8201
#define ID_RTP_AUTH_TYPE   8202
#define ID_RTP_ENABLE_AUTH 8203
#define ID_RTP_ENABLE_ENCR 8204

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
    
    // WDR: method declarations for sipXVideoSettingsDlg
    static wxGridSizer* grid;
    static wxStaticBox* video;
    
private:
    // WDR: member variable declarations for sipXVideoSettingsDlg
    
private:
    // WDR: handler declarations for sipXVideoSettingsDlg
    void OnOk( wxCommandEvent &event );
    void OnCancel( wxCommandEvent &event );

private:
    DECLARE_EVENT_TABLE()
};

#endif
