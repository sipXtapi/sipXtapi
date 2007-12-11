//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsRWMutexLinux_h_
#define _OsRWMutexLinux_h_

// SYSTEM INCLUDES
#include <pthread.h>
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsRWMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Mutual exclusion semaphore handling multiple readers and writers
// Two kinds of concurrent tasks, called "readers" and "writers", share a
// single resource. The readers can use the resource simultaneously, but each
// writer must have exclusive access to it. When a writer is ready to use the
// resource, it should be enabled to do so as soon as possible.
class OsRWMutexLinux
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum QueueOptions
   {
      Q_FIFO     = 0x0, // queue blocked tasks on a first-in, first-out basis
      Q_PRIORITY = 0x1  // queue blocked tasks based on their priority
   };
     //!enumcode: Q_FIFO - queues blocked tasks on a first-in, first-out basis
     //!enumcode: Q_PRIORITY - queues blocked tasks based on their priority

/* ============================ CREATORS ================================== */

     /// Default constructor
   OsRWMutexLinux(const int queueOptions);

     /// Destructor
   virtual
   ~OsRWMutexLinux();

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus acquireRead();
     //:Block (if necessary) until the task acquires the resource for reading
     // Multiple simultaneous readers are allowed.

   virtual OsStatus acquireWrite();
     //:Block (if necessary) until the task acquires the resource for writing
     // Only one writer at a time is allowed (and no readers).

   virtual OsStatus tryAcquireRead();
     //:Conditionally acquire the resource for reading (i.e., don't block)
     // Multiple simultaneous readers are allowed.
     // Return OS_BUSY if the resource is held for writing by some other task

   virtual OsStatus tryAcquireWrite();
     //:Conditionally acquire the resource for writing (i.e., don't block).
     // Only one writer at a time is allowed (and no readers).
     // Return OS_BUSY if the resource is held for writing by some other task
     // or if there are running readers.

   virtual OsStatus releaseRead();
     //:Release the resource for reading

   virtual OsStatus releaseWrite();
     //:Release the resource for writing

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   pthread_rwlock_t mLockImp;

     /// Copy constructor (not implemented for this class)
   OsRWMutexLinux(const OsRWMutexLinux& rhs);

     /// Assignment operator (not implemented for this class)
   OsRWMutexLinux& operator=(const OsRWMutexLinux& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsRWMutexLinux_h_
