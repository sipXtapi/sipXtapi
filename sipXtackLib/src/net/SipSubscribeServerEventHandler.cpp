//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Dan Petrie (dpetrie AT SIPez DOT com)

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/UtlString.h>
#include <os/OsSysLog.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipMessage.h>
#include <net/Url.h>


// Private class to contain callback for eventTypeKey
class SubscribeServerSubscriptionState : public UtlString
{
public:
    SubscribeServerSubscriptionState();

    virtual ~SubscribeServerSubscriptionState();

    // Parent UtlString contains the dialog handle

private:
    //! DISALLOWED accidental copying
    SubscribeServerSubscriptionState(const SubscribeServerSubscriptionState& rSubscribeServerSubscriptionState);
    SubscribeServerSubscriptionState& operator=(const SubscribeServerSubscriptionState& rhs);
};



// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
SubscribeServerSubscriptionState::SubscribeServerSubscriptionState()
{
}
SubscribeServerSubscriptionState::~SubscribeServerSubscriptionState()
{
}

// Constructor
SipSubscribeServerEventHandler::SipSubscribeServerEventHandler()
{
}


// Copy constructor NOT IMPLEMENTED
SipSubscribeServerEventHandler::SipSubscribeServerEventHandler(const SipSubscribeServerEventHandler& rSipSubscribeServerEventHandler)
{
}


// Destructor
SipSubscribeServerEventHandler::~SipSubscribeServerEventHandler()
{
    // Iterate through and delete all the dialogs
    // TODO:
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipSubscribeServerEventHandler& 
SipSubscribeServerEventHandler::operator=(const SipSubscribeServerEventHandler& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean SipSubscribeServerEventHandler::getKeys(const SipMessage& subscribeRequest,
                                                   UtlString& resourceId,
                                                   UtlString& eventTypeKey)
{
    // default resourceId is the identity
    UtlString uriString;
    subscribeRequest.getRequestUri(&uriString);
    Url uri(uriString);
    uri.getIdentity(resourceId);

    // Default event key is the event type with no parameters
    subscribeRequest.getEventField(&eventTypeKey, NULL);

    return(TRUE);
}

UtlBoolean SipSubscribeServerEventHandler::isAuthenticated(const SipMessage& subscribeRequest,
                                                           const UtlString& resourceId,
                                                           const UtlString& eventTypeKey,
                                                           SipMessage& subscribeResponse)
{
    // By default no authentication required
    return(TRUE);
}

UtlBoolean SipSubscribeServerEventHandler::isAuthorized(const SipMessage& subscribeRequest,
                                                       const UtlString& resourceId,
                                                       const UtlString& eventTypeKey,
                                                       SipMessage& subscribeResponse)
{
    // By default no authorization required
    return(TRUE);
}

UtlBoolean SipSubscribeServerEventHandler::getNotifyContent(const UtlString& resourceId,
                                                            const UtlString& eventTypeKey,
                                                            SipPublishContentMgr& contentMgr,
                                                            const char* acceptHeaderValue,
                                                            SipMessage& notifyRequest)
{
    UtlBoolean gotBody = FALSE;
    // Default behavior is to just go get the content from
    // the content manager and attach it to the notify
    HttpBody* messageBody = NULL;
    UtlBoolean isDefaultEventContent;
    gotBody = contentMgr.getContent(resourceId,
                          eventTypeKey,
                          acceptHeaderValue,
                          messageBody,
                          isDefaultEventContent);

    // The body will be freed with the notify message.
    if(messageBody)
    {
        const char* contentTypePtr = messageBody->getContentType();
        UtlString contentType;
        if(contentTypePtr)
        {
            contentType = contentTypePtr;
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipSubscribeServerEventHandler::getNotifyContent body published for resourceId: %s eventTypeKey: %s with no content type",
                resourceId.data() ? resourceId.data() : "<null>", 
                eventTypeKey.data() ? eventTypeKey.data() : "<null>");

            contentType = "text/unknown";
        }
          
        notifyRequest.setContentType(contentType);
        notifyRequest.setBody(messageBody);
        
        UtlString body;
        int bodyLength;
        notifyRequest.getBytes(&body, &bodyLength);   
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipSubscribeServerEventHandler::getNotifyContent resourceId <%s>, eventTypeKey <%s> contentType <%s>\nNotify message length = %d, messageBody =\n%s\n",
                      resourceId.data(), eventTypeKey.data(), contentType.data(), bodyLength, body.data());
    }

    return(gotBody);
}

/* ============================ ACCESSORS ================================= */



/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

