//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
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
#include <pthread.h>
#include <assert.h>

// APPLICATION INCLUDES
#include "os/linux/OsRWMutexLinux.h"
#include "utl/UtlRscTrace.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/* ============================ CREATORS ================================== */

OsRWMutexLinux::OsRWMutexLinux(const int queueOptions)
{
   int res;
   res = pthread_rwlock_init(&mLockImp, NULL);
   assert(res == POSIX_OK);
   UtlRscCount::created(UtlRscCount::OSRWMUTEX);
}

OsRWMutexLinux::~OsRWMutexLinux()
{
   int res;
   res = pthread_rwlock_destroy(&mLockImp);
   assert(res == POSIX_OK);
   UtlRscCount::destroyed(UtlRscCount::OSRWMUTEX);
}

/* ============================ MANIPULATORS ============================== */

OsStatus OsRWMutexLinux::acquireRead()
{
   return pthread_rwlock_rdlock(&mLockImp) == POSIX_OK ? OS_SUCCESS : OS_BUSY;
}

OsStatus OsRWMutexLinux::acquireWrite()
{
   return pthread_rwlock_wrlock(&mLockImp) == POSIX_OK ? OS_SUCCESS : OS_BUSY;
}

OsStatus OsRWMutexLinux::tryAcquireRead()
{
   return pthread_rwlock_tryrdlock(&mLockImp) == POSIX_OK ? OS_SUCCESS : OS_BUSY;
}

OsStatus OsRWMutexLinux::tryAcquireWrite()
{
   return pthread_rwlock_trywrlock(&mLockImp) == POSIX_OK ? OS_SUCCESS : OS_BUSY;
}

OsStatus OsRWMutexLinux::releaseRead()
{
   return pthread_rwlock_unlock(&mLockImp) == POSIX_OK ? OS_SUCCESS : OS_BUSY;
}

OsStatus OsRWMutexLinux::releaseWrite()
{
   return pthread_rwlock_unlock(&mLockImp) == POSIX_OK ? OS_SUCCESS : OS_BUSY;
}

