//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "stdwx.h"
#include "ConferencePanel.h"
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
BEGIN_EVENT_TABLE(ConferencePanel, wxDialog)
   EVT_BUTTON(IDR_ADD_PARTY_BUTTON, ConferencePanel::OnAddParty)
   EVT_BUTTON(IDR_REMOVE_PARTY_BUTTON, ConferencePanel::OnRemoveParty)
END_EVENT_TABLE()


// Constructor
ConferencePanel::ConferencePanel(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   wxDialog(parent, IDR_CONFERENCE_PANEL, "", pos, size, wxNO_3D, "ConferencePanel")
{
    wxColor* pPanelColor = & (sipXezPhoneSettings::getInstance().getBackgroundColor());
    SetBackgroundColour(*pPanelColor);

    wxColor* wxLightBlue = wxTheColourDatabase->FindColour("LIGHT BLUE");
    mpGridSizer = new wxFlexGridSizer(4,1,5,5);

    SetSizer(mpGridSizer, true);
   

    wxSize parentSize = this->GetSize();

    wxBitmap bitmap("res/conference_title.bmp",wxBITMAP_TYPE_BMP);
    mpGridSizer->Add(new wxBitmapButton(this, -1, bitmap, wxPoint(0, 0), wxSize(bitmap.GetWidth(),bitmap.GetHeight()), 0), 0, wxALIGN_CENTRE);

    wxSize controlSize;
    wxGridSizer* pButtonSizer = new wxGridSizer(1, 2, 5, 5);
    controlSize.x = 30;
    controlSize.y = 30;
    pButtonSizer->Add(mpAddButton = new wxButton(this, IDR_ADD_PARTY_BUTTON, "+", wxDefaultPosition, controlSize), 0, wxALIGN_CENTRE);
    pButtonSizer->Add(mpRemoveButton = new wxButton(this, IDR_REMOVE_PARTY_BUTTON, "-", wxDefaultPosition, controlSize), 0, wxALIGN_CENTRE);
    
    wxFont bigFont = mpAddButton->GetFont();
    bigFont.SetPointSize(12);
    
    mpAddButton->SetFont(bigFont);
    mpRemoveButton->SetFont(bigFont);
    mpGridSizer->Add(pButtonSizer, 0, wxALIGN_CENTRE);

    controlSize.x = parentSize.GetWidth() - 20;
    controlSize.y = 20;
    mpComboBox = new wxComboBox(this, IDR_ADD_PARTY_COMBO, "", wxDefaultPosition, controlSize);
    mpComboBox->SetBackgroundColour(*pPanelColor);
    mpGridSizer->Add(mpComboBox, 0, wxALIGN_CENTRE);
    
    
    controlSize.y = 190;
    mpGridSizer->Add(mpConferenceList = new wxListCtrl(this, IDR_CONFERENCE_LIST, wxDefaultPosition, controlSize, wxLC_HRULES | wxLC_VRULES | wxLC_REPORT| wxLC_SINGLE_SEL | wxSUNKEN_BORDER ), 0, wxALIGN_CENTRE);
    wxListItem itemCol;
    
    itemCol.m_mask = wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH;
    itemCol.m_text = "Party";
    itemCol.SetWidth(141);
    mpConferenceList->InsertColumn(0, itemCol);
    itemCol.m_text = "Status";
    itemCol.SetWidth(70);
    mpConferenceList->InsertColumn(1, itemCol);
   
    SetAutoLayout(TRUE);
    Layout();
}


// Destructor
ConferencePanel::~ConferencePanel()
{
}


void ConferencePanel::OnAddParty(wxEvent& event)
{
    wxString sParty;
        
    sParty = mpComboBox->GetValue();
    
    AddPartyToList(sParty);
    sipXmgr::getInstance().addConfParty(sParty.c_str());
    
    return;
}

void ConferencePanel::AddPartyToList(wxString sParty)
{
    wxListItem newItem;
    newItem.SetText(sParty);
    
    newItem.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_STATE);
    int index = mpConferenceList->InsertItem(newItem);
    
    return;
}

void ConferencePanel::OnRemoveParty(wxEvent& event)
{
    wxString sParty;
    
    long item = mpConferenceList->GetNextItem(-1, wxLIST_NEXT_ALL,
                                        wxLIST_STATE_SELECTED);
    if (item != -1 )
    {
        sParty = mpConferenceList->GetItemText(item);
        sipXmgr::getInstance().removeConfParty(sParty.c_str());
        mpConferenceList->DeleteItem(item);
    }
    
    return;
}
