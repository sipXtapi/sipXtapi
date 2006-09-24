//
//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Scott Zuk <szuk@telusplanet.net>
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"

#include "fastdb/fastdb.h"
#include "xmlparser/tinyxml.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/DomainDB.h"
#include "sipdb/DomainRow.h"

#define DOMAIN_XML_NAMESPACE_URL "http://www.sipfoundry.org/sipX/schema/xml/domain-00-00"

REGISTER( DomainRow );

// STATIC INITIALIZERS
DomainDB* DomainDB::spInstance = NULL;
OsMutex   DomainDB::sLockMutex (OsMutex::Q_FIFO);

const UtlString DomainDB::DbName("domain");
const UtlString DomainDB::DomainNameKey("domainname");
const UtlString DomainDB::RealmKey("realm");


/* ============================ CREATORS ================================== */

DomainDB::DomainDB( const UtlString& name )
{
   // Access the shared table databse
   SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
   mpFastDB = pSIPDBManager->getDatabase(name);

   // If we are the first process to attach
   // then we need to load the DB
   int users = pSIPDBManager->getNumDatabaseProcesses(name);
   if ( users == 1 )
   {
      // Load the file implicitly
      this->load();
   }
}

DomainDB::~DomainDB()
{
   OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## DomainDB:: DESTRUCTOR");
}

DomainDB*
DomainDB::getInstance( const UtlString& name )
{
   // Critical Section here
   OsLock lock( sLockMutex );

   // See if this is the first time through for this process
   // Note that this being null => pgDatabase is also null
   if ( spInstance == NULL )
   {
      // Create the singleton class for clients to use
      spInstance = new DomainDB( name );
   }
   return spInstance;
}

void
DomainDB::releaseInstance()
{
   OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## DomainDB:: releaseInstance() spInstance=%p", spInstance);

   // Critical Section here
   OsLock lock( sLockMutex );

   // if it exists, delete the object and NULL out the pointer
   if (spInstance != NULL) {

      // unregister this table/process from the IMDB
      SIPDBManager::getInstance()->removeDatabase ( DbName );

      // NULL out the fastDB pointer also
      spInstance->mpFastDB = NULL;

      delete spInstance;
      spInstance = NULL;
   }
}

// Add a single mapping to the database.
void DomainDB::insertRow( const UtlString domainname, const UtlString realm )
{
   /*
    * The identity value may be the null string; this is a wildcard entry that matches
    * any caller to the given domain.
    */
   if ( !domainname.isNull() && !realm.isNull() && (mpFastDB != NULL) )
   {
      // Thread Local Storage
      mpFastDB->attach();

      // Search for a matching row before deciding to update or insert
      dbCursor<DomainRow> cursor(dbCursorForUpdate);

      DomainRow row;
      dbQuery query;

      // Primary Key is the domain name
      query="domainname=",domainname.data();

      if ( cursor.select( query ) > 0 )
      {
         // Should only be one row so update the contact
         do {
            cursor->realm = realm.data();
            cursor.update();
         } while ( cursor.next() );
      }
      else // Insert as the row does not exist
      {
         // Fill out the row columns
         row.domainname = domainname.data();
         row.realm = realm.data();
         insert (row);
      }
#     if VERBOSE_LOGGING
      OsSysLog::add(FAC_DB,PRI_DEBUG,
                    "DomainDB::insertRow "
                    "domainname='%s', realm='%s'",
                    domainname.data(), realm.data());
#     endif

      // Commit rows to memory - multiprocess workaround
      mpFastDB->detach(0);

      // Table Data changed
      SIPDBManager::getInstance()->
         setDatabaseChangedFlag(DbName, TRUE);
   }
   else
   {
      OsSysLog::add(FAC_DB,PRI_CRIT,
                    "DomainDB::insertRow failed "
                    "db=%p, domainname='%s', realm='%s'",
                    mpFastDB, domainname.data(), realm.data()
                    );
   }
}

OsStatus
DomainDB::load()
{
   // Critical Section here
   OsLock lock( sLockMutex );
   OsStatus result = OS_SUCCESS;

   if ( mpFastDB != NULL )
   {
      // Clean out the existing DB rows before loading
      // a new set from persistent storage
      removeAllRows ();

      UtlString fileName = 
         SIPDBManager::getInstance()->getConfigDirectory()
         + OsPath::separator
         + DbName
         + ".xml";

      OsSysLog::add(FAC_DB, PRI_DEBUG, "DomainDB::load loading '%s'", fileName.data());

      TiXmlDocument doc ( fileName );

      // Verify that we can load the file (i.e it must exist)
      if( doc.LoadFile() )
      {
         TiXmlNode * rootNode = doc.FirstChild ("items");
         if (rootNode != NULL)
         {
            // the folder node contains at least the name/displayname/
            // and autodelete elements, it may contain others
            for( TiXmlNode *itemNode = rootNode->FirstChild( "item" );
                 itemNode; 
                 itemNode = itemNode->NextSibling( "item" ) )
            {
               UtlString domainname;
               UtlString realm;

               for( TiXmlNode *elementNode = itemNode->FirstChild();
                    elementNode; 
                    elementNode = elementNode->NextSibling() )
               {
                  // Bypass comments and other element types only interested
                  // in parsing element attributes
                  if ( elementNode->Type() == TiXmlNode::ELEMENT )
                  {
                     UtlString column(elementNode->Value());

                     if (column.compareTo(DomainNameKey) == 0)
                     {
                        SIPDBManager::getAttributeValue(*itemNode, column, domainname);
                     }
                     else if (column.compareTo(RealmKey) == 0)
                     {
                        SIPDBManager::getAttributeValue(*itemNode, column, realm);
                     }
                     else
                     {
                        OsSysLog::add(FAC_DB, PRI_ERR,
                                      "Unrecognized column '%s' in item: ignored",
                                      column.data()
                                      );
                     }
                  }
               }
               // Insert the item row into the IMDB
               domainname.toLower();
               insertRow (domainname, realm);
            }
         }
      }
      else 
      {
         OsSysLog::add(FAC_DB, PRI_WARNING, "DomainDB::load failed to load '%s'",
                       fileName.data());
      }
   }
   else 
   {
      OsSysLog::add(FAC_DB, PRI_ERR, "DomainDB::load failed - no DB");
      result = OS_FAILED;
   }
   return result;
}

