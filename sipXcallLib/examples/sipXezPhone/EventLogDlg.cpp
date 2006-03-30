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

#include "EventLogDlg.h"
#include "sipXmgr.h"


#define IDC_REFRESH_LOG     8000
#define IDC_CLEAR_LOG       8001


BEGIN_EVENT_TABLE(EventLogDlg,wxDialog)
    EVT_BUTTON( IDC_REFRESH_LOG, EventLogDlg::OnRefreshLog )
    EVT_BUTTON( IDC_CLEAR_LOG, EventLogDlg::OnClearLog)
END_EVENT_TABLE()

EventLogDlg::EventLogDlg( wxWindow *parent, wxWindowID id, const wxString &title)
        : wxDialog( parent, id, title, wxDefaultPosition, wxDefaultSize, 
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{    
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    mpLogTextCtrl = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, 
            wxSize(350, 250), wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP) ;
      
    topsizer->Add(mpLogTextCtrl,
            1,            // make vertically stretchable
            wxEXPAND |    // make horizontally stretchable
            wxALL,        //   and make border all around
            10 );         // set border width to 10

    wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );

    button_sizer->Add(
            new wxButton( this, IDC_CLEAR_LOG, "&Clear Log" ),
            0,           // make horizontally unstretchable
            wxALL,       // make border all around (implicit top alignment)
            10 );        // set border width to 10

    button_sizer->Add(
            new wxButton( this, IDC_REFRESH_LOG, "&Refresh Log" ),
            0,           // make horizontally unstretchable
            wxALL,       // make border all around (implicit top alignment)
            10 );        // set border width to 10


    button_sizer->Add(
            new wxButton( this, wxID_OK, "Close" ),
            0,           // make horizontally unstretchable
            wxALL,       // make border all around (implicit top alignment)
            10 );        // set border width to 10

    topsizer->Add(
            button_sizer,
            0,                // make vertically unstretchable
            wxALIGN_CENTER ); // no border and centre horizontally

    SetSizer( topsizer );      // use the sizer for layout

    topsizer->SetSizeHints( this );   // set size hints to honour minimum size

    handleRefreshLog() ;
}

EventLogDlg::~EventLogDlg() 
{    
}


void EventLogDlg::OnClearLog(wxCommandEvent &event)
{
    handleClearLog() ;
}


void EventLogDlg::OnRefreshLog(wxCommandEvent &event)
{
    handleRefreshLog() ;
}


void EventLogDlg::handleClearLog() 
{
    sipXmgr::getInstance().clearEventLog() ;
    handleRefreshLog() ;
}


void EventLogDlg::handleRefreshLog() 
{
    wxString contents ;
    sipXmgr::getInstance().getEventLog(contents) ;

    mpLogTextCtrl->SetValue(contents) ;
}
