//
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
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
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <sys/time.h>
#ifdef ANDROID // [
#  include <sys/resource.h> // for setpriority()
#endif // !ANDROID ]


/* Make sure we get MCL_CURRENT and MCL_FUTURE (for mlockall) on OS X 10.3 */
#define _P1003_1B_VISIBLE
#include <sys/mman.h>
#undef _P1003_1B_VISIBLE

// Include to access sys_gettid() syscall for debug purposes
#include <sys/syscall.h>

// APPLICATION INCLUDES
#include "os/OsExcept.h"
#include "os/OsLock.h"
#include "os/OsUtil.h"

#include "os/linux/OsLinuxDefs.h"
#include "os/linux/OsTaskLinux.h"
#include "os/linux/OsUtilLinux.h"

// DEFINES
#ifdef ANDROID // [
// Bionic does not provide some functions, which we don't really need.
#  define pthread_cancel(x)           assert("There's no pthread_cancel T_T\n")
#  define pthread_setcanceltype(a,b)
#  define mlockall(a)                 POSIX_OK
#endif // ANDROID ]

// EXTERNAL FUNCTIONS
#ifndef ANDROID // [
static inline int sys_gettid() {return syscall(SYS_gettid);}
#endif // !ANDROID ]

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTaskLinux::OsTaskLinux(const UtlString& name,
                     void* pArg,
                     const int priority,
                     const int options,
                     const int stackSize)
:  OsTaskBase(name, pArg, priority, options, stackSize),
   mTaskId(0),
   mDeleteGuard(OsRWMutex::Q_PRIORITY),
   mSuspendCnt(0),
   mOptions(options),
   mPriority(priority),
   mStackSize(stackSize)
{
   pthread_mutex_init(&mStartupSyncMutex, NULL);
   pthread_cond_init(&mTaskInitializedEvent, NULL);
   pthread_cond_init(&mTaskStartedEvent, NULL);
}

// Destructor
OsTaskLinux::~OsTaskLinux()
{
   waitUntilShutDown();
   doLinuxTerminateTask(FALSE);
}

// Delete the task even if the task is protected from deletion.
// After calling this method, the user will still need to delete the
// corresponding OsTask object to reclaim its storage.
OsStatus OsTaskLinux::deleteForce(void)
{
   OsLock lock(mDataGuard);

   doLinuxTerminateTask(TRUE);

   return OS_SUCCESS;
}
/* ============================ MANIPULATORS ============================== */

// Restart the task.
// The task is first terminated, and then reinitialized with the same
// name, priority, options, stack size, original entry point, and
// parameters it had when it was terminated.
// Return TRUE if the restart of the task is successful.
UtlBoolean OsTaskLinux::restart(void)
{
   OsLock lock(mDataGuard);

   doLinuxTerminateTask(FALSE);
   return doLinuxCreateTask(getName());
}

// Resume the task.
// This routine resumes the task. The task suspension is cleared, and
// the task operates in the remaining state.
OsStatus OsTaskLinux::resume(void)
{
   OsLock lock(mDataGuard);

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   if (mSuspendCnt < 1) // we're already running
      return OS_SUCCESS;

   if (--mSuspendCnt == 0) // predecrement to perform the test afterward
      pthread_kill(mTaskId, SIGCONT);
   return OS_SUCCESS;
}

// Spawn a new task and invoke its run() method.
// Return TRUE if the spawning of the new task is successful.
// Return FALSE if the task spawn fails or if the task has already
// been started.
UtlBoolean OsTaskLinux::start(void)
{
   OsLock lock(mDataGuard);

   if (isStarted())
      return FALSE;

   return doLinuxCreateTask(getName());
}
     
// Suspend the task.
// This routine suspends the task. Suspension is additive: thus, tasks
// can be delayed and suspended, or pended and suspended. Suspended,
// delayed tasks whose delays expire remain suspended. Likewise,
// suspended, pended tasks that unblock remain suspended only.
OsStatus OsTaskLinux::suspend(void)
{
   OsLock lock(mDataGuard);

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   if (mSuspendCnt++ == 0) // postincrement to perform the test beforehand
      pthread_kill(mTaskId, SIGSTOP);
   return OS_SUCCESS;
}

// Set the errno status for the task.
OsStatus OsTaskLinux::setErrno(int errno)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_SUCCESS;
}

// Set the execution options for the task
// The only option that can be changed after a task has been created
// is whether to allow breakpoint debugging.
OsStatus OsTaskLinux::setOptions(int options)
{
   return OS_NOT_YET_IMPLEMENTED;
}

