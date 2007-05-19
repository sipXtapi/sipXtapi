//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsCallback.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Event notification method where a callback function is executed in the
// Notifier's context when the corresponding event occurs.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsCallback::OsCallback(const int userData, const OsCallbackFunc func)
: mFunc(func),
  mUserData(userData)
{
   // all work is done by the initializers, no other work required
}

// Destructor
OsCallback::~OsCallback()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Signal the occurrence of the event by executing the callback function.
// Always return OS_SUCCESS.
OsStatus OsCallback::signal(const int eventData)
{
   mFunc(mUserData, eventData);   // execute the callback function

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


