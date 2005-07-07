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
#include "MainPanel.h"
#include "sipXmgr.h"
#include "sipXezPhoneSettings.h"
#include "sipXezPhoneSettingsDlg.h"
#include "states/PhoneStateMachine.h"
#include "states/PhoneStateConnected.h"
#include "states/PhoneStateCallHeld.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS
BEGIN_EVENT_TABLE(MainPanel, wxPanel)
    EVT_INIT_DIALOG(OnInitDialog)
END_EVENT_TABLE()

// Constructor
MainPanel::MainPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   wxPanel(parent, IDR_MAINPANEL, pos, size, wxTAB_TRAVERSAL, "MainPanel")
{

   // load the settings
   sipXezPhoneSettings::getInstance().loadSettings();
   
   wxColor* pPanelColor = new wxColor(132,169,181);
   SetBackgroundColour(*pPanelColor);

   // create and add the Volume Controls
	wxPoint origin(220,0);
	wxSize panelSize(190, 350);
	mpVolumeControlPanel = new VolumeControlPanel(this, origin, panelSize,
	   sipXezPhoneSettings::getInstance().getSpeakerVolume(), sipXezPhoneSettings::getInstance().getMicGain());

   // create and add the DialPad
	origin.x = 20;
	origin.y = 134;
	panelSize.SetWidth(174);
	panelSize.SetHeight(148);
	mpDialPadPanel = new DialPadPanel(this, origin, panelSize);

   // create and add the Dial Entry 
	origin.x = 11;
	origin.y = 74;
	panelSize.SetWidth(201);
	panelSize.SetHeight(53);
	mpDialEntryPanel = new DialEntryPanel(this, origin, panelSize);

   // create and add the Button Panel
	origin.x = 42;
	origin.y = 37;
	panelSize.SetWidth(150);
	panelSize.SetHeight(33);
	mpButtonPanel = new ButtonPanel(this, origin, panelSize);

   // add the Call Status box
   origin.x = 11;
   origin.y = 10;
   panelSize.SetWidth(201);
   panelSize.SetHeight(19);
   mpCallStatus = new wxTextCtrl(this, IDR_STATUS_BOX, "Idle.", origin, panelSize, wxTE_READONLY | wxTE_CENTRE);
   wxColor* pColor = wxTheColourDatabase->FindColour("BLUE");
   mpCallStatus->SetForegroundColour(*pColor);


   // add the Caller ID box
   origin.x = 11;
   origin.y = 285;
   panelSize.SetWidth(201);
   panelSize.SetHeight(45);
   mpCallerID = new wxTextCtrl(this, IDR_CALLERID_BOX, "", origin, panelSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_CENTRE);
   mpCallerID->SetForegroundColour(*pColor);

   InitDialog();

// TODO - add a call history panel
/*
	origin.x = 2;
	origin.y = 0;
	panelSize.SetWidth(187);
	panelSize.SetHeight(320);
	mpCallHistoryPanel = new CallHistoryPanel(this, origin, panelSize);
*/
}

// Destructor
MainPanel::~MainPanel()
{
   if (dynamic_cast<PhoneStateConnected*>(PhoneStateMachine::getInstance().getState()) ||   // if its in the connected or held state
       dynamic_cast<PhoneStateCallHeld* >(PhoneStateMachine::getInstance().getState()))
   {
      PhoneStateMachine::getInstance().OnFlashButton();
   }
   sipXmgr::release();
   Sleep(3000);
}

// Event handler for the wxInitDialog event
void MainPanel::OnInitDialog(wxInitDialogEvent& event)
{

   // if settings are missing, open the dialog box 
   if ("" == sipXezPhoneSettings::getInstance().getIdentity() ||
       "" == sipXezPhoneSettings::getInstance().getRealm() )
   {
      sipXezPhoneSettingsDlg *pDlg = new sipXezPhoneSettingsDlg((wxWindow*)this, -1, "Configuration Settings");
      pDlg->ShowModal();
      delete pDlg;
   }

   // initialize the sipXtapi layer
   sipXmgr::getInstance().Initialize();
}