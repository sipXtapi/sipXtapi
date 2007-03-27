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
#ifndef HUNTGROUPDB_H
#define HUNTGROUPDB_H

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

/**
 * This class implements the Alias database abstract class
 *
 * @author John P. Coffey
 * @version 1.0
 */
class HuntgroupDB
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static HuntgroupDB* getInstance( const UtlString& name = "huntgroup" );

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();

    // Serialization
    OsStatus store();

    // Insert or update a row in the Huntgroup database.
    UtlBoolean insertRow ( const Url& identity );

    // delete huntgroup
    UtlBoolean removeRow ( const Url& identity );

    // Flushes the entire DB
    void removeAllRows ();

    // utility method for dumping all rows
    void getAllRows( ResultSet& rResultSet ) const;

    // Query interface to return a set of mapped full URI
    // contacts associated with the alias
    UtlBoolean isHuntGroup ( const Url& identity ) const;

protected:
    // deserialization
    OsStatus load();

    // Singleton Constructor is private
    HuntgroupDB ( const UtlString& name );

    // One step closer to common load/store code
    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    // There is only one singleton in this design
    static HuntgroupDB* spInstance;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gIdentityKey;

    // Fast DB instance
    dbDatabase* m_pFastDB;

    // the persistent filename for loading/saving
    UtlString mDatabaseName;

    // the working direcory for all database instances
    // the XML files are located here
    UtlString m_etcDirectory;

private:
    /**
     * Virtual Destructor
     */
    virtual ~HuntgroupDB();

};

#endif //HUNTGROUPDB_H