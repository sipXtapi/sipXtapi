// $Id$
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

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
   EVT_COMMAND_SCROLL(IDR_SPEAKER_SLIDER, OnSpeakerSlider)
   EVT_COMMAND_SCROLL(IDR_MICROPHONE_SLIDER, OnMicrophoneSlider)
END_EVENT_TABLE()   

// Constructor
VolumeControlPanel::VolumeControlPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size, const int volumeLevel, const int gainLevel) :
   wxPanel(parent, IDR_VOLUME_CONTROL_PANEL, pos, size, wxTAB_TRAVERSAL, "VolumeControlPanel")
{
   wxColor* pPanelColor = new wxColor(132,169,181);
   SetBackgroundColour(*pPanelColor);

	wxPoint origin(0,0);
	wxSize controlSize(0, 0);

    origin.x = 0;
    origin.y = 7;
    controlSize.SetHeight(16);
    controlSize.SetWidth(16);
    wxBitmap* pSpeakerBitmap = new wxBitmap("res/speaker.bmp", wxBITMAP_TYPE_BMP );
    pSpeakerBitmap->SetMask(new wxMask(*pSpeakerBitmap, * (wxTheColourDatabase->FindColour("RED"))));
    new wxStaticBitmap(this, -1, *pSpeakerBitmap, origin, controlSize);

	origin.x = 0;
	origin.y = 30;
	controlSize.SetHeight(100);
	controlSize.SetWidth(16);
	mpVolumeControl = new wxSlider(this, IDR_SPEAKER_SLIDER, 101 - volumeLevel, 1, 100, origin, controlSize, wxSL_VERTICAL);
	mpVolumeControl->SetThumbLength(10);

	origin.x = 16;
	mpMicGainControl = new wxSlider(this, IDR_MICROPHONE_SLIDER, 11 - gainLevel, 1, 10, origin, controlSize, wxSL_VERTICAL);
	mpMicGainControl->SetThumbLength(10);

    origin.x = 16;
    origin.y = 5;
    controlSize.SetHeight(16);
    controlSize.SetWidth(16);
    wxBitmap* pMicBitmap = new wxBitmap("res/microphone.bmp", wxBITMAP_TYPE_BMP );
    pMicBitmap->SetMask(new wxMask(*pMicBitmap, * (wxTheColourDatabase->FindColour("RED"))));
    new wxStaticBitmap(this, -1, *pMicBitmap, origin, controlSize);

}

void VolumeControlPanel::OnSpeakerSlider(wxEvent& event)
{
   if (11 - mpVolumeControl->GetValue() != sipXmgr::getInstance().getSpeakerVolume())
   {
      sipXmgr::getInstance().setSpeakerVolume(101 - mpVolumeControl->GetValue());
      sipXezPhoneSettings::getInstance().setSpeakerVolume(101 - mpVolumeControl->GetValue());
   }
}

void VolumeControlPanel::OnMicrophoneSlider(wxEvent& event)
{
   if (11 - mpMicGainControl->GetValue() != sipXmgr::getInstance().getMicGain())
   {
      sipXmgr::getInstance().setMicGain(11 - mpMicGainControl->GetValue());
      sipXezPhoneSettings::getInstance().setMicGain(11 - mpMicGainControl->GetValue());
   }
}

// Destructor
VolumeControlPanel::~VolumeControlPanel()
{
}