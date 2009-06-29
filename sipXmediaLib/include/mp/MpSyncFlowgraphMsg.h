//  
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpSyncFlowgraphMsg_h_
#define _MpSyncFlowgraphMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpFlowGraphMsg.h"
#include "utl/UtlSerialized.h"
#include "os/OsEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Message used to communicate with a flowgraph with ability to wait for
*         command completion and get its result.
*
*  This message does not own the event, thus it should be managed by some
*  outside means. As a special case, you should use pointer to a static OsEvent
*  variable with the great care. In this case you must wait infinitely until
*  event fires - if you do timed wait, you leave an opportunity to leave scope
*  of the event variable before it is fired, thus signal() will be called on
*  the freed stack variable and will lead to segfault. Here is an example:
*  <code>
*     OsEvent doneEvent;
*     MpSyncFlowgraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_GET_LATENCY_FOR_PATH, &doneEvent);
*
*     ...
*
*     // Send the message
*     stat = postMessage(msg);
*     if (stat != OS_SUCCESS)
*     {
*        // handle error
*        ...
*     }
*
*     // Wait for result infinitely.
*     stat = doneEvent.wait(OsTime::OS_INFINITY);
*     if (stat != OS_SUCCESS)
*     {
*        // This mustn't happen, but just in case.
*        ...
*     }
*     
*     // Retrieve result from the event.
*     intptr_t tmp;
*     doneEvent.getUserData(tmp);
*     latency = tmp;
*     doneEvent.getEventData(tmp);
*     stat = (OsStatus)tmp;
*
*     return stat;
*  </code>
*
*  If you need a timed wait on done event, you must allocate event from heap and
*  implement event hand off.
*/
class MpSyncFlowgraphMsg : public MpFlowGraphMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpSyncFlowgraphMsg(int msgType, OsEvent *pDoneEvent = NULL)
   : MpFlowGraphMsg(msgType)
   , mpDoneEvent(pDoneEvent)
   {
   };

     /// Copy constructor
   MpSyncFlowgraphMsg(const MpSyncFlowgraphMsg& rMsg)
   : MpFlowGraphMsg(rMsg)
   , mpDoneEvent(rMsg.mpDoneEvent)
   , mData(rMsg.mData)
   {
   };

     /// @copydoc OsMsg::createCopy()
   OsMsg* createCopy() const 
   {
      return new MpSyncFlowgraphMsg(*this);
   }

     /// Destructor
   ~MpSyncFlowgraphMsg() 
   {
   };

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   inline MpSyncFlowgraphMsg& operator=(const MpSyncFlowgraphMsg& rhs);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get stored data.
   inline UtlSerialized &getData();

     /// Get done-event.
   inline OsEvent *getDoneEvent();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsEvent         *mpDoneEvent; ///< Event to signal when command is finished
                               ///< and provide waiters with the result of
                               ///< operation. Event is not owned by this object.
   UtlSerialized    mData;     ///< Packed data to be passed to the resource,
};

/* ============================ INLINE METHODS ============================ */

MpSyncFlowgraphMsg& MpSyncFlowgraphMsg::operator=(const MpSyncFlowgraphMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MpFlowGraphMsg::operator=(rhs);       // assign fields for parent class

   mpDoneEvent = rhs.mpDoneEvent;
   mData       = rhs.mData;

   return *this;
}

UtlSerialized &MpSyncFlowgraphMsg::getData()
{
   return mData;
}

OsEvent *MpSyncFlowgraphMsg::getDoneEvent()
{
   return mpDoneEvent;
}

#endif  // _MpSyncFlowgraphMsg_h_
