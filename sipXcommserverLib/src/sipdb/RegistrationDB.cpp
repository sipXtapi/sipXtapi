// 
// 
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
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

// APPLICATION INCLUDES
#include "utl/UtlInt.h"
#include "utl/UtlLongLongInt.h"
#include "utl/UtlSListIterator.h"
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "net/Url.h"
#include "fastdb/fastdb.h"

#include "xmlparser/tinyxml.h"
#include "sipdb/RegistrationBinding.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/RegistrationRow.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SIPDBManager.h"

REGISTER( RegistrationRow );


// Smart DB accessor that attaches the DB on construction, and detaches on destruction.
// This is necessary to protect process/thread-local storage integrity.
class SmartDbAccessor
{
public:
   SmartDbAccessor(dbDatabase* fastDB, int lineNo) : mFastDB(fastDB), mLineNo(lineNo)
      {
         if (mFastDB != NULL)
         {
            mFastDB->attach();
         }
         else
         {
            OsSysLog::add(FAC_DB, PRI_ERR, "RegistrationDB.cpp line %d - no DB", lineNo);
         }
      }
   ~SmartDbAccessor()
      {
         if (mFastDB != NULL)
         {
            mFastDB->detach(0);
         }
      }

private:
   dbDatabase* mFastDB;
   int         mLineNo;
};

#define SMART_DB_ACCESS SmartDbAccessor accessor(m_pFastDB, __LINE__)


// Static Initializers
RegistrationDB* RegistrationDB::spInstance = NULL;
OsMutex         RegistrationDB::sLockMutex (OsMutex::Q_FIFO);

UtlString RegistrationDB::gIdentityKey("identity");
UtlString RegistrationDB::gUriKey("uri");
UtlString RegistrationDB::gCallidKey("callid");
UtlString RegistrationDB::gContactKey("contact");
UtlString RegistrationDB::gQvalueKey("qvalue");
UtlString RegistrationDB::gInstanceIdKey("instance_id");
UtlString RegistrationDB::gGruuKey("gruu");
UtlString RegistrationDB::gCseqKey("cseq");
UtlString RegistrationDB::gExpiresKey("expires");
UtlString RegistrationDB::gPrimaryKey("primary");
UtlString RegistrationDB::gUpdateNumberKey("update_number");

UtlString RegistrationDB::nullString("");

/* ============================ CREATORS ================================== */

/*
 *Function Name: RegistrationDB Constructor
 *
 *Parameters:
 *
 *Description: This is a protected method as this is a singleton
 *
 *Returns:
 *
 */
RegistrationDB::RegistrationDB( const UtlString& name ) :
    mDatabaseName( name )
{
    // Access the shared table databse
    SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
    m_pFastDB = pSIPDBManager->getDatabase(name);

    // If we are the first process to attach
    // then we need to load the DB
    int users = pSIPDBManager->getNumDatabaseProcesses(name);
    if ( users == 1 )
    {
        // Load the file implicitly
        this->load();
    }
}

RegistrationDB::~RegistrationDB()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## RegistrationDB:: DESTRUCTOR");
}

/* ============================ MANIPULATORS ============================== */

void
RegistrationDB::releaseInstance()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## RegistrationDB:: releaseInstance() spInstance=%p", spInstance);

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
RegistrationDB::load()
{
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL )
    {
        UtlString fileName =
            SIPDBManager::getInstance()->
                getConfigDirectory() +
                OsPath::separator + mDatabaseName + ".xml";

        OsSysLog::add(FAC_DB, PRI_DEBUG, "RegistrationDB::load loading \"%s\"",
                    fileName.data());

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
            OsSysLog::add(FAC_DB, PRI_WARNING, "RegistrationDB::load failed to load \"%s\"",
                    fileName.data());
        }
    } else
    {
        OsSysLog::add(FAC_DB, PRI_ERR, "RegistrationDB::load failed - no DB");
        result = OS_FAILED;
    }
    return result;
}