// Set the priority of the task.
// Priorities range from 0, the highest priority, to 255, the lowest priority.
OsStatus OsTaskLinux::setPriority(int priority)
{
   int    linuxRes;
   int    policy;
   struct sched_param param;

   OsLock lock(mDataGuard);

   if (!isStarted()) {
      mPriority = priority; // save mPriority for later use
      return OS_TASK_NOT_STARTED;
   }

#ifdef ANDROID // [
   int nice = OsUtilLinux::cvtOsPrioToLinuxPrio(priority);
   int res = setpriority(PRIO_PROCESS, 0, nice);
   OsSysLog::add(FAC_KERNEL, PRI_INFO,
                 "OsTaskLinux::setPriority(%d): setpriority(%d) for thread %s returned %d",
                 priority, nice, getName().data(), res);
   linuxRes = POSIX_OK;

   int rtPrio = OsUtilLinux::cvtOsPrioToLinuxRtPrio(priority);
   if (rtPrio > OsTaskLinux::RT_NO)
   {
      pthread_getschedparam(mTaskId, &policy, &param);
      param.sched_priority = rtPrio;
      linuxRes = pthread_setschedparam(mTaskId, SCHED_FIFO, &param);
      if (linuxRes == POSIX_OK)
      {
         OsSysLog::add(FAC_KERNEL, PRI_INFO, 
                       "OsTaskLinux::setPriority: setting RT priority %d for \"%s\"", 
                       rtPrio, mName.data());
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, 
                       "OsTaskLinux::setPriority: failed to set RT priority %d for task \"%s\"", 
                       rtPrio, mName.data());
      }
   }
#else // ANDROID ][
   pthread_getschedparam(mTaskId, &policy, &param);
   param.sched_priority = OsUtilLinux::cvtOsPrioToLinuxPrio(priority);
   linuxRes = pthread_setschedparam(mTaskId, policy, &param);
#endif // !ANDROID ]

   if (linuxRes == POSIX_OK)
   {
         mPriority = priority;
         return OS_SUCCESS;
   }

   return OS_INVALID_PRIORITY;
}

// Add a task variable to the task.
// This routine adds a specified variable pVar (4-byte memory
// location) to its task's context. After calling this routine, the
// variable is private to the task. The task can access and modify
// the variable, but the modifications are not visible to other tasks,
// and other tasks' modifications to that variable do not affect the
// value seen by the task. This is accomplished by saving and restoring
// the variable's value each time a task switch occurs to or from the
// calling task.
OsStatus OsTaskLinux::varAdd(int* pVar)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

// Remove a task variable from the task.
// This routine removes a specified task variable, pVar, from its
// task's context. The private value of that variable is lost.
OsStatus OsTaskLinux::varDelete(int* pVar)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

// Set the value of a private task variable.
// This routine sets the private value of the task variable for a
// specified task. The specified task is usually not the calling task,
// which can set its private value by directly modifying the variable.
// This routine is provided primarily for debugging purposes.
OsStatus OsTaskLinux::varSet(int* pVar, int value)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

// Delay a task from executing for the specified number of milliseconds.
// This routine causes the calling task to relinquish the CPU for the
// duration specified. This is commonly referred to as manual
// rescheduling, but it is also useful when waiting for some external
// condition that does not have an interrupt associated with it.
OsStatus OsTaskLinux::delay(const int milliSecs)
{
#if defined(__APPLE__) || defined(ANDROID)
   struct timespec ts;
   ts.tv_nsec = (milliSecs % 1000) * 1000000;
   ts.tv_sec = milliSecs / 1000;
   
   // WARNING: This version of delay() is signal-unsafe. 
   // An unmasked signal during sleep may cause to wake the thread. 
   nanosleep(&ts, NULL);
   
#else
   const clockid_t clock = CLOCK_REALTIME;
   struct timespec ts;
   int ret;
   assert(milliSecs >= 0);       // negative delays don't make sense

   ret = clock_gettime(clock, &ts);
   if (ret != 0)
      return OS_FAILED;

   ts.tv_nsec += (milliSecs % 1000) * 1000000;
   ts.tv_sec += milliSecs / 1000;
   while (ts.tv_nsec > 1000000000)
   {
      ts.tv_nsec -= 1000000000;
      ts.tv_sec ++;
   }
 
   do
   {
      ret = clock_nanosleep(clock, TIMER_ABSTIME, &ts, NULL);

      if (ret != 0 && ret != EAGAIN && ret != EINTR && ret != ENOENT)
      {
         return OS_FAILED;
      }

   } while (ret != 0);
#endif
   return OS_SUCCESS;
}


