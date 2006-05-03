/////////////////////////////////////////////////////////////////////////////
// Name:        sipXAudioSettingsDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "sipXAudioSettingsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "sipXAudioSettingsDlg.h"
#include "sipXezPhoneSettings.h"
#include "sipXmgr.h"

// WDR: class implementations
static    wxListBox* codecListControl = NULL;
static    wxChoice* codecPrefControl = NULL;

static wxString bwChoices[] = {
    "Low Bandwidth", "Normal Bandwidth", "High Bandwidth", "Custom"
};


BEGIN_EVENT_TABLE(sipXAudioSettingsDlg,wxDialog)
    EVT_BUTTON( wxID_OK, sipXAudioSettingsDlg::OnOk )
    EVT_BUTTON( wxID_CANCEL, sipXAudioSettingsDlg::OnCancel )
    EVT_CHOICE( ID_BANDWIDTH_CHOICE, sipXAudioSettingsDlg::OnChangeBandwidth )
END_EVENT_TABLE()


wxSizer *sipXAudioSettingsDlgFunc( wxWindow *parent, bool call_fit, bool set_sizer )
{
    codecListControl = NULL;
    wxGridSizer *grid0 = new wxGridSizer( 1, 0, 0 );

    new wxStaticBox(parent, -1, wxT("Audio"), wxDefaultPosition, wxSize(314, 75), wxALIGN_LEFT);

    new wxStaticText(parent, ID_TEXT, wxT("Enable echo cancellation?"), wxPoint(10,20), wxSize(130,20), wxALIGN_LEFT );
    wxCheckBox *item01 = new wxCheckBox(parent, ID_ENABLE_AEC, wxT(""), wxPoint(150,20), wxDefaultSize, 0 );
    item01->SetValue(sipXmgr::getInstance().isAECEnabled());

    new wxStaticText(parent, ID_TEXT, wxT("Enable ouf-of-band DTMF?"), wxPoint(10,45), wxSize(130, 20), wxALIGN_LEFT );
    wxCheckBox *item3 = new wxCheckBox(parent, ID_ENABLE_OUT_DTMF, wxT(""), wxPoint(150,45), wxDefaultSize, 0 );
    item3->SetValue(sipXmgr::getInstance().isOutOfBandDTMFEnabled());

    new wxStaticText(parent, ID_TEXT, wxT("Enable SRTP?"), wxPoint(190,20), wxSize(120, 20), wxALIGN_LEFT );
    wxCheckBox *item11 = new wxCheckBox(parent, ID_ENABLE_SRTP, wxT(""), wxPoint(290,20), wxDefaultSize, 0 );
    item11->SetValue(sipXmgr::getInstance().isSRTPEnabled());

    new wxStaticBox(parent, -1, wxT("Codecs"), wxPoint(-1,80), wxSize(314, 180), wxALIGN_LEFT);

    new wxStaticText(parent, ID_TEXT, wxT("Codec preferences"), wxPoint(10,100), wxSize(180, 20), wxALIGN_LEFT );
    wxChoice *item5 = new wxChoice(parent, ID_BANDWIDTH_CHOICE, wxPoint(180,98), wxSize(-1,-1), 4, bwChoices);
    codecPrefControl = item5;

    wxListBox *item8 = new wxListBox(parent, ID_TEXT, wxPoint(10,130), wxSize(295, 120), 0, NULL, wxLB_ALWAYS_SB | wxLB_MULTIPLE);
    codecListControl = item8;

    new wxButton(parent, wxID_OK, wxT("OK"), wxPoint(200, 270), wxDefaultSize);
    
    return grid0;
}

//----------------------------------------------------------------------------
// sipXAudioSettingsDlg
//----------------------------------------------------------------------------

sipXAudioSettingsDlg::sipXAudioSettingsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    // WDR: dialog function sipXAudioSettingsDlgFunc for sipXAudioSettingsDlg
    sipXAudioSettingsDlgFunc(this, TRUE, TRUE ); 

    sipXmgr::getInstance().getCodecPreferences((int*)&mOriginalBandwidth) ;
    sipXmgr::getInstance().getCodecList(mOriginalCodecList);

    if (sipXmgr::getInstance().getCodecPreferences((int*)&mBandwidth))
    {
        codecPrefControl->SetSelection(mBandwidth-1);
        PopulateCodecList() ;
    }
}

// WDR: handler implementations for sipXAudioSettingsDlg

