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

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlInt.h"
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "xmlparser/tinyxml.h"
#include "fastdb/fastdb.h"
#include "net/Url.h"

#include "sipdb/ResultSet.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/SubscriptionRow.h"
#include "sipdb/SubscriptionDB.h"

// DEFINES
REGISTER( SubscriptionRow );

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Static Initializers
SubscriptionDB* SubscriptionDB::spInstance = NULL;
OsMutex         SubscriptionDB::sLockMutex (OsMutex::Q_FIFO);
UtlString SubscriptionDB::gUriKey("uri");
UtlString SubscriptionDB::gCallidKey("callid");
UtlString SubscriptionDB::gContactKey("contact");
UtlString SubscriptionDB::gNotifycseqKey("notifycseq");
UtlString SubscriptionDB::gSubscribecseqKey("subscribecseq");
UtlString SubscriptionDB::gExpiresKey("expires");
UtlString SubscriptionDB::gEventtypeKey("eventtype");
UtlString SubscriptionDB::gIdKey("id");
UtlString SubscriptionDB::gToKey("to");
UtlString SubscriptionDB::gFromKey("from");
UtlString SubscriptionDB::gFileKey("file");
UtlString SubscriptionDB::gKeyKey("key");
UtlString SubscriptionDB::gRecordrouteKey("recordroute");
UtlBoolean     gsVerboseLoggingEnabled = FALSE;

/* ============================ CREATORS ================================== */

SubscriptionDB::SubscriptionDB( const UtlString& name )
: mDatabaseName( name )
{
    // Access the shared table databse
    SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
    m_pFastDB = pSIPDBManager->getDatabase(name);

    // If we are the first process to attach
    // then we need to load the DB
    int users = pSIPDBManager->getNumDatabaseProcesses(name);

    gsVerboseLoggingEnabled = SIPDBManager::isVerboseLoggingEnabled();
    if (gsVerboseLoggingEnabled)
        OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::_  user=%d \"%s\"",
                    users, SIPX_SAFENULL(name.data()));
    if ( users == 1 )
    {
        // Load the file implicitly
        this->load();
    }
}

SubscriptionDB::~SubscriptionDB()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## SubscriptionDB:: DESTRUCTOR");
}

/* ============================ MANIPULATORS ============================== */

void
SubscriptionDB::releaseInstance()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## SubscriptionDB:: releaseInstance() spInstance=%p", spInstance);

    // Critical Section here
    OsLock lock( sLockMutex );

    // if it exists, delete the object and NULL out the pointer
    if (spInstance != NULL) {

        // unregister this table/process from the IMDB
        SIPDBManager::getInstance()->removeDatabase ( spInstance->mDatabaseName );

        // NULL out the fastDB pointer also
        spInstance->m_pFastDB = NULL;

        delete spInstance;
        spInstance = NULL;
    }
}

OsStatus
SubscriptionDB::load()
{
    // Critical Section here
    OsLock lock( sLockMutex );
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL )
    {
        // Clean out the existing DB rows before loading
        // a new set from persistent storage
        removeAllRows ();

        UtlString fileName = 
            SIPDBManager::getInstance()->
                getConfigDirectory() + 
                OsPath::separator + mDatabaseName + ".xml";

        OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::load loading \"%s\"",
                    SIPX_SAFENULL(fileName.data()));

        TiXmlDocument doc ( fileName );

        // Verify that we can load the file (i.e it must exist)
        if( doc.LoadFile() )
        {
            TiXmlNode * rootNode = doc.FirstChild ( "items" );
            if (rootNode != NULL)
            {
                // the folder node contains at least the name/displayname/
                // and autodelete elements, it may contain others
                for( TiXmlNode *itemNode = rootNode->FirstChild( "item" );
                     itemNode; 
                     itemNode = itemNode->NextSibling( "item" ) )
                {
                    // Create a hash dictionary for element attributes
                    UtlHashMap nvPairs;

                    for( TiXmlNode *elementNode = itemNode->FirstChild();
                         elementNode; 
                         elementNode = elementNode->NextSibling() )
                    {
                        // Bypass comments and other element types only interested
                        // in parsing element attributes
                        if ( elementNode->Type() == TiXmlNode::ELEMENT )
                        {
                            UtlString elementName = elementNode->Value();
                            UtlString elementValue;

                            result = SIPDBManager::getAttributeValue (
                                *itemNode, elementName, elementValue);

                            if (result == OS_SUCCESS)
                            {
                                UtlString* collectableKey = 
                                    new UtlString( elementName ); 
                                UtlString* collectableValue = 
                                    new UtlString( elementValue ); 
                                nvPairs.insertKeyAndValue ( 
                                    collectableKey, collectableValue );
                            } else if ( elementNode->FirstChild() == NULL )
                            {
                                // Null Element value creaete a special 
                                // char string we have key and value so insert
                                UtlString* collectableKey = 
                                    new UtlString( elementName ); 
                                UtlString* collectableValue = 
                                    new UtlString( SPECIAL_IMDB_NULL_VALUE ); 
                                nvPairs.insertKeyAndValue ( 
                                    collectableKey, collectableValue );
                            }
                        }
                    }
                    // Insert the item row into the IMDB
                    insertRow ( nvPairs );
                }
            }
        } else 
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "SubscriptionDB::load failed to load \"%s\"",
                    SIPX_SAFENULL(fileName.data()));
        }
    } else 
    {
        OsSysLog::add(FAC_DB, PRI_ERR, "SubscriptionDB::load failed - no DB");
        result = OS_FAILED;
    }
    return result;
}

