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
#include <os/OsDefs.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __posix_under_pingtel__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#endif

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif // TEST

// APPLICATION INCLUDES
#include "os/OsFileSystem.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsFileSystem::OsFileSystem()
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif // TEST

}

// Copy constructor
OsFileSystem::OsFileSystem(const OsFileSystem& rOsFileSystem)
{
}

// Destructor
OsFileSystem::~OsFileSystem()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsFileSystem&
OsFileSystem::operator=(const OsFileSystem& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsStatus OsFileSystem::setReadOnly(const OsPath& rPath, UtlBoolean isReadOnly)
{
    OsStatus retval = OS_FAILED;

#ifdef _VXWORKS

#else
    int mode = S_IREAD;

    if (!isReadOnly)
        mode |= S_IWRITE;

    if (chmod(rPath.data(),mode) != -1)
        retval = OS_SUCCESS;
#endif

    return retval;
}


//: Returns OS_SUCCESS if file copied ok.
//  see OsFile for more return codes
OsStatus OsFileSystem::copy(const OsPath& source, const OsPath& dest)
{
    OsFile sourcefile(source);
    return sourcefile.copy(dest);
}

//: Removes the directory or file specified by path
OsStatus OsFileSystem::remove(const OsPath& path, UtlBoolean bRecursive, UtlBoolean bForce)
{
    OsStatus retval = OS_INVALID;
    OsFileInfo info;
    OsPath testpath = path;
    getFileInfo(testpath,info);

    if (info.isDir())
    {
        if (bRecursive)
        {
            retval = removeTree(path,bForce);
        }
        else
        {
            if (rmdir((char *)path.data()) != -1)
                retval = OS_SUCCESS;
        }
    }
    else
    {
       if (bForce)
           setReadOnly(path,FALSE);
       if (::remove(path.data()) != -1)
           retval = OS_SUCCESS;
    }

    return retval;
}



//: Renames the directory or file specified by path
//  (no path may be specifed on 2nd parameter)
OsStatus OsFileSystem::rename(const OsPath& source, const OsPath& dest)
{
    OsDir dir(source);
    return dir.rename(dest);
}


//: Change the current working directory to the specified location
OsStatus OsFileSystem::change(const OsPath& path)
{
    OsStatus stat = OS_INVALID;
    OsPath dir = path;
    if (chdir((char*) path.data()) != -1)
        stat = OS_SUCCESS;

    return stat;
}

//: Creates the specified directory
//  Fails if a file by the same name exist in the parent directory
OsStatus OsFileSystem::createDir(const OsPath& path)
{
    OsDir dir(path);
    return dir.create();
}

//: returns the current working directory for the process
//
OsStatus OsFileSystem::getWorkingDirectory(OsPath& path)
{
    char buf[256];
    OsStatus stat = OS_INVALID;

    if (getcwd(buf,256))
    {
        stat = OS_SUCCESS;
        path=buf;
    }

    return stat;
}

/* ============================ ACCESSORS ================================= */

//: Retrieve system info for specified directory or file
OsStatus OsFileSystem::getFileInfo(OsPath& filespec, OsFileInfo& fileInfo)
{

    OsDir dir(filespec);
    return dir.getFileInfo(fileInfo);

}

/* ============================ INQUIRY =================================== */

//returns true if the specified file exists
UtlBoolean OsFileSystem::exists(const OsPath& filename)
{
    OsDir direntry(filename);

    return direntry.exists();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

//: Removes the directory annd all sub-dirs
OsStatus OsFileSystem::removeTree(const OsPath& path,UtlBoolean bForce)
{
    UtlBoolean bFailed = FALSE;
    OsPath origDir;
    OsFileSystem::getWorkingDirectory(origDir);
    OsStatus retval = OS_INVALID;
    OsFileInfo info;
    OsPath testpath = path;
    getFileInfo(testpath,info);
    OsFileIterator::OsFileType fileType = OsFileIterator::ANY_FILE;
    //only do this if it is a directory
    if (info.isDir())
    {

      if (OsFileSystem::change(path) == OS_SUCCESS)
      {
          OsFileIterator *files = new OsFileIterator();
          OsPath entry;

          OsStatus filestat = files->findFirst(entry,".*", fileType);
          while (!bFailed && filestat == OS_SUCCESS)
          {
              if (entry != "." && entry != "..")
              {
                 getFileInfo(entry,info);
                 if (info.isDir())
                 {
                     if (removeTree(entry,bForce) != OS_SUCCESS)
                     {
                        bFailed = TRUE;
                     }
                 }
                 else
                 {
                    OsFile tmpfile(entry);
                    if (tmpfile.remove(bForce) != OS_SUCCESS)
                    {
                        osPrintf("ERROR: can't removing file %s\n",entry.data());
                        retval = OS_FAILED;
                        bFailed = TRUE;
                    }

                 }
              }

              filestat = files->findNext(entry);

          }

          delete files;

          if (OsFileSystem::change(origDir) == OS_SUCCESS)
          {
            if (!bFailed && OsFileSystem::remove(path,FALSE,FALSE) != OS_SUCCESS)
            {
                osPrintf("ERROR: can't remove dir %s\n",path.data());
                retval = OS_FAILED;
            }
            else
            {
                retval = OS_SUCCESS;
            }
          }
          else
          {
              retval = OS_FAILED;
          }
      }
    }
    return retval;
}

/* ============================ FUNCTIONS ================================= */



