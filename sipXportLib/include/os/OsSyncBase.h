//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsSyncBase_h_
#define _OsSyncBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"

// DEFINES
// If OS_SYNC_DEBUG, enable debugging information for binary semaphores and
// mutexes.
// #define OS_SYNC_DEBUG

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Base class for the synchronization mechanisms in the OS abstraction layer

class OsSyncBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   virtual
      ~OsSyncBase() { };
     //:Destructor

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   OsSyncBase& operator=(const OsSyncBase& rhs);
     //:Assignment operator

   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY) = 0;
     //:Block until the sync object is acquired or the timeout expires

   virtual OsStatus tryAcquire(void) = 0;
     //:Conditionally acquire the semaphore (i.e., don't block)
     // Return OS_BUSY if the sync object is held by some other task.

   virtual OsStatus release(void) = 0;
     //:Release the sync object

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsSyncBase() { };
     //:Default constructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsSyncBase(const OsSyncBase& rOsSyncBase);
     //:Copy constructor

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsSyncBase_h_

