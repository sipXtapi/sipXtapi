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
#ifndef _DialEntrypanel_h_
#define _DialEntrypanel_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "states/PhoneStateMachineObserver.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * A panel containing a text control and a button for inputing a number and dialing.
 */
class DialEntryPanel : public wxPanel
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend class DialEntryPhoneStateMachineObserver;

/* ============================ CREATORS ================================== */
   /**
    * DialEntryPanel contructor.
    */
   DialEntryPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size);

   /**
    * DialEntryPanel destructor.
    */
   virtual ~DialEntryPanel();

   /**
    * Dial Button event handler
    */
   void OnButtonClick(wxEvent& event);
   
   /**
    * retrieve the text from the combo box
    */
   const wxString getEnteredText();

   /**
    *
    */
   wxComboBox& getComboBox() {return *mpComboBox;};

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   DECLARE_EVENT_TABLE()

	wxStaticBoxSizer* mpSizer;
	wxBitmapButton* mpDialButton;
	wxComboBox* mpComboBox;
	wxStaticBox* mpOutline;
	
	/**
	 * Specialized state machine observer class.  Allows us to listen for Dialing and Ringing state changes.
	 * We can then update the combo box with dialed and received numbers.
	 */
	class DialEntryPhoneStateMachineObserver : public PhoneStateMachineObserver
	{
	    public:
	        /**
	         * Constructor - takes a pointer to the owner object
	         */
	        DialEntryPhoneStateMachineObserver (DialEntryPanel* pOwner) : mpOwner(pOwner) { } ;
	        
	        /**
	         * Implementation of the IStateTransitions::OnDial method.  Called by the state machine
	         * through its collection of observers.
	         */
	        PhoneState* OnDial(const wxString phoneNumber);

            /** Implementation of the IStateTransitions::OnRinging method.  Called by the state machine
             * through its collection of observers.
             */	        
	        PhoneState* OnRinging(SIPX_CALL hCall);
	    private:
	        /**
	         * UI Panel object which contains this object.
	         */
    	    DialEntryPanel* mpOwner;
	};
	
	/** 
	 * Its the instance of the DialEntryPhoneStateMachineObserver.
	 */
	DialEntryPhoneStateMachineObserver* mpListener;

};


#endif