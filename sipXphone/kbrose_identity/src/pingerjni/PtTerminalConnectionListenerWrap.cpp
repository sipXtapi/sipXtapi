// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/PtTerminalConnectionListenerWrap.cpp#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include "jni.h"
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "pingerjni/PtTerminalConnectionListenerWrap.h"
#include "pingerjni/CallListenerHelpers.h"
#include "ptapi/PtTerminalConnectionEvent.h"
#include "ptapi/PtTerminalConnectionEvent.h"
#include "ptapi/PtTerminalConnection.h"
#include "ptapi/PtCall.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtTerminalConnectionListenerWrap::PtTerminalConnectionListenerWrap(PtEventMask*  pMask, 
																	                long          lHashID,
																	                jobject		   jobjListener,
                                                                   unsigned long ulExcludeMask)
: PtTerminalConnectionListener(pMask)
{
   mlHashID = lHashID ;
   mjobjListener = jobjListener ;
   mulExcludeMask = ulExcludeMask ;
}


// Copy constructor
PtTerminalConnectionListenerWrap::PtTerminalConnectionListenerWrap(const PtTerminalConnectionListenerWrap& callListenerWrap)
{

}


// Destructor
PtTerminalConnectionListenerWrap::~PtTerminalConnectionListenerWrap()
{


}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtTerminalConnectionListenerWrap& 
PtTerminalConnectionListenerWrap::operator=(const PtTerminalConnectionListenerWrap& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/*
 * Terminal Connection Stuff
 */

void PtTerminalConnectionListenerWrap::terminalConnectionCreated(const PtTerminalConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_TERM_CONN_CREATED))	   
	   fireTerminalConnectionEvent(mjobjListener, "terminalConnectionCreated", rEvent) ;
}


void PtTerminalConnectionListenerWrap::terminalConnectionIdle(const PtTerminalConnectionEvent& rEvent)
{
	if (!(mulExcludeMask & EMASK_TERM_CONN_IDLE))	   
      fireTerminalConnectionEvent(mjobjListener, "terminalConnectionIdle", rEvent) ;
}


void PtTerminalConnectionListenerWrap::terminalConnectionRinging(const PtTerminalConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_TERM_CONN_RINGING))
	   fireTerminalConnectionEvent(mjobjListener, "terminalConnectionRinging", rEvent) ;
}


void PtTerminalConnectionListenerWrap::terminalConnectionDropped(const PtTerminalConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_TERM_CONN_DROPPED))
	   fireTerminalConnectionEvent(mjobjListener, "terminalConnectionDropped", rEvent) ;
}


void PtTerminalConnectionListenerWrap::terminalConnectionUnknown(const PtTerminalConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_TERM_CONN_UNKNOWN))
	   fireTerminalConnectionEvent(mjobjListener, "terminalConnectionUnknown", rEvent) ;
}


void PtTerminalConnectionListenerWrap::terminalConnectionHeld(const PtTerminalConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_TERM_CONN_HELD))
	   fireTerminalConnectionEvent(mjobjListener, "terminalConnectionHeld", rEvent) ;
}


void PtTerminalConnectionListenerWrap::terminalConnectionTalking(const PtTerminalConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_TERM_CONN_TALKING))
	   fireTerminalConnectionEvent(mjobjListener, "terminalConnectionTalking", rEvent) ;
}


void PtTerminalConnectionListenerWrap::terminalConnectionInUse(const PtTerminalConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_TERM_CONN_IN_USE))
	   fireTerminalConnectionEvent(mjobjListener, "terminalConnectionInUse", rEvent) ;
}


/*
 * Connection Stuff
 */

void PtTerminalConnectionListenerWrap::connectionCreated(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_CREATED))
	   fireConnectionEvent(mjobjListener, "connectionCreated", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionAlerting(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_ALERTING))
	   fireConnectionEvent(mjobjListener, "connectionAlerting", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionDisconnected(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_DISCONNECTED))
	   fireConnectionEvent(mjobjListener, "connectionDisconnected", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionFailed(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_FAILED))
	   fireConnectionEvent(mjobjListener, "connectionFailed", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionUnknown(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_UNKNOWN))
	   fireConnectionEvent(mjobjListener, "connectionUnknown", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionDialing(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_DIALING))
	   fireConnectionEvent(mjobjListener, "connectionDialing", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionEstablished(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_ESTABLISHED))
	   fireConnectionEvent(mjobjListener, "connectionConnected", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionInitiated(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_INITIATED))
      fireConnectionEvent(mjobjListener, "connectionInitiated", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionNetworkAlerting(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_NETWORK_ALERTING))
	   fireConnectionEvent(mjobjListener, "connectionNetworkAlerting", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionNetworkReached(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_NETWORK_REACHED))
	   fireConnectionEvent(mjobjListener, "connectionNetworkReached", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionOffered(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_OFFERED))
	   fireConnectionEvent(mjobjListener, "connectionOffered", rEvent) ;
}


void PtTerminalConnectionListenerWrap::connectionQueued(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_QUEUED))
	   fireConnectionEvent(mjobjListener, "connectionQueued", rEvent) ;
}


/*
 * Call Stuff
 */


void PtTerminalConnectionListenerWrap::callEventTransmissionEnded(const PtCallEvent& rEvent)
{
	// fireCallEvent(mjobjListener, "callEventTransmissionEnded", rEvent) ;
}


void PtTerminalConnectionListenerWrap::callActive(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_ACTIVE))
	   fireCallEvent(mjobjListener, "callActive", rEvent) ;
}


void PtTerminalConnectionListenerWrap::callInvalid(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_INVALID))
	   fireCallEvent(mjobjListener, "callInvalid", rEvent) ;
}


void PtTerminalConnectionListenerWrap::callMetaProgressStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaProgressStarted", rEvent) ;
}


void PtTerminalConnectionListenerWrap::callMetaProgressEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaProgressEnded", rEvent) ;
}


void PtTerminalConnectionListenerWrap::callMetaSnapshotStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaSnapshotStarted", rEvent) ;
}


void PtTerminalConnectionListenerWrap::callMetaSnapshotEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaSnapshotEnded", rEvent) ;
}


void PtTerminalConnectionListenerWrap::callMetaAddPartyStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaAddPartyStarted", rEvent) ;
}


void PtTerminalConnectionListenerWrap::callMetaAddPartyEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaAddPartyEnded", rEvent) ;
}


void PtTerminalConnectionListenerWrap::callMetaRemovePartyStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaRemovePartyStarted", rEvent) ;
}


void PtTerminalConnectionListenerWrap::callMetaRemovePartyEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaRemovePartyEnded", rEvent) ;
}


void PtTerminalConnectionListenerWrap::multicallMetaMergeStarted(const PtMultiCallMetaEvent& rEvent)
{   
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireMultiCallMetaEvent(mjobjListener, "multiCallMetaMergeStarted", rEvent) ;
}


void PtTerminalConnectionListenerWrap::multicallMetaMergeEnded(const PtMultiCallMetaEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireMultiCallMetaEvent(mjobjListener, "multiCallMetaMergeEnded", rEvent) ;
}


void PtTerminalConnectionListenerWrap::multicallMetaTransferStarted(const PtMultiCallMetaEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireMultiCallMetaEvent(mjobjListener, "multiCallMetaTransferStarted", rEvent) ;
}


void PtTerminalConnectionListenerWrap::multicallMetaTransferEnded(const PtMultiCallMetaEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireMultiCallMetaEvent(mjobjListener, "multiCallMetaTransferEnded", rEvent) ;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtTerminalConnectionListenerWrap, PtTerminalConnectionListener)

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

