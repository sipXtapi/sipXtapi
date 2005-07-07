// $Id: //depot/OPENDEV/sipXphone/src/web/DialerCallListener.cpp#3 $
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

// APPLICATION INCLUDESb
#include "web/DialerCallListener.h"
#include "os/OsSocket.h"
#include "ptapi/PtEvent.h"
#include "ptapi/PtCallEvent.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtConnection.h"
#include "ptapi/PtConnectionEvent.h"
#include "ptapi/PtSessionDesc.h"
#include "cp/CpCallManager.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
DialerCallListener::DialerCallListener(PtEventMask* mask) :
PtConnectionListener(mask)
{
		osPrintf("DialerCallListener constructor called\n") ;
}

// Copy constructor
DialerCallListener::DialerCallListener(const DialerCallListener& rDialerCallListener)
{

	osPrintf("DialerCallListener constructor called\n") ;
}

// Destructor
DialerCallListener::~DialerCallListener()
{
	osPrintf("DialerCallListener destructor called\n") ;
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
DialerCallListener& 
DialerCallListener::operator=(const DialerCallListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void DialerCallListener::callEventTransmissionEnded(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callEventTransmissionEnded\n") ;
}

void DialerCallListener::callActive(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callActive\n") ;
}

void DialerCallListener::callInvalid(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callInvalid\n") ;
}

void DialerCallListener::callMetaProgressStarted(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callMetaProgressStarted\n") ;
}

void DialerCallListener::callMetaProgressEnded(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callMetaProgressEnded\n") ;
}

void DialerCallListener::callMetaSnapshotStarted(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callMetaSnapshotStarted\n") ;
}

void DialerCallListener::callMetaSnapshotEnded(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callMetaSnapshotEnded\n") ;
}

void DialerCallListener::callMetaAddPartyStarted(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callMetaAddPartyStarted\n") ;
}

void DialerCallListener::callMetaAddPartyEnded(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callMetaAddPartyEnded\n") ;
}

void DialerCallListener::callMetaRemovePartyStarted(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callMetaRemovePartyStarted\n") ;
}

void DialerCallListener::callMetaRemovePartyEnded(const PtCallEvent& rEvent)
{
	osPrintf("DialerCallListner::callMetaRemovePartyEnded\n") ;
}

void DialerCallListener::multicallMetaMergeStarted(const PtMultiCallMetaEvent& rEvent)
{
	osPrintf("DialerCallListner::multicallMetaMergeStarted\n") ;
}

void DialerCallListener::multicallMetaMergeEnded(const PtMultiCallMetaEvent& rEvent)
{
	osPrintf("DialerCallListner::multicallMetaMergeEnded\n") ;
}

void DialerCallListener::multicallMetaTransferStarted(const PtMultiCallMetaEvent& rEvent)
{
	osPrintf("DialerCallListner::multicallMetaTransferStarted\n") ;
}

void DialerCallListener::multicallMetaTransferEnded(const PtMultiCallMetaEvent& rEvent)
{
	osPrintf("DialerCallListner::multicallMetaTransferEnded\n") ;
}

// PtConnectionListener methods

void DialerCallListener::connectionCreated(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionCreated\n") ;
}

void DialerCallListener::connectionAlerting(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionAlerting\n") ;

}


void DialerCallListener::connectionDisconnected(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionDisconnected\n") ;

}

void DialerCallListener::connectionFailed(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionDisconnected\n") ;

	PtConnectionEvent ev = (PtConnectionEvent) rEvent;
	PtStatus status ;
	PtEvent::PtEventCause cause ;

	
	// If connection failed because callee is busy, need to return that info
	status = ev.getCause(cause) ;

	osPrintf("DialerCallListener::connectionFailed status = %d\n", cause) ;	
	switch (cause) {
	case (PtEvent::CAUSE_BUSY) :
		break ;
	default :
		break ;
	}


}

void DialerCallListener::connectionUnknown(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionUnknown\n ") ;

}

void DialerCallListener::connectionDialing(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionDialing\n ") ;
}


void DialerCallListener::connectionEstablished(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionEstablished\n ") ;
}


void DialerCallListener::connectionNetworkAlerting(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionNetworkAlerting\n") ;
}

void DialerCallListener::connectionNetworkReached(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionNetworkReached\n") ;
}

void DialerCallListener::connectionOffered(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionOffered\n") ;
}

void DialerCallListener::connectionQueued(const PtConnectionEvent& rEvent)
{
	osPrintf("DialerCallListner::connectionQueued\n") ;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

