/////////////////////////////////////////////////////////////////////////////
// Name:        sipXVideoSettingsDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "sipXVideoSettingsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "sipXVideoSettingsDlg.h"
#include "sipXezPhoneSettings.h"
#include "sipXmgr.h"

// WDR: class implementations

static wxListBox* codecListControl = NULL;
static wxChoice* codecPrefControl = NULL;
static wxStaticText* helpControl = NULL;
static wxButton* selectButton = NULL;
static wxTextCtrl* qualityControl = NULL;
static wxTextCtrl* bitrateControl = NULL;
static wxTextCtrl* framerateControl = NULL;
static int g_bandWidth;

static wxString bwChoices[] = {
    "Low Bandwidth", "Normal Bandwidth", "High Bandwidth"
};


BEGIN_EVENT_TABLE(sipXVideoSettingsDlg,wxDialog)
    EVT_BUTTON( wxID_OK, sipXVideoSettingsDlg::OnOk )
    EVT_BUTTON( wxID_CANCEL, sipXVideoSettingsDlg::OnCancel )
    EVT_BUTTON( ID_VIDEO_SELECT_SINGLE, sipXVideoSettingsDlg::OnSelect )
    EVT_CHOICE( ID_VIDEO_BANDWIDTH_CHOICE, sipXVideoSettingsDlg::OnCodec )
END_EVENT_TABLE()


wxSizer *sipXVideoSettingsDlgFunc(wxWindow *parent, bool call_fit, bool set_sizer )
{
    codecListControl = NULL;
    wxGridSizer *grid0 = new wxGridSizer( 1, 0, 0 );
    char buffer[32];

    int iQuality, iBitrate, iFramerate;

    sipXezPhoneSettings::getInstance().getVideoParameters(iQuality, iBitrate, iFramerate);

    new wxStaticBox(parent, -1, wxT("Video"), wxDefaultPosition, wxSize(314, 75), wxALIGN_LEFT);
    new wxStaticText(parent, -1, wxT("Quality (1-3)"), wxPoint(15,20), wxSize(70,20), wxALIGN_LEFT);
    sprintf(buffer, "%d", iQuality);
    wxTextCtrl *quality = new wxTextCtrl(parent, ID_VIDEO_QUALITY, wxT(buffer), wxPoint(90, 18), wxSize(50, 20), wxALIGN_LEFT);

    new wxStaticText(parent, -1, wxT("Bit rate"), wxPoint(15,43), wxSize(70,20), wxALIGN_LEFT);
    sprintf(buffer, "%d", iBitrate);
    wxTextCtrl *bitRate = new wxTextCtrl(parent, ID_VIDEO_BITRATE, wxT(buffer), wxPoint(90, 41), wxSize(50, 20), wxALIGN_LEFT);

    new wxStaticText(parent, -1, wxT("Frame rate"), wxPoint(170,20), wxSize(70,20), wxALIGN_LEFT);
    sprintf(buffer, "%d", iFramerate);
    wxTextCtrl *frameRate = new wxTextCtrl(parent, ID_VIDEO_FRAMERATE, wxT(buffer), wxPoint(240, 20), wxSize(50, 20), wxALIGN_LEFT);

    new wxStaticBox(parent, -1, wxT("Codecs"), wxPoint(-1,80), wxSize(314, 180), wxALIGN_LEFT);

    new wxStaticText(parent, ID_TEXT, wxT("Codec preferences"), wxPoint(10,100), wxSize(180, 20), wxALIGN_LEFT );
    wxChoice *item5 = new wxChoice(parent, ID_VIDEO_BANDWIDTH_CHOICE, wxPoint(180,98), wxSize(-1,-1), 3, bwChoices);

    wxButton* item7 = new wxButton(parent, ID_VIDEO_SELECT_SINGLE, wxT("Select codec"), wxPoint(25, 177), wxDefaultSize);
    selectButton = item7;
    codecPrefControl = item5;
    wxStaticText* item6 = new wxStaticText(parent, ID_TEXT, wxT(""), wxPoint(10,205), wxSize(130, 40), wxALIGN_LEFT );
    helpControl = item6;
    qualityControl =  quality;
    bitrateControl = bitRate;
    framerateControl = frameRate;

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
    
    return grid0;
}