/// Garbage collect and persist database
///
/// Garbage collect - delete all rows older than the specified
/// time, and then write all remaining entries to the persistent
/// data store (xml file).
OsStatus RegistrationDB::cleanAndPersist( const int &newerThanTime )
{
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL )
    {
        SMART_DB_ACCESS;

        // Purge all expired field entries from the DB
        // Note: callid set to null indicates provisioned entries and
        //        these should not be removed
        dbCursor< RegistrationRow > expireCursor( dbCursorForUpdate );
        dbQuery query;
        query = "expires <", static_cast<const int&>(newerThanTime), " and (callid != '#')";
        int rows = expireCursor.select( query );
        if ( rows > 0 )
        {
            OsSysLog::add( FAC_SIP, PRI_DEBUG
                          ,"RegistrationDB::cleanAndPersist cleaning out %d rows\n"
                          ,rows
                          );
            expireCursor.removeAllSelected();
        }

        UtlString fileName =
            SIPDBManager::getInstance()->
                getConfigDirectory() +
                OsPath::separator + mDatabaseName + ".xml";

        // Search our memory for rows
        dbCursor< RegistrationRow > cursor;

        // Select everything in the IMDB and add as item elements if present
        rows = cursor.select();
        if ( rows > 0 )
        {
            OsSysLog::add( FAC_SIP, PRI_DEBUG
                          ,"RegistrationDB::cleanAndPersist writing %d rows\n"
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
            dbTableDescriptor* pTableMetaData = &RegistrationRow::dbDescriptor;

            do {
                // Create an item container
                TiXmlElement itemElement ("item");

                byte* base = (byte*)cursor.get();

                // Add the column name value pairs
                for ( dbFieldDescriptor* fd = pTableMetaData->getFirstField();
                      fd != NULL; fd = fd->nextField )
                {
                    // if the column name does not contain the
                    // np_prefix we must persist it
                    if ( strstr( fd->name, "np_" ) == NULL )
                    {
                        // Create a column element named after the IMDB column name
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
        }
        else
        {
            // database contains no rows so delete the file
            UtlString fileName =
                SIPDBManager::getInstance()->
                    getConfigDirectory() +
                    OsPath::separator + mDatabaseName + ".xml";
            if ( OsFileSystem::exists ( fileName ) ) {
                 OsFileSystem::remove( fileName );
            }
        }
    }
    else
    {
        result = OS_FAILED;
    }
    return result;
}

void
RegistrationDB::insertRow (const UtlHashMap& nvPairs)
{
    UtlString expStr = *((UtlString*)nvPairs.findValue(&gExpiresKey));
    int expires = (int) atoi( expStr );

    UtlString cseqStr = *((UtlString*)nvPairs.findValue(&gCseqKey));
    int cseq = (int) atoi( cseqStr );

    // If the IMDB does not specify a Q-Value % will be found here
    // (representing a null IMDB column)
    UtlString qvalue = *((UtlString*)nvPairs.findValue(&gQvalueKey));

    UtlString* updateNumberStr = dynamic_cast<UtlString*>(nvPairs.findValue(&gUpdateNumberKey));
    Int64 updateNumber = (updateNumberStr ? UtlLongLongInt::stringToLongLong(updateNumberStr->data()) : 0);

    // Get the remaining fields so that we can substitute the null string
    // if the fetched value is 0 (the null pointer) because the field
    // is not present in the disk file.
    UtlString* contact = (UtlString*) nvPairs.findValue(&gContactKey);
    UtlString* callId = (UtlString*) nvPairs.findValue(&gCallidKey);
    UtlString* instanceId = (UtlString*) nvPairs.findValue(&gInstanceIdKey);
    UtlString* gruu = (UtlString*) nvPairs.findValue(&gGruuKey);
    UtlString* primary = (UtlString*) nvPairs.findValue(&gPrimaryKey);

    // Note: identity inferred from the uri
    updateBinding (
        Url(*((UtlString*)nvPairs.findValue(&gUriKey))),
        *(contact ? contact : &nullString),
        qvalue,
        *(callId ? callId : &nullString),
        cseq,
        expires,
        *(instanceId ? instanceId : &nullString),
        *(gruu ? gruu : &nullString),
        *(primary ? primary : &nullString),
        updateNumber
        );
}


void
RegistrationDB::updateBinding(const RegistrationBinding& reg)
{
   updateBinding(*(reg.getUri()),    // must not be null
                 *(reg.getContact() ? reg.getContact() : &nullString),
                 *(reg.getQvalue() ? reg.getQvalue() : &nullString),
                 *(reg.getCallId() ? reg.getCallId() : &nullString),
                 reg.getCseq(),
                 reg.getExpires(),
                 *(reg.getInstanceId() ? reg.getInstanceId() : &nullString),
                 *(reg.getGruu() ? reg.getGruu() : &nullString),
                 *(reg.getPrimary() ? reg.getPrimary() : &nullString),
                 reg.getUpdateNumber());
}


void
RegistrationDB::updateBinding( const Url& uri
                              ,const UtlString& contact
                              ,const UtlString& qvalue
                              ,const UtlString& callid
                              ,const int& cseq
                              ,const int& expires
                              ,const UtlString& instance_id
                              ,const UtlString& gruu
                              ,const UtlString& primary
                              ,const Int64& update_number
                              )
{
    UtlString identity;
    uri.getIdentity(identity);
    UtlString fullUri = uri.toString();

    if ( !identity.isNull() && (m_pFastDB != NULL) )
    {
        SMART_DB_ACCESS;

        // Search for a matching row before deciding to update or insert
        dbCursor< RegistrationRow > cursor( dbCursorForUpdate );
        dbQuery query;

        query="np_identity=",identity,
            "  and contact=",contact;
        int existingBinding = cursor.select( query ) > 0;
        RegistrationRow row;
        switch ( existingBinding )
        {
            default:
                // Should not happen - there should either be 1 or none
                OsSysLog::add( FAC_SIP, PRI_ERR,
                              "RegistrationDB::updateBinding %d bindings for %s -> %s"
                              ,existingBinding, identity.data(), contact.data()
                              );
                // recover by clearing them out
                cursor.removeAllSelected();
                // and falling through to insert the new one

            case 0:
                // Insert new row
                row.np_identity   = identity;
                row.uri           = fullUri;
                row.callid        = callid;
                row.cseq          = cseq;
                row.contact       = contact;
                row.qvalue        = qvalue;
                row.expires       = expires;
                row.instance_id   = instance_id;
                row.gruu          = gruu;
                row.primary       = primary;
                row.update_number = update_number;
                insert (row);
                break;

            case 1:
                // this id->contact binding exists - update it
                cursor->uri           = fullUri;
                cursor->callid        = callid;
                cursor->cseq          = cseq;
                cursor->qvalue        = qvalue;
                cursor->expires       = expires;
                cursor->instance_id   = instance_id;
                cursor->gruu          = gruu;
                cursor->primary       = primary;
                cursor->update_number = update_number;
                cursor.update();
                break;
        }
    }
    else
    {
        OsSysLog::add( FAC_SIP, PRI_ERR,
                      "RegistrationDB::updateBinding bad state %s %p"
                      ,identity.data(), m_pFastDB
                      );
    }
}

/// clean out any bindings for this callid and older cseq values.
void
RegistrationDB::expireOldBindings( const Url& uri
                                  ,const UtlString& callid
                                  ,const int& cseq
                                  ,const int& timeNow
                                  ,const UtlString& primary
                                  ,const Int64& update_number
                                  )
{
    UtlString identity;
    uri.getIdentity(identity);

    int expirationTime = timeNow-1;

    if ( !identity.isNull() && ( m_pFastDB != NULL ) )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="np_identity=",identity,
           "and callid=",callid,
          "and cseq<",cseq,
          "and expires>=",expirationTime
          ;
        if (cursor.select(query) > 0)
        {
          do
            {
              cursor->expires = expirationTime;
              cursor->cseq = cseq;
              cursor->primary = primary;
              cursor->update_number = update_number;
              cursor.update();
            } while ( cursor.next() );
        }
    }
}

/// expireAllBindings for this URI as of 1 second before timeNow
void RegistrationDB::expireAllBindings( const Url& uri
                                       ,const UtlString& callid
                                       ,const int& cseq
                                       ,const int& timeNow
                                       ,const UtlString& primary
                                       ,const Int64& update_number
                                       )
{
    UtlString identity;
    uri.getIdentity(identity);
    int expirationTime = timeNow-1;

    if ( !identity.isNull() && ( m_pFastDB != NULL ) )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor(dbCursorForUpdate);
        dbQuery query;
        query="np_identity=",identity," and expires>=",expirationTime;

        if (cursor.select(query) > 0)
        {
           do
           {
              cursor->expires = expirationTime;
              cursor->callid  = callid;
              cursor->cseq    = cseq;
              cursor->primary = primary;
              cursor->update_number = update_number;
              cursor.update();
           } while ( cursor.next() );
        }
    }
}

UtlBoolean
RegistrationDB::isOutOfSequence( const Url& uri
                                ,const UtlString& callid
                                ,const int& cseq
                                ) const
{
  UtlBoolean isOlder;

  UtlString identity;
  uri.getIdentity( identity );

  if ( !identity.isNull() && ( m_pFastDB != NULL) )
    {
      SMART_DB_ACCESS;
      dbCursor< RegistrationRow > cursor;
      dbQuery query;
      query="np_identity=",identity,
        " and callid=",callid,
        " and cseq>=",cseq;
      isOlder = ( cursor.select(query) > 0 );
    }
  else
    {
      OsSysLog::add( FAC_SIP, PRI_ERR,
                    "RegistrationDB::isOutOfSequence bad state @ %d %p"
                    ,__LINE__, m_pFastDB
                    );
      isOlder = TRUE; // will cause 500 Server Internal Error, which is true
    }

  return isOlder;
}


void
RegistrationDB::removeAllRows ()
{
    if ( m_pFastDB != NULL )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor( dbCursorForUpdate );
        if (cursor.select() > 0)
        {
            cursor.removeAllSelected();
        }
    }
}

