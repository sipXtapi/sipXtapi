//
// Copyright (C) 2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
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
// APPLICATION INCLUDES
#include "os/Wnt/OsFileSystemWnt.h"
#include "os/Wnt/OsFileWnt.h"
#include "os/Wnt/OsPathWnt.h"
#include "os/Wnt/OsFileInfoWnt.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const unsigned long CopyBufLen = 32768;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsFileWnt::OsFileWnt(const OsPathBase& filename) :
OsFileBase(filename)
{
}

// Copy constructor
OsFileWnt::OsFileWnt(const OsFileWnt& rOsFileWnt) :
OsFileBase(rOsFileWnt)
{
    OsPathWnt path;
    rOsFileWnt.getFileName(path);
    mFilename = path;
    mOsFileHandle = rOsFileWnt.mOsFileHandle;
}

// Destructor
OsFileWnt::~OsFileWnt()
{
    if (mOsFileHandle)
        close(); //call our close
}

/* ============================ MANIPULATORS ============================== */




OsStatus OsFileWnt::setLength(unsigned long newLength)
{
    OsStatus stat = OS_SUCCESS;
    
    return stat;
}



OsStatus OsFileWnt::fileunlock()
{
    OsStatus retval = OS_SUCCESS;

    return retval;
}


OsStatus OsFileWnt::filelock(const bool wait)
{
    OsStatus retval = OS_FAILED;

    return retval;
}

OsStatus OsFileWnt::getFileInfo(OsFileInfoBase& fileinfo) const
{
    OsStatus ret = OS_INVALID;

#ifdef WINCE
	printf( "JEP - TODO in OsFileWnt::getFileInfo( )\n" );
    //  JEP - TODO - implement this...
#else
    struct stat stats;
    if (stat(mFilename,&stats) == 0)
    {
        ret = OS_SUCCESS;
        if (stats.st_mode & _S_IWRITE)
            fileinfo.mbIsReadOnly = FALSE;
        else
            fileinfo.mbIsReadOnly = TRUE;

        OsTime createTime((const long)(stats.st_ctime),0);
        fileinfo.mCreateTime = createTime;

        OsTime modifiedTime((const long)(stats.st_mtime),0);
        fileinfo.mModifiedTime = modifiedTime;
        
        fileinfo.mSize = stats.st_size;
    }
#endif

    return ret;
}

OsStatus OsFileWnt::touch()
{
    OsStatus stat = OS_INVALID;

    if (exists() == OS_SUCCESS)
    {
        FILETIME ft;
        SYSTEMTIME st;
        BOOL fileTimeOk;

        // Grab the windows file handle for use in
        // windows file system calls.
        HANDLE fileHnd;
        fileHnd = CreateFile(mFilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
                             NULL, OPEN_EXISTING, 0, NULL);

        GetSystemTime(&st);               // gets current time
        SystemTimeToFileTime(&st, &ft);   // converts to file time format
        fileTimeOk = SetFileTime(fileHnd, // sets last-write time for file
            (LPFILETIME) NULL, (LPFILETIME) NULL, &ft);
        if (fileTimeOk)
        {
            stat = OS_SUCCESS;
        }
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

UtlBoolean OsFileWnt::isReadonly() const
{
    UtlBoolean retval = FALSE;
    
    OsFileInfoWnt info;
    getFileInfo(info);
    
    return info.mbIsReadOnly;
}





/* //////////////////////////// PROTECTED ///////////////////////////////// */
// Assignment operator
OsFileWnt& 
OsFileWnt::operator=(const OsFileWnt& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



