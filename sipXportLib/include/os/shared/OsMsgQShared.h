//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsMsgQShared_h_
#define _OsMsgQShared_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsCSem.h"
#include "os/OsDefs.h"
#include "os/OsMsg.h"
#include "os/OsMsgQ.h"
#include "os/OsMutex.h"
#include "os/OsTime.h"
#include "utl/UtlDList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
class UtlString;

// FORWARD DECLARATIONS

// #define OS_MSGQ_DEBUG
// #define OS_MSGQ_REPORTING

// debug class
class DebugMutex
{
   public:
   DebugMutex(OsMutex& mutex, const char* label)
   {
      printf("DebugMutex %s:\n", label);
      mutex.OsMutexShow();
   };

   private:
   DebugMutex();
};

/**
*  Message queue implementation for OS's with no native message queue support
*
*  Two kinds of concurrent tasks, called "senders" and "receivers",
*  communicate using a message queue. When the queue is empty, receivers are
*  blocked until there are messages to receive. When the queue is full,
*  senders are blocked until some of the queued messages are received --
*  freeing up space in the queue for more messages.
*
*  This implementation is based on the description from the book "Operating
*  Systems Principles" by Per Brinch Hansen, 1973.  This solution uses:
*  <pre>
*    - a counting semaphore (mEmpty) to control the delay of the sender in
*      the following way:
*        initially:      the "empty" semaphore count is set to maxMsgs
*        before send:    acquire(empty)
*        after receive:  release(empty)
*    - a counting semaphore (mFull) to control the delay of the receiver in
*      the following way:
*        initially:      the "full" semaphore count is set to 0
*        before receive: acquire(full)
*        after send:     release(full)
*    - a binary semaphore (mGuard) to ensure against concurrent access to
*      internal object data
*  </pre>
*/
class OsMsgQShared : public OsMsgQBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

     /// Constructor
   OsMsgQShared(
      const int       maxMsgs=DEF_MAX_MSGS,      ///< Max number of messages.
      const int       maxMsgLen=DEF_MAX_MSG_LEN, ///< Max msg length (bytes).
      const int       options=Q_PRIORITY,        ///< How to queue blocked tasks.
      const UtlString& name=""                   ///< Global name for this queue.
      );
     /**<
     *  If name is specified but is already in use, throw an exception.
     */

     /// Destructor
   virtual  ~OsMsgQShared();

/* ============================ MANIPULATORS ============================== */

     /// @copydoc OsMsgQBase::send()
   virtual OsStatus send(const OsMsg& rMsg,
                         const OsTime& rTimeout=OsTime::OS_INFINITY);

     /// @copydoc OsMsgQBase::sendNoCopy()
   virtual OsStatus sendNoCopy(OsMsg *pMsg,
                               const OsTime& rTimeout=OsTime::OS_INFINITY);

     /// @copydoc OsMsgQBase::sendUrgent()
   virtual OsStatus sendUrgent(const OsMsg& rMsg,
                               const OsTime& rTimeout=OsTime::OS_INFINITY);

     /// @copydoc OsMsgQBase::sendFromISR()
   virtual OsStatus sendFromISR(OsMsg& rMsg);

     /// @copydoc OsMsgQBase::receive()
   virtual OsStatus receive(OsMsg*& rpMsg,
                            const OsTime& rTimeout=OsTime::OS_INFINITY);

/* ============================ ACCESSORS ================================= */

#ifdef OS_MSGQ_DEBUG
   int getFullCount() { return mFull.getValue();}
   int getEmptyCount() { return mEmpty.getValue();}
   UtlDList& getList() { return mDlist;} 
#endif

     /// @copydoc OsMsgQBase::numMsgs()
   virtual int numMsgs();

#ifdef MSGQ_IS_VALID_CHECK 
     /// Print information on the message queue to the console
   virtual void show();
#endif

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

#ifdef MSGQ_IS_VALID_CHECK 
   virtual void testMessageQ();
#endif


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex  mGuard;  ///< Mutex used to synchronize access to the msg queue.
   OsCSem   mEmpty;  ///< Counting semaphore used to coordinate sending msgs to
                     ///<  the queue and blocking senders to keep the number
                     ///<  of messages less than or equal to maxMsgs.
   OsCSem   mFull;   ///< Counting semaphore used to coordinate receiving msgs
                     ///<  from the queue and blocking receivers when there are
                     ///<  no messages to receive.
   UtlDList mDlist;  ///< Doubly-linked list used to store messages.

#ifdef MSGQ_IS_VALID_CHECK
   int      mOptions; ///< Message queue options.
   int      mHighCnt; ///< High water mark for the number of msgs in the queue.
#endif

#ifdef OS_MSGQ_REPORTING
   int      mIncreaseLevel;   ///< Emit a message to the log when the number
                              ///<  of messages reaches the mIncreaseLevel.
   int      mDecreaseLevel;   ///< Emit a message to the log when the number
                              ///<  of messages goes below the mDecreaseLevel.
   int      mIncrementLevel;  ///< When the mIncreaseLevel or mDecreaseLevels
                              ///<  are reached, increment/decrement the level
                              ///<  by mIncrementLevel.
#endif

     /// Helper function for sending messages
   OsStatus doSend(const OsMsg& rMsg, const OsTime& rTimeout,
                   const UtlBoolean isUrgent, const UtlBoolean needCopy);

     /// Helper function for removing a message from the head of the queue
   OsStatus doReceive(OsMsg*& rpMsg, const OsTime& rTimeout);

     /// Copy constructor (not implemented for this class)
   OsMsgQShared(const OsMsgQShared& rOsMsgQShared);

     /// Assignment operator (not implemented for this class)
   OsMsgQShared& operator=(const OsMsgQShared& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsMsgQShared_h_

