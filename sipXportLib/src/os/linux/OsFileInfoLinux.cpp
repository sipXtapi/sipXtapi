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
#include "os/linux/OsFileInfoLinux.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsFileInfoLinux::OsFileInfoLinux()
{
}

// Copy constructor
OsFileInfoLinux::OsFileInfoLinux(const OsFileInfoLinux& rOsFileInfo)
{
}

// Destructor
OsFileInfoLinux::~OsFileInfoLinux()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsFileInfoLinux&
OsFileInfoLinux::operator=(const OsFileInfoLinux& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean  OsFileInfoLinux::isReadOnly() const
{
    UtlBoolean retval = TRUE;

    return retval;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */



