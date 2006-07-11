// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/UtlString.h>
#include <os/OsSysLog.h>
#include <net/SipPublishServerEventStateCompositor.h>
#include <net/SipMessage.h>
#include <net/Url.h>


// Constructor
SipPublishServerEventStateCompositor::SipPublishServerEventStateCompositor()
{
}


// Copy constructor NOT IMPLEMENTED
SipPublishServerEventStateCompositor::SipPublishServerEventStateCompositor(const SipPublishServerEventStateCompositor& rSipPublishServerEventStateCompositor)
{
}


// Destructor
SipPublishServerEventStateCompositor::~SipPublishServerEventStateCompositor()
{
    // Iterate through and delete all the dialogs
    // TODO:
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipPublishServerEventStateCompositor& 
SipPublishServerEventStateCompositor::operator=(const SipPublishServerEventStateCompositor& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean SipPublishServerEventStateCompositor::getKeys(const SipMessage& publishRequest,
                                                         UtlString& resourceId,
                                                         UtlString& eventTypeKey)
{
    // default resourceId is the identity
    UtlString uriString;
    publishRequest.getRequestUri(&uriString);
    Url uri(uriString);
    uri.getIdentity(resourceId);

    // Default event key is the event type with no parameters
    publishRequest.getEventField(&eventTypeKey, NULL);

    return(TRUE);
}

UtlBoolean SipPublishServerEventStateCompositor::isAuthenticated(const SipMessage& publishRequest,
                                                                 const UtlString& resourceId,
                                                                 const UtlString& eventTypeKey,
                                                                 SipMessage& subscribeResponse)
{
    // By default no authentication required
    return(TRUE);
}

UtlBoolean SipPublishServerEventStateCompositor::isAuthorized(const SipMessage& publishRequest,
                                                              const UtlString& resourceId,
                                                              const UtlString& eventTypeKey,
                                                              SipMessage& subscribeResponse)
{
    // By default no authorization required
    return(TRUE);
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

