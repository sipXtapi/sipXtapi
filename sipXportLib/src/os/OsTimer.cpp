//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>
#ifdef __pingtel_on_posix__
#  include <pthread.h>
#endif

#include "utl/UtlRscTrace.h"

// APPLICATION INCLUDES
#include "os/OsTimer.h"
#include "os/OsTimerTask.h"
#include "os/OsQueuedEvent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType OsTimer::TYPE = "OsTimer" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
// Timer expiration event notification happens using the 
// newly created OsQueuedEvent object
// The address of "this" OsTimer object is the eventData that is
// conveyed to the Listener when the notification is signaled.
OsTimer::OsTimer(OsMsgQ* pQueue, const int userData) :
   mpNotifier(new OsQueuedEvent(*pQueue, userData)) ,
   mState(CREATED),
   mTimerId(0),
   mType(UNSPECIFIED),
   mbManagedNotifier(true),
   mUserData(userData)   
{
}

OsTimer::OsTimer(OsNotification& rNotifier) :
   mpNotifier(&rNotifier) ,
   mState(CREATED),
   mTimerId(0),
   mType(UNSPECIFIED),
   mbManagedNotifier(false),
   mUserData(0)
{
    // deprecated.  Please use the other constructor.
}

// Destructor
OsTimer::~OsTimer()
{
   OsStatus res;

   switch (mState)
   {
   case CREATED: // fall through and do nothing
   case STOPPED:
      break;
   case STARTED:
      res = stop();
      assert(res == OS_SUCCESS);
      break;
   default:
      assert(FALSE);
   }
   
   if (mbManagedNotifier)
   {
      delete mpNotifier;
   }
   mpNotifier = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Arm the timer to fire once at the indicated date and time
OsStatus OsTimer::oneshotAt(const OsDateTime& when)
{
   OsTime   curTime;
   OsTime   expireTime;
   OsStatus res;

   OsDateTimeBase::getCurTime(curTime);
   res = when.cvtToTimeSinceEpoch(expireTime);
   assert(res == OS_SUCCESS);

   mExpiresAt = expireTime - curTime;
   mPeriod    = OsTime::OS_INFINITY;
   mType      = ONESHOT;
   doStartTimer();

   return OS_SUCCESS;
}
     
// Arm the timer to fire once at the current time + offset
OsStatus OsTimer::oneshotAfter(const OsTime& offset)
{
   mExpiresAt = offset;
   mPeriod    = OsTime::OS_INFINITY;
   mType      = ONESHOT;
   doStartTimer();

   return OS_SUCCESS;
}

// Arm the timer to fire periodically starting at the indicated date/time
OsStatus OsTimer::periodicAt(const OsDateTime& when, OsTime period)
{
   OsTime   curTime;
   OsTime   expireTime;
   OsStatus res;

   OsDateTimeBase::getCurTime(curTime);
   res = when.cvtToTimeSinceEpoch(expireTime);
   assert(res == OS_SUCCESS);

   mExpiresAt = expireTime - curTime;
   mPeriod    = period;

   // Since periodic timers are specified to start at a particular time,
   //  they are given a timer type of ONESHOT + PERIODIC.  After the
   //  first timer event occurs, the timer type is changed to just PERIODIC
   mType      = ONESHOT + PERIODIC;
   doStartTimer();

   return OS_SUCCESS;
}

// Arm the timer to fire periodically starting at current time + offset
OsStatus OsTimer::periodicEvery(OsTime offset, OsTime period)
{
   mExpiresAt = offset;
   mPeriod    = period;

   // Since periodic timers are specified to start at a particular time,
   //  they are given a timer type of ONESHOT + PERIODIC.  After the
   //  first timer event occurs, the timer type is changed to just PERIODIC
   mType      = ONESHOT + PERIODIC;
   doStartTimer();

   return OS_SUCCESS;
}

// Disarm the timer
OsStatus OsTimer::stop(void)
{
   doStopTimer();
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

// Return the OsNotification object for this timer.
OsNotification* OsTimer::getNotifier(void) const
{
   return mpNotifier;
}

unsigned OsTimer::hash() const
{
    return (unsigned) this;
}


UtlContainableType OsTimer::getContainableType() const
{
    return OsTimer::TYPE;
}

/* ============================ INQUIRY =================================== */

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

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Start the OsTime object and indicate that it has entered the STARTED state
void OsTimer::doStartTimer(void)
{
   assert(mState == CREATED || mState == STOPPED);

   mState = STARTED;
   OsTimerTask::startTimer(*this);
}

// Stop the OsTimer object and modify its object state accordingly
void OsTimer::doStopTimer(void)
{
   if (mState == CREATED || mState == STOPPED) // already stopped, just return
      return;

   // in the following assertion, we allow mState == STOPPED because the timer
   //  may have expired while we are in the midst of stopping it
   assert(mState == STARTED || mState == STOPPED);

   OsTimerTask::stopTimer(*this);
   mState = STOPPED;
}

// Return the period of a periodic timer.
// If the timer is not periodic, return an infinite period.
const OsTime& OsTimer::getPeriod(void) const
{
   return mPeriod;
}
     
// Return the state value for this OsTimer object
int OsTimer::getState(void) const
{
   return mState;
}

// Return the system timer id for this object.
// This method should only be called by the OsSysTimer object.
int OsTimer::getTimerId(void) const
{
   return mTimerId;
}

// Return the timer type.
int OsTimer::getType(void) const
{
   return mType;
}

// Return TRUE if this is a one-shot timer, otherwise FALSE.
UtlBoolean OsTimer::isOneshot(void) const
{
   return ((mType & ONESHOT) == ONESHOT);
}

// Return TRUE if this is a periodic timer, otherwise FALSE.
UtlBoolean OsTimer::isPeriodic(void) const
{
   return ((mType & PERIODIC) == PERIODIC);
}

// Set the state value for this OsTimer object
void OsTimer::setState(int state)
{
   mState = state;
}

// Set the system timer id for this object
// This method should only be called by the OsSysTimer object.
void OsTimer::setTimerId(int id)
{
   mTimerId = id;
}

// Set the timer type for this object.
void OsTimer::setTimerType(int timerType)
{
   mType = timerType;
}

/* ============================ FUNCTIONS ================================= */
