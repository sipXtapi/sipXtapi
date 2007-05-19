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

class EventLogDlg: public wxDialog
{
public:
    // constructors and destructors
    EventLogDlg( wxWindow *parent, wxWindowID id, const wxString &title) ;
    virtual ~EventLogDlg() ;
    
    void OnClearLog(wxCommandEvent &event) ;
    void OnRefreshLog(wxCommandEvent &event) ;

protected:
    void handleClearLog() ;
    void handleRefreshLog() ;

private:
    DECLARE_EVENT_TABLE()

    wxTextCtrl* mpLogTextCtrl ;
};


#endif /* _EVENTLOGDLG_H ] */

