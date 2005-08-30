//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "stdwx.h"
#include "DialEntryPanel.h"
#include "sipXezPhoneSettings.h"
#include "sipXmgr.h"
#include "DialerThread.h"
#include "states/PhoneStateMachine.h"
#include "utl/UtlDListIterator.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
BEGIN_EVENT_TABLE(DialEntryPanel, wxPanel)
   EVT_BUTTON(IDR_DIAL_ENTRY_BUTTON, DialEntryPanel::OnButtonClick)
   EVT_TEXT_ENTER(wxID_ANY, DialEntryPanel::OnEnter)
END_EVENT_TABLE()


// Constructor
DialEntryPanel::DialEntryPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   wxPanel(parent, IDR_DIAL_ENTRY_PANEL, pos, size, wxTAB_TRAVERSAL, "DialEntryPanel"),
   mpSizer(NULL),
   mpComboBox(NULL),
   mpOutline(NULL),
   mpListener(NULL)
{

    wxColor* pPanelColor = & (sipXezPhoneSettings::getInstance().getBackgroundColor());
    SetBackgroundColour(*pPanelColor);

    wxPoint origin(0,0);
    mpOutline = new wxStaticBox(this, -1, "Dial", origin, size);
    mpOutline->SetBackgroundColour(*pPanelColor);

    mpSizer = new wxStaticBoxSizer(mpOutline, wxHORIZONTAL);

    wxSize controlSize(150, 20);
    mpComboBox = new wxComboBox(this, IDR_DIAL_ENTRY_TEXT, "", wxDefaultPosition, controlSize);
        mpComboBox->SetBackgroundColour(*pPanelColor);
    mpSizer->Add(mpComboBox);
    // init the combo
    UtlDList& list = sipXezPhoneSettings::getInstance().getRecentNumberList();
    UtlDListIterator iterator(list);
    UtlString* pTemp;
    while (pTemp = (UtlString*)iterator())
    {
        mpComboBox->Append(pTemp->data());
    }


    mpSizer->Add(10, 20); // add a spacer

    controlSize.SetWidth(30);
    controlSize.SetHeight(30);
    wxBitmap bitmap("res/dial.bmp", wxBITMAP_TYPE_BMP );
    bitmap.SetMask(new wxMask(bitmap, * (wxTheColourDatabase->FindColour("RED"))));

    wxColor* wxLightBlue = wxTheColourDatabase->FindColour("LIGHT BLUE");
    mpDialButton = new wxBitmapButton(this, IDR_DIAL_ENTRY_BUTTON, bitmap, wxDefaultPosition, controlSize);
    mpDialButton->SetBackgroundColour(*wxLightBlue);
    mpSizer->Add(mpDialButton);

    SetSizer(mpSizer, true);
    SetAutoLayout(TRUE);
    Layout();

    // add a state machine observer
    mpListener = new DialEntryPhoneStateMachineObserver(this);
    PhoneStateMachine::getInstance().addObserver(mpListener);
}

// Destructor
DialEntryPanel::~DialEntryPanel()
{
    PhoneStateMachine::getInstance().removeObserver(mpListener);
    delete mpListener;
    mpListener = NULL;
}

const wxString DialEntryPanel::getEnteredText()
{
   wxString phoneNumber;

    wxComboBox* pCtrl = dynamic_cast<wxComboBox*>(FindWindowById(IDR_DIAL_ENTRY_TEXT, this));
    if (pCtrl)
    {
        phoneNumber = pCtrl->GetValue();
    }
    return phoneNumber;
}

// Event handler for the button
void DialEntryPanel::OnButtonClick(wxEvent& event)
{
   wxString phoneNumber = getEnteredText();

    wxComboBox* pCtrl = dynamic_cast<wxComboBox*>(FindWindowById(IDR_DIAL_ENTRY_TEXT, this));
    if (phoneNumber != "")
    {
        PhoneStateMachine::getInstance().OnDial(phoneNumber);
    }
}

void DialEntryPanel::OnEnter(wxEvent& event)
{
    wxString phoneNumber = getEnteredText();

    wxComboBox* pCtrl = dynamic_cast<wxComboBox*>(FindWindowById(IDR_DIAL_ENTRY_TEXT, this));
    if (phoneNumber != "")
    {
        PhoneStateMachine::getInstance().OnDial(phoneNumber);
    }
}

PhoneState* DialEntryPanel::DialEntryPhoneStateMachineObserver::OnDial(const wxString phoneNumber)
{
    if (mpOwner->getComboBox().FindString(phoneNumber) == -1)
    {
        mpOwner->getComboBox().Append(phoneNumber);
    }
    return NULL;
}

PhoneState* DialEntryPanel::DialEntryPhoneStateMachineObserver::OnRinging(SIPX_CALL hCall)
{
    char szIncomingNumber[256];
    sipxCallGetRemoteID(hCall, szIncomingNumber, 256);
    wxString incomingNumber(szIncomingNumber);

    if (mpOwner->getComboBox().FindString(incomingNumber) == -1)
    {
        mpOwner->getComboBox().Append(incomingNumber);
    }
    return NULL;
}

