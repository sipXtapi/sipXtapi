//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
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
   MainPanel(wxWindow* parent, const wxPoint& pos, const wxSize& size);

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
   void OnConferencingButton(wxEvent& event);

   /**
    * Call History Button event handler
    */
   void OnCallHistoryButton(wxEvent& event);
   
   /**
    * Video button event handler
    */
   void OnVideoButton(wxEvent& event);

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

        /**
         * Text control for displaying the phone's state.
         */
   wxTextCtrl* mpCallStatus;

   /**
    * Text control for displaying caller-id info.  Can also be
    * used for displaying debug messages.
    */
   wxTextCtrl* mpCallerID;

   void CreateCallHistoryButton();
   void CreateConferencingButton();
   void CreateVideoButton();
   wxBitmapButton* mpCallHistoryBtn;
   wxBitmapButton* mpConferencingBtn;
   wxBitmapButton* mpVideoBtn;

};


#endif
