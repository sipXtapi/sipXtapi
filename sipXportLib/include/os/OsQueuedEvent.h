//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsQueuedEvent_h_
#define _OsQueuedEvent_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsMsgQ.h"
#include "os/OsNotification.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Queued events are used to send event notifications using a message queue.
*
*  When the corresponding event occurs, the Notifier sends a message the
*  designated message queue. The Listener must wait on the queue to receive
*  the event messages.
*
*     <h3>Background</h3>
*  First, a little bit of terminology.  The task that wishes to be notified
*  when an event occurs is the "Listener" task. The task that signals when
*  a given event occurs is the "Notifier" task.  A Notifier informs the
*  Listener that a given event has occurred by sending an "Event
*  Notification".
*
*     <h3>Expected Usage</h3>
*  The Listener passes an OsQueuedEvent object to the Notifier which
*  includes a message queue identifier for that message queue that will be
*  used for event notifications.  When the corresponding event occurs,
*  the Notifier sends a message the designated message queue.  The
*  Listener waits on the queue to receive the event notification.
*  This mechanism allows a task to receive notifications for multiple
*  events. The same message queue that is used to receive event
*  notifications may also be used to receive other types of messages.
*/
class OsQueuedEvent : public OsNotification
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

     /// Constructor
   OsQueuedEvent(OsMsgQ& rMsgQ, const intptr_t userData);

     /// Destructor
   virtual ~OsQueuedEvent();

/* ============================ MANIPULATORS ============================== */

     /// Set the event data and send an event message to the designated queue.
   virtual OsStatus signal(const intptr_t eventData);
     /**<
     *  @return The result of the message send operation.
     */

     /// Set the user data value for this object.
   virtual OsStatus setUserData(intptr_t userData);
     /**<
     *  @returns Always OS_SUCCESS.
     */

/* ============================ ACCESSORS ================================= */

     /// Return the user data specified when this object was constructed.
   virtual OsStatus getUserData(intptr_t& rUserData) const;
     /**<
     *  @returns Always OS_SUCCESS.
     */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   intptr_t  mUserData;    ///< Data specified on behalf of the user and
                           ///<  not otherwise used by this class -- the user
                           ///<  data is specified as an argument to the class
                           ///<  constructor.
   OsMsgQ*   mpMsgQ;       ///< Message queue where event notifications will
                           ///<  be sent.

     /// Send an event message to the designated message queue.
   OsStatus doSendEventMsg(const int msgType, const intptr_t eventData) const;
     /**
     *  @return The result of the message send operation.
     */

     /// Copy constructor (not implemented for this class).
   OsQueuedEvent(const OsQueuedEvent& rOsQueuedEvent);

     /// Assignment operator (not implemented for this class).
   OsQueuedEvent& operator=(const OsQueuedEvent& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsQueuedEvent_h_

