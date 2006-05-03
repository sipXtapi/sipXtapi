//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _h_sipxezphoneframe
#define _h_sipxezphoneframe

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "sipXezPhoneAboutDlg.h"
#include "ConferencePanel.h"
#include "MainPanel.h"

#ifdef _WIN32
#include <windows.h>
#endif
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
enum AUTO_POSITION {
    POSITION_INIT,
    POSITION_SAVE,
    POSITION_STATE
};
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
    sipXezPhoneFrame(const wxString& title, const wxPoint& pos, const wxSize& size, bool bLogo);

 
/* ============================ MANIPULATORS ============================== */

   /**
    * Auto-position windows
    */
    void autoPosition(enum AUTO_POSITION evt);

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

    void OnAVWizard(wxCommandEvent& WXUNUSED(event));
    
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
     * View or hide the video preview window
     */
    void OnVideoPreview(wxCommandEvent& event);
    

    /** 
     * View or hide the video window
     */
    void OnVideo(wxCommandEvent& event);
    
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
    * EventHandler for the AudioWizardTest menu item
    */
#ifdef VOICE_ENGINE    
    void OnAudioWizardTest(wxCommandEvent& event);
#endif
    
   /**
    * EventHandler for the video configuration menu item.
    */
    void OnVideoSettings(wxCommandEvent& event);

   /**
    * EventHandler for the video configuration menu item.
    */
    void OnSrtpSettings(wxCommandEvent& event);

   /**
    * Event handler for importing a pkcs12 with a private key
    */
    void OnImportCertificate(wxCommandEvent& event);
    
    /**
     * Event handler for selecting the capture device.
     */
    void OnCameraSettings(wxCommandEvent& event);
    
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
     * EventHandler for the renegotiate codecs item
     */
    void OnRenegotiate(wxCommandEvent& event);

    /**
     * EventHandler for the Show Sys Log menu item.
     */
    void OnShowSysLog(wxCommandEvent& event) ;

    /**
     * EventHandler for selecting a contact
     */
    void OnSelectContact(wxCommandEvent& event) ;

    /** 
     * Event updater (For checking) the selected contact
     */
    void OnUpdateContact(wxUpdateUIEvent& event) ;

    /**
     * EventHandler for displaying current contacts
     */
    void OnDisplayContacts(wxCommandEvent& event) ;

    /**
     * EventHandler for changing background color
     */
    void OnColorDialog(wxCommandEvent& event) ;

    /**
     * EventHandler for testing tabbed controls
     */
    void OnTabs(wxCommandEvent& event) ;
     
    
/* ============================ ACCESSORS ================================= */

    /**
     * Accessor for the visibility state of the Conferencing dialog
     */
    const bool getConferencingVisible() const;

    /**
     * Mutator for the visibility state of the Conferencing dialog
     */
    void setConferencingVisible(const bool bVisible);

    /**
     * Accessor for the visibility state of the Call History dialog
     */
    const bool getCallHistoryVisible() const;
    
    /**
     * Mutator for the visibility state of the CallHistory dialog
     */
    void setCallHistoryVisible(const bool bVisible);
    
    /**
     * Accessor for the visibility state of the Video dialog
     */
    const bool getVideoVisible() const;
    
    /**
     * Mutator for the visibility state of the Video dialog
     */
    void setVideoVisible(const bool bVisible);

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
    wxMenu *mpMenuContactSelection;
    MainPanel *mpMainPanel;
    bool mBoolMinimalView;
    wxDialog* mpCallHistory;
    wxDialog* mpVideoPanel;
    ConferencePanel* mpConferencingWnd;
    VideoPanel* mpVideoWindow;
    CallHistoryPanel* mpCallHistoryWnd;
    bool mCallHistoryVisible;
    bool mVideoVisible;
    bool mConferencingVisible;
    EventLogDlg* mpEventLogDlg;
    int mOrigWidth;
    int mOrigHeight;
    bool mbLogo;
#ifdef _WIN32
    //void registerVideoWindowClasses();
    //HWND mhPreviewWnd;
    //HWND mhVideoWnd;
#endif    

    DECLARE_EVENT_TABLE()    
};


#endif // _h_sipxezphoneframe

