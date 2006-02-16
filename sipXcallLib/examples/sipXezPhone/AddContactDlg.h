/////////////////////////////////////////////////////////////////////////////
// Name:        EventLogDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   Copyright (c) 2005 Pingtel Corp.
/////////////////////////////////////////////////////////////////////////////

#ifndef _EVENTLOGDLG_H
#define _EVENTLOGDLG_H /* [ */

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

//#include "sipXezPhone_wdr.h"

//----------------------------------------------------------------------------
// EventLogDlg
//----------------------------------------------------------------------------

class AddContactDlg: public wxDialog
{
public:
    // constructors and destructors
    AddContactDlg( wxWindow *parent, wxWindowID id, const wxString &title) ;
    virtual ~AddContactDlg() ;

    wxString getName() { return mName; } ;
    wxString getUrl() { return mUrl; } ;

    virtual void OnOK(wxCommandEvent& event) ;
    
protected:    

private:
    DECLARE_EVENT_TABLE()

    wxTextCtrl* mpNameCtrl ;
    wxTextCtrl* mpUrlCtrl ;

    wxString mName ;
    wxString mUrl ;
};


#endif /* _EVENTLOGDLG_H ] */