OsStatus
SubscriptionDB::store()
{
    // Critical Section here
    OsLock lock( sLockMutex );
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL )
    {
        UtlString fileName = 
            SIPDBManager::getInstance()->
                getConfigDirectory() + 
                OsPath::separator + mDatabaseName + ".xml";

        // Thread Local Storage
        m_pFastDB->attach();

        // Search our memory for rows
        dbCursor< SubscriptionRow > cursor;

        // Select everything in the IMDB and add as item elements if present
        int rows = cursor.select();
        if ( rows > 0 )
        {
            OsSysLog::add( FAC_SIP, PRI_DEBUG
                          ,"SubscriptionDB::store writing %d rows\n"
                          ,rows
                          );

            // Create an empty document
            TiXmlDocument document;

            // Create a hard coded standalone declaration section
            document.Parse ("<?xml version=\"1.0\" standalone=\"yes\"?>");

            // Create the root node container
 	        TiXmlElement itemsElement ( "items" );
           itemsElement.SetAttribute( "type", mDatabaseName.data() );

            int timeNow = OsDateTime::getSecsSinceEpoch();
            itemsElement.SetAttribute( "timestamp", timeNow );

            // metadata contains column names
            dbTableDescriptor* pTableMetaData = &SubscriptionRow::dbDescriptor;

            do {
                // Create an item container
                TiXmlElement itemElement ("item");

                byte* base = (byte*)cursor.get();

                // Add the column name value pairs
                for ( dbFieldDescriptor* fd = pTableMetaData->getFirstField();
                      fd != NULL; fd = fd->nextField ) 
                {
                    // if the column name does not contain the 
                    // np_prefix we must_presist it
                    if ( strstr( fd->name, "np_" ) == NULL )
                    {
                        // Create the a column element named after the IMDB column name
                        TiXmlElement element (fd->name );

                        // See if the IMDB has the predefined SPECIAL_NULL_VALUE
                        UtlString textValue;
                        SIPDBManager::getFieldValue(base, fd, textValue);

                        // If the value is not null append a text child element
                        if ( textValue != SPECIAL_IMDB_NULL_VALUE ) 
                        {
                            // Text type assumed here... @todo change this
                            TiXmlText value ( textValue.data() );
                            // Store the column value in the element making this
                            // <colname>coltextvalue</colname>
                            element.InsertEndChild  ( value );
                        }

                        // Store this in the item tag as follows
                        // <item>
                        // .. <col1name>col1textvalue</col1name>
                        // .. <col2name>col2textvalue</col2name>
                        // .... etc
                        itemElement.InsertEndChild  ( element );
                    }
                }
                // add the line to the element
                itemsElement.InsertEndChild ( itemElement );
            } while ( cursor.next() );
            // Attach the root node to the document
            document.InsertEndChild ( itemsElement );
            document.SaveFile ( fileName );
        } else 
        {
            // database contains no rows so delete the file
            // :TODO: This is bogus, we should write out a file with no rows
            // rather than deleting the file, so that when the file is missing
            // we know that's bad.  Get rid of this clause, we don't
            // need to treat this as a special case.
            UtlString fileName = 
                SIPDBManager::getInstance()->
                    getConfigDirectory() + 
                    OsPath::separator + mDatabaseName + ".xml";
            if ( OsFileSystem::exists ( fileName ) ) {
                 OsFileSystem::remove( fileName );
            }
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    } else
    {
        result = OS_FAILED;
    }
    return result;
}

