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
#include "os/OsEvent.h"
#include "os/OsSysTimer.h"
#include "os/OsTimer.h"
#include "os/OsTimerMsg.h"
#include "os/OsTimerTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
// If OS_TIMER_ASYNC_START_TIMER is defined, timers are started
// asynchronously.  That is, the call to startTimer() returns as soon
// as the OsTimerMsg::START request message is posted.  We do this to
// avoid unnecessarily blocking the caller when the OsTimer thread is
// busy.
// Note: Calls to stopTimer() must still block because we don't allow
// the caller to run before the OsTimer has stopped using the timer.
#define OS_TIMER_ASYNC_START_TIMER

// STATIC VARIABLE INITIALIZATIONS
OsTimerTask* OsTimerTask::spInstance = 0;
OsBSem       OsTimerTask::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
const int    OsTimerTask::TIMER_MAX_REQUEST_MSGS = 1000;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the timer task, creating it if necessary
OsTimerTask* OsTimerTask::getTimerTask(void)
{
   UtlBoolean isStarted;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
       spInstance = new OsTimerTask();
   
   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }
   sLock.release();

   return spInstance;
}

// Destroy the singleton instance of the sys timer
void OsTimerTask::destroyTimer(void)
{
    sLock.acquire();
    if (spInstance)
    {
        delete spInstance ;
        spInstance = NULL ;
    }
    sLock.release();
}



// Destructor
OsTimerTask::~OsTimerTask()
{
   delete mpTimerSubsys;
   mpTimerSubsys = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Submit a service request to start (arm) the specified timer
void OsTimerTask::startTimer(OsTimer& rTimer)
{
   OsEvent      rpcEvent;
   OsTimerMsg   msg(OsTimerMsg::START, rpcEvent, rTimer);
   OsTimerTask* pTimerTask;
   OsStatus     res;
   
   pTimerTask = OsTimerTask::getTimerTask();
   res = pTimerTask->postMessage(msg);
   assert(res == OS_SUCCESS);

#ifndef OS_TIMER_ASYNC_START_TIMER
   int rpcRetVal;   

   res = rpcEvent.wait();
   assert(res == OS_SUCCESS);

   res = rpcEvent.getEventData(rpcRetVal);
   assert(res == OS_SUCCESS && rpcRetVal == OS_SUCCESS);

   res = rpcEvent.reset();
   assert(res == OS_SUCCESS);
#endif
}

// Submit a service request to cancel (disarm) the specified timer
void OsTimerTask::stopTimer(OsTimer& rTimer)
{
   OsEvent      rpcEvent;
   OsTimerMsg   msg(OsTimerMsg::STOP, rpcEvent, rTimer);
   OsTimerTask* pTimerTask;
   OsStatus     res;
   int          rpcRetVal;

   pTimerTask = OsTimerTask::spInstance;
   if (pTimerTask)
   {
    res = pTimerTask->postMessage(msg);
    assert(res == OS_SUCCESS);

    res = rpcEvent.wait();
    assert(res == OS_SUCCESS);

    res = rpcEvent.getEventData(rpcRetVal);
    assert(res == OS_SUCCESS && rpcRetVal == OS_SUCCESS);

    res = rpcEvent.reset();
    assert(res == OS_SUCCESS);
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Default constructor (called only indirectly via getTimerTask())
OsTimerTask::OsTimerTask(void)
:  OsServerTask("OsTimer-%d", NULL, TIMER_MAX_REQUEST_MSGS
#ifdef __pingtel_on_posix__
                , 5 // high priority so that we get reasonable clock heartbeats for media
#endif
               )
{
   mpTimerSubsys = OsSysTimer::getSysTimer();
}

// Handle a timer service request.
// Return TRUE if the request was handled, otherwise FALSE.
UtlBoolean OsTimerTask::handleMessage(OsMsg& rMsg)
{
   OsTimer*    pTimer;
   OsTimerMsg* pTimerMsg;
   OsStatus    res;

#ifdef OS_TIMER_ASYNC_START_TIMER
   UtlBoolean   doSignalRpcEvent = TRUE;
#endif

   if (rMsg.getMsgType() != OsMsg::OS_TIMER)
      return FALSE;

   pTimerMsg = (OsTimerMsg*) &rMsg;
   pTimer    = pTimerMsg->getTimer();

   switch (rMsg.getMsgSubType())
   {
   case OsTimerMsg::START:
      mpTimerSubsys->startTimer(pTimer);
#ifdef OS_TIMER_ASYNC_START_TIMER
      doSignalRpcEvent = FALSE;
#endif
      break;

   case OsTimerMsg::STOP:
      mpTimerSubsys->stopTimer(pTimer);
      break;

   default:
      assert(FALSE);                    // unexpected message type
      return FALSE;
      break;
   }

#ifdef OS_TIMER_ASYNC_START_TIMER
   if (doSignalRpcEvent)
   {
#endif
   OsEvent* pEvent;
   pEvent = pTimerMsg->getEvent();      // get the RPC notification event
   
   res = pEvent->signal(OS_SUCCESS);    // signal the request completion
   assert(res == OS_SUCCESS);           //  indication to the client
#ifdef OS_TIMER_ASYNC_START_TIMER
   }
#endif

   return TRUE;
}
/* ============================ FUNCTIONS ================================= */


