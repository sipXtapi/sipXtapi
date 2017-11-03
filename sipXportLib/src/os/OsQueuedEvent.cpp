//
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
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
#include <os/OsDateTime.h>
#include <os/OsEventMsg.h>
#include <os/OsQueuedEvent.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsQueuedEvent::OsQueuedEvent(OsMsgQ& rMsgQ, const intptr_t userData)
:  mUserData(userData),
   mpMsgQ(&rMsgQ)
{
}

// Destructor
OsQueuedEvent::~OsQueuedEvent()
{
   mpMsgQ = NULL;
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Set the event data and send an event message to the designated queue
// Return the result of the message send operation.
OsStatus OsQueuedEvent::signal(const intptr_t eventData)
{
   OsStatus res;

   res = doSendEventMsg(OsEventMsg::NOTIFY, eventData);
   return res;
}

// Set the user data value for this object
// Always returns OS_SUCCESS.
OsStatus OsQueuedEvent::setUserData(intptr_t userData)
{
   mUserData = userData;
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

// Return the user data specified when this object was constructed.
// Always returns OS_SUCCESS.
OsStatus OsQueuedEvent::getUserData(intptr_t& rUserData) const
{
   rUserData = mUserData;
   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Send an event message to the designated message queue.
// Return the result of the message send operation.
OsStatus OsQueuedEvent::doSendEventMsg(const int msgType,
                                       const intptr_t eventData) const
{
   OsTime timestamp;

   OsDateTime::getCurTimeSinceBoot(timestamp);

   OsEventMsg msg(msgType,     // event msg type
                  *this,       // this event
                  eventData,   // data signaled with event
                  timestamp);  // event timestamp (time since boot)

   OsStatus rc;
   if (mpMsgQ)
   {
      rc = mpMsgQ->send(msg, mSignalTimeout);
      if(rc == OS_WAIT_TIMEOUT)
      {
          OsSysLog::add(FAC_KERNEL, PRI_ERR,
              "OsQueuedEvent failed to queue \"%s\" (%p) event %p due to timeout (%d.%6d).  Queue contains %d messages with a max of %d",
              mpMsgQ->getName().data(), mpMsgQ, this, (int)mSignalTimeout.seconds(), (int)mSignalTimeout.usecs(), 
              mpMsgQ->numMsgs(), mpMsgQ->maxMsgs());
      }
   }
   else
   {
      rc = OS_FAILED;
   }
   return rc;
}

/* ============================ FUNCTIONS ================================= */


