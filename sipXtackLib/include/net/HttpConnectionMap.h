// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
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
#include <os/OsTimeLog.h>
#include <os/OsMsgQ.h>
#include <utl/UtlHashMap.h>
#include <os/OsBSem.h>

// DEFINES

class HttpConnectionMapEntry : public UtlString
{
public:
    /// Constructor
    HttpConnectionMapEntry(const UtlString& name);
    
    /// Destructor
    virtual ~HttpConnectionMapEntry();
    
    OsSocket* mpSocket; //< pointer to a connection socket
    OsBSem              mLock;    //< protects access to the connection
    bool                mbInUse;  //< true if entry is in use, false if not
    static int          count;    //< used to udentify the entry
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
   
   /// Clear all entries in map. Close all sockets and delete them.
   void clearHttpConnectionMap();
   
   /// Return a map entry for an existing connection or NULL. Locks the connection if non-NULL
   HttpConnectionMapEntry* getPersistentConnection(const Url& url, OsSocket*& socket);
   
   /**<
    * @returns
    * - pointer to a connection map entry and a connection socket. If no entry exists for a 
    *   given URI one will be created and th socket pointer will be set to NULL.
    * - NULL if the connection does not exist
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
    
    /// Translate Url into key string that will be used for all further access
    void getPersistentUriKey(const Url& url, UtlString& key);       

    static HttpConnectionMap* pInstance; ///< pointer to the instance
    static OsBSem mLock;                 ///< protects access to map
    
    /// no copy constructor
    HttpConnectionMap(const HttpConnectionMap& nocopy);

    /// no assignment operator
    HttpConnectionMap& operator=(const HttpConnectionMap& noassignment);    
};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpConnectionMap_h_
