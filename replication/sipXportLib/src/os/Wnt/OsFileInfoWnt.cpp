//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/wnt/OsFileInfoWnt.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsFileInfoWnt::OsFileInfoWnt()
{
}

// Copy constructor
OsFileInfoWnt::OsFileInfoWnt(const OsFileInfoWnt& rOsFileInfo)
{
}

// Destructor
OsFileInfoWnt::~OsFileInfoWnt()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsFileInfoWnt& 
OsFileInfoWnt::operator=(const OsFileInfoWnt& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean  OsFileInfoWnt::isReadOnly() const
{
    UtlBoolean retval = TRUE;

    return retval;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



