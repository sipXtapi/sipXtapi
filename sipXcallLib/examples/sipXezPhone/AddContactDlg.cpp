/////////////////////////////////////////////////////////////////////////////
// Name:        EventLogDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   Copyright (c) 2005 Pingtel Corp.
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif
#define ID_CERT_FILE_BUTTON 8001

#include "AddContactDlg.h"
#include "sipXmgr.h"


BEGIN_EVENT_TABLE(AddContactDlg, wxDialog)
    EVT_BUTTON( wxID_OK, AddContactDlg::OnOK )
    EVT_BUTTON( ID_CERT_FILE_BUTTON, AddContactDlg::OnCertFileButton )
END_EVENT_TABLE()

AddContactDlg::AddContactDlg( wxWindow *parent, wxWindowID id, const wxString &title)
        : wxDialog( parent, id, title, wxDefaultPosition, wxSize(500, 200), 
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{    
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    mpNameCtrl = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, 
            wxSize(300, 20)) ;
      
    mpUrlCtrl = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, 
            wxSize(300, 20)) ;

    mpCertCtrl = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, 
            wxSize(280, 20)) ;

    topsizer->Add(new wxStaticText(this, -1, "Name:")) ;

    topsizer->Add(mpNameCtrl,
            1,            // make vertically stretchable
            wxEXPAND |    // make horizontally stretchable
            wxALL,        //   and make border all around
            10 );         // set border width to 10

    topsizer->Add(new wxStaticText(this, -1, "URI:")) ;

    topsizer->Add(mpUrlCtrl,
            1,            // make vertically stretchable
            wxEXPAND |    // make horizontally stretchable
            wxALL,        //   and make border all around
            10 );         // set border width to 10


    topsizer->Add(new wxStaticText(this, -1, "Certificate:")) ;
    wxBoxSizer *cert_sizer = new wxBoxSizer( wxHORIZONTAL  );
    cert_sizer->Add(mpCertCtrl, 1, wxEXPAND | wxALL, 10);
    cert_sizer->Add( new wxButton (this, ID_CERT_FILE_BUTTON, "...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT ),
                     0, wxALL, 10);

    topsizer->Add(cert_sizer);


    wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL  );

    button_sizer->Add(
            new wxButton( this, wxID_OK, "OK" ),
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

AddContactDlg::~AddContactDlg() 
{    
}


void AddContactDlg::OnOK(wxCommandEvent& event) 
{
    mName = mpNameCtrl->GetValue() ;
    mUrl = mpUrlCtrl->GetValue() ;
    mCert = mpCertCtrl->GetValue();

    wxDialog::OnOK(event) ;
}

void AddContactDlg::OnCertFileButton(wxCommandEvent& event) 
{
    wxFileDialog dialog
                 (
                    this,
                    "Add Contact's public key file",
                    "",
                    "",
                    "Certificate files (*.der)|*.der"
                 );

    dialog.SetDirectory(wxGetCwd());

    if (dialog.ShowModal() == wxID_OK)
    {
        mpCertCtrl->SetLabel(dialog.GetFilename().c_str() );
    }

}