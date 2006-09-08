//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsFileWnt_h_
#define _OsFileWnt_h_

// SYSTEM INCLUDES
#include "os/OsStatus.h"

// APPLICATION INCLUDES


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsFileInfoBase;
class OsFileInfoWnt;
class OsPathWnt;

//:OS class for creating,reading, writing, manipulating files.
class OsFileWnt : public OsFileBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   OsFileWnt(const OsPathBase& filename);
     //:Default constructor

   virtual
   ~OsFileWnt();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus fileunlock();
     //: Unlocks the file for across process access


   OsStatus filelock(const int mode);
     //: Locks the specified OPEN file using the specified  mode (one of the FSLOCK_ modes)
     //: Returns:
     //:        OS_SUCCESS if successful
     //:        OS_FAILED if unsuccessful
     //: Notes: Use FSLOCK_READ only on read or read/write files
     //:        Use FSLOCK_WRITE only only files you can write to.
   

   OsStatus setLength(unsigned long newLength);
     //: Sets the length of the file specified by the object to the new size
     //: Sets the length of the file specified by the object to the new size
     //: Shrinking or Growing the file as needed.


 

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

    UtlBoolean isReadonly() const;
    //: Returns TRUE if file is readonly



    virtual OsStatus getFileInfo(OsFileInfoBase& rFileinfo) const;
    //: Returns all the relevant info on this file

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsFileWnt(const OsFileWnt& rOsFileWnt);
     //:Copy constructor

   OsFileWnt& operator=(const OsFileWnt& rhs);
     //:Assignment operator

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsFileWnt_h_