UtlBoolean
SubscriptionDB::insertRow (const UtlHashMap& nvPairs) 
{
    UtlString expStr = *((UtlString*)nvPairs.findValue(&gExpiresKey));
    int expires = (int) atoi( expStr );
    
    UtlString cSubseqStr = *((UtlString*)nvPairs.findValue(&gSubscribecseqKey));
    int subCseq = (int) atoi( cSubseqStr );

    UtlString cNotifySeqStr = *((UtlString*)nvPairs.findValue(&gNotifycseqKey));
    int notifyCseq = (int) atoi( cNotifySeqStr );
    
    // Note: identity inferred from the uri
    return insertRow (
        *((UtlString*)nvPairs.findValue(&gUriKey)),
        *((UtlString*)nvPairs.findValue(&gCallidKey)),
        *((UtlString*)nvPairs.findValue(&gContactKey)),
        expires,
        subCseq,
        *((UtlString*)nvPairs.findValue(&gEventtypeKey)),
        *((UtlString*)nvPairs.findValue(&gIdKey)),
        *((UtlString*)nvPairs.findValue(&gToKey)),
        *((UtlString*)nvPairs.findValue(&gFromKey)),
        *((UtlString*)nvPairs.findValue(&gKeyKey)),
        *((UtlString*)nvPairs.findValue(&gRecordrouteKey)),
        notifyCseq);
}

UtlBoolean
SubscriptionDB::insertRow (
    const UtlString& uri,
    const UtlString& callid,
    const UtlString& contact,
    const int& expires,
    const int& subscribeCseq,
    const UtlString& eventType,
    const UtlString& id,
    const UtlString& to,
    const UtlString& from,
    const UtlString& key,
    const UtlString& recordRoute,
    const int& notifyCseq)
{
    UtlBoolean result = FALSE;
    if ( !uri.isNull() && ( m_pFastDB != NULL ) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Search for a matching row before deciding to update or insert
        dbCursor< SubscriptionRow > cursor( dbCursorForUpdate );
        dbQuery query;

        // Firstly purge all expired field entries from the DB that are expired for this identity
        // we can not purge all rows because a Notify should be to user
        int timeNow = OsDateTime::getSecsSinceEpoch();
        query = "expires <", static_cast<const int&>(timeNow);

        if ( cursor.select( query ) > 0 ) {
            cursor.removeAllSelected();
        }

        // query all sessions (should only be one here)
        query="to=",to,
              "and from=",from,
              "and callid=",callid,
	      "and eventtype=",eventType,
              "and id=",( id.isNull() ? SPECIAL_IMDB_NULL_VALUE : id.data() );
        if ( cursor.select( query ) > 0 )
        {
            // Should only be one row, only updating this
            do 
            {
                // only update the row if the subscribe is newer 
                // than the last IMDB update
                if ( cursor->subscribecseq < subscribeCseq ) 
                { // refreshing subscribe request
                    cursor->uri = uri;
                    cursor->expires = static_cast<const int&>(expires);
                    cursor->subscribecseq = subscribeCseq;
                    cursor->recordroute = recordRoute;
                    cursor->contact = contact;
                    cursor.update();
                } // do nothing as the the input cseq is <= the db cseq
            } while ( cursor.nextAvailable() );
        } else
        {
            // Insert new row
            SubscriptionRow row;
            row.callid = callid;
            row.contact = contact;
            row.expires = expires;
            row.uri = uri;
            row.subscribecseq = subscribeCseq;
            row.notifycseq = notifyCseq;
            row.eventtype = eventType;
            row.id = id;
            row.from = from;
            row.key = key;
            row.to = to;
            row.recordroute = recordRoute;
            insert (row);
        }
        // Either did an insert or an update
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);

        result = TRUE;
    }
    // Most likely an arg problem
    return result;
}

void
SubscriptionDB::removeRow (
    const UtlString& to,
    const UtlString& from,        
    const UtlString& callid,
    const int& subscribeCseq )
{
    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

        // ensure we filter off of the subcribecseq since there 
        // could be multiple removes from the IMDB and the one with
        // the highest cseq should be remain, note that the 
        // < subscribeCseq comparison is important since under UDP conditions
        // the Status server may be busy and UDP may retransmit the 
        // same message multiple times, this would cause the just subscribed row
        // to be incirrectly removed while the status server was sending down 
        // its acknowledgement
        dbQuery query;
        query="to=",to,
              "and from=",from,
              "and callid=",callid,
              "and subcribecseq <",subscribeCseq;
        if (cursor.select(query) > 0)
        {
            cursor.removeAllSelected();
        }
        else
        {
           OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::removeRow row not found:\n"
                         "to='%s' from='%s' callid='%s'\n"
                         "cseq='%d'",
                         SIPX_SAFENULL(to.data()), SIPX_SAFENULL(from.data()), SIPX_SAFENULL(callid.data()),
                         subscribeCseq
                         );
        }
        
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
SubscriptionDB::removeErrorRow (
   const UtlString& to,
   const UtlString& from,        
   const UtlString& callid )
{
   if ( m_pFastDB != NULL )
   {
      // Thread Local Storage
      m_pFastDB->attach();

      dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

      dbQuery query;
      query="to=",to,
         "and from=",from,
         "and callid=",callid;
      if (cursor.select(query) > 0)
      {
         cursor.removeAllSelected();
      }
      else
      {
         OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::removeErrorRow row not found:\n"
                       "to='%s' from='%s' callid='%s'\n",
                       SIPX_SAFENULL(to.data()), SIPX_SAFENULL(from.data()), SIPX_SAFENULL(callid.data())
                       );
      }
        
      // Commit rows to memory - multiprocess workaround
      m_pFastDB->detach(0);
   }
}