OsStatus
DomainDB::store()
{
   UtlString fileName = 
      SIPDBManager::getInstance()->
      getConfigDirectory() + 
      OsPath::separator + DbName + ".xml";

   // Create an empty document
   TiXmlDocument document;

   // Create a hard coded standalone declaration section
   document.Parse ("<?xml version=\"1.0\" standalone=\"yes\"?>");

   // Create the root node container
   TiXmlElement itemsElement ( "items" );
   itemsElement.SetAttribute( "type", DbName.data() );
   itemsElement.SetAttribute( "xmlns", DOMAIN_XML_NAMESPACE_URL );

   // Critical Section while actually opening and using the database
   {
      OsLock lock( sLockMutex );

      if ( mpFastDB != NULL ) 
      {
         // Thread Local Storage
         mpFastDB->attach();

         // Search our memory for rows
         dbCursor<DomainRow> cursor;

         // Select everything in the IMDB and add as item elements if present
         int rowNumber;
         int rows;
         for (rowNumber = 0, rows = cursor.select();
              rowNumber < rows;
              rowNumber++, cursor.next()
              )
         {
            // Create an item container
            TiXmlElement itemElement ("item");

            // add the domain name element and put the value in it
            TiXmlElement domainnameElement(DomainNameKey.data());
            TiXmlText    domainnameValue(cursor->domainname);
            domainnameElement.InsertEndChild(domainnameValue);

            // add the realm element and put the value in it
            TiXmlElement realmElement(RealmKey.data());
            TiXmlText    realmValue(cursor->realm);
            realmElement.InsertEndChild(realmValue);

            // add this item (row) to the parent items container
            itemsElement.InsertEndChild ( itemElement );
         }

         // Commit rows to memory - multiprocess workaround
         mpFastDB->detach(0);
      }
   } // release mutex around database use

   // Attach the root node to the document
   document.InsertEndChild ( itemsElement );
   document.SaveFile ( fileName );

   return OS_SUCCESS;
}


void
DomainDB::removeAllRows ()
{
   // Thread Local Storage
   if (mpFastDB != NULL) 
   {
      mpFastDB->attach();

      dbCursor<DomainRow> cursor(dbCursorForUpdate);

      if (cursor.select() > 0)
      {
         cursor.removeAllSelected();
      }
      // Commit rows to memory - multiprocess workaround
      mpFastDB->detach(0);

      // Table Data changed
      SIPDBManager::getInstance()->
         setDatabaseChangedFlag(DbName, TRUE);
   }
}

UtlBoolean DomainDB::isDomain(const UtlString& domainname) const
{
   UtlBoolean isDomain = FALSE;

   if ( !domainname.isNull() && (mpFastDB != NULL) )
   {
      // Thread Local Storage
      mpFastDB->attach();

      // Match a all rows where the domain name matches
      dbQuery query;
      query="domainname=",domainname;

      // Search to see if we have a Domain Row
      dbCursor<DomainRow> cursor;

      if ( cursor.select(query) > 0 )
      {
         isDomain = TRUE;
      }

      // Commit the rows to memory - multiprocess workaround
      mpFastDB->detach(0);
   }
   return isDomain;
}

UtlBoolean DomainDB::getRealm(const UtlString& domainname, UtlString& rRealm) const
{
   UtlBoolean found = FALSE;

   if ( !domainname.isNull() && (mpFastDB != NULL) )
   {
      // Thread Local Storage
      mpFastDB->attach();

      dbQuery query;

      // Primary Key is the domain name
      query="domainname=",domainname;

      // Search to see if we have a Domain Row
      dbCursor<DomainRow> cursor;

      if ( cursor.select(query) > 0 )
      {
         // should only be one row
         do {
            rRealm = cursor->realm;
         } while ( cursor.next() );
         found = TRUE;
      }
      // Commit the rows to memory - multiprocess workaround
      mpFastDB->detach(0);
   }
   return found;
}

void DomainDB::getAllDomains(UtlContainer* domains) const
{
   if ((mpFastDB != NULL) && (domains != NULL))
   {
      // must do this first to ensure process/tls integrity
      mpFastDB->attach();

      dbCursor<DomainRow> cursor;
      if ( cursor.select() > 0 )
      {
         do {
            UtlString* domain = new UtlString(cursor->domainname);
            domain->toLower();
            domains->insert(domain);
         } while (cursor.next());
      }
      // commit rows and also ensure process/tls integrity
      mpFastDB->detach(0);
   }
}