// Make the calling task safe from deletion.
// This routine protects the calling task from deletion. Tasks that
// attempt to delete a protected task will block until the task is
// made unsafe, using unsafe(). When a task becomes unsafe, the
// deleter will be unblocked and allowed to delete the task.
// The safe() primitive utilizes a count to keep track of
// nested calls for task protection. When nesting occurs,
// the task becomes unsafe only after the outermost unsafe()
// is executed.
OsStatus OsTaskLinux::safe(void)
{
   OsTask*  pTask;
   OsStatus res;

   pTask = getCurrentTask();
   res = pTask->mDeleteGuard.acquireRead();
   assert(res == OS_SUCCESS);

   return res;
}

// Make the calling task unsafe from deletion.
// This routine removes the calling task's protection from deletion.
// Tasks that attempt to delete a protected task will block until the
// task is unsafe. When a task becomes unsafe, the deleter will be
// unblocked and allowed to delete the task.
// The unsafe() primitive utilizes a count to keep track of nested
// calls for task protection. When nesting occurs, the task becomes
// unsafe only after the outermost unsafe() is executed.
OsStatus OsTaskLinux::unsafe(void)
{
   OsTask*  pTask;
   OsStatus res;

   pTask = getCurrentTask();
   res = pTask->mDeleteGuard.releaseRead();
   assert(res == OS_SUCCESS);

   return res;
}

// Yield the CPU if a task of equal or higher priority is ready to run.
void OsTaskLinux::yield(void)
{
   delay(0);
}

/* ============================ ACCESSORS ================================= */

// Return a pointer to the OsTask object for the currently executing task
// Return NULL if none exists.
OsTaskLinux* OsTaskLinux::getCurrentTask(void)
{
   return OsTaskLinux::getTaskById(pthread_self());
}


// Convert a taskId into a UtlString
void OsTaskLinux::getIdString_d(UtlString &dest, OsTaskId_t tid)
{
   dest.appendFormat("%ld", tid);
}

// Convert a taskId into a UtlString
void OsTaskLinux::getIdString_x(UtlString &dest, OsTaskId_t tid)
{
   dest.appendFormat("%lx", tid);
}

// Convert a taskId into a UtlString
void OsTaskLinux::getIdString_X(UtlString &dest, OsTaskId_t tid)
{
   dest.appendFormat("%lX", tid);
}

// Return an Id of the currently executing task
OsStatus OsTaskLinux::getCurrentTaskId(OsTaskId_t &rid)
{
   rid = pthread_self();
   return OS_SUCCESS;
}

// Return a pointer to the OsTask object corresponding to the named task
// Return NULL if there is no task object with that name.
OsTaskLinux* OsTaskLinux::getTaskByName(const UtlString& taskName)
{
   OsStatus res;
   intptr_t val;

   res = OsUtil::lookupKeyValue(TASK_PREFIX, taskName, &val);
   assert(res == OS_SUCCESS || res == OS_NOT_FOUND);

   if (res == OS_SUCCESS)
   {
      assert(val != 0);
      return ((OsTaskLinux*) val);
   }
   else
      return NULL;
}

// Return a pointer to the OsTask object corresponding to taskId
// Return NULL is there is no task object with that id.
OsTaskLinux* OsTaskLinux::getTaskById(const pthread_t taskId)
{
   UtlString idString;
   OsStatus res;
   intptr_t val;

   getIdString_d(idString, taskId);   // convert the id to a string
   res = OsUtil::lookupKeyValue(TASKID_PREFIX, idString, &val);
   assert(res == OS_SUCCESS || res == OS_NOT_FOUND);

   if (res == OS_SUCCESS)
   {
      assert(val != 0);
      return ((OsTaskLinux*) val);
   }
   else
      return NULL;

}

// Get the errno status for the task
// We do have per-thread errno's under Linux, but there's no way to
// get them except from the thread itself. We could use a message to
// the thread to report back its errno, but then we'd have to wait
// for it to run and get the message. Solution: don't bother.
OsStatus OsTaskLinux::getErrno(int& rErrno)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   rErrno = 0; // no error

   return OS_SUCCESS;
}

// Return the execution options for the task
int OsTaskLinux::getOptions(void)
{
   return mOptions;
}
     
