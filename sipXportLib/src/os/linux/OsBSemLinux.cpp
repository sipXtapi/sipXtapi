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

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/linux/OsBSemLinux.h"
#include "os/linux/OsUtilLinux.h"
#include "os/linux/pt_csem.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsBSemLinux::OsBSemLinux(const int queueOptions, const int initState)
{
   int res;

   mTaskId = 0;
   mOptions = queueOptions;
   res = pt_sem_init(&mSemImp, 1, initState);
   assert(res == POSIX_OK);
#ifdef OS_SYNC_DEBUG
   if (initState == EMPTY)
      mTaskId = pthread_self();
#endif
}

// Destructor
OsBSemLinux::~OsBSemLinux()
{
   int res;
   res = pt_sem_destroy(&mSemImp);

   mOptions = 0;
   mTaskId = 0;

   //assert(res == POSIX_OK);        // pt_sem_destroy should always return TRUE
   if (res != POSIX_OK)
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "OsBSemLinux::~OsBSemLinux OsBemLinx object %p could not be destroyed in task %s with res = %d\n", this,
                    OsTask::getCurrentTask()->getName().data(), res);
   }
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

// Block the task until the semaphore is acquired or the timeout expires
OsStatus OsBSemLinux::acquire(const OsTime& rTimeout)
{
   OsStatus retVal;
   struct timespec timeout;

   if(rTimeout.isInfinite())
      retVal = (pt_sem_wait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   else if(rTimeout.isNoWait())
      retVal = (pt_sem_trywait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   else
   {
      OsUtilLinux::cvtOsTimeToTimespec(rTimeout, &timeout);
      retVal = (pt_sem_timedwait(&mSemImp, &timeout) == POSIX_OK) ? OS_SUCCESS : OS_WAIT_TIMEOUT;
   }
#ifdef OS_SYNC_DEBUG
   if (retVal == OS_SUCCESS)
      mTaskId = pthread_self();
#endif
   return retVal;
}
     
// Conditionally acquire the semaphore (i.e., don't block)
// Return OS_BUSY if the semaphore is held by some other task
OsStatus OsBSemLinux::tryAcquire(void)
{
   OsStatus retVal;

   retVal = (pt_sem_trywait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
#ifdef OS_SYNC_DEBUG
   if (retVal == OS_SUCCESS)
      mTaskId = pthread_self();
#endif
   return retVal;
}
     
// Release the semaphore
OsStatus OsBSemLinux::release(void)
{
   OsStatus retVal;

   retVal = (pt_sem_post(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_ALREADY_SIGNALED;
#ifdef OS_SYNC_DEBUG
   if (retVal == OS_SUCCESS)
      mTaskId = 0;
#endif
   return retVal;
}

/* ============================ INQUIRY =================================== */

// Print semaphore information to the console
void OsBSemLinux::OsBSemShow(void)
{
   const char *pOptionStr;
   const char *pSemState;
   const char *pTaskName;

   switch (mOptions)
   {
   case Q_FIFO:
      pOptionStr = "Q_FIFO";
      break;
   case Q_PRIORITY:
      pOptionStr = "Q_PRIORITY";
      break;
   default:
      pOptionStr = "UNKNOWN";
      break;
   }

#ifdef OS_SYNC_DEBUG
   pSemState = (mTaskId == 0) ? "AVAILABLE" : "TAKEN";
   pTaskName = (mTaskId == 0) ? "N/A" : "FIXME"; /*taskName(mTaskId);*/ // FIXME?
#else
   pSemState = "UNKNOWN";
   pTaskName = "UNKNOWN";
#endif

   osPrintf("OsBSem object %p, semOptions=%s, state=%s, heldBy=%s\n",
            (void *) this, pOptionStr, pSemState, pTaskName);
   //semShow(mSemImp, 1);
   //I don't think there's such a function under Linux. - Mike
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */


