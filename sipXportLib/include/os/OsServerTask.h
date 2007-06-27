//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsServerTask_h_
#define _OsServerTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTask.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsMsg;

//:Abstract base class for tasks that process incoming msgs from an OsMsgQ

class OsServerTask : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int DEF_MAX_MSGS;

/* ============================ CREATORS ================================== */

     /// @brief Main constructor for OsServerTask
   OsServerTask(const UtlString& name="",
                void* pArg=NULL,
                const int maxRequestQMsgs=DEF_MAX_MSGS,
                const int priority=DEF_PRIO,
                const int options=DEF_OPTIONS,
                const int stackSize=DEF_STACKSIZE);
     /**<
     *  @param[in] name - the name of this OsServerTask
     *  @param[in] pArg - argument that is passed to the new thread as a
                   parameter of the thread run method.
     *  @param[in] maxRequestQMsgs - defines the size of the request message queue.
     *  @param[in] priority - the thread priority to use.  high = 0, low = 255
     *  @param[in] options - Thread execution options to set, such as whether
     *             to allow breakpoint debugging.
     *  @param[in] stackSize - The stack size to use for this task.
     */

   virtual
   ~OsServerTask();
     //:Destructor
     // As part of destroying the task, flush all messages from the incoming
     // OsMsgQ.

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& rMsg) = 0;
     //:Handles an incoming message
     // If the message is not one that the object is prepared to process,
     // the handleMessage() method in the derived class should return FALSE
     // which will cause the OsMessageTask::handleMessage() method to be
     // invoked on the message.

   virtual OsStatus postMessage(const OsMsg& rMsg,
                                const OsTime& rTimeout=OsTime::OS_INFINITY,
                                UtlBoolean sentFromISR=FALSE);
     //:Posts a message to this task
     // Return the result of the message send operation.

   virtual void requestShutdown(void);
     //:Call OsTask::requestShutdown() and then post an OS_SHUTDOWN message 
     //:to the incoming message queue to unblock the task

/* ============================ ACCESSORS ================================= */

   OsMsgQ* getMessageQueue();
     //:Get the pointer to the incoming message queue

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsMsgQ mIncomingQ;                 // Queue for incoming messages.

   virtual OsStatus receiveMessage(OsMsg*& rpMsg);
     //:Waits for a message to arrive on the task's incoming message queue

   virtual OsStatus receiveMessage(OsMsg*& rpMsg,
                                   const OsTime& rTimeout);
     //:Waits for a message to arrive on the task's incoming message queue,
     // with timeout.

   virtual int run(void* pArg);
     //:The entry point for the task
     // This method executes a message processing loop until either 
     // requestShutdown(), deleteForce(), or the destructor for this object
     // is called.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsServerTask(const OsServerTask& rOsServerTask);
     //:Copy constructor (not implemented for this class)

   OsServerTask& operator=(const OsServerTask& rhs);
     //:Assignment operator (not implemented for this task)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsServerTask_h_

