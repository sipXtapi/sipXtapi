/////////////////////////////////////////////////////////////////////////////
// Name:        sipXSrtpSettingsDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "sipXSrtpSettingsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "sipXSrtpSettingsDlg.h"

#include "sipXezPhoneSettings.h"
#include "sipXmgr.h"

// WDR: class implementations

static wxString srtpCipherTypes[] = {
    "NULL", "AES_128_COUNTER_MODE_1"
};

static wxString srtpAuthTypes[] = {
    "AUTH_NULL", "AUTH_HMAC_SHA1"
};

BEGIN_EVENT_TABLE(sipXSrtpSettingsDlg,wxDialog)
    EVT_BUTTON( wxID_OK, sipXSrtpSettingsDlg::OnOk )
    EVT_BUTTON( wxID_CANCEL, sipXSrtpSettingsDlg::OnCancel )
END_EVENT_TABLE()


wxSizer *sipXSrtpSettingsDlgFunc( wxWindow *parent, bool call_fit, bool set_sizer )
{

    wxGridSizer *grid0 = new wxGridSizer( 1, 0, 0 );

    new wxStaticBox(parent, -1, wxT("SRTP Security"), wxDefaultPosition, wxSize(314, 120), wxALIGN_LEFT);

    new wxStaticBox(parent, -1, wxT("SRTP Keys"), wxPoint(1,128), wxSize(314, 100), wxALIGN_LEFT);

    new wxStaticText(parent, ID_TEXT, wxT("Cipher type"), wxPoint(10,20), wxSize(70, 20), wxALIGN_LEFT );
    wxChoice *item1 = new wxChoice(parent, ID_RTP_CIPHER_TYPE, wxPoint(90, 18), wxSize(-1,-1), 2, srtpCipherTypes);
    item1->SetSelection(0);

    new wxStaticText(parent, ID_TEXT, wxT("Authentication"), wxPoint(10,47), wxSize(70, 20), wxALIGN_LEFT );
    wxChoice *item2 = new wxChoice(parent, ID_RTP_AUTH_TYPE, wxPoint(90, 45), wxSize(-1,-1), 2, srtpAuthTypes);
    item2->SetSelection(0);

    new wxStaticText(parent, ID_TEXT, wxT("Security"), wxPoint(10, 74), wxSize(70, 20), wxALIGN_LEFT );
    wxCheckBox *item3 = new wxCheckBox(parent, ID_RTP_ENABLE_AUTH, wxT("Authentication"), wxPoint(90, 74), wxDefaultSize, 0 );
    wxCheckBox *item4 = new wxCheckBox(parent, ID_RTP_ENABLE_ENCR, wxT("Encryption"), wxPoint(90,94), wxDefaultSize, 0 );



    /*wxButton* item7 = new wxButton(parent, ID_VIDEO_SELECT_SINGLE, wxT("Select codec"), wxPoint(25, 177), wxDefaultSize);
    selectButton = item7;
    codecPrefControl = item5;
    wxStaticText* item6 = new wxStaticText(parent, ID_TEXT, wxT(""), wxPoint(10,205), wxSize(130, 40), wxALIGN_LEFT );
    helpControl = item6;

    if (sipXmgr::getInstance().getVideoCodecPreferences(&g_bandWidth))
    {
        codecPrefControl->SetSelection(g_bandWidth-1);
    }

    new wxStaticText(parent, ID_TEXT, wxT("Supported codecs for this codec preference\n"), wxPoint(10,130), wxSize(120,-1), wxALIGN_LEFT );

    UtlString sData = "*sipXtapi error*";
    sipXmgr::getInstance().getVideoCodecList(sData);

    wxListBox *item8 = new wxListBox(parent, ID_TEXT, wxPoint(140,130), wxSize(160, 120), 0, NULL, wxLB_ALWAYS_SB);
    codecListControl = item8;

    char *tokTmp;
    char *str = (char*)sData.data();
    
    tokTmp = strtok(str, "\n");

    while (tokTmp != NULL)
    {
        item8->Append(wxT(tokTmp));
        tokTmp = strtok(NULL, "\n");
    }

    ((sipXVideoSettingsDlg*)parent)->DeselectedCodec();

    new wxButton(parent, wxID_OK, wxT("OK"), wxPoint(200, 270), wxDefaultSize);
    */
    return grid0;
}

//----------------------------------------------------------------------------
// sipXSrtpSettingsDlg
//----------------------------------------------------------------------------

sipXSrtpSettingsDlg::sipXSrtpSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    // WDR: dialog function sipXSrtpSettingsDlgFunc for sipXSrtpSettingsDlg
    sipXSrtpSettingsDlgFunc(this, TRUE, TRUE ); 

}

// WDR: handler implementations for sipXSrtpSettingsDlg

void sipXSrtpSettingsDlg::OnOk(wxCommandEvent &event)
{
    wxString x;
    wxChoice* pChoice;
    int pos;

/*    pChoice = (wxChoice*)sipXVideoSettingsDlg::FindWindowById(ID_VIDEO_BANDWIDTH_CHOICE, this);
    pos = pChoice->GetSelection();
    
    // Don't change preferences on custom setting
    if (g_bandWidth != 4)
    {
        sipXmgr::getInstance().setVideoCodecPreferences(pos+1);
        sipXezPhoneSettings::getInstance().setVideoCodecPref(pos+1);

        sipXezPhoneSettings::getInstance().saveSettings();
    }
    */
    
    event.Skip();
}

void sipXSrtpSettingsDlg::OnCancel(wxCommandEvent &event)
{
    event.Skip();
}
