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

OsTime OsFileWnt::fileTimeToOsTime(FILETIME ft)
{
    __int64 ll = (((__int64)ft.dwHighDateTime << 32) |
                                 ft.dwLowDateTime) - 116444736000000000;
    // See http://support.microsoft.com/?scid=kb%3Ben-us%3B167296&x=14&y=17

    return OsTime((long)(ll / 10000000), (long)((ll / 10) % 1000000));
}

OsStatus OsFileWnt::getFileInfo(OsFileInfoBase& fileinfo) const
{
    OsStatus ret = OS_INVALID;

    WIN32_FILE_ATTRIBUTE_DATA w32data;
    BOOL bRes = GetFileAttributesEx(mFilename.data(), GetFileExInfoStandard, &w32data);
    if (bRes)
    {
        ret = OS_SUCCESS;

        fileinfo.mbIsReadOnly = 
            (w32data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? TRUE : FALSE;

        fileinfo.mSize = ((ULONGLONG)w32data.nFileSizeHigh << 32) | w32data.nFileSizeLow;
        fileinfo.mCreateTime = fileTimeToOsTime(w32data.ftCreationTime);
        fileinfo.mModifiedTime = fileTimeToOsTime(w32data.ftLastWriteTime);
        
    }
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



