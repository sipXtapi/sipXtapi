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

// APPLICATION INCLUDES
#include "PromptCache.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Contructor
PromptCache::PromptCache()
   : mCacheGuard(OsMutex::Q_FIFO)
   , mPromptTable()
{
}


// Destructor
PromptCache::~PromptCache()
{
   mPromptTable.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Adds a url to the playlist
UtlString* PromptCache::lookup(Url& url, int flags)
{
   OsLock lock(mCacheGuard) ;

   OsStatus status;
   char* szSource;
   int length, readLength;
   UtlString  urlType;
   UtlString  pathName;
   UtlBoolean cacheEntry;
   UtlContainable* pKey;
   UtlContainable* pCachedKey;
   UtlString* pBuffer = NULL;
   UtlString* pCachedBuffer;


   url.getUrlType(urlType);
   if (urlType.compareTo("file", UtlString::ignoreCase) == 0)
   {
      url.getPath(pathName);
      if (pathName.index(STD_PROMPTS_DIR, UtlString::ignoreCase) == UTL_NOT_FOUND)
      {
         cacheEntry = FALSE;
         pKey = NULL;
      }
      else
      {
         cacheEntry = TRUE;
         pKey = mPromptTable.find(&pathName);
      }

      if (pKey == NULL)
      {
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "PromptCache::lookup - Did not find key: %s in prompt cache", pathName.data());
         mUrl = url;
         mpFile = NULL;

         status = open();
         if (status != OS_SUCCESS)
         {
            return pBuffer;
         }
         status = getLength(length);
         if (status != OS_SUCCESS)
         {
            return pBuffer;
         }
         szSource = new char[length];
         status = read(szSource, length, readLength);
         if ((status != OS_SUCCESS) || (length != readLength))
         {
            delete[] szSource;
            return pBuffer;
         }
         close();

         if (cacheEntry == TRUE)
         {
         // Now add the prompt to the table
         pCachedKey = new UtlString(pathName);
         pCachedBuffer = new UtlString(szSource, readLength);
         delete[] szSource;
         pKey = mPromptTable.insertKeyAndValue(pCachedKey, pCachedBuffer);
         if (pKey == NULL)
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "PromptCache::lookup - Insertion of key: %s failed!!", pathName.data());
            return pBuffer;
         }
         else
         {
            pBuffer = new UtlString(pCachedBuffer->data(), pCachedBuffer->length());
         }
         }
      }
      else
      {
         pCachedBuffer = (UtlString*)mPromptTable.findValue(pKey);
         if (pCachedBuffer == NULL)
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                          "PromptCache::lookup - Retrieval of cached buffer for key: %s failed!!", pathName.data());
         }
         else
         {
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "PromptCache::lookup - Found cached buffer for key: %s", pathName.data());
            pBuffer = new UtlString(pCachedBuffer->data(), pCachedBuffer->length());
         }
      }
   }

   return pBuffer ;
}


/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


// Copy constructor
PromptCache::PromptCache(const PromptCache& rPromptCache)
   : mCacheGuard(OsMutex::Q_FIFO)
   , mPromptTable()
{
}

// Assignment operator
PromptCache&
PromptCache::operator=(const PromptCache& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Opens the data source
OsStatus PromptCache::open()
{
   UtlString hostName ;
   UtlString pathName ;
   OsStatus status = OS_FAILED ;

   mUrl.getPath(pathName) ;
   mpFile = new OsFile(pathName) ;

   if (mpFile != NULL)
   {
      status = mpFile->open(OsFile::READ_ONLY) ;
   }

   return status ;
}


// Closes the data source
OsStatus PromptCache::close()
{
   OsStatus status = OS_FAILED ;

   if (mpFile != NULL)
   {
      mpFile->close() ;
      delete mpFile ;
      mpFile = NULL ;
      status = OS_SUCCESS ;
   }
   return status ;
}


// Gets the length of the stream (if available)
OsStatus PromptCache::getLength(int& iLength)
{
   OsStatus rc = OS_FAILED ;

   unsigned long lLength = 0;
   if (mpFile != NULL)
      rc = mpFile->getLength(lLength) ;

   iLength = lLength ;
   return rc ;
}


// Reads iLength bytes of data from the data source and places the data into
// the passed szBuffer buffer.
OsStatus PromptCache::read(char *szBuffer, int iLength, int& iLengthRead)
{
   OsStatus rc = OS_FAILED ;

   if (mpFile != NULL)
   {
      rc = mpFile->read(szBuffer, iLength, (unsigned long&) iLengthRead) ;
   }

   return rc ;
}


/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
