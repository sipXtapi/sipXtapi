/////////////////////////////////////////////////////////////////////////////
// Name:        sipXezPhoneAboutDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __sipXezPhoneAboutDlg_H__
#define __sipXezPhoneAboutDlg_H__

#ifdef __GNUG__
    #pragma interface "sipXezPhoneAboutDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "sipXezPhone_wdr.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// sipXezPhoneAboutDlg
//----------------------------------------------------------------------------

class sipXezPhoneAboutDlg: public wxDialog
{
public:
    // constructors and destructors
    sipXezPhoneAboutDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for sipXezPhoneAboutDlg
    
private:
    // WDR: member variable declarations for sipXezPhoneAboutDlg
    
private:
    // WDR: handler declarations for sipXezPhoneAboutDlg

private:
    DECLARE_EVENT_TABLE()
};




#endif
