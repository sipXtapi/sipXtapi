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

class CameraSettingsDlg :
    public wxDialog
{
public:
    CameraSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title);
    ~CameraSettingsDlg(void);
    
    virtual void OnOK(wxCommandEvent& event) ;
private:
    DECLARE_EVENT_TABLE()
    void populateCameraList();
    
};