void
SubscriptionDB::removeRows (
   const UtlString& key )
{
    if ( !key.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="key=",key;
        if (cursor.select(query) > 0)
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
SubscriptionDB::removeAllRows ()
{
    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< SubscriptionRow > cursor( dbCursorForUpdate );
        if (cursor.select() > 0)
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
SubscriptionDB::getAllRows ( ResultSet& rResultSet ) const
{
    // Clear the results
    rResultSet.destroyAll();

    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< SubscriptionRow > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* uriValue = 
                    new UtlString ( cursor->uri );
                UtlString* callidValue = 
                    new UtlString ( cursor->callid );
                UtlString* contactValue = 
                    new UtlString ( cursor->contact );
                UtlInt* expiresValue = 
                    new UtlInt ( cursor->expires );
                UtlInt* subscribecseqValue = 
                    new UtlInt ( cursor->subscribecseq );
                UtlString* eventtypeValue = 
                    new UtlString ( cursor->eventtype );
                UtlString* idValue = 
                    new UtlString ( cursor->id );
                UtlString* toValue = 
                    new UtlString ( cursor->to );
                UtlString* fromValue = 
                    new UtlString ( cursor->from );
                UtlString* keyValue = 
                    new UtlString ( cursor->key );
                UtlString* recordrouteValue = 
                    new UtlString ( cursor->recordroute );
                UtlInt* notifycseqValue = 
                    new UtlInt ( cursor->notifycseq );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* uriKey = new UtlString( gUriKey );
                UtlString* callidKey = new UtlString( gCallidKey );
                UtlString* contactKey = new UtlString( gContactKey );
                UtlString* expiresKey = new UtlString( gExpiresKey );
                UtlString* subscribecseqKey = new UtlString( gSubscribecseqKey );
                UtlString* eventtypeKey = new UtlString( gEventtypeKey );
                UtlString* idKey = new UtlString( gIdKey );
                UtlString* toKey = new UtlString( gToKey );
                UtlString* fromKey = new UtlString( gFromKey );
                UtlString* keyKey = new UtlString( gKeyKey );
                UtlString* recordrouteKey = new UtlString( gRecordrouteKey );
                UtlString* notifycseqKey = new UtlString( gNotifycseqKey );

                record.insertKeyAndValue ( 
                    uriKey, uriValue);
                record.insertKeyAndValue ( 
                    callidKey, callidValue);
                record.insertKeyAndValue ( 
                    contactKey, contactValue);
                record.insertKeyAndValue ( 
                    expiresKey, expiresValue);
                record.insertKeyAndValue ( 
                    subscribecseqKey, subscribecseqValue);
                record.insertKeyAndValue ( 
                    eventtypeKey, eventtypeValue);
                record.insertKeyAndValue ( 
                    idKey, idValue);
                record.insertKeyAndValue ( 
                    toKey, toValue);
                record.insertKeyAndValue ( 
                    fromKey, fromValue);
                record.insertKeyAndValue ( 
                    keyKey, keyValue);
                record.insertKeyAndValue ( 
                    recordrouteKey, recordrouteValue);
                record.insertKeyAndValue ( 
                    notifycseqKey, notifycseqValue);

                rResultSet.addValue(record);
            } while (cursor.next());
        }
        // Commit the tx
        m_pFastDB->detach(0);
    }
}

