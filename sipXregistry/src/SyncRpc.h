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
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;
class XmlRpcDispatch;


/// XML-RPC interface for synchronizing multiple registrars
class SyncRpc {
public:
   /// RPC method identifiers
   typedef enum
   {
       PullUpdates      ///< pull database updates
      ,PushUpdates      ///< push database updates
      ,Reset            ///< reset state with peer
      ,NumMethods
   } Method;

   // Hash keys
   static UtlString gNumUpdatesKey;
   static UtlString gUpdatesKey;

   SyncRpc(XmlRpcDispatch& xmlRpcDispatch,
           SipRegistrar& registrar);
   ~SyncRpc();

   XmlRpcDispatch& getXmlRpcDispatch();

   /// Register the XML-RPC method identified by the Method enum value
   void registerMethod(Method method);

protected:
   // Method names
   static const char* PULL_UPDATES;
   static const char* PUSH_UPDATES;
   static const char* RESET;

   XmlRpcDispatch& mXmlRpcDispatch;
   SipRegistrar& mRegistrar;

private:
   /// no copy constructor
   SyncRpc(const SyncRpc& nocopy);

   /// no assignment operator
   SyncRpc& operator=(const SyncRpc& noassignment);
};

#endif // _SYNCRPC_H_
