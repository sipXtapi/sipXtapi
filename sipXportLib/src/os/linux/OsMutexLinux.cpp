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
#include <assert.h>
//#include <pthread.h>

#include "utl/UtlRscTrace.h"

// APPLICATION INCLUDES
#include "os/linux/OsMutexLinux.h"
#include "os/linux/OsUtilLinux.h"
#include "os/linux/pt_mutex.h"
#include "os/OsTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor allowing the user to specify options
OsMutexLinux::OsMutexLinux(const unsigned options)
{
   int res;

   res = pt_mutex_init(&mMutexImp);
   assert(res == POSIX_OK);
}

// Destructor
OsMutexLinux::~OsMutexLinux()
{
   int res;
   res = pt_mutex_destroy(&mMutexImp);

   //assert(res == POSIX_OK);        // pt_mutex_destroy should always return TRUE
   if(res != POSIX_OK)
   {
       osPrintf("**** ERROR: OsMutex at %p could not be destroyed in thread %ld! ****\n", this, pthread_self());
//       osPrintf("****        OsMutex could not be destroyed in task \"%s\" ****\n", OsTask::getCurrentTask()->getName().data());
   }
}

/* ============================ MANIPULATORS ============================== */

// Block the task until the mutex is acquired or the timeout expires
OsStatus OsMutexLinux::acquire(const OsTime& rTimeout)
{
   struct timespec timeout;
   if(rTimeout.isInfinite())
      return (pt_mutex_lock(&mMutexImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   if(rTimeout.isNoWait())
      return (pt_mutex_trylock(&mMutexImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   OsUtilLinux::cvtOsTimeToTimespec(rTimeout, &timeout);
   return (pt_mutex_timedlock(&mMutexImp, &timeout) == POSIX_OK) ? OS_SUCCESS : OS_WAIT_TIMEOUT;
}

// Conditionally acquire the mutex (i.e., don't block)
// Return OS_BUSY if the lock is held by some other task
OsStatus OsMutexLinux::tryAcquire(void)
{
   return (pt_mutex_trylock(&mMutexImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
}

// Release the mutex
OsStatus OsMutexLinux::release(void)
{
   return (pt_mutex_unlock(&mMutexImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
}

/* ============================ ACCESSORS ================================= */

// Print mutex information to the console
void OsMutexLinux::OsMutexShow(void)
{
   osPrintf("OsMutex object %p\n", (void*) this);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


