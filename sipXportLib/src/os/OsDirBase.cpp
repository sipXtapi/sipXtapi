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
#include <stdlib.h>
#include <stdio.h>

#if defined(__linux__) || defined(sun) || defined(_VXWORKS)
 #include <unistd.h>
 #include <dirent.h>
#endif

#ifdef WIN32
 #ifndef WINCE
  #include <direct.h>
 #endif
#endif

#ifdef WINCE
#   include <types.h>
#else
#   include <sys/types.h>
#   include <sys/stat.h>
#endif

// APPLICATION INCLUDES
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// DEFINES
#if defined(_WIN32)
   #define S_DIR _S_IFDIR
   #define S_READONLY S_IWRITE
#elif defined(__pingtel_on_posix__) || defined(_VXWORKS)
   #define S_DIR S_IFDIR
   #define S_READONLY S_IWUSR
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

OsDirBase::OsDirBase(const char* pathname)
{
    mDirName = pathname;
}

OsDirBase::OsDirBase(const OsPathBase& pathname)
{
    mDirName = pathname;
}

// Copy constructor
OsDirBase::OsDirBase(const OsDirBase& rOsDirBase)
{
    mDirName = rOsDirBase.mDirName;
}

// Destructor
OsDirBase::~OsDirBase()
{
}

/* ============================ MANIPULATORS ============================== */
// Assignment operator
OsDirBase&
OsDirBase::operator=(const OsDirBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

   OsStatus OsDirBase::create() const
   {
       return OS_INVALID;
   }

   OsStatus OsDirBase::remove(UtlBoolean bRecursive, UtlBoolean bForce) const
   {
       return OsFileSystem::remove(mDirName,bRecursive, bForce);
   }

   OsStatus OsDirBase::rename(const char* name)
   {
       return OS_INVALID;
   }

/* ============================ ACCESSORS ================================= */



   void OsDirBase::getPath(OsPathBase& rOsPath) const
   {
       rOsPath = mDirName;
   }

/* ============================ INQUIRY =================================== */

  UtlBoolean OsDirBase::exists()
  {
      return FALSE;
  }





/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