// Return the priority of the task
OsStatus OsTaskLinux::getPriority(int& rPriority)
{
//   int    linuxRes;
//   int    policy;
//   struct sched_param param;

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

//   linuxRes = pthread_getschedparam(mTaskId, &policy, &param);
//   assert(linuxRes == POSIX_OK);

//   rPriority = OsUtilLinux::cvtLinuxPrioToOsPrio(param.sched_priority);
   rPriority = mPriority;

   return OS_SUCCESS;
}
     
// Get the value of a task variable.
// This routine returns the private value of a task variable for its
// task. The task is usually not the calling task, which can get its
// private value by directly accessing the variable. This routine is
// provided primarily for debugging purposes.
OsStatus OsTaskLinux::varGet(void)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

/* ============================ INQUIRY =================================== */

// Get the task ID for this task
OsStatus OsTaskLinux::id(OsTaskId_t &rId)
{
   OsStatus retVal = OS_SUCCESS;
   
   //if started, return the taskId, otherwise return -1
   if (isStarted())
      rId = mTaskId;
   else
   {
      retVal = OS_TASK_NOT_STARTED;
      rId = -1;  
   }


   return retVal;
}

// Check if the task is suspended.
// Return TRUE is the task is suspended, otherwise FALSE.
UtlBoolean OsTaskLinux::isSuspended(void)
{
   OsLock lock(mDataGuard);

   if (!isStarted())
      return FALSE;

   return mSuspendCnt > 0;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Do the real work associated with creating a new Linux task
UtlBoolean OsTaskLinux::doLinuxCreateTask(const char* pTaskName)
{
   int                       linuxRes;
   UtlString                 idString;
   pthread_attr_t            attributes;
   timeval                   threadStartTime;
   timespec                  threadStartTimeout;

  // construct thread attribute
   linuxRes = pthread_attr_init(&attributes);
   if (linuxRes != POSIX_OK) {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "doLinuxCreateTask: pthread_attr_init failed (%d) ", linuxRes);
   }

   // Set desired stack size.
   size_t stacksize = 0;
   linuxRes = pthread_attr_getstacksize(&attributes, &stacksize);
   if (linuxRes != POSIX_OK) {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask pthread_attr_getstacksize error, returned %d", linuxRes);
   } else {
      if (mStackSize < PTHREAD_STACK_MIN)
      {
         OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsTaskLinux:doLinuxCreateTask %s increasing stack size from %d to allowed minimium %d",
                       pTaskName, mStackSize, PTHREAD_STACK_MIN);
         mStackSize = PTHREAD_STACK_MIN;
      }

      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsTaskLinux:doLinuxCreateTask %s default stack size: %d setting to: %d",
                    pTaskName, (int)stacksize, mStackSize);
      linuxRes = pthread_attr_setstacksize(&attributes, mStackSize);
      if (linuxRes != POSIX_OK)
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask pthread_attr_setstacksize error, returned %d", linuxRes);
   }

   // Create threads detached
   linuxRes = pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
   if (linuxRes != POSIX_OK) {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask pthread_attr_setdetachstate error, returned %d", linuxRes);
   }

   // Create threads with given RT policy and given priority
   linuxRes = pthread_attr_setschedpolicy(&attributes, SCHED_FIFO);
   if (linuxRes != POSIX_OK) {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask pthread_attr_setschedpolicy error, returned %d", linuxRes);
   }
   {
      struct sched_param param;
      param.sched_priority = OsUtilLinux::cvtOsPrioToLinuxRtPrio(mPriority);
      linuxRes = pthread_attr_setschedparam(&attributes, &param);
      if (linuxRes != POSIX_OK) {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask pthread_attr_setschedparam error, returned %d", linuxRes);
      }
   }

   // Lock startup synchronization mutex. It will be used in conjunction with
   // mTaskInitializedEvent and mTaskStartedEvent conditional variables.
   pthread_mutex_lock(&mStartupSyncMutex);

   linuxRes = pthread_create(&mTaskId, &attributes, taskEntry, (void *)this);
   pthread_attr_destroy(&attributes);

   if (linuxRes != POSIX_OK)
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask "
                    "pthread_create failed, returned %d in %s (%p)",
                    linuxRes, mName.data(), this);

      // Unlock startup synchronization mutex. We do not need it more.
      pthread_mutex_unlock(&mStartupSyncMutex);

      return FALSE;
   }

   // Get current time and prepare thread startup timeout value.
   // Note that we need an absolute time.
   gettimeofday(&threadStartTime, NULL);
   threadStartTimeout.tv_sec = threadStartTime.tv_sec + OS_TASK_THREAD_STARTUP_TIMEOUT;
   // Timeval uses micro-seconds, while timespec uses nano-seconds.
   threadStartTimeout.tv_nsec = threadStartTime.tv_usec * 1000;

   // Wait for thread to startup.
   int pt_res = pthread_cond_timedwait(&mTaskStartedEvent, &mStartupSyncMutex,
                                       &threadStartTimeout);

   // If it will not startup in OS_TASK_THREAD_STARTUP_TIMEOUT seconds then
   // something gone terribly wrong.
   if (pt_res == ETIMEDOUT)
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask "
                    "thread %s did not started up in %d seconds. Give up on it.",
                    mName.data(), OS_TASK_THREAD_STARTUP_TIMEOUT);

      // Unlock startup synchronization mutex. We do not need it more.
      pthread_mutex_unlock(&mStartupSyncMutex);

      return FALSE;
   }


   // Enter the thread id into the global name database so that given the
   // thread id we will be able to find the corresponding OsTask object
   getIdString_d(idString, mTaskId);   // convert the id to a string
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsTaskLinux::doLinuxCreateTask, task ID: %ld/'%s'/0x%lX", mTaskId, idString.data(), mTaskId);
   OsUtil::insertKeyValue(TASKID_PREFIX, idString, (intptr_t) this);

   mState = STARTED;

   // Startup initialization finished. Signal this to started thread, so
   // it could go on.
   pthread_cond_signal(&mTaskInitializedEvent);

   // Unlock startup synchronization mutex. Synchronization finished.
   pthread_mutex_unlock(&mStartupSyncMutex);

   return TRUE;
}

