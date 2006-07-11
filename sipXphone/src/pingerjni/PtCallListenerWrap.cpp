// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/PtCallListenerWrap.cpp#3 $
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
#include "pingerjni/PtCallListenerWrap.h"
#include "pingerjni/CallListenerHelpers.h"
#include "ptapi/PtCallListener.h"
#include "ptapi/PtEventMask.h"
#include "ptapi/PtCallEvent.h"
#include "ptapi/PtCall.h"

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtCallListenerWrap::PtCallListenerWrap(PtEventMask* pMask, 
										         long		    lHashID,
										         jobject		 jobjListener,
                                       unsigned long ulExcludeMask)
	: PtCallListener(pMask)
{
   mlHashID = lHashID ;
   mjobjListener = jobjListener ;  
   mulExcludeMask = ulExcludeMask ;
}


// Copy constructor
PtCallListenerWrap::PtCallListenerWrap(const PtCallListenerWrap& callListenerWrap)
{

}


// Destructor
PtCallListenerWrap::~PtCallListenerWrap()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtCallListenerWrap& 
PtCallListenerWrap::operator=(const PtCallListenerWrap& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


void PtCallListenerWrap::callEventTransmissionEnded(const PtCallEvent& rEvent)
{   
   // fireCallEvent(mjobjListener, "callEventTransmissionEnded", rEvent) ;
}


void PtCallListenerWrap::callActive(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_ACTIVE))
	   fireCallEvent(mjobjListener, "callActive", rEvent) ;
}


void PtCallListenerWrap::callInvalid(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_INVALID))
	   fireCallEvent(mjobjListener, "callInvalid", rEvent) ;
}


void PtCallListenerWrap::callMetaProgressStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaProgressStarted", rEvent) ;
}


void PtCallListenerWrap::callMetaProgressEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaProgressEnded", rEvent) ;
}


void PtCallListenerWrap::callMetaSnapshotStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaSnapshotStarted", rEvent) ;
}


void PtCallListenerWrap::callMetaSnapshotEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaSnapshotEnded", rEvent) ;
}


void PtCallListenerWrap::callMetaAddPartyStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaAddPartyStarted", rEvent) ;
}


void PtCallListenerWrap::callMetaAddPartyEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaAddPartyEnded", rEvent) ;
}


void PtCallListenerWrap::callMetaRemovePartyStarted(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaRemovePartyStarted", rEvent) ;
}


void PtCallListenerWrap::callMetaRemovePartyEnded(const PtCallEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
	   fireCallEvent(mjobjListener, "callMetaRemovePartyEnded", rEvent) ;
}


void PtCallListenerWrap::multicallMetaMergeStarted(const PtMultiCallMetaEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
      fireMultiCallMetaEvent(mjobjListener, "multiCallMetaMergeStarted", rEvent) ;
}


void PtCallListenerWrap::multicallMetaMergeEnded(const PtMultiCallMetaEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
      fireMultiCallMetaEvent(mjobjListener, "multiCallMetaMergeEnded", rEvent) ;
}


void PtCallListenerWrap::multicallMetaTransferStarted(const PtMultiCallMetaEvent& rEvent)
{    
   if (!(mulExcludeMask & EMASK_CALL_META))
      fireMultiCallMetaEvent(mjobjListener, "multiCallMetaTransferStarted", rEvent) ;
}


void PtCallListenerWrap::multicallMetaTransferEnded(const PtMultiCallMetaEvent& rEvent)
{
   if (!(mulExcludeMask & EMASK_CALL_META))
      fireMultiCallMetaEvent(mjobjListener, "multiCallMetaTransferEnded", rEvent) ;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtCallListenerWrap, PtCallListener)

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

