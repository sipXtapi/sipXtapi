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

#ifndef CREDENTIALDB_H
#define CREDENTIALDB_H

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Url;
class dbDatabase;
class dbFieldDescriptor;
class UtlHashMap;
class TiXmlNode;
class ResultSet;

class CredentialDB
{
public:
    // Singleton Accessor with implicit db load
    static CredentialDB* getInstance( const UtlString& name = "credential" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    OsStatus store();

    UtlBoolean insertRow (
        const Url& uri,
        const UtlString& realm,
        const UtlString& userid,
        const UtlString& passToken,
        const UtlString& authType = "DIGEST" );

    void removeRows (
        const Url& uri,
        const UtlString& realm );

    void removeRows ( const Url& uri );

    void removeAllRows ();

    // utility method for dumping all rows
    void getAllRows ( ResultSet& rResultset ) const;

    UtlBoolean getCredentialByUserid (
        const Url& uri,
        const UtlString& realm,
        const UtlString& userid,
        UtlString& passtoken,
        UtlString& authType ) const;

    UtlBoolean getCredential (
        const Url& uri,
        const UtlString& realm,
        UtlString& userid,
        UtlString& passtoken,
        UtlString& authType ) const;

    UtlBoolean getCredential (
        const UtlString& userid,
        const UtlString& realm,
        Url& uri,
        UtlString& passtoken,
        UtlString& authType ) const;

    void getAllCredentials (
        const Url& uri,
        ResultSet& cursor ) const;

    UtlBoolean isUriDefined (
        const Url& uri,
        UtlString& realm,
        UtlString& authType ) const;

protected:

    // this is implicit
    OsStatus load();

    // Singleton Constructor is private
    CredentialDB( const UtlString& name = "credentials" );

    // One step closer to common load/store code
    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // There is only one singleton in this design
    static CredentialDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gUriKey;
    static UtlString gRealmKey;
    static UtlString gUseridKey;
    static UtlString gPasstokenKey;
    static UtlString gAuthtypeKey;

    // Fast DB instance
    dbDatabase* m_pFastDB;

    // the persistent filename for loading/saving
    UtlString mDatabaseName;

private:

    virtual ~CredentialDB();
};

#endif //CREDENTIALSDB_H

