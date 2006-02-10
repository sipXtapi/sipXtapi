// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _HttpConnectionMap_h_
#define _HttpConnectionMap_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <net/HttpServer.h>
#include <net/HttpBody.h>
#include <net/NameValuePair.h>
#include <os/OsSocket.h>
#include <os/OsConnectionSocket.h>
#include <os/OsTimeLog.h>
#include <os/OsMsgQ.h>
#include <utl/UtlHashMap.h>
#include <os/OsBSem.h>

// DEFINES

class HttpConnectionMapEntry : public UtlString
{
public:
    /// Constructor
    HttpConnectionMapEntry();
    
    /// Destructor
    virtual ~HttpConnectionMapEntry();
    
    OsConnectionSocket* pSocket; //< pointer to a connection socket
    OsBSem              mLock;   //< protects access to the connection
};

class HttpConnectionMap : public UtlHashMap
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /// Get pointer to singleton instance of the connection map
   static HttpConnectionMap* getHttpConnectionMap();
   
   /**<
    * @returns
    * - pointer to the instance of the connectiomn map
    */   
   
   /// Release instance of connection map
   void releaseHttpConnectionMap();
   
   /// Clear all entries in map. Close all socketsa nd deletes them.
   void clearHttpConnectionMap();

   /// Translate Url into key string that will be used for all further access
   void getPersistentUriKey(const Url& url, UtlString& key);   
   
   /// Return a socket for an existing connection or NULL. Locks the connection if non-NULL
   OsConnectionSocket* getPersistentConnection(const char* key);
   
   /**<
    * @returns
    * - pointer to a connection socket if connectionb exists
    * - NULL if the connection does not exist
    */      

   ///  Add a socket to the connection map for a new key. Lock connection if succssful. 
   OsStatus addPersistentConnection(const char* key, OsConnectionSocket* socket);
   
   /**<
    * @returns
    * - OS_SUCCESS if sucessful
    * - OS_FAILED if the connnection could not be added
    */         
  
   /// Release lock on a connection
   void releasePersistentConnectionLock(const char* key);

   ///* Remove connection from connection map. Close and delete the socket.
   OsStatus removePersistentConnection(const char* key);
   
   /**<
    * @returns
    * - OS_SUCCESS if sucessful
    * - OS_FAILED if the connnection could not be removed
    */            

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Constructor
    HttpConnectionMap();

    //! Destructor
    virtual ~HttpConnectionMap();

    static HttpConnectionMap* pInstance; ///< pointer to the instance
    static OsBSem mLock;                 ///< protects access to map
    
    /// no copy constructor
    HttpConnectionMap(const HttpConnectionMap& nocopy);

    /// no assignment operator
    HttpConnectionMap& operator=(const HttpConnectionMap& noassignment);    
};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpConnectionMap_h_
