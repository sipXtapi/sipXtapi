//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _CallHistorypanel_h_
#define _CallHistorypanel_h_

// SYSTEM INCLUDES
#include <wx/notebook.h>
// APPLICATION INCLUDES
#include "ReceivedCallList.h"
#include "PlacedCallList.h"
#include "ContactListPanel.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * This panel has a "notebook control" which contains placed and received calls.
 */
class CallHistoryPanel : public wxPanel
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * CallHistoryPanel contructor.
    */
   CallHistoryPanel(wxWindow* parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

   /**
    * CallHistoryPanel destructor.
    */
   virtual ~CallHistoryPanel();
   void OnPageChanged( wxNotebookEvent &event );

   void UpdateBackground(wxColor color);

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   DECLARE_EVENT_TABLE()
   wxNotebook* mpNotebook;
   ReceivedCallList* mpReceivedCallListCtrl;
   PlacedCallList*   mpPlacedCallListCtrl;
   ContactListPanel* mpContactListPanel;

};


#endif
