//  
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <sergey DOT kostanbaev AT SIPez DOT com>

// SYSTEM INCLUDES
#include <os/OsIntTypes.h>
#include <os/OsSysLog.h>

#include <sched.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef __APPLE__
#include <assert.h>

#warning PosixTimer is not implimented in MacOS X
#define timer_create(x,y,z)      (assert(!"timer_create is not implemented"), -1)
#define timer_settime(x,y,z,u)   (assert(!"timer_settime is not implemented"), -1)
#define clock_getres(x,y)        (assert(!"clock_getres is not implemented"), -1)
#define timer_delete(x)          (assert(!"timer_delete is not implemented"), -1)
#define timer_getoverrun(x)      (assert(!"timer_getoverrun is not implemented"), -1)
#define SIGRTMIN 0
#define SIGRTMAX 0

#define si_ptr    si_addr

struct itv
{
    int tv_sec;
    int tv_nsec;
};

struct itimerspec
{
   struct itv it_value;
   struct itv it_interval;
};

#endif

// APPLICATION INCLUDES
#include "mp/MpMMTimerPosix.h"

#define TIMER_SIGNAL    SIGRTMIN
#ifdef ANDROID // [
#  define TIMER_SIGTERM SIGCONT
#else // ANDROID ][
#  define TIMER_SIGTERM SIGTERM
#endif // ANDROID ]

const char * const MpMMTimerPosix::TYPE = "POSIX Timer";

MpMMTimerPosix::PosixSignalReg gPosixTimerReg(TIMER_SIGNAL, MpMMTimerPosix::signalHandler);

MpMMTimerPosix::MpMMTimerPosix(MpMMTimer::MMTimerType type)
: MpMMTimer(type)
, mbTimerStarted(FALSE)
, mbTerminate(FALSE)
{
   if (mTimerType == Linear)
   {
      int res = sem_init(&mSyncSemaphore, 0, 0);
      assert( res == 0);
   }

   sem_init(&mIoSem, 0, 0);

   int res = pthread_create(&mThread, NULL, threadIoWrapper, this);
   assert(res == 0);

   sem_wait(&mIoSem);
}

MpMMTimerPosix::~MpMMTimerPosix()
{
   mbTerminate = TRUE;
#ifndef ANDROID // [
   // Under Android this leads to hang. Not sure why.
   // Maybe not anymore with checkins on 20180130 to clean up the teardown of this object
   pthread_join(mThread, NULL);
#endif // ANDROID ]
   if (mTimerType == Linear)
   {
      sem_destroy(&mSyncSemaphore);
   }

   sem_destroy(&mIoSem);

}

OsStatus MpMMTimerPosix::run(unsigned usecPeriodic)
{
  OsStatus status = OS_SUCCESS;
   if(mbTimerStarted)
   {
      return OS_INVALID_STATE;
   }

   struct sigevent evnt;
   // valgrind does not like uninialized vars
   memset(&evnt, 0, sizeof(struct sigevent));

   evnt.sigev_notify = SIGEV_SIGNAL;
   evnt.sigev_value.sival_ptr = this;
   evnt.sigev_signo = TIMER_SIGNAL;

   int res = timer_create(CLOCK_REALTIME, &evnt, &mTimer);
   if (res != 0)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
         "Couldn't create POSIX timer");
      return OS_INVALID_ARGUMENT;
   }

   struct itimerspec ts;
   ts.it_value.tv_sec = usecPeriodic / 1000000;
   ts.it_value.tv_nsec = (usecPeriodic % 1000000) * 1000;
   ts.it_interval.tv_sec = usecPeriodic / 1000000;
   ts.it_interval.tv_nsec = (usecPeriodic % 1000000) * 1000;

//CLOCK_REALTIME //TIMER_ABSTIME
   res = timer_settime(mTimer, CLOCK_REALTIME, &ts, NULL);
   if (res != 0)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
         "Couldn't set POSIX timer with %d us period", 
         usecPeriodic);

      return OS_INVALID_ARGUMENT;
   }

   mbTimerStarted = TRUE;
   return status;
}

OsStatus MpMMTimerPosix::stop()
{
   if (mbTimerStarted)
   {
      struct itimerspec ts;
      ts.it_value.tv_sec = 0;
      ts.it_value.tv_nsec = 0;
      ts.it_interval.tv_sec = 0;
      ts.it_interval.tv_nsec = 0;
      timer_settime(mTimer, CLOCK_REALTIME, &ts, NULL);

      int res = timer_delete(mTimer);
      assert (res == 0);

      mbTimerStarted = FALSE;

      return OS_SUCCESS;
   }
   else
      return OS_INVALID_STATE;
}


OsStatus MpMMTimerPosix::getPeriodRange(unsigned* pMinUSecs, 
                                        unsigned* pMaxUSecs)
{
   if (pMaxUSecs)
      *pMaxUSecs = INT_MAX;

   if (pMinUSecs)
      return getResolution(*pMinUSecs);

   return OS_SUCCESS;
}

