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
     //: Cross-process unlocks this file.
     //: Notes: This method should only be called by OsFileBase::close()!


   OsStatus filelock(const bool wait);
     //: Cross-process locks this file, optionally waiting for the lock.
     //: Returns:
     //:        OS_SUCCESS if successful
     //:        OS_FAILED if unsuccessful
     //: Notes: This method should only be called by OsFileBase::open()!
   

   OsStatus setLength(unsigned long newLength);
     //: Sets the length of the file specified by the object to the new size
     //: Sets the length of the file specified by the object to the new size
     //: Shrinking or Growing the file as needed.


   OsStatus touch();
     //: Updates the date and time on the file.  Creates if needed.


/* ============================ ACCESSORS ================================= */

      /// Convert Windows FILETIME to an OsTime.
    static OsTime OsFileWnt::fileTimeToOsTime(FILETIME ft);
      /**<
      *  This static function converts a windows FILETIME to a sipX OsTime
      *  @param ft - The Windows FILETIME to convert
      *  @returns an OsTime representing the FILETIME
      */

      /// Get information about a file.
    virtual OsStatus getFileInfo(OsFileInfoBase& rFileinfo) const;
      /**< 
      *  Returns all the relevant info on this file
      *  @param rFileInfo - [OUT] The object that is filled with the
      *         file information.
      */

/* ============================ INQUIRY =================================== */

    UtlBoolean isReadonly() const;
    //: Returns TRUE if file is readonly

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


