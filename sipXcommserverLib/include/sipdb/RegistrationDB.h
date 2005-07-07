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
#ifndef REGISTRATIONDB_H
#define REGISTRATIONDB_H

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

/// Database of all information aquired through REGISTER messages
class RegistrationDB
{
public:
    // Singleton Accessor
    static RegistrationDB* getInstance(
        const UtlString& name = "registration" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    /// Queries

    ///
    UtlBoolean isOutOfSequence( const Url& uri
                              ,const UtlString& callid
                              ,const int& cseq
                              ) const;

    // utility method for dumping all rows
    void getAllRows ( ResultSet& rResultSet ) const;

    /// getUnexpiredContacts is used to generate lookup responses
    void getUnexpiredContacts ( const Url& uri
                               ,const int& timeNow
                               ,ResultSet& rResultSet
                               ) const;

    /// updateBinding of uri to contact: does insert or update as needed
    void updateBinding( const Url& uri
                       ,const UtlString& contact
                       ,const UtlString& qvalue
                       ,const UtlString& callid
                       ,const int& cseq
                       ,const int& expires
                       );

    /// expireAllBindings for this URI as of 1 second before timeNow
    void expireAllBindings( const Url& uri
                           ,const UtlString& callid
                           ,const int& cseq
                           ,const int& timeNow
                           );

    /// expireOldBindings for this callid and older cseq values.
    void expireOldBindings( const Url& uri
                           ,const UtlString& callid
                           ,const int& cseq
                           ,const int& timeNow
                           );

    void removeAllRows ();

    /// Garbage collect and persist database
    ///
    /// Garbage collect - delete all rows older than the specified
    /// time, and then write all remaining entries to the persistant
    /// data store (xml file).
    OsStatus cleanAndPersist( const int &newerThanTime );

    // Added this to make load and store code identical
    // in all database implementations, One step closer
    // to a template version of the code
    void insertRow ( const UtlHashMap& nvPairs );

  protected:
    // this is implicit now
    OsStatus load();

    // Singleton Constructor is private
    RegistrationDB( const UtlString& name );

    // There is only one singleton in this design
    static RegistrationDB* spInstance;

    // Fast DB instance
    static dbDatabase* spDBInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gUriKey;
    static UtlString gCallidKey;
    static UtlString gContactKey;
    static UtlString gExpiresKey;
    static UtlString gCseqKey;
    static UtlString gQvalueKey;

    // Fast DB instance
    dbDatabase* m_pFastDB;

    // the persistent filename for loading/saving
    UtlString mDatabaseName;

private:
    /// No destructor, no no no
    virtual ~RegistrationDB();

};

#endif //REGISTRATIONDB_H
