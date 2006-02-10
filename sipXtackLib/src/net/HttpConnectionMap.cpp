// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <string.h>
#include <ctype.h>
#ifdef _VXWORKS
#include <resparse/vxw/hd_string.h>
#endif

#ifdef __pingtel_on_posix__  //needed by linux
#include <wctype.h>
#endif

#include <stdlib.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include <net/HttpMessage.h>
#include <net/HttpConnectionMap.h>
#include <net/NameValuePair.h>
// Needed for SIP_SHORT_CONTENT_LENGTH_FIELD.
#include <net/SipMessage.h>
#include <utl/UtlHashMapIterator.h>

#include <os/OsConnectionSocket.h>
#ifdef HAVE_SSL
#include <os/OsSSLConnectionSocket.h>
#endif /* HAVE_SSL */
#include <os/OsSysLog.h>
#include <os/OsTask.h>
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define HTTP_READ_TIMEOUT_MSECS  30000

#ifdef WIN32
#  define strcasecmp stricmp
#  define strncasecmp strnicmp
#endif

HttpConnectionMap* HttpConnectionMap::pInstance = NULL;
OsBSem HttpConnectionMap::mLock(OsBSem::Q_FIFO, OsBSem::FULL);    

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

HttpConnectionMap* HttpConnectionMap::getHttpConnectionMap()
{
    mLock.acquire();
    if (pInstance == NULL)
    {
        pInstance = new HttpConnectionMap();
    }
    mLock.release();
    return pInstance;
}

void HttpConnectionMap::releaseHttpConnectionMap()
{
    mLock.acquire();    
    if (pInstance)
    {
        delete pInstance;
        pInstance = NULL;
    }
    mLock.release();
}

void HttpConnectionMap::clearHttpConnectionMap()
{
    destroyAll();
}
/* ============================ MANIPULATORS ============================== */

OsConnectionSocket* HttpConnectionMap::getPersistentConnection(const char* key)
{
    UtlString keyString(key);
    HttpConnectionMapEntry* pEntry = dynamic_cast<HttpConnectionMapEntry*>(findValue(&keyString));
    OsConnectionSocket* socket = NULL;
    
    if (pEntry)
    {
        socket = pEntry->pSocket;
        pEntry->mLock.acquire();
    }

    OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                  "HttpConnectionMap::getPersistentConnection - Found persistent connection for %s = %p", key, socket);

    return socket;
}

OsStatus HttpConnectionMap::addPersistentConnection(const char* key, OsConnectionSocket* socket)
{
    OsStatus ret = OS_FAILED;
    
    if (socket)
    {
        HttpConnectionMapEntry* entry = new HttpConnectionMapEntry();
        if (entry)
        {
            entry->pSocket = socket;
            entry->mLock.acquire();
   
            if (insertKeyAndValue(new UtlString(key), entry) != NULL)
            {
                ret = OS_SUCCESS;
                OsSysLog::add(FAC_HTTP, PRI_DEBUG, 
                              "HttpConnectionMap::addPersistentConnection - Adding persistent connection for %s", key);            
            }
            else
            {
                delete entry;
                OsSysLog::add(FAC_HTTP, PRI_ERR,   
                              "HttpConnectionMap::addPersistentConnection - adding %s failed)",
                               key);               
            }
        }
        else
        {
            OsSysLog::add(FAC_HTTP, PRI_ERR,   
                          "HttpConnectionMap::addPersistentConnection - allocating entry memory for %s failed)",
                          key);                 
        }
    }
    else
    {
        OsSysLog::add(FAC_HTTP, PRI_ERR,   
                      "HttpConnectionMap::addPersistentConnection - adding %s failed (socket==NULL)",
                       key);             
    }
    return ret;
}

OsStatus HttpConnectionMap::removePersistentConnection(const char* key)
{
    OsStatus ret = OS_FAILED;
    UtlString keyString(key);
    HttpConnectionMapEntry* pEntry = dynamic_cast<HttpConnectionMapEntry*>(findValue(&keyString)); 

    if (pEntry)
    {
        destroy(&keyString);        
        ret = OS_SUCCESS;
        
        OsSysLog::add(FAC_HTTP, PRI_DEBUG, 
                      "HttpConnectionMap::removePersistentConnection - Removing persistent connection for %s", key);        
    }
    else
    {
        OsSysLog::add(FAC_HTTP, PRI_ERR, 
                      "HttpConnectionMap::removePersistentConnection - Removing persistent connection for %s failed", key);                
    }
    return ret;
}

void HttpConnectionMap::releasePersistentConnectionLock(const char* key)
{
    UtlString keyString(key);
    HttpConnectionMapEntry* pEntry = dynamic_cast<HttpConnectionMapEntry*>(findValue(&keyString));
    
    if (pEntry != NULL)
    {
        pEntry->mLock.release();        
    }
    else
    {
        OsSysLog::add(FAC_HTTP, PRI_ERR, 
                      "HttpConnectionMap::releasePersistentConnection - release for %s failed - entry not found", key);               
    }
}

void HttpConnectionMap::getPersistentUriKey(const Url& url, UtlString& key)
{
    UtlString urlType;
    UtlString httpHost;
    UtlString httpPort;
    
    url.getUrlType(urlType);
    url.getHostAddress(httpHost);
    
    int tempPort = url.getHostPort();
    
    UtlString httpType = (url.getScheme() == Url::HttpsUrlScheme) ? "https" : "http";
    if (tempPort == PORT_NONE)
    {
        if (httpType == "https")
        {
            httpPort = "443";
        }
        else
        {
            httpPort = "80";
        }
    }
    else
    {
        char t[10];
        sprintf(t, "%d", tempPort);
        httpPort = t;
    }
    key = httpType + ":" + httpHost + ":" + httpPort;
    key.toLower();
}

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */
HttpConnectionMap::HttpConnectionMap()
{
}

HttpConnectionMap::~HttpConnectionMap()
{
    clearHttpConnectionMap();
}

HttpConnectionMapEntry::HttpConnectionMapEntry() :
                                 UtlString("HttpConnectionMapEntry"),
                                 mLock(OsBSem::Q_FIFO, OsBSem::FULL)
{
}

HttpConnectionMapEntry::~HttpConnectionMapEntry()
{
    if (pSocket)
    {
        delete pSocket;
        pSocket = NULL;
    }
}
