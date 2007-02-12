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

#ifndef AUTHEXCEPTIONDB_H
#define AUTHEXCEPTIONDB_H

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

/**
 * This class implements the Alias database abstract class
 *
 * @author John P. Coffey
 * @version 1.0
 */
class AuthexceptionDB
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static AuthexceptionDB* getInstance( const UtlString& name = "authexception" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    // Serialization
    OsStatus store();

    // Insert or update a row in the ExternalForwarding database.
    UtlBoolean insertRow ( const UtlString& rUser );

    // delete ExternalForwarding
    UtlBoolean removeRow ( const UtlString& rUser );

    // Flushes the entire DB
    void removeAllRows ();

    // utility method for dumping all rows
    void getAllRows( ResultSet& rResultSet ) const;

    // Query interface
    UtlBoolean isException ( const UtlString& rUser ) const;

protected:
    // deserialization
    OsStatus load();

    // Singleton Constructor is private
    AuthexceptionDB ( const UtlString& name = "authexception" );

    // One step closer to common load/store code
    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // There is only one singleton in this design
    static AuthexceptionDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gUserKey;

    // Fast DB instance
    dbDatabase* m_pFastDB;

    // the persistent filename for loading/saving
    UtlString mDatabaseName;

private:
    /**
     * Virtual Destructor
     */
    virtual ~AuthexceptionDB();

};

#endif //AUTHEXCEPTIONDB_H