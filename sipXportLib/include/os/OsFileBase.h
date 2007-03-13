// 
// Copyright (C) 2005, 2007 SIPez LLC.
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

#ifndef _OsFile_h_
#define _OsFile_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsPathBase.h"
#include "os/OsLock.h"
#include "os/OsBSem.h"
#include "os/OsMutex.h"
#include "os/OsConfigDb.h"
#include <utl/UtlString.h>

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
class OsPathBase;

//:OS class for creating,reading, writing, manipulating files.
class OsFileBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    
   enum Mode {
      READ_ONLY = 1,
      WRITE_ONLY = 2,
      READ_WRITE = 4,
      CREATE = 8,
      TRUNCATE = 16,
      APPEND = 32,
      FSLOCK = 64,
      FSLOCK_WAIT = 128
   };
     //!enumcode: FSLOCK       - Opens the file exclusively (advisory locking).
     //!enumcode: FSLOCK_WAIT  - Waits to open the file locked rather than fail.
   
   //: Options for the origin in setting the file position.
    enum FilePositionOrigin {
      START     = 0,   
      CURRENT   = 1, 
      END       = 2      
    };
      //!enumcode: Start       - Set position relative to start of file.
      //!enumcode: Current     - Set position relative to current file position.
      //!enumcode: End         - Set position relative to end of file.

/* ============================ CREATORS ================================== */

   OsFileBase(const OsPathBase& filename);
     //:Default constructor

   virtual
   ~OsFileBase();
     //:Destructor

/* ============================ MANIPULATORS ============================== */


   //! Opens and reads the contents of the named file into the given UtlString
   static long openAndRead(const char* filename, UtlString& fileContentsRead);

   //! Opens and write the given UtlString to the named file 
   static long openAndWrite(const char* filename, const UtlString& fileContentsToWrite);

   //! Opens and write the given data to the named file 
   static long openAndWrite(const char* filename, 
                            const char* fileContentsToWrite,
                            unsigned int contentLength);

   virtual OsStatus open(const int mode = READ_WRITE);
     //: Opens the specified file using the specified mode
     //: Returns:
     //:        OS_SUCCESS if successful
     //:        OS_FILE_ACCESS_DENIED if file is readonly or currently in use
     //:        OS_FILE_NOT_FOUND if the file specified was not found.
     //: For file locking, use FSLOCK or FSLOCK_WAIT in mode. Note that you
     //: must open the file read-write to lock it, and that file locking is
     //: advisory: other callers must also use FSLOCK or FSLOCK_WAIT.
   
   virtual OsStatus fileunlock();
     //: Cross-process unlocks this file.
     //: Notes: This method should only be called by OsFileBase::close()!

   virtual OsStatus filelock(const bool wait);
     //: Cross-process locks this file, optionally waiting for the lock.
     //: Returns:
     //:        OS_SUCCESS if successful
     //:        OS_FAILED if unsuccessful
     //: Notes: This method should only be called by OsFileBase::open()!
   
   virtual OsStatus flush();
     //: Flushes any pending output

   virtual OsStatus write(const void* pBuf, unsigned long bufLen, unsigned long& rBytesWritten);
     //: Write X bytes to file
     //: Returns:
     //:        OS_SUCCESS if successful
     //:        OS_FILE_DISKFULL if (you guessed it) disk full.  :)
     //:        OS_localFileLocks
     //:        FILE_INVALID_HANDLE if something has gone wrong an handle is invalid.

   virtual OsStatus setLength(unsigned long newLength);
     //: Sets the length of the file specified by the object to the new size
     //: Sets the length of the file specified by the object to the new size
     //: Shrinking or Growing the file as needed.

   virtual OsStatus setPosition(long pos, FilePositionOrigin origin = START);
     //: Set the current active position in the file for the next read or write
     //: operation. The pos variable is a signed number which is
     //: added to the specified origin. For origin == OsFile::Start
     //: only positive values for pos are meaningful. For
     //: origin == OsFile::End only negative values for
     //: pos are meaningful


   virtual OsStatus remove(UtlBoolean bForce = FALSE);
     //: Removes the file specified by this object 
     //: Set bForce to TRUE to remove read-only files
     //: Returns: 
     //:        OS_SUCCESS if successful
     //:        OS_INVALID if failed

   virtual OsStatus rename(const OsPathBase& rNewFilename); 
     //: Rename this file to the new file name 
     //: Returns: 
     //:        OS_SUCCESS if successful
     //:        OS_INVALID if failed

   virtual OsStatus copy(const OsPathBase& rNewFilename);
     //: Copy this file to the new specified location
     //: Returns: 
     //:        OS_SUCCESS if successful
     //:        OS_FILE_WRITE_FAILED if it fails to create the file.

   virtual OsStatus setReadOnly(UtlBoolean isReadOnly);
     //: Sets the file to the new state
     //: Returns: 
     //:        OS_SUCCESS if successful
     //:         OS_INVALID if failed
    
   virtual OsStatus touch();
     //: Updates the date and time on the file.  Creates if needed.

/* ============================ ACCESSORS ================================= */

   virtual OsStatus getPosition(unsigned long &pos);
     //: Get the current active position in the file for the next read or write operation.

   virtual void getFileName(OsPathBase& rOsPath) const;
     //: Returns the fully qualified filename for this File object

   virtual OsStatus read(void *pBuf, unsigned long bufLen, unsigned long &rBytesRead);
     //: Read X bytes from file
    
   virtual OsStatus readLine(UtlString &str);
     //: Read bytes up to \n or eof, whichever comes first
     //: Return
   virtual UtlBoolean close();
     //: Closes the file. 

   OsStatus getLength(unsigned long &length);
     //: Returns the length of the file specified by the object

   FILE* getFileDescriptor() { return mOsFileHandle; };

   OsConfigDb* getFileLocks() { return mpFileLocks; };

/* ============================ INQUIRY =================================== */

   UtlBoolean isReadonly() const;
     //: Returns TRUE if file is readonly


   UtlBoolean exists() ;
     //: Returns TRUE if file object filename exists


   virtual OsStatus getFileInfo(OsFileInfoBase& rFileinfo) const = 0;
     //: Returns all the relevant info on this file

   UtlBoolean isEOF();
     //: Returns TRUE if stream is past end of file

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsMutex fileMutex;
     //: Block other methods like close
     //  while we are busy reading,writing and close.

   OsFileBase(const OsFileBase& rOsFile);
     //:Copy constructor

   OsFileBase& operator=(const OsFileBase& rhs);
     //:Assignment operator

   FILE *mOsFileHandle;
     //: Handle to file

   OsPathBase mFilename;
     //:Fully qualified name where file is (or will be) located

   int mMode;
     //: The open file's mode

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static OsConfigDb *mpFileLocks;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsFile_h_


