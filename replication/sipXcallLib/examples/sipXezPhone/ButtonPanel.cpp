//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "stdwx.h"
#include "ButtonPanel.h"
#include "states/PhoneStateMachine.h"
#include "sipXmgr.h"
#include "sipXezPhoneApp.h"
#include "sipXezPhoneSettings.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
BEGIN_EVENT_TABLE(ButtonPanel, wxPanel)
   EVT_BUTTON(IDR_DIAL_BUTTON_HANDSET, ButtonPanel::OnHandsetClick)
   EVT_BUTTON(IDR_DIAL_BUTTON_HOLD, ButtonPanel::OnHoldButton)
   EVT_BUTTON(IDR_BUTTON_TRANSFER,  ButtonPanel::OnTransferButton)
   EVT_BUTTON(IDR_MUTE_BUTTON, ButtonPanel::OnMuteButton)
END_EVENT_TABLE()


// Constructor
ButtonPanel::ButtonPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   wxPanel(parent, IDR_BUTTON_PANEL, pos, size, wxTAB_TRAVERSAL, "ButtonPanel")
{
   wxColor* pPanelColor = & (sipXezPhoneSettings::getInstance().getBackgroundColor());
   SetBackgroundColour(*pPanelColor);

   wxColor* wxLightBlue = wxTheColourDatabase->FindColour("LIGHT BLUE");
   mpGridSizer = new wxGridSizer(1,4,5,5);

   wxBitmap bitmapHandset("res/handset.bmp",wxBITMAP_TYPE_BMP);
   bitmapHandset.SetMask(new wxMask(bitmapHandset, * (wxTheColourDatabase->FindColour("RED"))));
   mpButtonHandset = new wxBitmapButton(this, IDR_DIAL_BUTTON_HANDSET, bitmapHandset, wxDefaultPosition, wxSize(36,30));
   mpButtonHandset->SetBackgroundColour(*wxLightBlue);
#ifdef _WIN32
   mpButtonHandset->SetToolTip("Pick Up / Hang Up.");
#endif
   
   wxBitmap bitmapHold("res/hold.bmp",wxBITMAP_TYPE_BMP);
   bitmapHold.SetMask(new wxMask(bitmapHold, * (wxTheColourDatabase->FindColour("RED"))));
   mpButtonHold = new wxBitmapButton(this, IDR_DIAL_BUTTON_HOLD, bitmapHold, wxDefaultPosition, wxSize(36,30));
   mpButtonHold->SetBackgroundColour(*wxLightBlue);
#ifdef _WIN32
   mpButtonHold->SetToolTip("Hold.");
#endif


   wxBitmap bitmapTransfer("res/xfer.bmp",wxBITMAP_TYPE_BMP);
   bitmapTransfer.SetMask(new wxMask(bitmapTransfer, * (wxTheColourDatabase->FindColour("RED"))));
   mpButtonTransfer = new wxBitmapButton(this, IDR_BUTTON_TRANSFER, bitmapTransfer, wxDefaultPosition, wxSize(36,30));
   mpButtonTransfer->SetBackgroundColour(*wxLightBlue);
#ifdef _WIN32
   mpButtonTransfer->SetToolTip("Transfer.");
#endif


   wxBitmap bitmapMute("res/mute.bmp",wxBITMAP_TYPE_BMP);
   bitmapMute.SetMask(new wxMask(bitmapMute, * (wxTheColourDatabase->FindColour("BLUE"))));
   mpButtonMute = new wxBitmapButton(this, IDR_MUTE_BUTTON, bitmapMute, wxDefaultPosition, wxSize(36,30));
   mpButtonMute->SetBackgroundColour(*wxLightBlue);
#ifdef _WIN32
   mpButtonMute->SetToolTip("Mute On/Off.");
#endif

   mpGridSizer->Add(mpButtonHandset, 1);
   mpGridSizer->Add(mpButtonHold, 1);
   mpGridSizer->Add(mpButtonTransfer, 1);
   mpGridSizer->Add(mpButtonMute, 1);

   SetSizer(mpGridSizer, true);
   SetAutoLayout(TRUE);
   Layout();
}


// Destructor
ButtonPanel::~ButtonPanel()
{
}

void ButtonPanel::OnHandsetClick(wxEvent& event)
{
   PhoneStateMachine::getInstance().OnFlashButton();
}

void ButtonPanel::OnHoldButton(wxEvent& event)
{
    PhoneStateMachine::getInstance().OnHoldButton();
}

void ButtonPanel::OnMuteButton(wxEvent &event)
{
    sipXmgr::getInstance().toggleMute();
}

void ButtonPanel::OnTransferButton(wxEvent& event)
{
    PhoneStateMachine::getInstance().OnTransferRequested(thePhoneApp->getEnteredText());
}
