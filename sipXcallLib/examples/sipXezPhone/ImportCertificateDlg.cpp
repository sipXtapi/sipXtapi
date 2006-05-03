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
#include "ImportCertificateDlg.h"
#include "sipXezPhoneSettings.h"

#include <os/OsDefs.h>
#include <net/SmimeBody.h>
#include <os/OsFS.h>

//#include <tapi/sipXtapiInternal.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define ID_CERT_FILE_BUTTON 8001
// STATIC VARIABLE INITIALIZATIONS
// MACROS

BEGIN_EVENT_TABLE(ImportCertificateDlg, wxDialog)
    EVT_BUTTON( wxID_OK, ImportCertificateDlg::OnOK )
    EVT_BUTTON( ID_CERT_FILE_BUTTON, ImportCertificateDlg::OnCertFileButton )
END_EVENT_TABLE()

ImportCertificateDlg::ImportCertificateDlg( wxWindow *parent, wxWindowID id, const wxString &title)
        : wxDialog( parent, id, title, wxDefaultPosition, wxSize(300, 200), 
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{    
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    topsizer->Add(new wxStaticText(this, -1, "Private Key Password:"), 1, wxALL, 2) ;

    mpPkcs12PasswordCtrl = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, 
            wxSize(100, 20), wxTE_PASSWORD) ;
            
    mpCertCtrl = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, 
            wxSize(280, 20)) ;
            
    topsizer->Add(mpPkcs12PasswordCtrl,
            1,            // make vertically stretchable
            wxALL,        //   and make border all around
            2);         // set border width to 10
            
            
    topsizer->Add(new wxStaticText(this, -1, "Certificate:"), 1, wxALL, 2) ;
    wxBoxSizer *cert_sizer = new wxBoxSizer( wxHORIZONTAL  );
    cert_sizer->Add(mpCertCtrl, 1, wxEXPAND | wxALL, 2);
    cert_sizer->Add( new wxButton (this, ID_CERT_FILE_BUTTON, "...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
                     0, wxALL | wxEXPAND, 2);

    topsizer->Add(cert_sizer);
            
    wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL  );

    button_sizer->Add(
            new wxButton( this, wxID_OK, "Import" ),
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

ImportCertificateDlg::~ImportCertificateDlg(void)
{
}

void ImportCertificateDlg::OnCertFileButton(wxCommandEvent& event) 
{
    wxFileDialog dialog
                 (
                    this,
                    "Importing a private key in PKCS12 format",
                    "",
                    "",
                    "PKCS12 Certificate files (*.p12)|*.p12"
                 );

    dialog.SetDirectory(wxGetCwd());

    if (dialog.ShowModal() == wxID_OK)
    {
        mpCertCtrl->SetLabel(dialog.GetFilename().c_str() );
    }
}

void ImportCertificateDlg::OnOK(wxCommandEvent& event) 
{
    OsFile privateKeyFile(mpCertCtrl->GetLabel().c_str());
    // build a key
    char szPkcs12[4096];
    unsigned long actualRead = 0;

    privateKeyFile.open();
    privateKeyFile.read((void*)szPkcs12, sizeof(szPkcs12), actualRead);
    privateKeyFile.close();

    UtlString dbLocation; 
    UtlString certNickname;
    UtlString certDbPassword;
    
    sipXezPhoneSettings::getInstance().getSmimeParameters(dbLocation, certNickname, certDbPassword);

    bool bRet = false;
//    SIPX_RESULT rc =  sipxConfigLoadSecurityRuntime();
//    if (SIPX_RESULT_SUCCESS == rc)
    {
#ifdef HAVE_NSS
        bRet = SmimeBody::importPKCS12Object(szPkcs12,
                                    actualRead,
                                    mpPkcs12PasswordCtrl->GetLabel().c_str(),
                                    dbLocation,
                                    certDbPassword);
#endif
    }
                                  
    if (bRet)
    {
        wxMessageDialog message(this, "Success.", "Import Certificate", wxOK);
        message.ShowModal();
        wxDialog::OnOK(event) ;
    }
    else
    {
//        if (SIPX_RESULT_SUCCESS != rc)
//        {
//            wxMessageDialog message(this, "Failure.  Could not load runtime security modules.", "Import Certificate", wxOK);
//            message.ShowModal();
//        }
//        else
        {
            wxMessageDialog message(this, "Failure.  Make sure the Security Settings (see menu) and the passwords are correct.", "Import Certificate", wxOK);
            message.ShowModal();
        }
    }                                  
                                  
}