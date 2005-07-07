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
#ifndef _h_sipxezphoneframe
#define _h_sipxezphoneframe

// SYSTEM INCLUDES
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
 * This is the application's frame window.
 */
class sipXezPhoneFrame: public wxFrame
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * Contructor.
    */
    sipXezPhoneFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

 
/* ============================ MANIPULATORS ============================== */
   /**
    * EventHandler for the Configuration menu item.
    */
    void OnConfiguration(wxCommandEvent& event);

   /**
    * EventHandler for the About menu item.
    */
    void OnAbout(wxCommandEvent& event);

   /**
    * EventHandler for the Minimal/Normal View menu item.
    */
    void OnMinimalView(wxCommandEvent& event);
    
   /** 
    * Event handler for the wxCloseEvent
    */
    void OnClose(wxCloseEvent& event);


/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    wxMenu *mpMenuSettings;
    bool mBoolMinimalView;

    DECLARE_EVENT_TABLE()
};


#endif // _h_sipxezphoneframe

