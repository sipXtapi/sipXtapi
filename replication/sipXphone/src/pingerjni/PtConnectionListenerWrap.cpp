// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/PtConnectionListenerWrap.cpp#3 $
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
#include "pingerjni/PtConnectionListenerWrap.h"
#include "pingerjni/CallListenerHelpers.h"
#include "ptapi/PtConnectionEvent.h"
#include "ptapi/PtConnection.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtEvent.h"

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtConnectionListenerWrap::PtConnectionListenerWrap(	PtEventMask* pMask, 
													               long		    lHashID,
                                                      jobject		 jobjListener,
                                                      unsigned long ulExcludeMask)
	: PtConnectionListener(pMask)
{
   mlHashID = lHashID ;
   mjobjListener = jobjListener ;   
   mulExcludeMask = ulExcludeMask ;
}


// Copy constructor
PtConnectionListenerWrap::PtConnectionListenerWrap(const PtConnectionListenerWrap& callListenerWrap)
{

}


// Destructor
PtConnectionListenerWrap::~PtConnectionListenerWrap()
{


}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtConnectionListenerWrap& 
PtConnectionListenerWrap::operator=(const PtConnectionListenerWrap& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


void PtConnectionListenerWrap::connectionCreated(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_CREATED))
	   fireConnectionEvent(mjobjListener, "connectionCreated", rEvent) ;
}


void PtConnectionListenerWrap::connectionAlerting(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_ALERTING))
	   fireConnectionEvent(mjobjListener, "connectionAlerting", rEvent) ;
}


void PtConnectionListenerWrap::connectionDisconnected(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_DISCONNECTED))
	   fireConnectionEvent(mjobjListener, "connectionDisconnected", rEvent) ;
}


void PtConnectionListenerWrap::connectionFailed(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_FAILED))
	   fireConnectionEvent(mjobjListener, "connectionFailed", rEvent) ;
}


void PtConnectionListenerWrap::connectionUnknown(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_UNKNOWN))
	   fireConnectionEvent(mjobjListener, "connectionUnknown", rEvent) ;
}


void PtConnectionListenerWrap::connectionDialing(const PtConnectionEvent& rEvent)
{
	// fireConnectionEvent(mjobjListener, "connectionDialing", rEvent) ;
}


void PtConnectionListenerWrap::connectionEstablished(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_ESTABLISHED))
	   fireConnectionEvent(mjobjListener, "connectionConnected", rEvent) ;
}


void PtConnectionListenerWrap::connectionInitiated(const PtConnectionEvent& rEvent)
{   
   if (!(mulExcludeMask & EMASK_CONN_INITIATED))
	   fireConnectionEvent(mjobjListener, "connectionInitiated", rEvent) ;
}


void PtConnectionListenerWrap::connectionNetworkAlerting(const PtConnectionEvent& rEvent)
{   
   if (!(mulExcludeMask & EMASK_CONN_NETWORK_ALERTING))
	   fireConnectionEvent(mjobjListener, "connectionNetworkAlerting", rEvent) ;
}


void PtConnectionListenerWrap::connectionNetworkReached(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_NETWORK_REACHED))
	   fireConnectionEvent(mjobjListener, "connectionNetworkReached", rEvent) ;
}


void PtConnectionListenerWrap::connectionOffered(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_OFFERED))
	   fireConnectionEvent(mjobjListener, "connectionOffered", rEvent) ;
}


void PtConnectionListenerWrap::connectionQueued(const PtConnectionEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CONN_QUEUED))
	   fireConnectionEvent(mjobjListener, "connectionQueued", rEvent) ;
}


void PtConnectionListenerWrap::callEventTransmissionEnded(const PtCallEvent& rEvent)
{
	// fireCallEvent(mjobjListener, "callEventTransmissionEnded", rEvent) ;
}


void PtConnectionListenerWrap::callActive(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_ACTIVE))
	   fireCallEvent(mjobjListener, "callActive", rEvent) ;
}


void PtConnectionListenerWrap::callInvalid(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_INVALID))
	   fireCallEvent(mjobjListener, "callInvalid", rEvent) ;
}


void PtConnectionListenerWrap::callMetaProgressStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaProgressStarted", rEvent) ;
}


void PtConnectionListenerWrap::callMetaProgressEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaProgressEnded", rEvent) ;
}


void PtConnectionListenerWrap::callMetaSnapshotStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaSnapshotStarted", rEvent) ;
}


void PtConnectionListenerWrap::callMetaSnapshotEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaSnapshotEnded", rEvent) ;
}


void PtConnectionListenerWrap::callMetaAddPartyStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaAddPartyStarted", rEvent) ;
}


void PtConnectionListenerWrap::callMetaAddPartyEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaAddPartyEnded", rEvent) ;
}


void PtConnectionListenerWrap::callMetaRemovePartyStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaRemovePartyStarted", rEvent) ;
}


void PtConnectionListenerWrap::callMetaRemovePartyEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaRemovePartyEnded", rEvent) ;
}


void PtConnectionListenerWrap::multicallMetaMergeStarted(const PtMultiCallMetaEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireMultiCallMetaEvent(mjobjListener, "multiCallMetaMergeStarted", rEvent) ;
}


void PtConnectionListenerWrap::multicallMetaMergeEnded(const PtMultiCallMetaEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireMultiCallMetaEvent(mjobjListener, "multiCallMetaMergeEnded", rEvent) ;
}


void PtConnectionListenerWrap::multicallMetaTransferStarted(const PtMultiCallMetaEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireMultiCallMetaEvent(mjobjListener, "multiCallMetaTransferStarted", rEvent) ;
}


void PtConnectionListenerWrap::multicallMetaTransferEnded(const PtMultiCallMetaEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireMultiCallMetaEvent(mjobjListener, "multiCallMetaTransferEnded", rEvent) ;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtConnectionListenerWrap, PtConnectionListener)

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

