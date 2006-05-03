//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#pragma once

class ImportCertificateDlg :
    public wxDialog
{
public:
    ImportCertificateDlg( wxWindow *parent, wxWindowID id, const wxString &title);
    ~ImportCertificateDlg(void);
    
    virtual void OnOK(wxCommandEvent& event) ;
    virtual void OnCertFileButton(wxCommandEvent& event) ;    
private:
    DECLARE_EVENT_TABLE()
    
    wxTextCtrl* mpPkcs12PasswordCtrl ;
    wxTextCtrl* mpCertCtrl;
};