void sipXAudioSettingsDlg::OnOk(wxCommandEvent &event)
{
    wxString x;
    wxCheckBox* pCheck;
    wxChoice* pChoice;
    int pos;
    bool b;

    pCheck = (wxCheckBox*)sipXAudioSettingsDlg::FindWindowById(ID_ENABLE_OUT_DTMF, this);
    b = pCheck->GetValue();
    sipXmgr::getInstance().enableOutOfBandDTMF(b);
    sipXezPhoneSettings::getInstance().setEnableOOBDTMF(b);

    pCheck = (wxCheckBox*)sipXAudioSettingsDlg::FindWindowById(ID_ENABLE_AEC, this);
    b = pCheck->GetValue();
    sipXmgr::getInstance().enableAEC(b);
    sipXezPhoneSettings::getInstance().setEnableAEC(b);

    pCheck = (wxCheckBox*)sipXAudioSettingsDlg::FindWindowById(ID_ENABLE_SRTP, this);
    b = pCheck->GetValue();
    sipXmgr::getInstance().enableSRTP(b);
    sipXezPhoneSettings::getInstance().setEnableSRTP(b);

    pChoice = (wxChoice*)sipXAudioSettingsDlg::FindWindowById(ID_BANDWIDTH_CHOICE, this);
    pos = pChoice->GetSelection();
    
    sipXmgr::getInstance().setCodecPreferences(pos+1);
    
    // loop through the list box and save each as a selected codec
    int count = codecListControl->GetCount();
    wxString codecName;
    UtlString codecList ;
    UtlString cleanCodecList ;
    for (int i = 0; i < count; i++)
    {
        if (codecListControl->Selected(i))
        {
            codecName = codecListControl->GetString(i);
            codecList.append(codecName) ;
            codecList.append(" " ) ;

            UtlString temp = codecName.c_str() ;
            int index = temp.first(' ') ;
            if (index > 0)
            {
                temp.remove(index) ;
            }

            cleanCodecList.append(temp) ;
            cleanCodecList.append(" ") ;
        }
    }
    
    sipXmgr::getInstance().setAudioCodecByName((const char *)codecList.data());
        
    sipXezPhoneSettings::getInstance().setSelectedAudioCodecs(cleanCodecList.data()) ;
    sipXezPhoneSettings::getInstance().setCodecPref(pos+1);
    sipXezPhoneSettings::getInstance().saveSettings();

    event.Skip();
}

void sipXAudioSettingsDlg::OnCancel(wxCommandEvent &event)
{
    sipXmgr::getInstance().setCodecPreferences(mOriginalBandwidth) ;
    sipXezPhoneSettings::getInstance().setSelectedAudioCodecs(mOriginalCodecList) ;
    sipXmgr::getInstance().setAudioCodecByName(mOriginalCodecList.data());
    
    event.Skip();
}

void sipXAudioSettingsDlg::OnChangeBandwidth(wxCommandEvent &event)
{
    int sel = event.GetSelection();

    mBandwidth = (bandwidth)(sel + 1);
    PopulateCodecList() ;
}

void sipXAudioSettingsDlg::PopulateCodecList() 
{
    if (codecListControl)
    {
        switch (mBandwidth)
        {
            case LOW:
            case MEDIUM:
            case HIGH:
            {
                // Unselect all
                // codecListControl->Enable() ;
                sipXmgr::getInstance().setCodecPreferences(mBandwidth);
                codecListControl->Clear();

                UtlString sData = "*sipXtapi error*";
                sipXmgr::getInstance().getCodecList(sData);

                char *tokTmp;
                char *str = (char*)sData.data();

                tokTmp = strtok(str, "\n");

                while (tokTmp != NULL)
                {
                    int iID = codecListControl->Append(wxT(tokTmp));
                    tokTmp = strtok(NULL, "\n");
                }
                break ;
            }
            case CUSTOM:
            {
                // codecListControl->Enable() ;

                sipXmgr::getInstance().setCodecPreferences(HIGH-1);
                codecListControl->Clear();

                UtlString sData = "*sipXtapi error*";
                sipXmgr::getInstance().getCodecList(sData);

                char *tokTmp;
                char *str = (char*)sData.data();

                tokTmp = strtok(str, "\n");
                while (tokTmp != NULL)
                {
                    int iID = codecListControl->Append(wxT(tokTmp));
                    if (mOriginalCodecList.contains(tokTmp))
                    {
                        codecListControl->SetSelection(iID) ;
                    }

                    tokTmp = strtok(NULL, "\n");
                }               
                break ;
            }
        }
    }
}
