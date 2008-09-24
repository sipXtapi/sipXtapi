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
class OsFileInfoWnt;
class OsPathWnt;
class OsFileIteratorBase;
class UtlString;

//:Abstraction class to iterate through files and/or directories
class OsFileIteratorWnt : public OsFileIteratorBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   OsFileIteratorWnt();

   OsFileIteratorWnt(const OsPathWnt& rPathName);

    virtual ~OsFileIteratorWnt();
     //:Destructor


/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */
    //: Returns total files enumerated thus far.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    // Override this function for releasing mSearchHandle.
    virtual void Release();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    OsStatus getNextEntryName(UtlString &rName, OsFileType &rFileType);
      //: Platform dependant call for getting entry
    OsStatus getFirstEntryName(UtlString &rName, OsFileType &rFileType);
      //: Platform dependant call for getting entry


};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsFileIterator_h_


