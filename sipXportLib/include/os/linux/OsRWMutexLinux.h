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
#ifdef ANDROID // [
// Bionic's pthreads implementation does not provide support for RW-locks.
// As a first attempt we simply replaced RW-locks with usual mutexes.
// I think this way is good way to go, because RW-locks for ARM-based device
// is an overkill. But if we discover any problems with this solution, we can
// switch to our own RW-locks implementation in sipXportLib/src/shared/OsRWMutexShared.cpp.
   typedef pthread_mutex_t pthread_rwlock_t;
#  define pthread_rwlock_init      pthread_mutex_init
#  define pthread_rwlock_destroy   pthread_mutex_destroy
#  define pthread_rwlock_rdlock    pthread_mutex_lock
#  define pthread_rwlock_wrlock    pthread_mutex_lock
#  define pthread_rwlock_tryrdlock pthread_mutex_trylock
#  define pthread_rwlock_trywrlock pthread_mutex_trylock
#  define pthread_rwlock_unlock    pthread_mutex_unlock
#endif // ANDROID ]

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

/* ============================ CREATORS ================================== */

     /// Default constructor
   OsRWMutexLinux(const int queueOptions);

     /// Destructor
   ~OsRWMutexLinux();

/* ============================ MANIPULATORS ============================== */

   OsStatus acquireRead();
     //:Block (if necessary) until the task acquires the resource for reading
     // Multiple simultaneous readers are allowed.

   OsStatus acquireWrite();
     //:Block (if necessary) until the task acquires the resource for writing
     // Only one writer at a time is allowed (and no readers).

   OsStatus tryAcquireRead();
     //:Conditionally acquire the resource for reading (i.e., don't block)
     // Multiple simultaneous readers are allowed.
     // Return OS_BUSY if the resource is held for writing by some other task

   OsStatus tryAcquireWrite();
     //:Conditionally acquire the resource for writing (i.e., don't block).
     // Only one writer at a time is allowed (and no readers).
     // Return OS_BUSY if the resource is held for writing by some other task
     // or if there are running readers.

   OsStatus releaseRead();
     //:Release the resource for reading

   OsStatus releaseWrite();
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
