//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _h_sipxezphoneframe
#define _h_sipxezphoneframe

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "sipXezPhoneAboutDlg.h"
#include "ConferencePanel.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class EventLogDlg ;

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
    * EventHandler for the Test Info menu item.
    */
    void OnTestInfo(wxCommandEvent& event);

   /**
    * EventHandler for the Test Teardown menu item.
    */
    void OnTestTeardown(wxCommandEvent& event);
    
   /**
    * EventHandler for the Test LineRemove menu item.
    */
    void OnTestLineRemove(wxCommandEvent& event);
    
    
    /** 
     * EventHandler for line unregistration test menu
     */
    void OnTestUnRegister(wxCommandEvent& WXUNUSED(event));
    
    /**
     * eventhandler for testing dns srv enable / disable
     */
    void OnTestDNS(wxCommandEvent& WXUNUSED(event));

    
#ifdef VOICE_ENGINE
    /** 
     * EventHandler for the Voice Engine test menu
     */
    void OnTestVoiceEngine(wxCommandEvent& WXUNUSED(event));
#endif
    
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

   /**
    * Event handler for the wxMoveEvent
    */
    void OnMove(wxMoveEvent& moveEvent);

   /**
    * EventHandler for the Configuration menu item.
    */
    void OnAudioSettings(wxCommandEvent& event);

   /**
    * EventHandler for the Playfile menu item.
    */
    void OnPlayFile(wxCommandEvent& event);

   /**
    * EventHandler for the Stopfile menu item.
    */
    void OnStopFile(wxCommandEvent& event);

    /**
     * EventHandler for the Show SIP Log menu item.
     */
    void OnShowSipLog(wxCommandEvent& event);

    /**
     * EventHandler for the Show Event Log menu item.
     */
    void OnShowEventLog(wxCommandEvent& event);

    /**
     * EventHandler for the Show Sys Log menu item.
     */
    void OnShowSysLog(wxCommandEvent& event) ;
      
    
/* ============================ ACCESSORS ================================= */

    /**
     * Accessor for the visibility state of the Conferencing dialog
     */
    const bool getConferencingVisible() const;

    /**
     * Accessor for the visibility state of the Call History dialog
     */
    const bool getCallHistoryVisible() const;
    
    /**
     * Mutator for the visibility state of the Conferencing dialog
     */
    void setConferencingVisible(const bool bVisible);
    
    /**
     * Mutator for the visibility state of the CallHistory dialog
     */
    void setCallHistoryVisible(const bool bVisible);
    
    /**
     * Position the Call History and Conferencing panels
     */
    void positionPanels();
    
     
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    sipXezPhoneAboutDlg* m_pAboutDlg;
    wxMenu *mpMenuSettings;
    wxMenu *mpMenuTest;
    bool mBoolMinimalView;
    wxDialog* mpCallHistoryWnd;
    ConferencePanel* mpConferencingWnd;
    bool mCallHistoryVisible;
    bool mConferencingVisible;
    EventLogDlg* mpEventLogDlg;

    DECLARE_EVENT_TABLE()    
};


#endif // _h_sipxezphoneframe

