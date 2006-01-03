//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "stdwx.h"
#include "ContactListPanel.h"
#include "states/PhoneStateMachine.h"
#include "sipXmgr.h"
#include "sipXezPhoneApp.h"
#include "sipXezPhoneSettings.h"
#include "AddContactDlg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
BEGIN_EVENT_TABLE(ContactListPanel, wxPanel)
   EVT_BUTTON(IDR_ADD_PARTY_BUTTON, ContactListPanel::OnAddContact)
   EVT_BUTTON(IDR_REMOVE_PARTY_BUTTON, ContactListPanel::OnRemoveContact)
   EVT_LIST_ITEM_ACTIVATED(IDR_CONFERENCE_LIST, ContactListPanel::OnSelectContact)
END_EVENT_TABLE()


// Constructor
ContactListPanel::ContactListPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   wxPanel(parent, IDR_CONFERENCE_PANEL, pos, size, wxNO_3D, "ContactListPanel")
{
    wxColor* pPanelColor = & (sipXezPhoneSettings::getInstance().getBackgroundColor());
    SetBackgroundColour(*pPanelColor);

    mpGridSizer = new wxFlexGridSizer(4,1,5,5);
    SetSizer(mpGridSizer, true);
   
    wxSize parentSize = this->GetSize();

    wxSize controlSize;
    wxGridSizer* pButtonSizer = new wxGridSizer(1, 2, 5, 5);
    controlSize.x = 20;
    controlSize.y = 20;
    pButtonSizer->Add(mpAddButton = new wxButton(this, IDR_ADD_PARTY_BUTTON, "+", wxDefaultPosition, controlSize), 0, wxALIGN_CENTRE);
    pButtonSizer->Add(mpRemoveButton = new wxButton(this, IDR_REMOVE_PARTY_BUTTON, "-", wxDefaultPosition, controlSize), 0, wxALIGN_CENTRE);
    
    wxFont bigFont = mpAddButton->GetFont();
    bigFont.SetPointSize(12);
    
    mpAddButton->SetFont(bigFont);
    mpRemoveButton->SetFont(bigFont);
    mpGridSizer->Add(pButtonSizer, 0, wxALIGN_CENTRE);
    
    controlSize.x = parentSize.GetWidth();    
    controlSize.y = parentSize.GetHeight() - 50 ;    
    mpGridSizer->Add(mpContactList = new wxListCtrl(this, IDR_CONFERENCE_LIST, wxDefaultPosition, controlSize, wxLC_HRULES | wxLC_VRULES | wxLC_REPORT| wxLC_SINGLE_SEL | wxSUNKEN_BORDER ), 0, wxALIGN_CENTRE);
    wxListItem itemCol;
    
    itemCol.m_mask = wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH;
    itemCol.m_text = "Name";
    itemCol.SetWidth((parentSize.GetWidth()/2) - 15);
    mpContactList->InsertColumn(0, itemCol);
    itemCol.m_text = "Address";
    itemCol.SetWidth((parentSize.GetWidth()/2) - 15);
    mpContactList->InsertColumn(1, itemCol);
   
    SetAutoLayout(TRUE);
    Layout();
}


// Destructor
ContactListPanel::~ContactListPanel()
{
}


void ContactListPanel::OnAddContact(wxEvent& event)
{
    AddContactDlg dlg(this, -1, "Add Contact") ;

    if (dlg.ShowModal() == wxID_OK)
    {
        long index = mpContactList->InsertItem(mpContactList->GetItemCount(),
                dlg.getName()) ;
        mpContactList->SetItem(index, 1, dlg.getUrl()) ;
        sipXezPhoneSettings::getInstance().addContact(dlg.getName().c_str(), dlg.getUrl().c_str());
    }
}


void ContactListPanel::OnRemoveContact(wxEvent& event)
{
    long itemIndex = mpContactList->GetNextItem(-1, wxLIST_NEXT_ALL,
                                        wxLIST_STATE_SELECTED);
    if (itemIndex != -1 )
    {
        wxListItem nameItem;
        nameItem.SetMask(wxLIST_MASK_TEXT);
        nameItem.SetId(itemIndex);
        nameItem.SetColumn(0);
        mpContactList->GetItem(nameItem);
        
        wxListItem urlItem;
        urlItem.SetMask(wxLIST_MASK_TEXT);
        urlItem.SetId(itemIndex);
        urlItem.SetColumn(1);
        mpContactList->GetItem(urlItem);

        sipXezPhoneSettings::getInstance().removeContact(nameItem.GetText().c_str(),
                                                         urlItem.GetText().c_str());
        mpContactList->DeleteItem(itemIndex);
    }    
}


void ContactListPanel::OnSelectContact(wxListEvent& event)
{
    wxString name ;
    wxString url ;
    wxListItem item ;

    item.m_itemId = event.m_itemIndex;
    item.m_col = 0;
    item.m_mask = wxLIST_MASK_TEXT;

    if (mpContactList->GetItem(item))
    {
        name = item.GetText() ;                
    }

    item.m_itemId = event.m_itemIndex;
    item.m_col = 1;
    item.m_mask = wxLIST_MASK_TEXT;
    
    if (mpContactList->GetItem(item))
    {
        url = item.GetText() ;                
    }

    wxString dialString ;
    dialString = "\"" + name + "\" <" + url + ">" ;

    PhoneStateMachine::getInstance().OnDial(dialString);        
}