// $Id$
//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "SyncRpc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

const char* MethodName[SyncRpc::NumMethods] =
{
   "syncRpc.check"
   ,"syncRpc.pullUpdates"
   ,"syncRpc.pushUpdates"
};

// STATIC VARIABLE INITIALIZATIONS

bool SyncRpc::sRegistered = false;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

// Constructor
SyncRpc::SyncRpc()
{
}

// Destructor
SyncRpc::~SyncRpc()
{
}


class SyncRpc_check : public XmlRpcMethod
{
public:
   static XmlRpcMethod* get()
      {
         return new SyncRpc_check();
      }

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status)
      {
         return false;    // :TODO: implement this method
      }
};

class SyncRpc_pullUpdates : public XmlRpcMethod
{
public:
   static XmlRpcMethod* get()
      {
         return new SyncRpc_pullUpdates();
      }

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status)
      {
         return false;    // :TODO: implement this method
      }
};

class SyncRpc_pushUpdates : public XmlRpcMethod
{
public:
   static XmlRpcMethod* get()
      {
         return new SyncRpc_pushUpdates();
      }

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status)
      {
         return false;    // :TODO: implement this method
      }
};



// Must be called once to connect the configurationParameter methods
void SyncRpc::registerMethods(XmlRpcDispatch& rpc /* xmlrpc dispatch service to use */)
{
   if (!sRegistered)
   {
      rpc.addMethod(MethodName[Check],       SyncRpc_check::get,       NULL);
      rpc.addMethod(MethodName[PullUpdates], SyncRpc_pullUpdates::get, NULL);
      rpc.addMethod(MethodName[PushUpdates], SyncRpc_pushUpdates::get, NULL);

      sRegistered = true;
   }
}