//----------------------------------------------------------------------------
// sipXVideoSettingsDlg
//----------------------------------------------------------------------------

sipXVideoSettingsDlg::sipXVideoSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    // WDR: dialog function sipXVideoSettingsDlgFunc for sipXVideoSettingsDlg
    sipXVideoSettingsDlgFunc(this, TRUE, TRUE ); 

}

// WDR: handler implementations for sipXVideoSettingsDlg

void sipXVideoSettingsDlg::OnOk(wxCommandEvent &event)
{
    wxString x;
    wxChoice* pChoice;
    int pos;

    int iQuality;
    int iBitRate;
    int iFrameRate;

    x = qualityControl->GetLineText(0);
    iQuality = atoi(x);

    if (iQuality<1 || iQuality>3)
    {
        iQuality = 2;
    }

    x = bitrateControl->GetLineText(0);
    iBitRate = atoi(x);

    x = framerateControl->GetLineText(0);
    iFrameRate = atoi(x);

    sipXmgr::getInstance().setVideoParameters(iQuality, iBitRate, iFrameRate);

    pChoice = (wxChoice*)sipXVideoSettingsDlg::FindWindowById(ID_VIDEO_BANDWIDTH_CHOICE, this);
    pos = pChoice->GetSelection();
    
    // Don't change preferences on custom setting
    if (g_bandWidth != 4)
    {
        sipXmgr::getInstance().setVideoCodecPreferences(pos+1);
        sipXezPhoneSettings::getInstance().setVideoCodecPref(pos+1);
    }

    sipXezPhoneSettings::getInstance().setVideoParameters(iQuality, iBitRate, iFrameRate);
    sipXezPhoneSettings::getInstance().saveSettings();
    event.Skip();
}

void sipXVideoSettingsDlg::OnCancel(wxCommandEvent &event)
{
    event.Skip();
}

void sipXVideoSettingsDlg::OnCodec(wxCommandEvent &event)
{
    int i = event.GetSelection();

    if (codecListControl && i < 3)
    {
        sipXmgr::getInstance().setVideoCodecPreferences(i+1);
        codecListControl->Clear();

        UtlString sData = "*sipXtapi error*";
        sipXmgr::getInstance().getVideoCodecList(sData);

        char *tokTmp;
        char *str = (char*)sData.data();
    
        tokTmp = strtok(str, "\n");

        while (tokTmp != NULL)
        {
            codecListControl->Append(wxT(tokTmp));
            tokTmp = strtok(NULL, "\n");
        }
        // Count == 4 indicates we were working with one selected codec
        // and are now changing back to bandwidth selection
        if (codecPrefControl->GetCount() == 4)
        {
            codecPrefControl->Delete(g_bandWidth-1);
            DeselectedCodec();
        }
        g_bandWidth = i + 1;
    }
}

void sipXVideoSettingsDlg::OnSelect(wxCommandEvent &event)
{
    wxString x;

    int i = codecListControl->GetSelection();

    if (i != -1)
    {
        x = codecListControl->GetStringSelection();
        g_bandWidth = 4;

        codecPrefControl->Append("By name:");
        codecPrefControl->SetSelection(g_bandWidth-1);

        sipXmgr::getInstance().setVideoCodecByName((const char *)x);
        codecListControl->Clear();

        UtlString sData = "*sipXtapi error*";
        sipXmgr::getInstance().getVideoCodecList(sData);

        char *tokTmp;
        char *str = (char*)sData.data();

        tokTmp = strtok(str, "\n");

        while (tokTmp != NULL)
        {
            codecListControl->Append(wxT(tokTmp));
            tokTmp = strtok(NULL, "\n");
        }
        SelectedCodec();
    }
}

void sipXVideoSettingsDlg::SelectedCodec()
{
    helpControl->SetLabel("(Reset single codec\nselection by selecting a\ndifferent bandwidth.)");
    selectButton->Disable();
}

void sipXVideoSettingsDlg::DeselectedCodec()
{
    helpControl->SetLabel("(Select highlighted codec\nand use only that\ncodec.)");
    selectButton->Enable();
}


