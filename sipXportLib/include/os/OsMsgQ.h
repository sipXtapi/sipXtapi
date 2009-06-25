//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsMsgQ_h_
#define _OsMsgQ_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

// FORWARD DECLARATIONS
class OsMsg;

// TYPEDEFS
typedef UtlBoolean (*OsMsgQSendHookPtr) (const OsMsg& rMsg);
typedef void      (*OsMsgQFlushHookPtr) (const OsMsg& rMsg);

//:Message queue for inter-task communication

class OsMsgQBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int DEF_MAX_MSGS;      ///< Default maximum number of messages
   static const int DEF_MAX_MSG_LEN;   ///< Default maximum msg length (in bytes)
   static const UtlString MSGQ_PREFIX; ///< Prefix for OsMsgQ names stored in
                                       ///<  the name database

   enum Options
   {
      Q_FIFO     = 0x0, ///< queue blocked tasks on a first-in, first-out basis
      Q_PRIORITY = 0x1  ///< queue blocked tasks based on their priority
   };


/* ============================ CREATORS ================================== */

     /// Constructor
   OsMsgQBase(const UtlString& name);
     /**<
     *  If name is specified but is already in use, throw an exception.
     */

     /// Destructor
   virtual ~OsMsgQBase();

     /// Return a pointer to the named queue, or NULL if not found.
   static OsMsgQBase* getMsgQByName(const UtlString& name);

/* ============================ MANIPULATORS ============================== */


     /// Insert a copy of the message at the tail of the queue
   virtual OsStatus send(const OsMsg& rMsg,
                         const OsTime& rTimeout=OsTime::OS_INFINITY) = 0;
     /**<
     *  Wait until there is either room on the queue or the timeout expires.
     *
     *  This method creates a copy of the \p pMsg, before inserting it
     *  to the queue.
     */

     /// Insert an original of the message at the tail of the queue
   virtual OsStatus sendNoCopy(OsMsg *pMsg,
                               const OsTime& rTimeout=OsTime::OS_INFINITY) = 0;
     /**<
     *  Wait until there is either room on the queue or the timeout expires.
     *
     *  This method does not create a copy of the \p pMsg, inserting it
     *  to the queue as is.
     */

     /// Insert a copy of the message at the head of the queue
   virtual OsStatus sendUrgent(const OsMsg& rMsg,
                               const OsTime& rTimeout=OsTime::OS_INFINITY) = 0;
     /**<
     *  Wait until there is either room on the queue or the timeout expires.
     */

     /// Insert a copy of the message at the tail of the queue with ISR flag.
   virtual OsStatus sendFromISR(OsMsg& rMsg) = 0;
     /**<
     *  Sending from an ISR has a couple of implications.  Since we can't
     *  allocate memory within an ISR, we don't create a copy of the message
     *  before sending it and the sender and receiver need to agree on a
     *  protocol (outside this class) for when the message can be freed.
     *  The sentFromISR flag in the OsMsg object will be TRUE for messages
     *  sent using this method.
     */

     /// Remove a message from the head of the queue
   virtual OsStatus receive(OsMsg*& rpMsg,
                            const OsTime& rTimeout=OsTime::OS_INFINITY) = 0;
     /**<
     *  Wait until either a message arrives or the timeout expires.
     *  Other than for messages sent from an ISR, the receiver is responsible
     *  for freeing the received message.
     */

     /// Delete all messages currently in the queue
   virtual void flush();

     /// Set the function that is invoked whenever a msg is sent to the queue
   virtual void setSendHook(OsMsgQSendHookPtr func);
     /**<
     *  The function takes the message to be sent as an argument and returns a
     *  boolean value indicating whether the SendHook method has handled the
     *  message. If TRUE, the message is not inserted into the queue (since it
     *  has already been handled. If FALSE, the (possibly modified) message is
     *  inserted into the queue.
     */

     /// Set the function that is invoked whenever a msg is flushed from the queue.
   virtual void setFlushHook(OsMsgQFlushHookPtr func);
     /**<
     *  Messages get flushed when the OsMsgQ is deleted while there are messages
     *  still queued.
     *
     *  The function takes an OsMsg reference as an argument.
     */

/* ============================ ACCESSORS ================================= */

     /// Return the number of messages in the queue
   virtual int numMsgs() = 0;

     /// Returns the maximum number of messages that can be queued
   int maxMsgs() const;

     /// Return a pointer to the current send hook function
   virtual OsMsgQSendHookPtr getSendHook() const;

/* ============================ INQUIRY =================================== */

     /// Return TRUE if the message queue is empty, FALSE otherwise
   virtual UtlBoolean isEmpty();
     
     /// Get the name associated with the queue.
   const UtlString& getName() const { return mName; }

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Method that is invoked whenever a message is sent to the queue
   OsMsgQSendHookPtr  mSendHookFunc;

     /// Method that is invoked whenever a message is flushed from the queue
   OsMsgQFlushHookPtr mFlushHookFunc;

/* ---------------------------- DEBUG SCAFFOLDING ------------------------- */
protected:

   int      mMaxMsgs; ///< maximum number of messages the queue can hold

#if MSGQ_IS_VALID_CHECK
   virtual void testMessageQ() = 0;

   unsigned int mNumInsertEntry;
   unsigned int mNumInsertExitOk;
   unsigned int mNumInsertExitFail;

   unsigned int mNumRemoveEntry;
   unsigned int mNumRemoveExitOk;
   unsigned int mNumRemoveExitFail;

   unsigned int mLastSuccessTest;
#endif

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   UtlString mName;   ///< global name associated with the msg queue

     /// Copy constructor (not implemented for this class)
   OsMsgQBase(const OsMsgQBase& rOsMsgQBase);

     /// Assignment operator (not implemented for this class)
   OsMsgQBase& operator=(const OsMsgQBase& rhs);
};

/* ============================ INLINE METHODS ============================ */

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsMsgQ)
// with the OS-dependent realization of that type (e.g., OsMsgQWnt).
#if defined(_WIN32)
#  include "os/shared/OsMsgQShared.h"
   typedef class OsMsgQShared OsMsgQ;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsMsgQVxw.h"
   typedef class OsMsgQVxw OsMsgQ;
#elif defined(__pingtel_on_posix__)
#  include "os/shared/OsMsgQShared.h"
   typedef class OsMsgQShared OsMsgQ;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsMsgQ_h_
