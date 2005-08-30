/////////////////////////////////////////////////////////////////////////////
// Name:        sipXezPhoneSettingsDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __sipXAudioSettingsDlg_H__
#define __sipXAudioSettingsDlg_H__

#ifdef __GNUG__
    #pragma interface "sipXAudioSettingsDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "sipXezPhone_wdr.h"

#define ID_ENABLE_AEC 8000
#define ID_ENABLE_OUT_DTMF 8001
#define ID_BANDWIDTH_CHOICE 8002
#define ID_AUDIO_OK_BUTTON 8003
#define ID_ENABLE_SRTP 8004
#define ID_SELECT_SINGLE 8005

// WDR: class declarations

//----------------------------------------------------------------------------
// sipXezPhoneSettingsDlg
//----------------------------------------------------------------------------

class sipXAudioSettingsDlg: public wxDialog
{
public:
    // constructors and destructors
    sipXAudioSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for sipXAudioSettingsDlg
    static wxGridSizer* grid;
    static wxStaticBox* audio;

    void SelectedCodec();
    void DeselectedCodec();
    
private:
    // WDR: member variable declarations for sipXAudioSettingsDlg
    
private:
    // WDR: handler declarations for sipXAudioSettingsDlg
    void OnOk( wxCommandEvent &event );
    void OnCancel( wxCommandEvent &event );
    void OnCodec( wxCommandEvent &event );
    void OnSelect( wxCommandEvent &event );

private:
    DECLARE_EVENT_TABLE()
};




#endif
