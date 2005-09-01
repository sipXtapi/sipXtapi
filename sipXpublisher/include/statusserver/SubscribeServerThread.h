// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////// SubscribeServerThread.h: interface for the SubscribeServerThread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SUBSCRIBESERVERTHREAD_H
#define SUBSCRIBESERVERTHREAD_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "net/SipNonceDb.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;    
class Notifier;
class PluginXmlParser;
class SubscribeServerPluginBase;

class SubscribeServerThread : public OsServerTask  
{
public:
    UtlBoolean initialize (
        SipUserAgent* userAgent,
        int defaultSubscribePeriod,
        const UtlString& minExpiresTime,
        const UtlString& defaultDomain,
        const UtlBoolean& useCredentialDB,
        const UtlString& realm,
        PluginXmlParser* pluginTable);

    SubscribeServerThread();

    virtual ~SubscribeServerThread();

    //utility functions
    typedef enum subcribeStatus
    {
        STATUS_SUCCESS = 0,
        STATUS_LESS_THAN_MINEXPIRES ,
        STATUS_INVALID_REQUEST,	
        STATUS_FORBIDDEN,	
        STATUS_NOT_FOUND,
        STATUS_QUERY
    } SubscribeStatus;

protected:
    SipUserAgent* mpSipUserAgent;
    UtlBoolean mIsCredentialDB;
    UtlBoolean mIsStarted;
    UtlString mMinExpiresTimeStr;
    UtlString mRealm;
    int mDefaultSubscribePeriod;
    int mMinExpiresTimeint;
    int mDefaultDomainPort;
    UtlString mDefaultDomain;
    UtlString mDefaultDomainHostFQDN;
    UtlString mDefaultDomainHostIP;
    SipNonceDb mNonceDb;

    PluginXmlParser* mPluginTable;

    //int addUserToDirectory(const int timeNow, const SipMessage* registerMessage);
    UtlBoolean handleMessage( OsMsg& eventMessage );

    UtlBoolean isAuthenticated( 
        const SipMessage* message, 
        SipMessage *responseMessage, 
        UtlString& authenticatedUser, 
        UtlString& authenticatedRealm  );

    UtlBoolean isAuthorized(
        const SipMessage* message, 
        SipMessage *responseMessage, 
        StatusPluginReference* plugin = NULL );

    UtlBoolean isValidDomain(
        const SipMessage* message, 
        SipMessage * responseMessage );

    // Process the message as a prospective database change.
    SubscribeStatus addSubscription (const int timeNow,
                                     const SipMessage* registerMessage, ///< request message
                                     const char* domain,
                                     const UtlString& eventType, ///< package name
                                     const UtlString& eventId,   ///< event header id parameter (may be null)
                                     SipMessage& response        ///< to be returned
                                     );

    /**
     * 
     * @param sipMessage
     * 
     * @return
     */
    int removeErrorSubscription(const SipMessage& sipMessage
                                ) const;

};
#endif // SUBSCRIBESERVERTHREAD_H
