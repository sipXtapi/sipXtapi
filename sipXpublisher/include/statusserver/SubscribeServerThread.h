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
// 
// 
// Copyright (c) Pingtel Corp. (work in progress)
//
// This is an unpublished work containing Pingtel Corporation's confidential
// and proprietary information.  Disclosure, use or reproduction without
// written authorization of Pingtel Corp. is prohibited.
//! author="Surbhi Dua"
//! lib=net
//////////////////////////////////////////////////////////////////////////////

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
    enum subcribeStatus
    {
        STATUS_SUCCESS = 0,
        STATUS_LESS_THAN_MINEXPIRES ,
        STATUS_INVALID_REQUEST,	
        STATUS_FORBIDDEN,	
        STATUS_NOT_FOUND,
        STATUS_QUERY
    };

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

    /**
     * 
     * @param timeNow
     * @param registerMessage
     * @param domain   The domain should not need to
     *                 be sent int.  This is a temporary bogus work around
     *                 for stuff that should be done in the SubscribeServer NOT
     *                 in the plugin.
     * @param response
     * 
     * @return
     */
    int addSubscription (
        const int timeNow,
        const SipMessage* registerMessage,
        const char* domain,
        SipMessage& response);

    /**
     * 
     * @param sipMessage
     * 
     * @return
     */
    int removeSubscription ( const SipMessage& sipMessage ) const;

};
#endif // SUBSCRIBESERVERTHREAD_H
