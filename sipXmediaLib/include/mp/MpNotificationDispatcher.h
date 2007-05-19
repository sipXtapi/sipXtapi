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

#ifndef _MpNotificationDispatcher_h_
#define _MpNotificationDispatcher_h_

// SYSTEM INCLUDES
#include <os/OsMsgQ.h>

// APPLICATION INCLUDES
#include "mp/MpResourceNotificationMsg.h"

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
*  The Notification Dispatcher is used to hold and notify users of 
*  notification messages.  This is first being created to be used by
*  resources, held in the flowgraph, and used to pass notification up
*  to the application level.  In the future, this could be extended to
*  allow filtering of notification messages - The one setting up the 
*  notification dispatcher could set properties on it to enable only
*  certain types of messages to be sent up through it's framework.
*/
class MpNotificationDispatcher : public OsMsgQ
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MpNotificationDispatcher()
      : OsMsgQ()
      {};

     /// Destructor
   virtual ~MpNotificationDispatcher() {};

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// Post a resource notification message.
   virtual OsStatus postNotification(const MpResourceNotificationMsg& msg);
     /**<
     *  This posts a resource notification message to the Notification queue.
     *  @NOTE If the queue is full, a flag will be set that notifications
     *  were lost, and the new notifications will be dropped on the floor
     *  until such time as the queue empties until there is room available.
     *  It is the application's duty to service and receive messages from
     *  the queue.
     *
     *  @param MpResourceNotificationMsg - the notification message to post.
     *  @returns OS_SUCCESS if it was able to add to the queue, 
     *  OS_LIMIT_REACHED otherwise.
     */

     /// Receive a notification message from the queue.
   virtual OsStatus receive(MpResourceNotificationMsg*& rpMsg,
                            const OsTime& rTimeout=OsTime::OS_INFINITY);
     /**< 
     *  Remove a message from the head of the queue
     *  Wait until either a message arrives or the timeout expires.
     *  Other than for messages sent from an ISR, the receiver is responsible
     *  for freeing the received message.
     *  @param rpMsg - pointer to resource notification msg will be stored here.
     *  @param rTimeout - how long to wait for a notification..
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   UtlBoolean mNotificationsLost; ///< Whether any notifications have 
     ///< have been dropped on the floor due to the queue being full.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Set that there were notifications lost.
   void setNotificationsLost() { mNotificationsLost = TRUE; };

     /// Copy constructor (not implemented for this class)
   MpNotificationDispatcher(const MpNotificationDispatcher& rMpNotifyDispatcher);

     /// Assignment operator (not implemented for this class)
   MpNotificationDispatcher& operator=(const MpNotificationDispatcher& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpNotificationDispatcher_h_
