//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsQueuedEvent.h>
#include <os/OsEventMsg.h>
#include <os/OsSysLog.h>
#include <Conference.h>
#include <Leg.h>
#include <cp/CpMediaInterface.h>
#include <cp/CpMultiStringMessage.h>
#include <cp/CpIntMessage.h>
#include <ptapi/PtConnection.h>
#include <ptapi/PtCall.h>
#include <ptapi/PtTerminalConnection.h>
#include <tao/TaoProviderAdaptor.h>
#include <tao/TaoListenerEventMessage.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType Conference::TYPE = "Conference" ;

// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Conference::Conference(ConferenceController* parent,
                       UtlString& user_name) :
   mUserName(user_name),
   mConferenceController(parent),
   mSipxLine(SIPX_LINE_NULL),
   mSipxConference(SIPX_CONF_NULL)
{
   // Add a "line" to the user agent.
   char identity[100];
   sprintf(identity, "\"Conference %s\" <sip:%s@%s>",
           mUserName.data(), mUserName.data(),
           mConferenceController->mDomainHostPort.data());
   SIPX_RESULT result =
      sipxLineAdd(mConferenceController->mSipxInstance, identity, &mSipxLine);
   if (result != SIPX_RESULT_SUCCESS)
   {
      OsSysLog::add(FAC_CONFERENCE, PRI_ERR,
                    "Conference::_ sipxLineAdd failed, result = %d",
                    result);
      return;
   }

#if 0
   sipxLineAddCredential(g_hLine, "myidentity", "mypassword", "example.com");
#endif


   // Create a sipXtapi "conference" to do the audio mixing.

   result = sipxConferenceCreate(mConferenceController->mSipxInstance,
                                 &mSipxConference);
   if (result != SIPX_RESULT_SUCCESS)
   {
      OsSysLog::add(FAC_CONFERENCE, PRI_ERR,
                    "Conference::_ sipxConferenceCreate failed, result = %d",
                    result);
      return;
   }
}

// Destructor
Conference::~Conference()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/**
 * Get the ContainableType for a UtlContainable derived class.
 */
UtlContainableType Conference::getContainableType() const
{
   return Conference::TYPE;
}

/**
 * Calculate a unique hash code for this object.  If the equals
 * operator returns true between two objects, then both objects
 * must have the same hash code.
 */
unsigned Conference::hash() const
{
   // default implementation
   return (unsigned) this;
}

/**
 * Compare the this object to another like object.  Results for 
 * comparing with a non-like object are undefined.
 *
 * @returns 0 if equal, <0 if less than and >0 if greater.
 */
int Conference::compareTo(const UtlContainable* otherObject) const
{
   return ((unsigned) this) - ((unsigned) otherObject);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

Leg* Conference::handleNewCall(SIPX_CALL hCall,
                               SIPX_CALLSTATE_MINOR eMinor)
{
   // Get the Call-Id.
   char cCallId[512];
   sipxCallGetID(hCall, cCallId, sizeof(cCallId)); 	
   OsSysLog::add(FAC_CONFERENCE, PRI_DEBUG,
                 "Conference::handleNewCall call Id: %s",
                 cCallId);
   
   // Construct the leg object.
   Leg* leg = new Leg(this, hCall, cCallId);
   // Add it to the list of legs of this conference.
   mLegs.insert(leg);

   // Return it to our caller (in ConferenceController) so it can add it to
   // the lookup table of legs.
   return leg;
}
