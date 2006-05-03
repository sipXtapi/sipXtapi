//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _sipxezphoneapp_h
#define _sipxezphoneapp_h

// SYSTEM INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "sipXezPhoneFrame.h"

// APPLICATION INCLUDES
// DEFINES

// define the event table macros
#define EVT_STATUS_MESSAGE_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY ( \
        ezEVT_STATUS_MESSAGE_COMMAND, id, -1, \
        (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)&fn, \
        (wxObject*) NULL \
    ),

#define EVT_LOG_MESSAGE_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY ( \
        ezEVT_LOG_MESSAGE_COMMAND, id, -1, \
        (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)&fn, \
        (wxObject*) NULL \
    ),

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * This is the main wxApp derived class for the phone.
 */
class sipXezPhoneApp  : public wxApp
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
    /**
    * Contructor.
    */
    sipXezPhoneApp();

    /**
    * Destructor.
    */
    virtual ~sipXezPhoneApp();

    /* ============================ MANIPULATORS ============================== */
    /**
    * Initialization routine called by wxWidgets.  Creates the frame window.
    */
    virtual bool OnInit();

    /** 
    * Displays a message in the UI.  Good for testing / debugging.
    */
    void addLogMessage(UtlString message);

    /**
    * Displays a status message in the UI.  User feedback to indicate the state of the phone
    */
    void setStatusMessage(const wxString& message);

    /**
    * Retrieves the string value entered in the combo box;
    */
    const wxString getEnteredText();   

    /**
    * Accessor for the Frame derived class object
    */
    sipXezPhoneFrame& getFrame() const;
    

    void OnProcessStatusMessage(wxCommandEvent& event);
    void OnProcessLogMessage(wxCommandEvent& event);

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    sipXezPhoneFrame* mpFrame;
    wxString mStatusMessage;
    wxString mLogMessage;
    DECLARE_EVENT_TABLE()
};

#endif
