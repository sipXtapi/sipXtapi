//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _DialerThread_h_
#define _DialerThread_h_

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
 * Displays a button bar.
 */
class DialerThread : public wxThread
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * DialerThread contructor.
    */
        DialerThread::DialerThread(wxString phoneNumber);
   /**
    * DialerThread destructor.
    */

/* ============================ MANIPULATORS ============================== */
        void* Entry();

        void OnExit();
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

        wxString mPhoneNumber;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};


#endif