void
SubscriptionDB::updateUnexpiredSubscription (
        const UtlString& to,
        const UtlString& from,        
        const UtlString& callid,
        const UtlString& eventType,
        const UtlString& id,
        const int& timeNow,
        const int& updatedNotifyCseq ) const
{
    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Create an update cursor to purge the DB
        // of all expired contacts. This is a hack since this
        // should be implemented via a daemon garbage collector
        // thread.
        dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="to=",to,
              "and from=",from,
              "and callid=",callid,
              "and eventtype=",eventType,
              "and id=",(id.isNull() ? SPECIAL_IMDB_NULL_VALUE : id.data())
	  ;	  
        if ( cursor.select(query) > 0 )
        {
            do {
                // Purge any expired rows 
                if ( cursor->expires < timeNow ) 
                {
                    // note cursor.remove() auto updates the database
                    cursor.remove();
                } else
                {
                   cursor->notifycseq = updatedNotifyCseq;
                   // Update to the new NotifySCeq Number
                   cursor.update();
                }

                // Next replaced with nextAvailable - better when 
                // selective updates applied to the cursor object
            } while ( cursor.nextAvailable() );
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
SubscriptionDB::getUnexpiredSubscriptions (
    const UtlString& key,
    const UtlString& eventType,
    const int& timeNow,
    ResultSet& rResultSet ) const
{
    // Clear the results
    rResultSet.destroyAll();

    if ( !key.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Create an update cursor to purge the DB
        // of all expired contacts. 
        // This should be eventually done via a daemon thread
        dbCursor< SubscriptionRow > cursor( dbCursorForUpdate );
        dbQuery query;
        query="expires <",timeNow;
        if ( cursor.select( query ) > 0 ) {
            cursor.removeAllSelected();
        }

        // Now select the remaining current events
        query="key=",key,"and eventtype=",eventType;
        if ( cursor.select(query) > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* uriValue = 
                    new UtlString ( cursor->uri );
                UtlString* callidValue = 
                    new UtlString ( cursor->callid );
                UtlString* contactValue = 
                    new UtlString ( cursor->contact );
                UtlInt* expiresValue = 
                    new UtlInt ( cursor->expires - timeNow );
                UtlInt* subscribecseqValue = 
                    new UtlInt ( cursor->subscribecseq );
                UtlString* eventtypeValue = 
                    new UtlString ( cursor->eventtype );
                UtlString* idValue = new UtlString(
                  (  (0 == strcmp(cursor->id, SPECIAL_IMDB_NULL_VALUE))
                   ? ""
                   : cursor->id
                   ));
                UtlString* toValue = 
                    new UtlString ( cursor->to );
                UtlString* fromValue = 
                    new UtlString ( cursor->from );
                UtlString* keyValue = 
                    new UtlString ( cursor->key );
                UtlString* recordrouteValue = 
                    new UtlString ( cursor->recordroute );
                UtlInt* notifycseqValue = 
                    new UtlInt ( cursor->notifycseq );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* uriKey = new UtlString( gUriKey );
                UtlString* callidKey = new UtlString( gCallidKey );
                UtlString* contactKey = new UtlString( gContactKey );
                UtlString* expiresKey = new UtlString( gExpiresKey );
                UtlString* subscribecseqKey = new UtlString( gSubscribecseqKey );
                UtlString* eventtypeKey = new UtlString( gEventtypeKey );
                UtlString* idKey = new UtlString( gIdKey );
                UtlString* toKey = new UtlString( gToKey );
                UtlString* fromKey = new UtlString( gFromKey );
                UtlString* keyKey = new UtlString( gKeyKey );
                UtlString* recordrouteKey = new UtlString( gRecordrouteKey );
                UtlString* notifycseqKey = new UtlString( gNotifycseqKey );

                record.insertKeyAndValue ( 
                    uriKey, uriValue);
                record.insertKeyAndValue ( 
                    callidKey, callidValue);
                record.insertKeyAndValue ( 
                    contactKey, contactValue);
                record.insertKeyAndValue ( 
                    expiresKey, expiresValue);
                record.insertKeyAndValue ( 
                    subscribecseqKey, subscribecseqValue);
                record.insertKeyAndValue ( 
                    eventtypeKey, eventtypeValue);
                record.insertKeyAndValue ( 
                    idKey, idValue);
                record.insertKeyAndValue ( 
                    toKey, toValue);
                record.insertKeyAndValue ( 
                    fromKey, fromValue);
                record.insertKeyAndValue ( 
                    keyKey, keyValue);
                record.insertKeyAndValue ( 
                    recordrouteKey, recordrouteValue);
                record.insertKeyAndValue ( 
                    notifycseqKey, notifycseqValue);

                rResultSet.addValue(record);
            } while ( cursor.nextAvailable() );
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

SubscriptionDB*
SubscriptionDB::getInstance( const UtlString& name )
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new SubscriptionDB ( name );
    }
    return spInstance;
}


