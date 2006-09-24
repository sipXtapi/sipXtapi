//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Scott Zuk <szuk@telusplanet.net>
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef DOMAINDB_H
#define DOMAINDB_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "utl/UtlString.h"

// DEFINES

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class dbDatabase;

/// Database of all virtual domains configured for the server
class DomainDB
{
  public:
   static const UtlString DbName;

   // Singleton Accessor
   static DomainDB* getInstance(const UtlString& name = DbName);

   /// releaseInstance - cleans up the singleton (for use at exit)
   static void releaseInstance();

   /// Add a single mapping to the database.
   void insertRow(const UtlString domainname, /// domain name
                  const UtlString realm       /// authorization realm for the domain
                  );

   // Clear out all rows in the database.
   void removeAllRows();

   //! Find a domain name in the database.
   /*!
    * \param domainname domain name string to look up in the database of the form "example.com".
    * \returns TRUE if domainname exists in the database or FALSE otherwise.
    */
   UtlBoolean isDomain(const UtlString& domainname) const;

   //! Get the authorization realm for a domain name in the database.
   /*!
    * \param domainname domain name string to look up in the database of the form "example.com".
    * \param rRealm authorization realm string returned if it exists.
    * \returns TRUE if domainname exists in the database or FALSE otherwise.
    */
   UtlBoolean getRealm(const UtlString& domainname, UtlString& rRealm) const;

   //! Get a list of all domain names in the database.
   /*!
    * \param domains a UtlContainer to store each domain name in the database.
    * Domain names are inserted into the container as UtlString* objects and the caller is
    * responsible for freeing the allocated memory, eg. by using domains.destroyAll().
    */
   void getAllDomains(UtlContainer* domains) const;

   // Write the current rows to the persistent store.
   OsStatus store();

  protected:
   // Fast DB instance
   static dbDatabase* spDBInstance;

   // Fast DB instance
   dbDatabase* mpFastDB;

   // There is only one singleton in this design
   static DomainDB* spInstance;

   // Singleton and Serialization mutex
   static OsMutex sLockMutex;

   // The persistent filename for loading/saving
   static const UtlString DatabaseName;

   // Implicitly called when constructed - reads data from persistent store.
   OsStatus load();

   // Singleton Constructor is private
   DomainDB(const UtlString& name);

   static const UtlString DomainNameKey;
   static const UtlString RealmKey;

  private:
   /// No destructor, no no no
   ~DomainDB();

   static UtlString nullString;
};

#endif //DOMAINDB_H

