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
#include <time.h>
#include <signal.h>

// APPLICATION INCLUDES
#include "os/OsTimerTask.h"
#include "os/OsMsgQ.h"
#include "os/OsStatus.h"
#include "os/shared/OsTimerMessage.h"
#include "os/shared/OsSysSoftTimer.h"

// EXTERNAL FUNCTIONS

/* provide a way of getting a clock-independent time if necessary */
#if defined(__pingtel_on_posix__) && !defined(sun)
pt_tick_t pt_get_ticks(void)
{
   static pt_tick_t ticks = 0;
   static struct timeval reference = {0, 0};

   if(reference.tv_sec)
   {
      struct timeval now;
      gettimeofday(&now, NULL);
      ticks += 1000000 * (now.tv_sec - reference.tv_sec);
      ticks += now.tv_usec - reference.tv_usec;
      reference = now;
   }
   else
      gettimeofday(&reference, NULL);

   return ticks;
}
#endif

// EXTERNAL VARIABLES

// CONSTANTS
// If OS_SYS_SOFT_TIMER_ASYNC_START_TIMER is defined, timers are started
// asynchronously.  That is, the call to startTimer() returns as soon
// as the startTimer request message is posted.  We do this to avoid
// unnecessarily blocking the caller when the syssofttimer thread is busy.
// Note: Operations to stop a timer and to flush a timer must still block
// so that we don't allow the caller to run before the syssofttimer has
// stopped using the timer.
#define OS_SYS_SOFT_TIMER_ASYNC_START_TIMER

// STATIC VARIABLE INITIALIZATIONS
OsSysSoftTimer*     OsSysSoftTimer::spInstance = NULL;
OsBSem              OsSysSoftTimer::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
struct TIMER_NODE * OsSysSoftTimer::timerQ = NULL;
OsMsgQ*             OsSysSoftTimer::requestQ = NULL;
const int           OsSysSoftTimer::MAX_REQUEST_MSGS = 1000;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the singleton object, creating it if necessary
OsSysSoftTimer* OsSysSoftTimer::getSysTimer(void)
{
   // If the singleton object already exists, then use it.
   if (spInstance != NULL)
      return spInstance;

   // If the object does not yet exist, then acquire the lock to ensure
   // that only one instance of the object is created
   sLock.acquire();
   if (spInstance == NULL)
   {
      requestQ = new OsMsgQ(MAX_REQUEST_MSGS);
      spInstance = new OsSysSoftTimer();
      if(spInstance)
         spInstance->start();
   }
   sLock.release();

   return spInstance;
}

// Destructor
OsSysSoftTimer::~OsSysSoftTimer()
{
   sLock.acquire();

   /* The soft timer thread must be shut down before we delete the OsMsgQ */

   OsBSem synch(OsBSem::Q_PRIORITY, OsBSem::EMPTY);
   /* This makes a SHUTDOWN message */
   OsTimerMessage message(&synch);

   requestQ->send(message);
   synch.acquire();

   delete requestQ;

   requestQ = NULL;
   spInstance = NULL;

   sLock.release();
}

/* ============================ MANIPULATORS ============================== */

// Start a software system timer
void OsSysSoftTimer::startTimer(OsTimer* pTimer)
{
   OsBSem synch(OsBSem::Q_PRIORITY, OsBSem::EMPTY);
   /* This makes an ADD message */
   OsTimerMessage message(pTimer, &synch);

   if (requestQ)
   {
      requestQ->send(message);
   }

#ifndef OS_SYS_SOFT_TIMER_ASYNC_START_TIMER
   synch.acquire();
#endif
}

// Stop and release a software system timer
void OsSysSoftTimer::stopTimer(OsTimer* pTimer)
{
   OsBSem synch(OsBSem::Q_PRIORITY, OsBSem::EMPTY);
   /* This makes a REMOVE message */
   OsTimerMessage message(pTimer->getTimerId(), &synch);

   requestQ->send(message);

   synch.acquire();

   pTimer->setState(OsTimer::STOPPED);
}

/* ============================ ACCESSORS ================================= */

