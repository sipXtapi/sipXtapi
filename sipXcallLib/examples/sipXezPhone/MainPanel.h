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
#ifndef _mainpanel_h_
#define _mainpanel_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "VolumeControlPanel.h"
#include "DialPadPanel.h"
#include "DialEntryPanel.h"
#include "ButtonPanel.h"
#include "CallHistoryPanel.h"

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
    void OnConferencingButton(wxCommandEvent& event);

    /**
    * Call History Button event handler
    */
    void OnCallHistoryButton(wxCommandEvent& event);

    /**
    * Timer Event handler - for keeping track of
    * the length of the call.
    */
    void OnTimer(wxTimerEvent& event);

    /**
    * Initializes the timer that keeps track of
    * the length of the call.
    */
    void InitializeTimer();
    
    /**
    * Tears down the timer that keeps track of
    * the length of the call.
    */
    void UnInitializeTimer();
    
    /**
    * Starts the timer that keeps track of
    * the length of the call.
    */
    void StartTimer(const bool bStartTimer);


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
    wxBitmapButton* mpCallHistoryBtn;
    wxBitmapButton* mpConferencingBtn;
    
    int mHour;
    int mMin;
    int mSec;
    bool mStartTimer;
    wxTimer mTimer;

};


#endif
