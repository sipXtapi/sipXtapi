//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _mainpanel_h_
#define _mainpanel_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "VolumeControlPanel.h"
#include "DialPadPanel.h"
#include "DialEntryPanel.h"
#include "ButtonPanel.h"
#include "CallHistoryPanel.h"
#include "VideoPanel.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// MACRO CALLS



/**
 * The Panel that is the only child of the wxFrame object for this app.
 */
class MainPanel : public wxPanel
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * MainPanel contructor.
    */
   MainPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size, bool bLogo);

   /**
    * MainPanel destructor.
    */
   virtual ~MainPanel();

/* ============================ MANIPULATORS ============================== */
   /**
    * EventHandler for InitDialog
    */
    void OnInitDialog(wxInitDialogEvent& event);

   /**
    * Conferencing Button event handler
    */
   void OnConferencingButton(wxCommandEvent& event);

   /**
    * Call History Button event handler
    */
   void OnCallHistoryButton(wxCommandEvent& event);
   
   /**
    * Video button event handler
    */
   void OnVideoButton(wxCommandEvent& event);

   void UpdateBackground();

   /** 
    * Timer handler 
    */
   void OnTimer(wxTimerEvent& event);


/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   DECLARE_EVENT_TABLE()
        VolumeControlPanel* mpVolumeControlPanel;
        DialPadPanel* mpDialPadPanel;
        DialEntryPanel* mpDialEntryPanel;
        ButtonPanel* mpButtonPanel;
        CallHistoryPanel* mpCallHistoryPanel;
        VideoPanel*       mpVideoPanel;
        wxTimer m_timer ;

        /**
         * Text control for displaying the phone's state.
         */
   wxTextCtrl* mpCallStatus;

   /**
    * Text control for displaying caller-id info.  Can also be
    * used for displaying debug messages.
    */
   wxTextCtrl* mpCallerID;

   void CreateCallHistoryButton(bool bLogo);
   void CreateConferencingButton(bool bLogo);
   void CreateVideoButton(bool bLogo);
   wxBitmapButton* mpCallHistoryBtn;
   wxBitmapButton* mpConferencingBtn;
   wxBitmapButton* mpVideoBtn;
   wxStaticBitmap* mpLogo;
};


#endif