OsStatus MpMMTimerPosix::getResolution(unsigned& resolution)
{
   struct timespec ts;
   int res = clock_getres(CLOCK_REALTIME, &ts);
   if (res != 0)
     return OS_FAILED;

   resolution = ts.tv_nsec / 1000;
   if (resolution == 0)
      resolution++;

   return OS_SUCCESS;
}

OsStatus MpMMTimerPosix::waitForNextTick()
{
   if(mTimerType != Linear)
   {
      return OS_INVALID_STATE;
   }

   if(mbTimerStarted == FALSE)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
                    "MpMMTimerPosix::waitForNextTick "
                    "- Timer not started or timer not initialized!");
      return OS_FAILED;
   }


   int res = sem_wait(&mSyncSemaphore);
   if (res == 0)
      return OS_SUCCESS;

   return OS_FAILED;
}


OsTime MpMMTimerPosix::getAbsFireTime() const
{
   return OsTime::OS_INFINITY;
}

OsStatus MpMMTimerPosix::setNotification(OsNotification* notification)
{ 
   mpNotification = notification;
   return OS_SUCCESS;
}

void MpMMTimerPosix::callback()
{
   if(mbTimerStarted)
   {
       int overruns = timer_getoverrun(mTimer);
       do
       {
          if (mTimerType == Notification)
          {
              if(mpNotification != NULL)
              {
                // Then signal it to indicate a tick.
                mpNotification->signal((intptr_t)this);
              }
          }
          else
          {
              sem_post(&mSyncSemaphore);
          }
       } while (overruns-- > 0);
   }
}

void* MpMMTimerPosix::threadIoWrapper(void* arg)
{
   MpMMTimerPosix* obj = (MpMMTimerPosix*)arg;

   sigset_t mask, fmask;
   sigemptyset(&mask);
   sigaddset (&mask, gPosixTimerReg.getSignalNum());

   sigfillset(&fmask);
   pthread_sigmask (SIG_SETMASK, &fmask, NULL);

   struct sched_param realtime;
   if (geteuid() == 0)
   {
       realtime.sched_priority = sched_get_priority_max(SCHED_FIFO);
       pthread_setschedparam(pthread_self(), SCHED_FIFO, &realtime);
   }

   int signum;

   sem_post(&obj->mIoSem);

   for(;;)
   {
      sigwait(&mask, &signum);
      if (obj->mbTerminate)
      {
#ifdef ANDROID // [
         OsSysLog::add(FAC_MP, PRI_DEBUG, "threadIoWrapper received signal TIMER_SIGTERM\n");
#endif // ANDROID ]
         obj->stop();
         return NULL;
      }
      assert(signum == gPosixTimerReg.getSignalNum());
#ifdef ANDROID // [
      if (signum == gPosixTimerReg.getSignalNum())
      {
         //OsSysLog::add(FAC_MP, PRI_DEBUG, "threadIoWrapper received signal gPosixTimerReg.getSignalNum()\n");
      }
      else
      {
         OsSysLog::add(FAC_MP, PRI_WARNING, "threadIoWrapper unknown signal: %d\n", signum);
      }
#endif // ANDROID ]

      obj->callback();
   }
}

void MpMMTimerPosix::signalHandler(int signum, siginfo_t *siginfo, void *context)
{
   assert(siginfo->si_signo == TIMER_SIGNAL);

   MpMMTimerPosix* object = (MpMMTimerPosix*)siginfo->si_ptr;
   object->callback();
}

MpMMTimerPosix::PosixSignalReg* MpMMTimerPosix::getSignalDescriptor()
{
   return &gPosixTimerReg;
}

MpMMTimerPosix::PosixSignalReg::PosixSignalReg(int sigNum, void (*sa)(int, siginfo_t *, void *))
: mSigNum(sigNum)
{
   sigemptyset (&mBlockSigMask);
   sigaddset (&mBlockSigMask, mSigNum);

   // Block this signal for all the threads by defalut
   // Interested in signal thread should use unblockThreadSig() to allow to catch it
   sigprocmask (SIG_BLOCK, &mBlockSigMask, NULL);   
}

int MpMMTimerPosix::PosixSignalReg::getSignalNum() const
{
   return mSigNum;
}

int MpMMTimerPosix::PosixSignalReg::blockThreadSig()
{
   return pthread_sigmask (SIG_BLOCK, &mBlockSigMask, NULL);
}

int MpMMTimerPosix::PosixSignalReg::unblockThreadSig()
{
   return pthread_sigmask (SIG_UNBLOCK, &mBlockSigMask, NULL);
}

MpMMTimerPosix::PosixSignalReg::~PosixSignalReg()
{
   int res = sigaction(mSigNum, &mOldAction, NULL);
   assert (res == 0);

   sigprocmask (SIG_UNBLOCK, &mBlockSigMask, NULL);  
}
