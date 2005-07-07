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
#ifndef SUBSCRIPTIONDB_H
#define SUBSCRIPTIONDB_H

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
class dbDatabase;
class dbFieldDescriptor;
class UtlHashMap;
class TiXmlNode;
class ResultSet;

class SubscriptionDB
{
public:
    // Singleton Accessor
    static SubscriptionDB* getInstance(
        const UtlString& name = "subscription" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    //serialize methods
    OsStatus store();

    //set methods
    UtlBoolean insertRow (
        const UtlString& uri,
        const UtlString& callid,
        const UtlString& contact,
        const int& expires,
        const int& subscribeCseq,
        const UtlString& eventType,
        const UtlString& to,
        const UtlString& from,
        const UtlString& file,
        const UtlString& key,
        const UtlString& recordRoute,
        const int& notifyCseq);

    //delete methods - delete a subscription session
    void removeRow (
        const UtlString& to,
        const UtlString& from,
        const UtlString& callid,
        const UtlString& eventType,
        const int& subscribeCseq );

    void removeRows ( const UtlString& uri );

    void removeAllRows ();

    // utility method for dumping all rows
    void getAllRows ( ResultSet& rResultSet ) const;

    void getUnexpiredSubscriptions (
        const UtlString& key,
        const UtlString& eventType,
        const int& timeNow,
        ResultSet& rResultSet ) const;

    void updateUnexpiredSubscription (
        const UtlString& to,
        const UtlString& from,
        const UtlString& callid,
        const UtlString& eventType,
        const int& timeNow,
        const int& updatedNotifyCseq) const;

protected:
    // this is implicit now
    OsStatus load();

    // Singleton Constructor is private
    SubscriptionDB( const UtlString& name );

    // Added this to make load and store code identical
    // in all database implementations, One step closer
    // to a template version of the code
    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // There is only one singleton in this design
    static SubscriptionDB* spInstance;

    // Fast DB instance
    static dbDatabase* spDBInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gUriKey;
    static UtlString gCallidKey;
    static UtlString gContactKey;
    static UtlString gNotifycseqKey;
    static UtlString gSubscribecseqKey;
    static UtlString gExpiresKey;
    static UtlString gEventtypeKey;
    static UtlString gToKey;
    static UtlString gFromKey;
    static UtlString gFileKey;
    static UtlString gKeyKey;
    static UtlString gRecordrouteKey;

    // Fast DB instance
    dbDatabase* m_pFastDB;

    // the persistent filename for loading/saving
    UtlString mDatabaseName;

private:
    virtual ~SubscriptionDB();

};

#endif //SUBSCRIPTIONDB_H
