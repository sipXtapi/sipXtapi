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
#include "os/OsLock.h"
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
class SipRegistrar;
class SipUserAgent;
class PluginHooks;
class SyncRpc;

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
    SipRegistrarServer(SipRegistrar& registrar);
    /**<
     * Defer init to the initialize method to allow the SipRegistrarServer object
     * to be accessed before the associated thread has been started.
     */

    /// Initialize the Registration Server
    void initialize(OsConfigDb*   configDb,        ///< Configuration parameters
                    SipUserAgent* pSipUserAgent    ///< User Agent to use when sending responses
    );

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

    /// Retrieve all updates for registrarName whose update number is greater than updateNumber
    int pullUpdates(
       const UtlString& registrarName,
       intll            updateNumber,
       UtlSList&        updates);
    /**<
     * Retrieve all updates for registrarName whose update number is greater than updateNumber.
     * Return the updates in the updates list.  Each update is an object of type
     * RegistrationBinding.
     * The order of updates in the list is not specified.
     * Return the number of updates in the list.
     */

    /// Apply registry updates for a single registrar (local or peer) to the database
    intll applyUpdatesToDirectory(
       int timeNow,                         ///< current epoch time
       const UtlSList& updates,             ///< list of updates to apply
       UtlString*      errorMsg = NULL);    ///< fill in the error message on failure
    /**<
     * Return the maximum update number for that registrar after applying updates, or -1
     * if there is an error.  An empty updates list is an error.
     */

    /// Get the largest update number in the local database for this registrar as primary
    intll getDbUpdateNumber() const;

    /// Schedule garbage collection and persistence of the registration database
    void SipRegistrarServer::scheduleCleanAndPersist();

    /// Garbage-collect and persist the registration database
    void SipRegistrarServer::cleanAndPersist();
    /**<
     * Don't call cleanAndPersist directly.  Instead call scheduleCleanAndPersist so that
     * persistence is periodic instead of immediate, for efficiency.
     */

    /// Reset the DbUpdateNumber so that the upper half is the epoch time.
    void resetDbUpdateNumberEpoch();

    /// Recover the DbUpdateNumber from the local database
    void restoreDbUpdateNumber();

    /// Return the max update number for primaryRegistrar, or zero if there are no such updates
    intll getMaxUpdateNumberForRegistrar(const char* primaryName) const;

    /// Return true if there is a new update to send to the peer registrar and fill in bindings
    bool getNextUpdateToSend(RegistrarPeer *peer,       ///< peer to send the update to
                             UtlSList&   bindings);     ///< fill in bindings of the update
    
protected:
    SipRegistrar& mRegistrar;
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

    static OsMutex sLockMutex;

    /// Set the largest update number in the local database for this registrar as primary
    void setDbUpdateNumber(intll dbUpdateNumber);

    /// Validate bindings, and if all are OK then apply them to the registry db
    RegisterStatus applyRegisterToDirectory(
        const Url& toUrl,  ///< AOR from the message
        const int timeNow, ///< base time for all expiration calculations
        const SipMessage& registerMessage); ///< message containing bindings
    
    /// Update one binding for a peer registrar, or the local registrar (if peer is NULL)
    intll updateOneBinding(RegistrationBinding* update,
                           RegistrarPeer* peer,
                           RegistrationDB* imdb);
    /**<
     * 
     * Return the max updateNumber for the registrar that is primary for this binding
     * or -1 if there was an error.
     * Update state variables for the primary registrar.
     */

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

    /// If replication is configured, then name of this registrar as primary
    const UtlString& primaryName() const;
};

#endif // SIPREGISTRARSERVER_H
