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
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/linux/OsFileIteratorLinux.h"
#include "os/OsFileIteratorBase.h"
#include "os/linux/OsPathLinux.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor


OsFileIteratorLinux::OsFileIteratorLinux(const OsPathLinux& pathname) :
OsFileIteratorBase(pathname),
mSearchHandle(NULL)
{
}

// Destructor
OsFileIteratorLinux::~OsFileIteratorLinux()
{
    if (mSearchHandle) {
        closedir(mSearchHandle);
        mSearchHandle = NULL;
    }
}

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
OsFileIteratorLinux::OsFileIteratorLinux() :
mSearchHandle(NULL)
{
}



OsStatus OsFileIteratorLinux::getFirstEntryName(UtlString &name, OsFileType &type)
{
    OsStatus retstat = OS_FILE_NOT_FOUND;

    name = "";

    mFullSearchSpec.strip(UtlString::trailing, '/');
    if (mFullSearchSpec == "")
    {
        mFullSearchSpec = ".";
    }

    if(mSearchHandle)
        closedir(mSearchHandle);
    mSearchHandle = opendir((char*) mFullSearchSpec.data());

    if (mSearchHandle)
    {
        struct dirent* uriDirEntry = NULL;

        if ((uriDirEntry = readdir(mSearchHandle)) != NULL)
        {
            name = uriDirEntry->d_name;
            retstat = OS_SUCCESS;

            UtlString fullName = mUserSpecifiedPath;
            fullName += name;

            struct stat dirEntryStat;
            stat((char*)fullName.data(), &dirEntryStat);

            if(dirEntryStat.st_mode & S_IFDIR)
                type = DIRECTORIES;
            else
                type = FILES;
        }
    }
    else
    {

        retstat = OS_FILE_NOT_FOUND;
    }

    return retstat;
}



OsStatus OsFileIteratorLinux::getNextEntryName(UtlString &name, OsFileType &type)
{
    OsStatus retstat = OS_FILE_NOT_FOUND;

    name = "";

    if (mSearchHandle)
    {
        struct dirent* uriDirEntry = NULL;
        if ((uriDirEntry = readdir(mSearchHandle)) != NULL)
        {
            retstat = OS_SUCCESS;
            name = uriDirEntry->d_name;
            UtlString fullName = mUserSpecifiedPath;
            fullName += name;

            struct stat dirEntryStat;
            stat((char*)fullName.data(), &dirEntryStat);

            if(dirEntryStat.st_mode & S_IFDIR)
            {
                type = DIRECTORIES;
            }
            else
            {
                type = FILES;
            }
        }
    }

    return retstat;
}

/* ============================ FUNCTIONS ================================= */



