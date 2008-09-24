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

#ifndef _OsFileLinux_h_
#define _OsFileLinux_h_

// SYSTEM INCLUDES

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
class OsFileInfoLinux;
class OsPathLinux;

//:OS class for creating,reading, writing, manipulating files.
class OsFileLinux : public OsFileBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   OsFileLinux(const OsPathBase& filename);
     //:Default constructor

   virtual ~OsFileLinux();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus filelock(const bool wait);
     //: Cross-process locks this file, optionally waiting for the lock.
     //: Returns:
     //:        OS_SUCCESS if successful
     //:        OS_FAILED if unsuccessful
     //: Notes: This method should only be called by OsFileBase::open()!

   OsStatus fileunlock();
     //: Cross-process unlocks this file.
     //: Notes: This method should only be called by OsFileBase::close()!

   OsStatus setLength(unsigned long newLength);
     //: Sets the length of the file specified by the object to the new size
     //: Sets the length of the file specified by the object to the new size
     //: Shrinking or Growing the file as needed.


   OsStatus setReadOnly(UtlBoolean isReadOnly);
     //: Sets the file to the new state
     //: Returns:
     //:        OS_SUCCESS if successful
     //:         OS_INVALID if failed


   OsStatus touch();
     //: Updates the date and time on the file.  Creates if needed.

/* ============================ ACCESSORS ================================= */



/* ============================ INQUIRY =================================== */

    UtlBoolean isReadonly() const;
    //: Returns TRUE if file is readonly


    virtual OsStatus getFileInfo(OsFileInfoBase& rFileinfo) const;
    //: Returns all the relevant info on this file

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsFileLinux(const OsFileLinux& rOsFileLinux);
     //:Copy constructor

   OsFileLinux& operator=(const OsFileLinux& rhs);
     //:Assignment operator

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsFileLinux_h_


