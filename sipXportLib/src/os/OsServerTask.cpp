//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "os/OsMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int OsServerTask::DEF_MAX_MSGS = OsMsgQ::DEF_MAX_MSGS;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsServerTask::OsServerTask(const UtlString& name,
                           void* pArg,
                           const int maxRequestQMsgs,
                           const int priority,
                           const int options,
                           const int stackSize)
:  OsTask(name, pArg, priority, options, stackSize),
   mIncomingQ(maxRequestQMsgs, OsMsgQ::DEF_MAX_MSG_LEN, OsMsgQ::Q_PRIORITY)

   // other than initialization, no work required
{
   if (OsSysLog::willLog(FAC_KERNEL, PRI_INFO))
   {

           OsSysLog::add(FAC_KERNEL, PRI_INFO,
                                "OsServerTask::OsServerTask %s queue: %p queue limit: %d",
                                mName.data(), &mIncomingQ, maxRequestQMsgs);
   }
}

// Destructor
// As part of destroying the task, flush all messages from the incoming
// OsMsgQ.
OsServerTask::~OsServerTask()
{
   waitUntilShutDown(20000);  // upto 20 seconds

   mIncomingQ.flush();    // dispose of any messages in the request queue
}

/* ============================ MANIPULATORS ============================== */

// Handle an incoming message.
// This is the message handler of last resort. It should only be called when
// the handleMessage() method in the derived class returns FALSE (indicating
// that the message has not been handled.
UtlBoolean OsServerTask::handleMessage(OsMsg& rMsg)
{
   UtlBoolean handled;

   handled = FALSE;

   switch (rMsg.getMsgType())
   {
   case OsMsg::OS_SHUTDOWN:
      handled = TRUE;
      break;
   default:
      osPrintf(
         "OsServerTask::handleMessage(): msg type is %d.%d, not OS_SHUTDOWN\n",
         rMsg.getMsgType(), rMsg.getMsgSubType());
   //   assert(FALSE);
      break;
   }

   return handled;
}

// Post a message to this task.
// Return the result of the message send operation.
OsStatus OsServerTask::postMessage(const OsMsg& rMsg, const OsTime& rTimeout,
                                   UtlBoolean sentFromISR)
{
   OsStatus res;

   if (sentFromISR)
      res = mIncomingQ.sendFromISR(rMsg);
   else
      res = mIncomingQ.send(rMsg, rTimeout);
   return res;
}

// Call OsTask::requestShutdown() and then post an OS_SHUTDOWN message
// to the incoming message queue to unblock the task.
void OsServerTask::requestShutdown(void)
{
   OsMsg msg(OsMsg::OS_SHUTDOWN, 0);

   OsTask::requestShutdown();
   postMessage(msg);
}

/* ============================ ACCESSORS ================================= */

// Get the pointer to the incoming message queue
OsMsgQ* OsServerTask::getMessageQueue()
{
    return(&mIncomingQ);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Waits for a message to arrive on the task's incoming message queue.
OsStatus OsServerTask::receiveMessage(OsMsg*& rpMsg)
{
   return mIncomingQ.receive(rpMsg);
}

// The entry point for the task.
// This method executes a message processing loop until either
// requestShutdown(), deleteForce(), or the destructor for this object
// is called.
int OsServerTask::run(void* pArg)
{
   UtlBoolean doShutdown;
   OsMsg*    pMsg = NULL;
   OsStatus  res;

   do
   {
      res = receiveMessage((OsMsg*&) pMsg);          // wait for a message
      assert(res == OS_SUCCESS);

      doShutdown = isShuttingDown();
      if (!doShutdown)
      {                                              // comply with shutdown
         if (!handleMessage(*pMsg))                  // process the message
            OsServerTask::handleMessage(*pMsg);
      }

      if (!pMsg->getSentFromISR())
         pMsg->releaseMsg();                         // free the message
   }
   while (!doShutdown);

   ackShutdown();   // acknowledge the task shutdown request
   return 0;        // and then exit
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