void
RegistrationDB::getAllRows ( ResultSet& rResultSet ) const
{
    // Clear out any previous records
    rResultSet.destroyAll();

    if ( m_pFastDB != NULL )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* uriValue = new UtlString(cursor->uri);
                UtlString* callidValue = new UtlString(cursor->callid);
                UtlString* contactValue = new UtlString(cursor->contact);
                UtlInt* expiresValue = new UtlInt(cursor->expires);
                UtlInt* cseqValue = new UtlInt(cursor->cseq);
                UtlString* qvalueValue = new UtlString(cursor->qvalue);
                UtlString* primaryValue = new UtlString(cursor->primary);
                UtlLongLongInt* updateNumberValue = new UtlLongLongInt(cursor->update_number);

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* uriKey = new UtlString(gUriKey);
                UtlString* callidKey = new UtlString(gCallidKey);
                UtlString* contactKey = new UtlString(gContactKey);
                UtlString* expiresKey = new UtlString(gExpiresKey);
                UtlString* cseqKey = new UtlString(gCseqKey);
                UtlString* qvalueKey = new UtlString(gQvalueKey);
                UtlString* primaryKey = new UtlString(gPrimaryKey);
                UtlString* updateNumberKey = new UtlString(gUpdateNumberKey);

                record.insertKeyAndValue(uriKey, uriValue);
                record.insertKeyAndValue(callidKey, callidValue);
                record.insertKeyAndValue(contactKey, contactValue);
                record.insertKeyAndValue(expiresKey, expiresValue);
                record.insertKeyAndValue(cseqKey, cseqValue);
                record.insertKeyAndValue(qvalueKey, qvalueValue);
                record.insertKeyAndValue(primaryKey, primaryValue);
                record.insertKeyAndValue(updateNumberKey, updateNumberValue);

                rResultSet.addValue(record);
            } while (cursor.next());
        }
    }
}

