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
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

OsDirLinux::OsDirLinux(const char* pathname) :
OsDirBase(pathname)
{
}

OsDirLinux::OsDirLinux(const OsPathLinux& pathname) :
OsDirBase(pathname.data())
{
}

// Copy constructor
OsDirLinux::OsDirLinux(const OsDirLinux& rOsDirLinux) :
OsDirBase(rOsDirLinux.mDirName.data())
{
}

// Destructor
OsDirLinux::~OsDirLinux()
{
}

/* ============================ MANIPULATORS ============================== */
// Assignment operator
OsDirLinux&
OsDirLinux::operator=(const OsDirLinux& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


OsStatus OsDirLinux::create(int permissions) const
{
    OsStatus ret = OS_INVALID;

    OsPathBase path;
    
    if (mDirName.getNativePath(path) == OS_SUCCESS)
    {
        int err = mkdir((const char *)path.data(),permissions);
        if (err != -1)
        {
            ret = OS_SUCCESS;
        }
    }

    return ret;
}


OsStatus OsDirLinux::rename(const char* name)
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

OsStatus OsDirLinux::getFileInfo(OsFileInfoBase& fileinfo) const
{
    OsStatus ret = OS_INVALID;
    struct stat stats;
    if (stat((char *)mDirName.data(),&stats) == 0)
    {
        ret = OS_SUCCESS;
        if (stats.st_mode & S_IFDIR)
            fileinfo.mbIsDirectory = TRUE;
        else
            fileinfo.mbIsDirectory = FALSE;

        if (stats.st_mode & S_IWUSR)
            fileinfo.mbIsReadOnly = FALSE;
        else
            fileinfo.mbIsReadOnly = TRUE;

        OsTime createTime(stats.st_ctime,0);
        fileinfo.mCreateTime = createTime;

        OsTime modifiedTime(stats.st_mtime,0);
        fileinfo.mModifiedTime = modifiedTime;

        fileinfo.mSize = stats.st_size;

    }
    return ret;
}

/* ============================ INQUIRY =================================== */

UtlBoolean OsDirLinux::exists()
{
    UtlBoolean stat = FALSE;

    OsFileInfoLinux info;
    OsStatus retval = getFileInfo(info);
    if (retval == OS_SUCCESS)
        stat = TRUE;

    return stat;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */



