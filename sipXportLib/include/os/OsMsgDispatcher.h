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

#ifndef _OsMsgDispatcher_h_
#define _OsMsgDispatcher_h_

// SYSTEM INCLUDES
#include <os/OsMsgQ.h>

// APPLICATION INCLUDES
#include "os/OsMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Dispatcher for Resource Notification Messages.
*
*  The Message Dispatcher is used to hold and notify users of 
*  messages.  If users are not interested in particular types of messages,
*  they can subclass this Message dispatcher and provide filtering to enable 
*  only certain types of messages to be sent up through it's framework.
*/
class OsMsgDispatcher
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   OsMsgDispatcher();
     /**<
     *  This constructor creates a queue, which will be destroyed in destructor.
     */

     /// Constructor for using external message queue for dispatching.
   OsMsgDispatcher(OsMsgQ* msgQ);
     /**<
     *  This constructor does not own the passed queue, so won't be deleted
     *  in destructor. It's user responsibility to manage the queue.
     */

     /// Destructor
   virtual ~OsMsgDispatcher();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// Post a resource notification message.
   virtual OsStatus post(const OsMsg& msg);
     /**<
     *  This posts a message to the dispatcher queue.
     *  @NOTE If the queue is full, a flag will be set that messages
     *  were lost, and the new messages will be dropped on the floor
     *  until such time as the queue empties until there is room available.
     *  It is the application's duty to service and receive messages from
     *  the queue.
     *
     *  @param[in] msg - the message to post.
     *  @retval OS_SUCCESS - if it was able to add to the queue, 
     *  @retval OS_LIMIT_REACHED otherwise.
     */

     /// Receive a message from the dispatcher queue.
   virtual OsStatus receive(OsMsg*& rpMsg,
                            const OsTime& rTimeout=OsTime::OS_INFINITY);
     /**< 
     *  Remove a message from the head of the queue
     *  Wait until either a message arrives or the timeout expires.
     *  Other than for messages sent from an ISR, the receiver is responsible
     *  for freeing the received message.
     *
     *  @param[in] rpMsg - pointer to msg will be stored here.
     *  @param[in] rTimeout - how long to wait for a notification..
     *  @retval OS_SUCCESS if the message was received and rpMsg filled in.
     *  @retval OS_WAIT_TIMEOUT if no message is in the queue before the timeout value.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return the number of messages in the queue
   inline virtual int numMsgs(void);

     /// Returns the maximum number of messages that can be queued
   inline int maxMsgs() const;

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// Return TRUE if the message queue is empty, FALSE otherwise
   inline virtual UtlBoolean isEmpty(void);

   inline UtlBoolean isMsgsLost() const;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsMsgQ* mMsgQueue; ///< The message queue that this dispatcher is using.
   UtlBoolean mQueueOwned; ///< Indicates whether or not mMsgQueue is owned by the dispatcher

   UtlBoolean mMsgsLost; ///< Whether any messages have been dropped on the 
                         ///< floor due to the queue being full.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Set that there were notifications lost.
   inline void setMsgsLost();

     /// Copy constructor (not implemented for this class)
   OsMsgDispatcher(const OsMsgDispatcher& rMsgDispatcher);

     /// Assignment operator (not implemented for this class)
   OsMsgDispatcher& operator=(const OsMsgDispatcher& rhs);
};

/* ============================ INLINE METHODS ============================ */


int OsMsgDispatcher::numMsgs( void )
{
   return mMsgQueue->numMsgs();
}

int OsMsgDispatcher::maxMsgs() const
{
   return mMsgQueue->maxMsgs();
}

UtlBoolean OsMsgDispatcher::isEmpty( void )
{
   return mMsgQueue->isEmpty();
}

UtlBoolean OsMsgDispatcher::isMsgsLost() const
{
   return mMsgsLost;
}

void OsMsgDispatcher::setMsgsLost()
{
    mMsgsLost = TRUE;
}

#endif  // _OsMsgDispatcher_h_