Int64
RegistrationDB::getMaxUpdateNumberForRegistrar(const UtlString& primaryRegistrar) const
{
   Int64 maxUpdateForPrimary = 0;

   if ( m_pFastDB != NULL )
   {
      SMART_DB_ACCESS;
      dbCursor<RegistrationRow> cursor;
      dbQuery query;
      query = "primary = ", primaryRegistrar, "order by update_number desc";
   
      int numRows = cursor.select(query);
      if (numRows > 0)
      {
         maxUpdateForPrimary = cursor->update_number;
      }
   }
   else
   {
      assert(false);    // when this method is called, the DB pointer should not be null
   }

   return maxUpdateForPrimary;
}

Int64
RegistrationDB::getNextUpdateNumberForRegistrar(const UtlString& primaryRegistrar,
                                                Int64            updateNumber) const
{
   Int64 nextUpdateNumber = 0;

   if ( m_pFastDB != NULL )
   {
      SMART_DB_ACCESS;
      dbCursor<RegistrationRow> cursor;
      dbQuery query;
      query = "primary = ", primaryRegistrar,
              " and update_number > ", updateNumber,
              " order by update_number asc";
   
      int numRows = cursor.select(query);
      if (numRows > 0)
      {
         nextUpdateNumber = cursor->update_number;
      }
   }
   else
   {
      assert(false);    // when this method is called, the DB pointer should not be null
   }

   return nextUpdateNumber;
}

