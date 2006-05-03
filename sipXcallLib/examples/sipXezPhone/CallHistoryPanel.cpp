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
#include "CallHistoryPanel.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
BEGIN_EVENT_TABLE(CallHistoryPanel, wxPanel)
   EVT_NOTEBOOK_PAGE_CHANGED(IDR_CALL_HISTORY_NOTEBOOK, CallHistoryPanel::OnPageChanged)
END_EVENT_TABLE()

// Constructor
CallHistoryPanel::CallHistoryPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
   wxPanel(parent, IDR_CALL_HISTORY_PANEL, pos, size, wxTAB_TRAVERSAL, "CallHistoryPanel")
{
        mpNotebook = new wxNotebook(this, IDR_CALL_HISTORY_NOTEBOOK, pos, size);
        mpContactListPanel = new ContactListPanel(mpNotebook, pos, size);
//        mpReceivedCallListCtrl = new ReceivedCallList(mpNotebook, pos, size);
//        mpPlacedCallListCtrl = new PlacedCallList(mpNotebook, pos, size);        
        mpNotebook->AddPage(mpContactListPanel, "Contacts");
//        mpNotebook->AddPage(mpReceivedCallListCtrl, "Received Calls");
//        mpNotebook->AddPage(mpPlacedCallListCtrl, "Placed Calls");
        

}

void CallHistoryPanel::OnPageChanged( wxNotebookEvent &event )
{

}

// Destructor
CallHistoryPanel::~CallHistoryPanel()
{
}

void CallHistoryPanel::UpdateBackground(wxColor color)
{
    mpContactListPanel->SetBackgroundColour(color);
}