// (static) Display information about the active timers in the system.
void OsSysSoftTimer::showTimerInfo(void)
{
   osPrintf("Obsoleted. see http://www.aspectc.org for profiling code \
that's not intrusive to the source code -- DLH");
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Constructor (only available internal to the class)
OsSysSoftTimer::OsSysSoftTimer() :
// Use high priority to get accurate time heartbeats for tasks (e.g. mediatask)
 OsTask("OsSysSoftTimer-%d", NULL, 5)
{
}

/* DEBUG - REMOVE THIS */
/*extern "C" int printf(const char *, ...);
void node_dump(char * info, struct TIMER_NODE * node)
{
   printf("--== NODE DUMP (%s) ==--\n", info);
   while(node)
   {
      printf("NODE 0x%x:\n", node);
      printf("\tpTimer = 0x%x\n", node->pTimer);
      printf("\texpiry = %lld\n", node->expiry);
      printf("\tperiod = %lld\n", node->period);
      printf("\tprev = 0x%x\n", node->prev);
      printf("\tnext = 0x%x\n", node->next);
      node = node->next;
   }
}*/

int OsSysSoftTimer::run(void * pArg)
{
   /* 1: add dummy timer
    * 2: calculate next timer expiration
    * 3: sleep until then or until a timer operation request arrives
    * 4: handle any timer operation requests
    * 5: handle any expired timers
    * 6: handle any additional timer operation requests
    * 7: repeat from step 2 until shutdown request
    * 8: free all queued timers */
   bool running = true;
   pt_tick_t reference = pt_get_ticks();

   /* step 1 */
   struct TIMER_NODE * timer = new struct TIMER_NODE;
   timer->pTimer = NULL;
   timer->expiry = TICKS_PER_SECOND / 10;
   timer->period = TICKS_PER_SECOND / 10;
   flushQ();
   insertQ(timer);
   //node_dump("after thread start insert", timerQ);

   while(running)
   {
      /* step 2 */
      timer = peekQ();
      /* should always be the case, as we have a dummy timer */
      if(timer)
      {
         pt_tick_t ticks = reference;
         reference = pt_get_ticks();
         timer->expiry -= reference - ticks;
         while(timer->expiry > 0)
         {
            /* step 3 */
#if defined(sun)
            /* use the knowledge that ticks are nanoseconds on solaris */
            OsTime expiry(timer->expiry / TICKS_PER_SECOND,
                          (timer->expiry % TICKS_PER_SECOND) / 1000);
#elif defined(__pingtel_on_posix__)
            /* use the knowledge that ticks are microseconds on other posix systems */
            OsTime expiry(timer->expiry / TICKS_PER_SECOND,
                          timer->expiry % TICKS_PER_SECOND);
#elif defined(_WIN32)
            /* use the knowledge that ticks are milliseconds on WIN32 */
            OsTime expiry(timer->expiry / TICKS_PER_SECOND,
                          (timer->expiry % TICKS_PER_SECOND) * 1000);
#endif
            OsMsg*   message = NULL;
            OsStatus res;

            res = requestQ->receive(message, expiry);
            if (res == OS_SUCCESS)
            {
               /* step 4 */
               running = doServiceRequests(message);
               if (!running) 
               {
                   break ; 
               }
               // As a consequence of servicing timer requests, the timer
               // at the head of the queue may have changed.
               timer = peekQ();
            }

             ticks = reference;
             reference = pt_get_ticks();
             timer->expiry -= reference - ticks;
         }
         if(!running)
         {
             break ; 
         }
         /* use a time machine to minimize math */
         /* (A timer that expires 1 tick late would require the next timer in
          * the differential queue to have its expiry time decremented by 1.
          * However, the situation is equivalent to leaving that expiry alone
          * and pretending that we weren't late. This also simplifies re-arming
          * a periodic timer later on - we'll want to re-arm it relative to when
          * it should have gone off, not relative to when it did. By not
          * changing the expiry time of the next timer in the queue, we can
          * accomplish both of these goals.) */
         reference += timer->expiry;

         /* step 5 */
         // remove the timer from the queue
         advanceQ();
         if(timer->pTimer)
         {
            OsNotification* pNotifier = timer->pTimer->getNotifier();
            pNotifier->signal((int) timer->pTimer);
         }

         // if the expired timer is periodic, reset the expiry time
         if(timer->period)
         {
            /* why does this work right? see above. */
            timer->expiry = timer->period;
            insertQ(timer);
         }
         else
         {
            if(timer->pTimer)
               timer->pTimer->setState(OsTimer::STOPPED);
            delete timer;
            timer = 0;
         }
      }
      else
      {
          osPrintf("OsSysSoftTimer::run() detected NO TIMERS! (This is bad!)\n");
      }
      /* step 6 */
      running = doServiceRequests(NULL);
   }
   return 0;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* timer queue element operations */

void OsSysSoftTimer::doAddTimer(OsTimerMessage * msg)
{
   struct TIMER_NODE * node = new struct TIMER_NODE;

   node->pTimer = msg->getTimer();

   // Note: Since periodic timers are specified to start at a particular time,
   //  they are given a timer type of ONESHOT + PERIODIC.
   int timerType = node->pTimer->getType();
   if (timerType & OsTimer::ONESHOT)
   {
      long msecsTilExpire = 0;
      while (msecsTilExpire <= 0)
      {
         msecsTilExpire = node->pTimer->mExpiresAt.cvtToMsecs();

         if (msecsTilExpire <= 0)
         {
            // the timer has already
            //  expired, signal the listener
            OsNotification* pNotifier = node->pTimer->getNotifier();
            pNotifier->signal((int) node->pTimer);

            // if the expired timer is periodic, increment and try again
            if (node->pTimer->isPeriodic())
               node->pTimer->mExpiresAt += node->pTimer->mPeriod;
            else
            {
               // expired one-shot timer, just return
               node->pTimer->setState(OsTimer::STOPPED);

#ifndef OS_SYS_SOFT_TIMER_ASYNC_START_TIMER
               // allow the requester to continue, the action has been completed
               msg->getSynchSem()->release();
#endif
               
               delete node;
               return;
            }
         }
      }

      // set the interval until timer expiration
      node->expiry = msecsTilExpire * TICKS_PER_MSEC;
   }

   if (timerType & OsTimer::PERIODIC)
      node->period = node->pTimer->mPeriod.cvtToMsecs() * TICKS_PER_MSEC;
   else
      node->period = 0;

   node->pTimer->setTimerId((int) node->pTimer);

   insertQ(node);
   //node_dump("after doAddTimer() insert", timerQ);

#ifndef OS_SYS_SOFT_TIMER_ASYNC_START_TIMER
   // allow the requester to continue, the action has been completed
   msg->getSynchSem()->release();
#endif
}

void OsSysSoftTimer::doRemoveTimer(OsTimerMessage * msg)
{
   int id = msg->getTimerID();
   struct TIMER_NODE * node = timerQ;

   while(node && id != (int) node->pTimer)
      node = node->next;

   if(node && node->pTimer)
   {
      if(node->next)
      {
         node->next->prev = node->prev;
         node->next->expiry += node->expiry;
      }
      if(node->prev)
         node->prev->next = node->next;
      else
         timerQ = node->next;

      delete node;
      //node_dump("after doRemoveTimer() remove", timerQ);
   }
   else
   {
#      ifdef TEST_PRINT
       // this can be ok, so disable it for normal case
       osPrintf("Warning: OsSysSoftTimer attempted to delete non-existent timer!\n");
#      endif
   }

   // allow the requester to continue, the action has been completed
   msg->getSynchSem()->release();
}


// Service all timer requests and return a boolean indicating whether
// the task should continue running.
bool OsSysSoftTimer::doServiceRequests(OsMsg* pFirstMessage)
{
   bool   keepRunning = true;
   OsMsg* pMessage;

   while(pFirstMessage || (requestQ->numMsgs() > 0))
   {
      // if pFirstMessage is not NULL, service that request first, then
      // check for more requests in requestQ
      if (pFirstMessage != NULL)
      {
         pMessage = pFirstMessage;
         pFirstMessage = NULL;
      }
      else
      {
         requestQ->receive(pMessage);
      }

      switch(pMessage->getMsgSubType())
      {
      case OsTimerMessage::ADD:
         doAddTimer((OsTimerMessage *) pMessage);
         break;
      case OsTimerMessage::REMOVE:
         doRemoveTimer((OsTimerMessage *) pMessage);
         break;
      case OsTimerMessage::SHUTDOWN:
         flushQ();
         // allow the requester to continue, the action has been completed
         ((OsTimerMessage *) pMessage)->getSynchSem()->release();
         keepRunning = false;
         break;
      }

      pMessage->releaseMsg();
      if (!requestQ || !keepRunning) 
      {
          break ; 
      }
   }
   return keepRunning;
}


/* timer queue maintenance operations */

struct TIMER_NODE * OsSysSoftTimer::peekQ(void)
{
   return timerQ;
}

struct TIMER_NODE * OsSysSoftTimer::advanceQ(void)
{
   struct TIMER_NODE * node = timerQ;
   if(node)
   {
      /* yes, single equals here */
      if((timerQ = timerQ->next))
      {
         timerQ->prev = NULL;
      }
      node->next = NULL;
   }
   return node;
}

int OsSysSoftTimer::insertQ(struct TIMER_NODE * node)
{
   struct TIMER_NODE * scan = timerQ;
   struct TIMER_NODE * last = NULL;
   /* find the first element of the list that expires after "node" or set "last"
    * if "node" expires after all the timers in the list */
   while(scan && node->expiry > scan->expiry)
   {
      if(!scan->next)
         last = scan;
      node->expiry -= scan->expiry;
      scan = scan->next;
   }
   /* yes, single equals in if()s scattered here...
    * this code needs to run nice and fast */
   /* if "node" will not be the last thing in the list... */
   if((node->next = scan))
   {
      /* if "node" will not be the first thing in the list... */
      if((node->prev = scan->prev))
      {
         node->prev->next = node;
      }
      /* ...or if it will be */
      else
      {
         timerQ = node;
      }
      scan->expiry -= node->expiry;
      node->next->prev = node;
   }
   /* ...or if it will */
   else
   {
      /* if there is anything else in the list */
      if((node->prev = last))
      {
         last->next = node;
      }
      /* ...or if there is not */
      else
      {
         timerQ = node;
      }
      node->next = NULL;
   }
   return timerQ == node;
}

void OsSysSoftTimer::flushQ(void)
{
   struct TIMER_NODE * node = timerQ;
   while(node)
   {
      if(node->next)
      {
         node = node->next;
         delete node->prev;
      }
      else
      {
         delete node;
         node = NULL;
      }
   }
   timerQ = NULL;
}

/* ============================ FUNCTIONS ================================= */


