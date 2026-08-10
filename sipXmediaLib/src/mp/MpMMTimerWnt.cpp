//  
// Copyright (C) 2007-2026 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

// SYSTEM INCLUDES
#include <os/OsSysLog.h>

// APPLICATION INCLUDES
#include "mp/MpMMTimerWnt.h"

// APPLICATION INCLUDES
// DEFINES
// VC6 does not seem to have the MMTimer TIME_KILL_SYNCHRONOUS option
// that prevents timer events from firing after a timeKillEvent call has been made.
// So, on Microsoft compilers before msvc7, turn it off.
#if defined(_MSC_VER) && (_MSC_VER < 1300) // if < msvc7 (2003)
#define MPMMTIMER_EXTRA_TIMER_OPTIONS 0
#else
#define MPMMTIMER_EXTRA_TIMER_OPTIONS TIME_KILL_SYNCHRONOUS
#endif

// STATIC VARIABLES INITIALIZATION
const char * const MpMMTimerWnt::TYPE = "Windows Multimedia";

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpMMTimerWnt::MpMMTimerWnt(MpMMTimer::MMTimerType type)
: MpMMTimer(type)
, mbTimerStarted(FALSE)
, mbBeginPeriodActive(FALSE)
, mPeriodMSec(0)
, mEventHandle(0)
, mpNotification(NULL)
, mTimerId(0)
{
   // We support both types of timers --
   // Linear and Notification, so just indicate we're initialized,
   // There's nothing to do.
   mbInitialized = TRUE;

   unsigned usResolution;
   if(getResolution(usResolution) != OS_SUCCESS)
   {
      mResolution = 0;
      mbInitialized = FALSE;
   }
   else
   {
      // Resolution we want is in ms, not us, so convert it..
      mResolution = usResolution/1000;
   }
}

MpMMTimerWnt::~MpMMTimerWnt()
{
   // if a timer is started, stop it.
   if(mbTimerStarted)
   {
      stop();
   }
}

/* ============================= MANIPULATORS ============================= */

OsStatus MpMMTimerWnt::setNotification(OsNotification* notification)
{ 
   mpNotification = notification;
   return OS_SUCCESS;
}

OsStatus MpMMTimerWnt::run(unsigned usecPeriodic)
{
   OsStatus status = OS_SUCCESS;

   if(mbInitialized == FALSE)
   {
      return OS_FAILED;
   }

   if(mbTimerStarted)
   {
      return OS_INVALID_STATE;
   }

   unsigned minPeriodUsec = 0;
   unsigned maxPeriodUsec = 0;
   if(getPeriodRange(&minPeriodUsec, &maxPeriodUsec) != OS_SUCCESS)
   {
      // Couldn't get the timer resolution.
      return OS_FAILED;
   }

   // Windows Multimedia timers are only offer millisecond resolution...
   // If someone provides a usecPeriodic value requesting microsecond resolution,
   // then bitch.  Also bitch if the requested resolution is outside our 
   // resolution range.
   if((usecPeriodic % 1000 > 0) ||
      (usecPeriodic < minPeriodUsec || usecPeriodic > maxPeriodUsec))
   {
      return OS_INVALID_ARGUMENT;
   }

   unsigned msecPeriodic = usecPeriodic / 1000;

   // Set the timer resolution to the minimum possible,
   // not the requested period we'll run for, to increase accuracy.
   // (this used to be set to msecPeriodic)
   if(timeBeginPeriod(mResolution) != TIMERR_NOERROR)
   {
      // Non-fatal: we get coarser resolution, but the periodic event
      // below can still be armed. Remember that no matching
      // timeEndPeriod is owed, so stop() must not make one.
      mbBeginPeriodActive = FALSE;
      status = OS_LIMIT_REACHED;
      OsSysLog::add(FAC_MP, PRI_WARNING, 
         "Couldn't set minimum MMTIMER begin period to %d ms", 
         mResolution);
   }
   else
   {
      mbBeginPeriodActive = TRUE;
      status = OS_SUCCESS;
   }
   mPeriodMSec = msecPeriodic;

   if(mTimerType == Notification)
   {
      mTimerId =
         timeSetEvent(mPeriodMSec, mResolution, 
                      (LPTIMECALLBACK)&MpMMTimerWnt::timeProcCallback,
                      (DWORD_PTR)this,
                      TIME_PERIODIC | TIME_CALLBACK_FUNCTION |
                      MPMMTIMER_EXTRA_TIMER_OPTIONS
                      );
   }
   else if(mTimerType == Linear)
   {
      // Create the event.
      mEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);

      mTimerId = 
         timeSetEvent(mPeriodMSec, mResolution, (LPTIMECALLBACK)mEventHandle, 
                      NULL, 
                      TIME_PERIODIC | TIME_CALLBACK_EVENT_PULSE |
                      MPMMTIMER_EXTRA_TIMER_OPTIONS
                      );
   }

   // mbTimerStarted must mean "a periodic event is armed", because
   // that is what stop() keys off to decide whether to kill it. Do
   // not set it from timeBeginPeriod's result: that call can fail
   // while timeSetEvent still succeeds, which previously left an
   // armed timer that stop() refused to kill.
   if(mTimerId == 0)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpMMTimerWnt::run timeSetEvent failed for period %d ms",
         mPeriodMSec);
      if(mbBeginPeriodActive)
      {
         timeEndPeriod(mResolution);
         mbBeginPeriodActive = FALSE;
      }
      if(mTimerType == Linear && mEventHandle != 0)
      {
         CloseHandle(mEventHandle);
         mEventHandle = 0;
      }
      mPeriodMSec = 0;
      status = OS_FAILED;
   }
   else
   {
      mbTimerStarted = TRUE;
   }

   return status;
}

