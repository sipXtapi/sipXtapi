//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _ContactListPanel_h_
#define _ContactListPanel_h_

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
class ContactListPanel : public wxPanel
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * ContactListPanel contructor.
    */
   ContactListPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size);


   /**
    * Add Party Button event handler
    */
   void OnAddContact(wxEvent& event);

   /**
    * Add Remove Button event handler
    */
   void OnRemoveContact(wxEvent& event);

   /**
    * Add Select event handler
    */
   void OnSelectContact(wxListEvent& event);
      
   /**
    * ContactListPanel destructor.
    */
   virtual ~ContactListPanel();

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   DECLARE_EVENT_TABLE()
   wxFlexGridSizer* mpGridSizer;
   wxButton* mpAddButton;
   wxButton* mpRemoveButton;
   wxListCtrl* mpContactList;
};


#endif
