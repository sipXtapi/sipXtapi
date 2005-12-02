//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/linux/OsCSemLinux.h"
#include "os/linux/OsUtilLinux.h"
#include "os/linux/pt_csem.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor setting the initial and max semaphore values to maxCount
OsCSemLinux::OsCSemLinux(const int queueOptions, const int maxCount) :
   OsCSemBase(queueOptions, maxCount, maxCount)
{
   int res;
   init();

   res = pt_sem_init(&mSemImp, maxCount, maxCount);
   assert(res == POSIX_OK);
}

// Constructor allowing different initial and maximum semaphore values
OsCSemLinux::OsCSemLinux(const int queueOptions, const int maxCount,
                     const int initCount) :
  OsCSemBase(queueOptions, maxCount,initCount)
{
   int res;
   init();

   res = pt_sem_init(&mSemImp, maxCount, initCount);
   assert(res == POSIX_OK);
}

// Destructor
OsCSemLinux::~OsCSemLinux()
{
   int res;
   res = pt_sem_destroy(&mSemImp);

   assert(res == POSIX_OK);        // pt_sem_destroy should always return TRUE
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

// Block the task until the semaphore is acquired or the timeout expires
OsStatus OsCSemLinux::acquire(const OsTime& rTimeout)
{
   struct timespec timeout;
   OsStatus res;

   if (rTimeout.isInfinite())
      res = (pt_sem_wait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   else if (rTimeout.isNoWait())
      res = (pt_sem_trywait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   else
   {
      OsUtilLinux::cvtOsTimeToTimespec(rTimeout, &timeout);
      res = (pt_sem_timedwait(&mSemImp, &timeout) == POSIX_OK) ? OS_SUCCESS : OS_WAIT_TIMEOUT;
   }
   
#ifdef OS_CSEM_DEBUG
   if (res == OS_SUCCESS)
      updateAcquireStats();
#endif
    
   return res;
}
     
// Conditionally acquire the semaphore (i.e., don't block)
// Return OS_BUSY if the semaphore is held by some other task
OsStatus OsCSemLinux::tryAcquire(void)
{
   OsStatus res;
   
   res = (pt_sem_trywait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;

#ifdef OS_CSEM_DEBUG
   if (res == OS_SUCCESS)
      updateAcquireStats();
#endif

   return res;
}
/*
// Returns the current value of the semaphone
int OsCSemLinux::getValue(void)
{
   return pt_sem_getvalue(&mSemImp);
}
*/

// Release the semaphore
OsStatus OsCSemLinux::release(void)
{
   OsStatus res;

   res = (pt_sem_post(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;

#ifdef OS_CSEM_DEBUG
   if (res == OS_SUCCESS)
      updateReleaseStats();
#endif

    return res;
}    


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Common initialization shared by all (non-copy) constructors
void OsCSemLinux::init(void)
{
}

