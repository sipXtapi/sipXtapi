//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _sipxezphoneapp_h
#define _sipxezphoneapp_h

// SYSTEM INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "sipXezPhoneFrame.h"

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
    void addLogMessage(const UtlString message);

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
    


/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    sipXezPhoneFrame* mpFrame;
};

#endif
