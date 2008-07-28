// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
//  
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004-2007 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsTimer.h"
#include "os/OsTimerTask.h"
#include "os/OsQueuedEvent.h"
#include "os/OsLock.h"
#include "os/OsEvent.h"
#include "os/OsTimerMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType OsTimer::TYPE = "OsTimer" ;

#ifdef VALGRIND_TIMER_ERROR
// Dummy static variable to receive values from tracking variables.
static char dummy;
#endif /* VALGRIND_TIMER_ERROR */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
// Timer expiration event notification happens using the 
// newly created OsQueuedEvent object

OsTimer::OsTimer(OsMsgQ* pQueue, const int userData) :
   mBSem(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mApplicationState(0),
   mTaskState(0),
   // Always initialize mDeleting, as we may print its value.
   mDeleting(FALSE),
   mpNotifier(new OsQueuedEvent(*pQueue, userData)) ,
   mbManagedNotifier(TRUE),
   mOutstandingMessages(0),
   mWasFired(FALSE),
   mTimerQueueLink(0)
{
#ifdef VALGRIND_TIMER_ERROR
   // Initialize the variables for tracking timer access.
   mLastStartBacktrace = NULL;
   mLastDestructorBacktrace = NULL;
#endif /* VALGRIND_TIMER_ERROR */
}

// The address of "this" OsTimer object is the eventData that is
// conveyed to the Listener when the notification is signaled.
OsTimer::OsTimer(OsNotification& rNotifier) :
   mBSem(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mApplicationState(0),
   mTaskState(0),
   // Always initialize mDeleting, as we may print its value.
   mDeleting(FALSE),
   mpNotifier(&rNotifier) ,
   mbManagedNotifier(FALSE),
   mOutstandingMessages(0),
   mWasFired(FALSE),
   mTimerQueueLink(0)
{
#ifdef VALGRIND_TIMER_ERROR
   // Initialize the variables for tracking timer access.
   mLastStartBacktrace = NULL;
   mLastDestructorBacktrace = NULL;
#endif /* VALGRIND_TIMER_ERROR */
}

// Destructor
OsTimer::~OsTimer()
{
#ifndef NDEBUG
   CHECK_VALIDITY(this);
#endif

   // Update members and determine whether we need to send an UPDATE_SYNC
   // to stop the timer or ensure that the timer task has no queued message
   // about this timer.
   UtlBoolean sendMessage = FALSE;
   {
      OsLock lock(mBSem);

#ifndef NDEBUG
      assert(!mDeleting);
      // Lock out all further application methods.
      mDeleting = TRUE;
#endif

      // Check if the timer needs to be stopped.
      if (isStarted(mApplicationState)) {
         sendMessage = TRUE;
         mApplicationState++;
      }
      // Check if there are outstanding messages that have to be waited for.
      if (mOutstandingMessages > 0) {
         sendMessage = TRUE;
      }
      // If we have to send a message, make note of it.
      if (sendMessage) {
         mOutstandingMessages++;
      }
   }

   // Send a message to the timer task if we need to.
   if (sendMessage) {
      OsEvent event;
      OsTimerMsg msg(OsTimerMsg::OS_TIMER_UPDATE_SYNC, this, &event);
      OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
      assert(res == OS_SUCCESS);
      event.wait();
   }
   
   // If mbManagedNotifier, free *mpNotifier.
   if (mbManagedNotifier) {
      delete mpNotifier;
   }
}

// Non-blocking asynchronous delete operation
void OsTimer::deleteAsync(OsTimer* timer)
{
#ifndef NDEBUG
   CHECK_VALIDITY(timer);
#endif

   // Update members.
   {
      OsLock lock(mBSem);

#ifndef NDEBUG
      assert(!mDeleting);
      // Lock out all further application methods.
      mDeleting = TRUE;
#endif

      // Check if the timer needs to be stopped.
      if (isStarted(mApplicationState))
      {
         mApplicationState++;
      }

      // Note we will be sending a message.
      mOutstandingMessages++;
   }

   // Send the message.
   OsTimerMsg msg(OsTimerMsg::OS_TIMER_UPDATE_DELETE, this, NULL);
   OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
   assert(res == OS_SUCCESS);
}

/* ============================ MANIPULATORS ============================== */

// Arm the timer to fire once at the indicated date and time
OsStatus OsTimer::oneshotAt(const OsDateTime& when)
{
   OsTime whenTime;
   when.cvtToTimeSinceEpoch(whenTime);
   return startTimer(whenTime, FALSE, OsTime::NO_WAIT_TIME);
}
     
// Arm the timer to fire once at the current time + offset
OsStatus OsTimer::oneshotAfter(const OsTime& offset)
{
   OsTime curTime;
   OsDateTime::getCurTime(curTime);
   return startTimer(curTime + offset, FALSE, OsTime::NO_WAIT_TIME);
}

// Arm the timer to fire periodically starting at the indicated date/time
OsStatus OsTimer::periodicAt(const OsDateTime& when, OsTime period)
{
   OsTime whenTime;
   when.cvtToTimeSinceEpoch(whenTime);
   return startTimer(whenTime, TRUE, period);
}

// Arm the timer to fire periodically starting at current time + offset
OsStatus OsTimer::periodicEvery(OsTime offset, OsTime period)
{
   OsTime curTime;
   OsDateTime::getCurTime(curTime);
   return startTimer(curTime + offset, TRUE, period);
}

// Disarm the timer
OsStatus OsTimer::stop(UtlBoolean synchronous)
{
#ifndef NDEBUG
   CHECK_VALIDITY(this);
#endif

   OsStatus result;
   UtlBoolean sendMessage = FALSE;

   // Update members.
   {
      OsLock lock(mBSem);

#ifndef NDEBUG
      assert(!mDeleting);
#endif

      // Determine whether the call is successful.
      if (isStarted(mApplicationState))
      {
         mWasFired = FALSE;
         // Update state to stopped.
         mApplicationState++;
         result = OS_SUCCESS;
         if (mOutstandingMessages == 0)
         {
            // We will send a message.
            sendMessage = TRUE;
            mOutstandingMessages++;
         }
      }
      else
      {
         result = OS_FAILED;
      }
   }

   // If we need to, send an UPDATE message to the timer task.
   if (sendMessage)
   {
      if (synchronous) {
         // Send message and wait.
         OsEvent event;
         OsTimerMsg msg(OsTimerMsg::OS_TIMER_UPDATE_SYNC, this, &event);
         OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
         assert(res == OS_SUCCESS);
         event.wait();
      }
      else
      {
         // Send message.
         OsTimerMsg msg(OsTimerMsg::OS_TIMER_UPDATE, this, NULL);
         OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
         assert(res == OS_SUCCESS);
      }
   }

   return result;
}

/* ============================ ACCESSORS ================================= */

// Return the OsNotification object for this timer
OsNotification* OsTimer::getNotifier(void) const
{
   return mpNotifier;
}

// Get the userData value of a timer constructed with OsTimer(OsMsgQ*, int).
int OsTimer::getUserData()
{
   // Have to cast mpNotifier into OsQueuedEvent* to get the userData.
   OsQueuedEvent* e = dynamic_cast <OsQueuedEvent*> (mpNotifier);
   assert(e != 0);
   intptr_t userData;
   e->getUserData(userData);
   return (int) userData;
}

unsigned OsTimer::hash() const
{
    return (unsigned) this;
}


UtlContainableType OsTimer::getContainableType() const
{
    return OsTimer::TYPE;
}


UtlBoolean OsTimer::getWasFired()
{
   OsLock lock(mBSem);
   return mWasFired;
}

/* ============================ INQUIRY =================================== */

// Return the state value for this OsTimer object
OsTimer::OsTimerState OsTimer::getState(void)
{
   OsLock lock(mBSem);
   return isStarted(mApplicationState) ? STARTED : STOPPED;
}

int OsTimer::compareTo(UtlContainable const * inVal) const
{
   int result;

   if (inVal->isInstanceOf(OsTimer::TYPE))
   {
      result = ((unsigned) this) - ((unsigned) inVal);
   }
   else
   {
      result = -1; 
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Get the current time as a Time.
/*OsTimer::Time OsTimer::now()
{
   OsTime t;
   OsDateTime::getCurTime(t);
   return (Time)(t.seconds()) * 1000000 + t.usecs();
}
*/
// Start the OsTimer object.
OsStatus OsTimer::startTimer(OsTime start,
                             UtlBoolean periodic,
                             OsTime period)
{
#ifndef NDEBUG
   CHECK_VALIDITY(this);
#endif

   OsStatus result;
   UtlBoolean sendMessage = FALSE;

   // Update members.
   {
      OsLock lock(mBSem);
#ifndef NDEBUG
      assert(!mDeleting);
#endif

      // Determine whether the call is successful.
      if (isStopped(mApplicationState))
      {
         mWasFired = FALSE;
         // Update state to started.
         mApplicationState++;
         result = OS_SUCCESS;
         if (mOutstandingMessages == 0)
         {
            // We will send a message.
            sendMessage = TRUE;
            mOutstandingMessages++;
         }
         // Set time values.
         mExpiresAt = start;
         mPeriodic = periodic;
         mPeriod = period;
      }
      else
      {
         result = OS_FAILED;
      }
   }

   // If we need to, send an UPDATE message to the timer task.
   if (sendMessage)
   {
      OsTimerMsg msg(OsTimerMsg::OS_TIMER_UPDATE, this, NULL);
      OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
      assert(res == OS_SUCCESS);
   }

   return result;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
