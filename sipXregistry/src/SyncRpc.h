// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _SYNCRPC_H_
#define _SYNCRPC_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/XmlRpcDispatch.h"
#include "net/XmlRpcMethod.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// XML-RPC interface for synchronizing multiple registrars
class SyncRpc {
public:
   SyncRpc();
   ~SyncRpc();

   /// RPC method identifiers
   typedef enum
      {
         Check
         ,PullUpdates
         ,PushUpdates
         ,NumMethods
      } Method;


   /// Must be called once to connect the RPC methods
   static void registerMethods(XmlRpcDispatch& rpc /**< xmlrpc dispatch service to use */);
   
private:
   static bool       sRegistered;    /**< whether or not the SyncRpc methods have been
                                      *   registered with XmlRpcDispatch
                                      */

   /// no copy constructor
   SyncRpc(const SyncRpc& nocopy);

   /// no assignment operator
   SyncRpc& operator=(const SyncRpc& noassignment);
};

#endif // _SYNCRPC_H_
