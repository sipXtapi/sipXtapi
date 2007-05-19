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
#include <stdio.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

OsDirWnt::OsDirWnt(const char* pathname) :
OsDirBase(pathname)
{
}

OsDirWnt::OsDirWnt(const OsPathWnt& pathname) :
OsDirBase(pathname.data())
{
}

// Copy constructor
OsDirWnt::OsDirWnt(const OsDirWnt& rOsDirWnt) :
OsDirBase(rOsDirWnt.mDirName.data())
{
}

// Destructor
OsDirWnt::~OsDirWnt()
{
}

/* ============================ MANIPULATORS ============================== */
// Assignment operator
OsDirWnt& 
OsDirWnt::operator=(const OsDirWnt& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


OsStatus OsDirWnt::create() const
{
    OsStatus ret = OS_INVALID;
    OsPathBase path;
    
    if (mDirName.getNativePath(path) == OS_SUCCESS)
    {
        int err = _mkdir((const char *)path.data());
        if (err != -1)
        {
            ret = OS_SUCCESS;
        }
    }

    return ret;
}


OsStatus OsDirWnt::rename(const char* name)
{
    OsStatus ret = OS_INVALID;
    OsPathBase path;
    if (mDirName.getNativePath(path) == OS_SUCCESS)
    {
        int err = ::rename(path.data(),name);
        if (err != -1)
        {
            ret = OS_SUCCESS;
    
            //make this object point to new path
            mDirName = name;
    
        }
    }

    return ret;
}


/* ============================ ACCESSORS ================================= */




/* ============================ INQUIRY =================================== */

UtlBoolean OsDirWnt::exists()
{
    UtlBoolean stat = FALSE;
    
    OsFileInfoWnt info;
    OsStatus retval = getFileInfo(info);
    if (retval == OS_SUCCESS)
        stat = TRUE;

    return stat;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



