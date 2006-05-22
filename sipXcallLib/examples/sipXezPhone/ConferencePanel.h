//
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004, 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _ConferencePanel_h_
#define _ConferencePanel_h_

// SYSTEM INCLUDES
#include <wx/listctrl.h>
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Displays a button bar.
 */
class ConferencePanel : public wxDialog
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * ConferencePanel contructor.
    */
   ConferencePanel(wxWindow* parent, const wxPoint& pos, const wxSize& size);


   /**
    * Add Party Button event handler
    */
   void OnAddParty(wxCommandEvent& event);

   /**
    * Add Remove Button event handler
    */
   void OnRemoveParty(wxCommandEvent& event);
   
   void AddPartyToList(wxString sParty);
   
   /**
    * ConferencePanel destructor.
    */
   virtual ~ConferencePanel();

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   DECLARE_EVENT_TABLE()
   wxFlexGridSizer* mpGridSizer;
   wxComboBox*  mpComboBox;
   wxButton* mpAddButton;
   wxButton* mpRemoveButton;
   wxListCtrl* mpConferenceList;

};


#endif
