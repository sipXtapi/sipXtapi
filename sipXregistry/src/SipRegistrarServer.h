// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREGISTRARSERVER_H
#define SIPREGISTRARSERVER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "net/SipNonceDb.h"
#include "utl/UtlHashMap.h"
#include "utl/PluginHooks.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;
class SipUserAgent;
class PluginHooks;

/**
 * The Registrar Server is responsible for registering and unregistering
 * phones.  The servertask also looks up contacts for invite URI's. These
 * contacts are taken from the Url Mapping rules and also from the
 * registration database.
 */
class SipRegistrarServer : public OsServerTask
{
public:
    /**
     * Initialized the Registration Server
     */
    UtlBoolean initialize(
        SipUserAgent* SipUserAgent,
        PluginHooks* sipRegisterPlugins,
        int defaultRegistryPeriod,
        const UtlString& minExpiresTime,
        const UtlString& defaultDomain,
        const UtlString& domainAliases,
        int              proxyNormalPort,
        const UtlBoolean& useCredentialDB,
        const UtlString& realm);

    /** ctor */
    SipRegistrarServer();

    /** dtor */
    virtual ~SipRegistrarServer();

    /**
     * Registration Status values
     */
    enum RegisterStatus
    {
        REGISTER_SUCCESS = 0,
        REGISTER_LESS_THAN_MINEXPIRES ,
        REGISTER_INVALID_REQUEST,
        REGISTER_FORBIDDEN,
        REGISTER_NOT_FOUND,
        REGISTER_OUT_OF_ORDER,
        REGISTER_QUERY
    };

protected:
    UtlBoolean mIsStarted;
    SipUserAgent* mSipUserAgent;
    int mDefaultRegistryPeriod;
    UtlString mMinExpiresTimeStr;
    int mMinExpiresTimeint;
    UtlBoolean mIsCredentialDB;
    UtlString mRealm;

    UtlString mDefaultDomain;
    int mDefaultDomainPort;
    UtlString mDefaultDomainHost;
    UtlHashMap mValidDomains;
    // A port number, which if found on an AOR to register,
    // will be removed, or PORT_NONE
    int mProxyNormalPort;
    
    SipNonceDb mNonceDb;
    long mNonceExpiration;

    PluginHooks* mpSipRegisterPlugins;

    // The last update number assigned to a registration.  Equals zero if no
    // local registrations have been processed yet.
    UtlLongLongInt mDbUpdateNumber;

    // Temporary hack: create a dummy local registrar name until we can get
    // this value from the configuration
    static const UtlString gDummyLocalRegistrarName;

    /**
     *
     * @param timeNow
     * @param registerMessage
     *
     * @return
     */
    RegisterStatus applyRegisterToDirectory(
        const Url& toUrl,
        const int timeNow,
        const SipMessage& registerMessage );

    /**
     *
     * @param eventMessage
     *
     * @return
     */
    UtlBoolean handleMessage( OsMsg& eventMessage );

    /**
     *
     * @param message
     * @param responseMessage
     *
     * @return
     */
    UtlBoolean isAuthorized(
        const Url& toUrl,
        const SipMessage& message,
        SipMessage& responseMessage );

    /**
     *
     * @param message
     * @param responseMessage
     *
     * @return
     */
    UtlBoolean isValidDomain(
        const SipMessage& message,
        SipMessage& responseMessage );

    /**
     * Configure a domain as valid for registration at this server.
     * @param host the host part of a valid registration url
     * @param port the port number portion of a valid registration url
     */
    void addValidDomain(const UtlString& host, int port = PORT_NONE);
};

#endif // SIPREGISTRARSERVER_H
