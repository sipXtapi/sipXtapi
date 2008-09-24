//
// Copyright (C) 2005, 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004, 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const unsigned long CopyBufLen = 32768;


//needed this so vxworks macros will find OK for the stdio funcs
#ifdef _VXWORKS
#ifndef OK#include <fcntl.h>

#define OK              0
#endif /* OK */
#endif /* _VXWORKS */

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsFileLinux::OsFileLinux(const OsPathBase& filename) :
OsFileBase(filename)
{
}

// Copy constructor
OsFileLinux::OsFileLinux(const OsFileLinux& rOsFileLinux) :
OsFileBase(rOsFileLinux)
{
    OsPathLinux path;
    rOsFileLinux.getFileName(path);
    mFilename = path;
    mOsFileHandle = rOsFileLinux.mOsFileHandle;
}

// Destructor
OsFileLinux::~OsFileLinux()
{
    if (mOsFileHandle)
        close(); //call our close
}

/* ============================ MANIPULATORS ============================== */

OsStatus OsFileLinux::filelock(const bool wait)
{
    OsStatus retval = OS_FAILED;
    struct flock f;

    if (mOsFileHandle)
    {
        f.l_type = F_WRLCK;
        f.l_start = 0;
        f.l_whence = SEEK_SET;
        f.l_len = 0;
        f.l_pid = getpid();

        int fd = fileno(mOsFileHandle);
        if (fcntl(fd, wait ? F_SETLKW : F_SETLK, &f) != -1);
            retval = OS_SUCCESS;
    }

    return retval;
}

OsStatus OsFileLinux::fileunlock()
{
    OsStatus retval = OS_FAILED;
    struct flock f;

    if (mOsFileHandle)
    {
        f.l_type = F_UNLCK;
        f.l_start = 0;
        f.l_whence = SEEK_SET;
        f.l_len = 0;
        f.l_pid = getpid();

        int fd = fileno(mOsFileHandle);
        if (fcntl(fd, F_SETLK, &f) != -1)
            retval = OS_SUCCESS;
    }
    else
        retval = OS_SUCCESS;

    return retval;
}


OsStatus OsFileLinux::setLength(unsigned long newLength)
{
    OsStatus stat = OS_SUCCESS;

    return stat;
}




OsStatus  OsFileLinux::setReadOnly(UtlBoolean bState)
{
    OsStatus stat = OS_INVALID;

    int mode = S_IREAD;

    if (!bState)
        mode |= S_IWRITE;

    if (chmod(mFilename,mode) != -1)
        stat = OS_SUCCESS;

    return stat;
}

OsStatus OsFileLinux::touch()
{
    OsStatus stat = OS_INVALID;

    if (exists() == OS_SUCCESS)
    {
        if (utime(mFilename,NULL) == 0)
            stat = OS_SUCCESS;
    }
    else
    {
        stat = open(CREATE);
        close();
    }

    return stat;
}


/* ============================ ACCESSORS ================================= */




/* ============================ INQUIRY =================================== */

UtlBoolean OsFileLinux::isReadonly() const
{
    OsFileInfoLinux info;
    getFileInfo(info);

    return info.mbIsReadOnly;
}




OsStatus OsFileLinux::getFileInfo(OsFileInfoBase& fileinfo) const
{
    OsStatus ret = OS_INVALID;

    struct stat stats;
    if (stat(mFilename,&stats) == 0)
    {
        ret = OS_SUCCESS;

        fileinfo.mbIsReadOnly = (stats.st_mode & S_IWUSR) == 0;

        OsTime createTime(stats.st_ctime,0);
        fileinfo.mCreateTime = createTime;

        OsTime modifiedTime(stats.st_mtime, 0);
        fileinfo.mModifiedTime = modifiedTime;

        fileinfo.mSize = stats.st_size;

    }



    return ret;

}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
// Assignment operator
OsFileLinux&
OsFileLinux::operator=(const OsFileLinux& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