// Do the real work associated with terminating a Linux task
void OsTaskLinux::doLinuxTerminateTask(UtlBoolean doForce)
{
   OsStatus res;
   pthread_t savedTaskId;

   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "OsTaskLinux::doLinuxTerminateTask, deleting task thread: %x,"
                 " force = %d", (int)mTaskId, doForce);

   // if there is no low-level task, or entry in the name database, just return
   if ((mState != UNINITIALIZED) && ((int)mTaskId != 0))
   {
      // DEBUGGING HACK:  Suspend requester if target is suspended $$$
      while (isSuspended())
      {
         suspend();
      }
      
      if (!doForce)
      {
         // We are being well behaved and will wait until the task is no longer
         // safe from deletes.  A task is made safe from deletes by acquiring
         // a read lock on its mDeleteGuard. In order to delete a task, the
         // application must acquire a write lock. This will only happen after
         // all of the read lock holders have released their locks.
         res = mDeleteGuard.acquireWrite();
         assert(res == OS_SUCCESS);
      }

      savedTaskId = mTaskId; // taskUnregister sets mTaskId to zero;
      taskUnregister();
      
      // Send the thread the actual cancellation request.
      if (mState == STARTED)
      {
         requestShutdown();
         /* maybe replace this with a call to waitUntilShutDown() ? */
         for(int i = 0; i < 10 && isShuttingDown(); i++)
         {
            delay(100);
         }
      }
      if (mState == SHUTTING_DOWN)
      {
         if (savedTaskId != 0)
         {
            pthread_cancel(savedTaskId);
         }
      }
      
      if (!doForce)
      {
         res = mDeleteGuard.releaseWrite();     // release the write lock
         assert(res == OS_SUCCESS);
      }
   }

   mState = UNINITIALIZED;
}

extern "C" {int setLinuxTaskStartSuspended(int susp);}
extern "C" {int setVTSusp(int susp);}

int linuxTaskStartSuspended = 0;
int setLinuxTaskStartSuspended(int susp) {
   int save = linuxTaskStartSuspended;
   linuxTaskStartSuspended = susp;
   return save;
}

int setVTSusp(int susp)
{
   return setLinuxTaskStartSuspended(susp);
}

