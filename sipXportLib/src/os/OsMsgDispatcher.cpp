//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMsgDispatcher.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsMsgDispatcher::OsMsgDispatcher()
   : mMsgsLost(FALSE)
{
   mMsgQueue = new OsMsgQ();
   mQueueOwned = true;
}

OsMsgDispatcher::OsMsgDispatcher(OsMsgQ* msgQ) 
   : mMsgQueue(msgQ)
   , mQueueOwned(false)
   , mMsgsLost(FALSE)
{
}

OsMsgDispatcher::~OsMsgDispatcher() 
{
   // Only delete the queue if we own it.
   if(mQueueOwned) 
   {
      delete mMsgQueue;
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus OsMsgDispatcher::post(const OsMsg& msg)
{
   if (mMsgQueue->numMsgs() == mMsgQueue->maxMsgs())
   {
      setMsgsLost();
      return OS_LIMIT_REACHED;
   }
   else
   {
      // Send the message, give it 1 millisecond to send.
      mMsgQueue->send(msg, OsTime(1));
      return OS_SUCCESS;
   }
};

OsStatus OsMsgDispatcher::receive(OsMsg*& rpMsg,
                                  const OsTime& rTimeout)
{
   return mMsgQueue->receive(rpMsg, rTimeout);
};

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


