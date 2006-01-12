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
    /// Construct the thread to process REGISTER requests.
    SipRegistrarServer(OsConfigDb*   pOsConfigDb,  ///< Configuration parameters
                       SipUserAgent* pSipUserAgent ///< User Agent to use when sending responses
                       );

    /// Initialize the Registration Server
    void initialize(OsConfigDb& configDb);

    virtual ~SipRegistrarServer();

    /**
     * Registration Status values
     */
    enum RegisterStatus
    {
        REGISTER_SUCCESS = 0,           ///< contacts updated
        REGISTER_LESS_THAN_MINEXPIRES , ///< requested duration to short
        REGISTER_INVALID_REQUEST,       ///< some other error
        REGISTER_FORBIDDEN,             ///< authenticated id not valid for AOR
        REGISTER_NOT_FOUND,             ///< no contacts match AOR
        REGISTER_OUT_OF_ORDER,          ///< newer data already in registry database
        REGISTER_QUERY                  ///< request is a valid query for current contacts
    };

protected:
    UtlBoolean mIsStarted;
    SipUserAgent* mSipUserAgent;
    int mDefaultRegistryPeriod;
    UtlString mMinExpiresTimeStr;
    int mMinExpiresTimeint;
    bool mUseCredentialDB;
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

    /// Validate bindings, and if all are ok apply them to the registry db
    RegisterStatus applyRegisterToDirectory(
        const Url& toUrl,  ///< AOR from the message
        const int timeNow, ///< base time for all expiration calculations
        const SipMessage& registerMessage ///< message containing bindings
                                            );

    // Process a single REGISTER request
    UtlBoolean handleMessage( OsMsg& eventMessage );

    /// Check authentication for REGISTER request
    UtlBoolean isAuthorized(const Url& toUrl, ///< AOR from the message
                            const SipMessage& message, ///< REGISTER message
                            SipMessage& responseMessage /// response for challenge
                            );
    /**<
     * @return
     * - true if request is authenticated as user for To address
     * - false if not (responseMessage is then set up as a challenge)
     */

    // Is the target of this message this domain?
    UtlBoolean isValidDomain(const SipMessage& message,///< REGISTER message
                             SipMessage& responseMessage /// response if not valid
                             );
    /**<
     * @return
     * - true if request is targetted to this domain
     * - false if not (responseMessage is then set up as an error response)
     */

    /**
     * Configure a domain as valid for registration at this server.
     * @param host the host part of a valid registration url
     * @param port the port number portion of a valid registration url
     */
    void addValidDomain(const UtlString& host, int port = PORT_NONE);
};

#endif // SIPREGISTRARSERVER_H
