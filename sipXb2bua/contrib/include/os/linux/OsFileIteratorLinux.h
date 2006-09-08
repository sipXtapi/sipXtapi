//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsFileIterator_h_
#define _OsFileIterator_h_

// SYSTEM INCLUDES
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsFS.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsFileInfoLinux;
class OsPathLinux;
class OsFileIteratorBase;

//:Abstraction class to iterate through files and/or directories
class OsFileIteratorLinux : public OsFileIteratorBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   OsFileIteratorLinux();

   OsFileIteratorLinux(const OsPathLinux& rPathName);

    virtual ~OsFileIteratorLinux();
     //:Destructor


/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */
    //: Returns total files enumerated thus far.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    DIR* mSearchHandle;

    OsStatus getNextEntryName(UtlString &rName, OsFileType &rFileType);
      //: Platform dependant call for getting entry
    OsStatus getFirstEntryName(UtlString &rName, OsFileType &rFileType);
      //: Platform dependant call for getting entry

    OsFileType mMatchAttrib;
      //: Attributes for file matching

    long mFileCount;
      //: How many file did this class find

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsFileIterator_h_


