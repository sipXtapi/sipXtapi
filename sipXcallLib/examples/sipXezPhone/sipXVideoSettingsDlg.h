/////////////////////////////////////////////////////////////////////////////
// Name:        sipXezPhoneSettingsDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __sipXVideoSettingsDlg_H__
#define __sipXVideoSettingsDlg_H__

#ifdef __GNUG__
    #pragma interface "sipXVideoSettingsDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "sipXezPhone_wdr.h"

#define ID_VIDEO_BANDWIDTH_CHOICE 8102
#define ID_VIDEO_OK_BUTTON 8103
#define ID_VIDEO_SELECT_SINGLE 8105

// WDR: class declarations

//----------------------------------------------------------------------------
// sipXezPhoneSettingsDlg
//----------------------------------------------------------------------------

class sipXVideoSettingsDlg: public wxDialog
{
public:
    // constructors and destructors
    sipXVideoSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for sipXVideoSettingsDlg
    static wxGridSizer* grid;
    static wxStaticBox* video;

    void SelectedCodec();
    void DeselectedCodec();
    
private:
    // WDR: member variable declarations for sipXVideoSettingsDlg
    
private:
    // WDR: handler declarations for sipXVideoSettingsDlg
    void OnOk( wxCommandEvent &event );
    void OnCancel( wxCommandEvent &event );
    void OnCodec( wxCommandEvent &event );
    void OnSelect( wxCommandEvent &event );

private:
    DECLARE_EVENT_TABLE()
};




#endif
