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
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsPathWnt::OsPathWnt()
{
}

// Make one from a char string
OsPathWnt::OsPathWnt(const char *pathname) :
OsPathBase(pathname)
{
}
// Make one from a UtlStringchar string
OsPathWnt::OsPathWnt(const UtlString &pathname) :
OsPathBase(pathname)
{
}

// Copy constructor
OsPathWnt::OsPathWnt(const OsPathWnt& rOsPath)
{
    *this = rOsPath.data();
}

// Destructor
OsPathWnt::~OsPathWnt()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsPathWnt& 
OsPathWnt::operator=(const OsPathWnt& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
   
   OsPathBase::operator=(rhs.data());
   
   return *this;
}

/* ============================ ACCESSORS ================================= */



/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



