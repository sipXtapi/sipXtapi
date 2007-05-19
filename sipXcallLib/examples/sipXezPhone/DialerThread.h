//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _DialerThread_h_
#define _DialerThread_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "stdwx.h"
#include "sipXmgr.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
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

/**
 */
class TransferThread: public wxThread
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   /**
    * DialerThread contructor.
    */
        TransferThread::TransferThread(SIPX_CALL hCall, 
                                       wxString phoneNumber);
   /**
    * DialerThread destructor.
    */

/* ============================ MANIPULATORS ============================== */
        void* Entry();

        void OnExit();
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

        wxString mPhoneNumber;
        SIPX_CALL mhCall ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};


#endif
