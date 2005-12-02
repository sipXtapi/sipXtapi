/////////////////////////////////////////////////////////////////////////////
// Name:        sipXezPhoneAboutDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "sipXezPhoneAboutDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "sipXezPhoneAboutDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// sipXezPhoneAboutDlg
//----------------------------------------------------------------------------

// WDR: event table for sipXezPhoneAboutDlg

BEGIN_EVENT_TABLE(sipXezPhoneAboutDlg,wxDialog)
END_EVENT_TABLE()

sipXezPhoneAboutDlg::sipXezPhoneAboutDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    // WDR: dialog function sipXezPhoneAboutDlgFunc for sipXezPhoneAboutDlg
    sipXezPhoneAboutDlgFunc( this, TRUE ); 
}

// WDR: handler implementations for sipXezPhoneAboutDlg