int 
RegistrationDB::getNextUpdateForRegistrar(const UtlString& primaryRegistrar,
                                          Int64            updateNumber,
                                          UtlSList&        bindings) const
{
   int numRows = 0;
   Int64 nextUpdateNumber = getNextUpdateNumberForRegistrar(primaryRegistrar, updateNumber);
   if (nextUpdateNumber > 0)
   {
      dbQuery query;
      query =
         "primary = ", primaryRegistrar,
         " and update_number = ", nextUpdateNumber;
         numRows = getUpdatesForRegistrar(query, bindings);
      if (numRows > 0)
      {
         OsSysLog::add(
            FAC_SIP, PRI_DEBUG
            ,"RegistrationDB::getNextUpdateForRegistrar"
            " found %d rows for %s with updateNumber > %0#16llx"
            ,numRows
            ,primaryRegistrar.data()
            ,updateNumber);
      }   
   }
   return numRows;
}

int
RegistrationDB::getNewUpdatesForRegistrar(const UtlString& primaryRegistrar,
                                          Int64            updateNumber,
                                          UtlSList&        bindings) const
{
   dbQuery query;
   query = "primary = ", primaryRegistrar, " and update_number > ", updateNumber;
   int numRows = getUpdatesForRegistrar(query, bindings);
   if (numRows > 0)
   {
      OsSysLog::add(
         FAC_SIP, PRI_DEBUG
         ,"RegistrationDB::getNewUpdatesForRegistrar"
         " found %d rows for %s with updateNumber > %0#16llx"
         ,numRows
         ,primaryRegistrar.data()
         ,updateNumber);
   }   
   return numRows;
}

int
RegistrationDB::getUpdatesForRegistrar(dbQuery&  query,
                                       UtlSList& bindings) const
{
   int numRows = 0;
   if ( m_pFastDB != NULL )
   {
      SMART_DB_ACCESS;
      dbCursor<RegistrationRow> cursor(dbCursorForUpdate);
      numRows = cursor.select(query);
      if (numRows > 0)
      {
         do {
            RegistrationBinding* reg = copyRowToRegistrationBinding(cursor);
            bindings.append(reg);
         }
         while (cursor.next());
      }
   }
   return numRows;
}

