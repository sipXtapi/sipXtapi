/////////////////////////////////////////////////////////////////////////////
// Name:        sipXezPhoneSettingsDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __sipXezPhoneSettingsDlg_H__
#define __sipXezPhoneSettingsDlg_H__

#ifdef __GNUG__
    #pragma interface "sipXezPhoneSettingsDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "sipXezPhone_wdr.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// sipXezPhoneSettingsDlg
//----------------------------------------------------------------------------

class sipXezPhoneSettingsDlg: public wxDialog
{
public:
    // constructors and destructors
    sipXezPhoneSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for sipXezPhoneSettingsDlg
    
private:
    // WDR: member variable declarations for sipXezPhoneSettingsDlg
    
private:
    // WDR: handler declarations for sipXezPhoneSettingsDlg
    void OnOk( wxCommandEvent &event );
    void OnCancel( wxCommandEvent &event );

private:
    DECLARE_EVENT_TABLE()
};




#endif