// Function that serves as the starting address for a Linux thread
void * OsTaskLinux::taskEntry(void* arg)
{
   OsStatus                  res;
   int                       linuxRes;
   OsTaskLinux*              pTask;
   pthread_attr_t            attributes;
   struct sched_param        param;

   pTask = (OsTaskLinux*) arg;

   // If we ever receive a thread cancel request, it means that the OsTask
   // object is in the process of being destroyed.  To avoid the situation
   // where a thread attempts to run after its containing OsTask object has
   // been freed, we set the thread up so that the cancel takes effect
   // immediately (as opposed to waiting until the next thread cancellation
   // point).
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

   if (linuxTaskStartSuspended)
   {
      pthread_kill(pthread_self(), SIGSTOP);
   }

   // construct thread attribute
   linuxRes = pthread_attr_init(&attributes);
   if (linuxRes != 0) {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux::taskEntry: pthread_attr_init failed (%d) ", linuxRes);
   }

   // Lock synchronization mutex. Begin synchronization of started thread with
   // doLinuxCreateTask() which created it.
   pthread_mutex_lock(&pTask->mStartupSyncMutex);

   // Thread started up. Signal this to doLinuxCreateTask().
   pthread_cond_signal(&pTask->mTaskStartedEvent);


   // Wait until our init in doLinuxCreateTask() is finished.
   //
   // The actual thread is created and started with pthread_create(), then
   // doLinuxCreateTask() enters the thread in the name database and
   // sets mState=STARTED. However, if OsTaskLinux::taskEntry() runs before
   // this initialization completes, callers might think (among other things)
   // that the thread is not started.
   pthread_cond_wait(&pTask->mTaskInitializedEvent, &pTask->mStartupSyncMutex);

   // Unlock startup synchronization mutex. Synchronization finished.
   pthread_mutex_unlock(&pTask->mStartupSyncMutex);

   // Log Thread ID for debug purposes
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsTaskLinux::taskEntry: Started task %s with lwp=%d, pid=%d",
                 pTask->mName.data(), sys_gettid(), getpid());

#ifdef ANDROID // [
   pTask->setPriority(pTask->mPriority);
#else // ANDROID ][
   int linuxPriority = OsUtilLinux::cvtOsPrioToLinuxPrio(pTask->mPriority);

   if(linuxPriority != RT_NO)
   {
#ifndef __APPLE__
      // Use FIFO realtime scheduling
      param.sched_priority = linuxPriority;
      linuxRes = sched_setscheduler(0, SCHED_FIFO, &param); 
      if (linuxRes == POSIX_OK)
      {
         OsSysLog::add(FAC_KERNEL, PRI_INFO, 
                       "OsTaskLinux::taskEntry: starting %s at RT linux priority: %d", 
                       pTask->mName.data(), linuxPriority);
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, 
                       "OsTaskLinux::taskEntry: failed to set RT linux priority: %d for task: %s", 
                       linuxPriority, pTask->mName.data());
      }
#else
      linuxRes = ~POSIX_OK;
      OsSysLog::add(FAC_KERNEL, PRI_INFO, 
                    "OsTaskLinux not starting at RT priority under MacOs") ;
#endif

      // keep all memory locked into physical mem, to guarantee realtime-behaviour
      if (linuxRes == POSIX_OK)
      {
         linuxRes = mlockall(MCL_CURRENT|MCL_FUTURE);
         if (linuxRes != POSIX_OK)
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, 
                          "OsTaskLinux::taskEntry: failed to lock memory for task: %s", 
                          pTask->mName.data());
         }
      }
   }
#endif // !ANDROID ]


   // Run the code the task is supposed to run, namely the run()
   // method of its class.

   // Mark the task as not safe to delete.
   res = pTask->mDeleteGuard.acquireRead();
   assert(res == OS_SUCCESS);

   unsigned int returnCode = pTask->run(pTask->getArg());

   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
           "OsTaskLinux::taskEntry run method exited with return: %d for task: %s",
           returnCode, pTask->mName.data());

   // After run returns be sure to mark the thread as shut down.
   pTask->ackShutdown();

   // Then remove it from the OsNameDb.
   pTask->taskUnregister();

   // Mark the task as now safe to delete.
   res = pTask->mDeleteGuard.releaseRead();
   assert(res == OS_SUCCESS);

   return ((void *)returnCode);
}

void OsTaskLinux::taskUnregister(void)
{
   OsStatus res;
   UtlString idString;
   
   if ( 0 != (int)mTaskId )
   {
      // Remove the key from the internal task list, before terminating it
      getIdString_d(idString, mTaskId);   // convert the id to a string
      res = OsUtil::deleteKeyValue(TASKID_PREFIX, idString);
   }
   else
   {
      res = OS_SUCCESS;
   }

   if (res != OS_SUCCESS)
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux::doLinuxTerminateTask, failed to delete"
                    " mTaskId = 0x%08x, key '%s', returns %d",
                    (int) mTaskId, idString.data(), res);
   }
   mTaskId = 0;

   assert(res == OS_SUCCESS || res == OS_NOT_FOUND);

}

/* ============================ FUNCTIONS ================================= */