void
RegistrationDB::getUnexpiredContacts (
   const Url& uri,
   const int& timeNow,
   ResultSet& rResultSet) const
{
    // Clear the results
    rResultSet.destroyAll();

    UtlString identity;
    uri.getIdentity( identity );

    if ( !identity.isNull() && ( m_pFastDB != NULL) )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor;
        dbQuery query;
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "RegistrationDB::getUnexpiredContacts "
                      "identity = '%s'",
                      identity.data());
        if (strncmp(identity.data(), GRUU_PREFIX,
                    sizeof (GRUU_PREFIX) - 1) == 0)
        {
           // This is a GRUU, search for it in the gruu column.
           query="gruu=",identity," and expires>",timeNow;
           OsSysLog::add(FAC_DB, PRI_DEBUG,
                         "RegistrationDB::getUnexpiredContacts recognized GRUU");
        }
        else
        {
           // This is not a GRUU, search for it in the identity column.
           query="np_identity=",identity," and expires>",timeNow;
        }

        if ( cursor.select(query) > 0 )
        {
            // Copy all the unexpired contacts into the result hash
            do
            {
                UtlHashMap record;
                UtlString* uriValue = new UtlString(cursor->uri);
                UtlString* callidValue = new UtlString(cursor->callid);
                UtlString* contactValue = new UtlString(cursor->contact);
                UtlInt* expiresValue = new UtlInt(cursor->expires);
                UtlInt* cseqValue = new UtlInt(cursor->cseq);
                UtlString* qvalueValue = new UtlString(cursor->qvalue);
                UtlString* primaryValue = new UtlString(cursor->primary);
                UtlLongLongInt* updateNumberValue = new UtlLongLongInt(cursor->update_number);

                UtlString* instanceIdValue = new UtlString(cursor->instance_id);
                UtlString* gruuValue = new UtlString(cursor->gruu);
                OsSysLog::add(FAC_DB, PRI_DEBUG,
                              "RegistrationDB::getUnexpiredContacts Record found "
                              "uri = '%s', contact = '%s', instance_id = '%s', "
                              "gruu = '%s'",
                              uriValue->data(), contactValue->data(),
                              instanceIdValue->data(), gruuValue->data());

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* uriKey = new UtlString(gUriKey);
                UtlString* callidKey = new UtlString(gCallidKey);
                UtlString* contactKey = new UtlString(gContactKey);
                UtlString* expiresKey = new UtlString(gExpiresKey);
                UtlString* cseqKey = new UtlString(gCseqKey);
                UtlString* qvalueKey = new UtlString(gQvalueKey);
                UtlString* primaryKey = new UtlString(gPrimaryKey);
                UtlString* updateNumberKey = new UtlString(gUpdateNumberKey);

                UtlString* instanceIdKey = new UtlString(gInstanceIdKey);
                UtlString* gruuKey = new UtlString(gGruuKey);

                record.insertKeyAndValue(uriKey, uriValue);
                record.insertKeyAndValue(callidKey, callidValue);
                record.insertKeyAndValue(contactKey, contactValue);
                record.insertKeyAndValue(expiresKey, expiresValue);
                record.insertKeyAndValue(cseqKey, cseqValue);
                record.insertKeyAndValue(qvalueKey, qvalueValue);
                record.insertKeyAndValue(primaryKey, primaryValue);
                record.insertKeyAndValue(updateNumberKey, updateNumberValue);
 
                record.insertKeyAndValue(instanceIdKey, instanceIdValue);
                record.insertKeyAndValue(gruuKey, gruuValue);

                rResultSet.addValue(record);

            } while ( cursor.next() );
        }
    }
}

RegistrationDB*
RegistrationDB::getInstance( const UtlString& name )
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new RegistrationDB ( name );
    }
    return spInstance;
}

RegistrationBinding*
RegistrationDB::copyRowToRegistrationBinding(dbCursor<RegistrationRow>& cursor) const
{
   RegistrationBinding *reg = new RegistrationBinding();
   reg->setUri(* new UtlString(cursor->uri));
   reg->setCallId(* new UtlString(cursor->callid));
   reg->setContact(* new UtlString(cursor->contact));
   reg->setQvalue(* new UtlString(cursor->qvalue));
   reg->setInstanceId(* new UtlString(cursor->instance_id));
   reg->setGruu(* new UtlString(cursor->gruu));
   reg->setCseq(cursor->cseq);
   reg->setExpires(cursor->expires);
   reg->setPrimary(* new UtlString(cursor->primary));
   reg->setUpdateNumber(cursor->update_number);
   return reg;
}