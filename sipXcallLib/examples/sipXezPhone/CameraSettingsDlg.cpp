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
#include "CameraSettingsDlg.h"
#include "sipXezPhoneSettings.h"
#include "tapi/sipXtapi.h"

#include <os/OsDefs.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define ID_CAMERA_LIST 8001
// STATIC VARIABLE INITIALIZATIONS
// MACROS

BEGIN_EVENT_TABLE(CameraSettingsDlg, wxDialog)
    EVT_BUTTON( wxID_OK, CameraSettingsDlg::OnOK )
END_EVENT_TABLE()

CameraSettingsDlg::CameraSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title)
        : wxDialog( parent, id, title, wxDefaultPosition, wxSize(300, 200), 
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{    
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

            
    wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL  );
    
    topsizer->Add(
            new wxListBox(this,
                          ID_CAMERA_LIST,
                          wxDefaultPosition,
                          wxSize(250, 150), 0, NULL));
    populateCameraList();
                          
    button_sizer->Add(
            new wxButton( this, wxID_OK, "Ok" ),
            0,           // make horizontally unstretchable
            wxALL,       // make border all around (implicit top alignment)
            10 );        // set border width to 10

    button_sizer->Add(
            new wxButton( this, wxID_CANCEL, "Cancel" ),
            0,           // make horizontally unstretchable
            wxALL,       // make border all around (implicit top alignment)
            10 );        // set border width to 10

    topsizer->Add(
            button_sizer,
            0,                // make vertically unstretchable
            wxALIGN_CENTER ); // no border and centre horizontally

    SetSizer( topsizer );      // use the sizer for layout

    topsizer->SetSizeHints( this );   // set size hints to honour minimum size    
            

}

CameraSettingsDlg::~CameraSettingsDlg(void)
{
}


void CameraSettingsDlg::OnOK(wxCommandEvent& event) 
{
#ifdef VIDEO
    wxListBox* pList = (wxListBox*)FindWindowById(ID_CAMERA_LIST, this);
    
    wxString selection = pList->GetStringSelection();

    sipxConfigSetVideoCaptureDevice(sipXmgr::getInstance().getSipxInstance(), 
                                    selection.c_str());
    wxDialog::OnOK(event) ;
#endif    
}

void CameraSettingsDlg::populateCameraList()
{
#ifdef VIDEO
    char arrDevices[MAX_VIDEO_DEVICES][MAX_VIDEO_DEVICE_LENGTH];
    sipxConfigGetVideoCaptureDevices(sipXmgr::getInstance().getSipxInstance(), 
                                     (char**)arrDevices,
                                     MAX_VIDEO_DEVICE_LENGTH,
                                     MAX_VIDEO_DEVICES);
    wxListBox* pList = (wxListBox*)FindWindowById(ID_CAMERA_LIST, this);
    
    int index = 0;
    while (arrDevices[index][0] != '\0' )
    {
        pList->Append(arrDevices[index]);
        index++;
    }
#endif    
}