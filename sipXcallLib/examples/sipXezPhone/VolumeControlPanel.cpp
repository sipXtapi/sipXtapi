//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "stdwx.h"
#include "VolumeControlPanel.h"
#include "sipXmgr.h"
#include "sipXezPhoneSettings.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
BEGIN_EVENT_TABLE(VolumeControlPanel, wxPanel)
   EVT_COMMAND_SCROLL(IDR_SPEAKER_SLIDER, VolumeControlPanel::OnSpeakerSlider)
   EVT_COMMAND_SCROLL(IDR_MICROPHONE_SLIDER, VolumeControlPanel::OnMicrophoneSlider)
END_EVENT_TABLE()

// Constructor
VolumeControlPanel::VolumeControlPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size, const int volumeLevel, const int gainLevel) :
   wxPanel(parent, IDR_VOLUME_CONTROL_PANEL, pos, size, wxTAB_TRAVERSAL, "VolumeControlPanel")
{
   wxColor* pPanelColor = & (sipXezPhoneSettings::getInstance().getBackgroundColor());
   SetBackgroundColour(*pPanelColor);

        wxPoint origin(0,0);
        wxSize controlSize(0, 0);

    origin.x = 0;
    origin.y = 7;
    controlSize.SetHeight(16);
    controlSize.SetWidth(16);
    wxBitmap* pSpeakerBitmap = new wxBitmap("res/speaker.bmp", wxBITMAP_TYPE_BMP );
    pSpeakerBitmap->SetMask(new wxMask(*pSpeakerBitmap, * (wxTheColourDatabase->FindColour("RED"))));
    mpSpeakerBitmap = new wxStaticBitmap(this, -1, *pSpeakerBitmap, origin, controlSize);

        origin.x = 0;
        origin.y = 30;
        controlSize.SetHeight(size.GetHeight()-50);
        controlSize.SetWidth(16);
        mpVolumeControl = new wxSlider(this, IDR_SPEAKER_SLIDER, VOLUME_MAX + 1 - volumeLevel, 0, VOLUME_MAX, origin, controlSize, wxSL_VERTICAL);
        mpVolumeControl->SetThumbLength(10);

        origin.x = 16;
        mpMicGainControl = new wxSlider(this, IDR_MICROPHONE_SLIDER, GAIN_MAX + 1 - gainLevel, 0, GAIN_MAX, origin, controlSize, wxSL_VERTICAL);
        mpMicGainControl->SetThumbLength(10);

    origin.x = 16;
    origin.y = 5;
    controlSize.SetHeight(16);
    controlSize.SetWidth(16);
    wxBitmap* pMicBitmap = new wxBitmap("res/microphone.bmp", wxBITMAP_TYPE_BMP );
    pMicBitmap->SetMask(new wxMask(*pMicBitmap, * (wxTheColourDatabase->FindColour("RED"))));
    mpMicBitmap = new wxStaticBitmap(this, -1, *pMicBitmap, origin, controlSize);

}

void VolumeControlPanel::OnSpeakerSlider(wxScrollEvent& event)
{
   if (VOLUME_MAX + 1 - mpVolumeControl->GetValue() != sipXmgr::getInstance().getSpeakerVolume())
   {
      sipXmgr::getInstance().setSpeakerVolume(VOLUME_MAX + 1 - mpVolumeControl->GetValue());
      sipXezPhoneSettings::getInstance().setSpeakerVolume(VOLUME_MAX + 1 - mpVolumeControl->GetValue());
   }
}

void VolumeControlPanel::OnMicrophoneSlider(wxScrollEvent& event)
{
   if (GAIN_MAX + 1 - mpMicGainControl->GetValue() != sipXmgr::getInstance().getMicGain())
   {
      sipXmgr::getInstance().setMicGain(GAIN_MAX + 1 - mpMicGainControl->GetValue());
      sipXezPhoneSettings::getInstance().setMicGain(GAIN_MAX + 1 - mpMicGainControl->GetValue());
   }
}

// Destructor
VolumeControlPanel::~VolumeControlPanel()
{
}

void VolumeControlPanel::UpdateBackground(wxColour color)
{
    SetBackgroundColour(color);

    mpMicBitmap->SetBackgroundColour(color);
    mpSpeakerBitmap->SetBackgroundColour(color);

    mpVolumeControl->SetFocus();
    mpVolumeControl->SetBackgroundColour(color);
    mpMicGainControl->SetFocus();
    mpMicGainControl->SetBackgroundColour(color);
}