OsStatus MpMMTimerWnt::stop()
{
   if(mbInitialized == FALSE)
   {
      return OS_FAILED;
   }
   if(mbTimerStarted == FALSE)
   {
      // If the timer wasn't started, it's invalid to stop it.
      return OS_INVALID_STATE;
   }

   // Kill the periodic event FIRST, and never return before doing so.
   // timeSetEvent was given TIME_KILL_SYNCHRONOUS, so timeKillEvent
   // does not return until any in-flight callback has completed.
   // The previous ordering returned OS_FAILED on a timeEndPeriod
   // failure without ever calling timeKillEvent, leaving the timer
   // armed while the caller went on to delete this object; the next
   // tick then dereferenced freed memory via dwUser.
   if(mTimerId == 0)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
         "MpMMTimerWnt::stop - No timer to kill!");
   }
   else
   {
      MMRESULT killResult = timeKillEvent(mTimerId);
      if(killResult != TIMERR_NOERROR)
      {
         OsSysLog::add(FAC_MP, PRI_ERR,
            "MpMMTimerWnt::stop - timeKillEvent(%u) failed: %u",
            mTimerId, killResult);
      }
      mTimerId = 0;
   }

   // Matching call for the timeBeginPeriod made in run(). Only make
   // it if that call actually succeeded, otherwise the system-wide
   // timer resolution refcount is unbalanced.
   if(mbBeginPeriodActive)
   {
      if(timeEndPeriod(mResolution) != TIMERR_NOERROR)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING, 
            "Couldn't set minimum MMTIMER end period to %d ms", 
            mResolution);
      }
      mbBeginPeriodActive = FALSE;
   }

   // Reset vars to stopped state.
   mPeriodMSec = 0;
   mbTimerStarted = FALSE;

   // If we're in linear mode, then we were using events, 
   // so clean up the event used.
   if(mTimerType == Linear && mEventHandle != 0)
   {
      // Close and reset the event handle.
      CloseHandle(mEventHandle);
      mEventHandle = 0;
   }

   return OS_SUCCESS;
}

OsStatus MpMMTimerWnt::waitForNextTick()
{
   if(mTimerType != Linear)
   {
      return OS_INVALID_STATE;
   }

   if(mbTimerStarted == FALSE || mbInitialized == FALSE)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
                    "MpMMTimerWnt::waitForNextTick "
                    "- Timer not started or timer not initialized!");
      return OS_FAILED;
   }

   if(WaitForSingleObject(mEventHandle, mPeriodMSec) == WAIT_FAILED)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
                    "MpMMTimerWnt - timer WaitForSingleObject failed with %s", 
                    GetLastError());
      // It failed, but it will just have waited till the period ended,
      // which is exactly what we want... so continue on..
   }

   return OS_SUCCESS;
}

/* ============================== ACCESSORS =============================== */

OsStatus MpMMTimerWnt::getResolution(unsigned& resolution)
{
   return getPeriodRange(&resolution);
}

OsStatus MpMMTimerWnt::getPeriodRange(unsigned* pMinUSecs, 
                                      unsigned* pMaxUSecs)
{
   OsStatus status = OS_FAILED;
   TIMECAPS timecaps;
   // timeGetDevCaps deals in msecs, not usecs.. as does the 
   // rest of the windows MMTimers - thus multiply by 1000.
   if(timeGetDevCaps(&timecaps, sizeof(TIMECAPS)) == TIMERR_NOERROR)
   {
      status = OS_SUCCESS;
      if(pMinUSecs)
      {
         *pMinUSecs = (unsigned)(timecaps.wPeriodMin) * 1000;
      }
      if(pMaxUSecs)
      {
         *pMaxUSecs = (unsigned)(timecaps.wPeriodMax) * 1000;
      }
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "Couldn't get windows MMTimer capabilities!");
   }

   return status;
}

/* =============================== INQUIRY ================================ */

/* ////////////////////////////// PROTECTED /////////////////////////////// */

void CALLBACK 
MpMMTimerWnt::timeProcCallback(UINT uID, UINT uMsg, DWORD_PTR dwUser,
    DWORD_PTR dw1, DWORD_PTR dw2)
{
   MpMMTimerWnt* srcObj = (MpMMTimerWnt*)dwUser;
   if(srcObj == NULL) return;

   assert(srcObj->getTimerType() == Notification);
   if(srcObj->getTimerType() != Notification)
   {
      // Don't do anything if the timer type is not notification
      // this shouldn't ever happen.
      return;
   }
   
   // If we have a notification pointer,
   if(srcObj->mpNotification != NULL)
   {
      // Then signal it to indicate a tick.
      srcObj->mpNotification->signal((intptr_t)srcObj);
   }
}

/* /////////////////////////////// PRIVATE //////////////////////////////// */

/* ============================== FUNCTIONS =============================== */

