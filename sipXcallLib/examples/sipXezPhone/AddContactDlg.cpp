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

#include "AddContactDlg.h"
#include "sipXmgr.h"


BEGIN_EVENT_TABLE(AddContactDlg, wxDialog)
    EVT_BUTTON( wxID_OK, AddContactDlg::OnOK )
END_EVENT_TABLE()

AddContactDlg::AddContactDlg( wxWindow *parent, wxWindowID id, const wxString &title)
        : wxDialog( parent, id, title, wxDefaultPosition, wxDefaultSize, 
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{    
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    mpNameCtrl = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, 
            wxDefaultSize) ;
      
    mpUrlCtrl = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, 
            wxDefaultSize) ;

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

    wxDialog::OnOK(event) ;
}