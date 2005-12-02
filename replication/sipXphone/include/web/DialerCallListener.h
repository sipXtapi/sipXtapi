// $Id: //depot/OPENDEV/sipXphone/include/web/DialerCallListener.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _MyCallListener_h_
#define _MyCallListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <ptapi/PtCallListener.h>
#include <ptapi/PtConnectionListener.h>
#include "ptapi/PtConnectionEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtMultiCallMetaEvent;
class PtCallEvent;

//:The DialerCallListener is used to register with PtAddress, PtTerminal, and 
//:PtCall objects to receive events from associated PtCall objects.

class DialerCallListener : public PtConnectionListener
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   PT_CLASS_INFO_MEMBERS

/* ============================ CREATORS ================================== */

   DialerCallListener(PtEventMask* pMask = NULL);
     //:Default constructor
     //!param: (in) pMask - Event mask defining events the listener is interested in.  This must be a subset of the events that the listener supports.  The mask may be NULL where it is assumed that all events applicable to the derived listener are of interest.

   virtual
   ~DialerCallListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

  virtual void callEventTransmissionEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:CALL_EVENT_TRANSMISSION_ENDED 
     //:indicating that the application will no longer receive call events on 
     //:this instance of the DialerCallListener.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callActive(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:CALL_ACTIVE 
     //:indicating that the state of the call object has changed to 
     //:PtCall::ACTIVE.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callInvalid(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:CALL_INVALID 
     //:indicating that the state of the call object has changed to 
     //:PtCall::INVALID.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaProgressStarted(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:CALL_META_PROGRESS_STARTED 
     //:indicating that the current call in the telephony platform has changed 
     //:state, and events will follow which indicate the changes to this call.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaProgressEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:CALL_META_PROGRESS_ENDED 
     //:indicating that the current call in the telephony platform has changed 
     //:state, and all the events that were associated with that change have 
     //:now been reported.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaSnapshotStarted(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:CALL_META_SNAPSHOT_STARTED 
     //:indicating that the Pingtel implementation is reporting to the 
     //:application the current state of the call on the associated telephony 
     //:platform, by reporting a set of simulated state changes that, in 
     //:effect, construct the current state of the call.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaSnapshotEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:CALL_META_SNAPSHOT_ENDED 
     //:indicating that the Pingtel implementation has finished reporting a 
     //:set of simulated state changes that, in effect, construct the current 
     //:state of the call.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaAddPartyStarted(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = CALL_META_ADD_PARTY_STARTED 
     //:indicating that a party has been added to the call. A "party" 
     //:corresponds to a PtConnection being added. Note that if a 
     //:PtTerminalConnection is added, it carries a meta event of 
     //:CALL_META_PROGRESS_STARTED.
     // The event parameter is valid only within this method. 
     // The implementation must copy the event if it is needed 
     // beyond the scope of an invocation.  The implementation of 
     // this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaAddPartyEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:CALL_META_ADD_PARTY_ENDED 
     //:indicates the end of the group of events related to the add party meta 
     //:event.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaRemovePartyStarted(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:CALL_META_REMOVE_PARTY_STARTED 
     //:indicating that a party (i.e. connection) has been removed from the 
     //:call by moving into the PtConnection::DISCONNECTED state.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaRemovePartyEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:CALL_META_REMOVE_PARTY_ENDED 
     //:indicating the end of the group of events related to the remove party 
     //:meta event.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void multicallMetaMergeStarted(const PtMultiCallMetaEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:MULTICALL_META_MERGE_STARTED 
     //:indicating that calls are merging, and events will follow which 
     //:indicate the changes to those calls.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void multicallMetaMergeEnded(const PtMultiCallMetaEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:MULTICALL_META_MERGE_ENDED 
     //:indicating that calls have merged, and that all state change events 
     //:resulting from this merge have been reported.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void multicallMetaTransferStarted(const PtMultiCallMetaEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:MULTICALL_META_TRANSFER_STARTED 
     //:indicating that a transfer is occurring, and events will follow which 
     //:indicate the changes to the affected calls.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void multicallMetaTransferEnded(const PtMultiCallMetaEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:MULTICALL_META_TRANSFER_ENDED 
     //:indicating that a transfer has completed, and that all state change 
     //:events resulting from this transfer have been reported.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

   virtual void connectionCreated(const PtConnectionEvent& rEvent) ;

   virtual void connectionAlerting(const PtConnectionEvent& rEvent);

   virtual void connectionDisconnected(const PtConnectionEvent& rEvent);

   virtual void connectionFailed(const PtConnectionEvent& rEvent);

   virtual void connectionUnknown(const PtConnectionEvent& rEvent);

   virtual void connectionDialing(const PtConnectionEvent& rEvent);

   virtual void connectionEstablished(const PtConnectionEvent& rEvent);

   virtual void connectionNetworkAlerting(const PtConnectionEvent& rEvent);

   virtual void connectionNetworkReached(const PtConnectionEvent& rEvent);

   virtual void connectionOffered(const PtConnectionEvent& rEvent);

   virtual void connectionQueued(const PtConnectionEvent& rEvent);

  
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

public:
	int			mState;
	UtlString	mCallId;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

	UtlString* mpOriginatingIP;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   DialerCallListener(const DialerCallListener& rDialerCallListener);
     //:Copy constructor

   DialerCallListener& operator=(const DialerCallListener& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MyCallListener_h_
