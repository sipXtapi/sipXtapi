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