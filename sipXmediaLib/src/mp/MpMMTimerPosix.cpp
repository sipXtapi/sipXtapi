//  
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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

// APPLICATION INCLUDES
#include "mp/MpMMTimerPosix.h"

#define TIMER_SIGNAL    SIGRTMIN

const char * const MpMMTimerPosix::TYPE = "POSIX Timer";

static MpMMTimerPosix::PosixSignalReg sPosixTimerReg(TIMER_SIGNAL, MpMMTimerPosix::signalHandler);

MpMMTimerPosix::MpMMTimerPosix(MpMMTimer::MMTimerType type)
: MpMMTimer(type)
, mbTimerStarted(FALSE)
{

}

MpMMTimerPosix::~MpMMTimerPosix()
{
   if (mbTimerStarted)
      stop();


}

OsStatus MpMMTimerPosix::run(unsigned usecPeriodic)
{
  OsStatus status = OS_SUCCESS;

   if(mbTimerStarted)
   {
      return OS_INVALID_STATE;
   }

   struct sigevent evnt;

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
   ts.it_value.tv_sec = 0;
   ts.it_value.tv_nsec = 0;
   ts.it_interval.tv_sec = usecPeriodic / 1000000;
   ts.it_interval.tv_nsec = (usecPeriodic % 1000000) * 1000;

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
      int res = timer_delete(mTimer);
      assert (res == 0);

      mbTimerStarted = FALSE;

      return OS_SUCCESS;
   }
   else
      return OS_INVALID_STATE;
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

  
   return OS_SUCCESS;
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
      assert(!"not implimented");
   }
}

void MpMMTimerPosix::signalHandler(int signum, siginfo_t *siginfo, void *context)
{
   assert(siginfo->si_signo == TIMER_SIGNAL);
   //assert(siginfo->si_code == SI_TIMER);
   
   MpMMTimerPosix* object = (MpMMTimerPosix*)siginfo->si_ptr;
   object->callback();
}

MpMMTimerPosix::PosixSignalReg::PosixSignalReg(int sigNum, void (*sa)(int, siginfo_t *, void *))
: mSigNum(sigNum)
{
   sigset_t mask;
   sigemptyset(&mask);
   sigaddset(&mask, mSigNum);

   struct sigaction act;
   act.sa_sigaction = sa;
   act.sa_flags = SA_SIGINFO;
   act.sa_mask = mask;

   int res = sigaction(mSigNum, &act, &mOldAction);
   assert (res == 0);
}

MpMMTimerPosix::PosixSignalReg::~PosixSignalReg()
{
   int res = sigaction(mSigNum, &mOldAction, NULL);
   assert (res == 0);
}
