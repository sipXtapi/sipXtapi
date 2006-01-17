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
#include "net/XmlRpcDispatch.h"
#include "net/XmlRpcMethod.h"
#include "os/OsDateTime.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlLongLongInt.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlString.h"
#include "sipdb/RegistrationBinding.h"
#include "sipdb/RegistrationDB.h"
#include "SipRegistrar.h"
#include "SipRegistrarServer.h"
#include "SyncRpc.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* SyncRpc::PULL_UPDATES = "syncRpc.pullUpdates";
const char* SyncRpc::PUSH_UPDATES = "syncRpc.pushUpdates";
const char* SyncRpc::RESET        = "syncRpc.reset";

// STATIC VARIABLE INITIALIZATIONS

// Hash keys
UtlString SyncRpc::gNumUpdatesKey("numUpdates");
UtlString SyncRpc::gUpdatesKey("updates");


/**
 * SyncRpcMethodBase contains common code for SyncRpc methods
 */
class SyncRpcMethodBase : public XmlRpcMethod
{
protected:
   // Make the constructor protected so that this class cannot be instantiated directly
   SyncRpcMethodBase() {}

   ExecutionStatus authenticateCaller(
      const HttpRequestContext& requestContext, ///< request context
      const UtlString&          peerName,       ///< name of the peer who is calling
      XmlRpcResponse&           response)
      {
         // :HA: Authenticate: Make sure that the RPC call is from a peer that is on
         // our list of configured peers.
         return XmlRpcMethod::OK;               // return dummy success value for now
      }

   /// Given opaque userData, which is a SipRegistrar*, get and return SipRegistrarServer&
   SipRegistrarServer& getRegistrarServer(void *userData)
      {
         SipRegistrar* registrar = static_cast<SipRegistrar*>(userData);
         return registrar->getRegistrarServer();
      }
};


/// Implements the XML-RPC method syncRpc.pullUpdates

/*
 * Retrieve all updates for primaryRegistrar whose update number is greater than updateNumber.
 * 
 * Parameters  Type          Name                    Description
 *  Inputs:
 *             string        callingRegistrar        Calling registrar name
 *             string        primaryRegistrar        Primary registrar name
 *             intll         updateNumber
 *  Outputs:
 *             struct
 *               int         numUpdates
 *               array       updates
 *                 struct    row
 *                   string  uri
 *                   string  callid
 *                   int     cseq
 *                   string  contact
 *                   int     expires
 *                   string  qvalue
 *                   string  instanceId
 *                   string  gruu
 *                   string  primary
 *                   intll   updateNumber
 */
class SyncRpcPullUpdates : public SyncRpcMethodBase
{
public:
   static XmlRpcMethod* get()
      {
         return new SyncRpcPullUpdates();
      }

   SyncRpcPullUpdates() {}

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status)
      {
         // Verify that the callingRegistrar a configured peer registrar.
         UtlString* callingRegistrar = dynamic_cast<UtlString*>(params.at(0));
         status = authenticateCaller(requestContext, *callingRegistrar, response);

         // Retrieve all updates for the primaryRegistrar whose update number is greater 
         // than updateNumber.
         UtlString* primaryRegistrar = dynamic_cast<UtlString*>(params.at(0));
         UtlLongLongInt* updateNumber = dynamic_cast<UtlLongLongInt*>(params.at(1));
         UtlSList updates;    // list of RegistrationBindings
         SipRegistrarServer& registrarServer = getRegistrarServer(userData);
         int numUpdates = registrarServer.pullUpdates(*primaryRegistrar, *updateNumber, updates);

         // Fill in the response

         UtlHashMap output;
         output.insertKeyAndValue(new UtlString(SyncRpc::gNumUpdatesKey),
                                  new UtlInt(numUpdates));
         // convert updates from RegistrationBindings to HashMaps for XML-RPC
         UtlSList updateMaps;
         UtlSListIterator updateIter(updates);
         RegistrationBinding* binding = NULL;
         while ((binding = dynamic_cast<RegistrationBinding*>(updateIter())))
         {
            UtlHashMap* map = new UtlHashMap();
            binding->copy(*map);
            updateMaps.append(map);
         }
         output.insertKeyAndValue(new UtlString(SyncRpc::gUpdatesKey),
                                  &updateMaps);

         return true;
      }

private:
   /// no copy constructor
   SyncRpcPullUpdates(const SyncRpcPullUpdates& nocopy);

   /// no assignment operator
   SyncRpcPullUpdates& operator=(const SyncRpcPullUpdates& noassignment);
};


/// Implements the XML-RPC method syncRpc.pushUpdates

/*
 * Send registry updates to a peer registrar
 * 
 * Parameters  Type        Name                    Description
 *  Inputs:
 *             string      callingRegistrar        Calling registrar name
 *             intll       lastSentUpdateNumber    Number of last update sent
 *             array       updates
 *               struct
 *                 string  uri
 *                 string  callid
 *                 int     cseq
 *                 string  contact
 *                 int     expires
 *                 string  qvalue
 *                 string  instanceId
 *                 string  gruu
 *                 string  primary
 *                 intll   updateNumber
 *  Outputs:
 *             intll       updateNumber
 */
class SyncRpcPushUpdates : public SyncRpcMethodBase
{
public:
   static XmlRpcMethod* get()
      {
         return new SyncRpcPushUpdates();
      }

   SyncRpcPushUpdates() {}

protected:
   /**
    * Execute the pushUpdates RPC method: apply updates to the registration DB.
    */
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status)
      {
         // Verify that the caller is a configured peer registrar.
         UtlString* callingRegistrar = dynamic_cast<UtlString*>(params.at(0));
         status = authenticateCaller(requestContext, *callingRegistrar, response);
         
         // :HA: Also, the hostname that the request is coming from must match
         // the callingRegistrar input, since peers only push updates for which
         // they are the primary registrar.

         // :HA: Make sure that lastSentUpdateNumber matches PeerReceivedDbUpdateNumber.
         // Otherwise we're missing one or more updates.  If there is a mismatch, then
         // reject the update, and return a fault in order to trigger a reset.
         //UtlLongLongInt* lastSentUpdateNumber = dynamic_cast<UtlLongLongInt*>(params.at(1));

         UtlSList* updateMaps = dynamic_cast<UtlSList*>(params.at(2));
         UtlSListIterator updateIter(*updateMaps);

         // Iterate over the updates and convert RPC params to RegistrationBindings
         intll updateNumber = 0;
         UtlHashMap* update;
         UtlSList* updateList = new UtlSList();    // collect all the updates
         while ((update = dynamic_cast<UtlHashMap*>(updateIter())))
         {
            // Convert the update from a hash map to a RegistrationBinding object
            RegistrationBinding* reg = new RegistrationBinding(*update);

            // If this is not the first update, then make sure that it has the same update
            // number as previous updates.
            if (updateNumber == 0)
            {
               updateNumber = reg->getUpdateNumber();
            }
            else
            {
               if (!checkUpdateNumber(*reg, updateNumber, response, status))
               {
                  break;
               }
            }

            // Add the row to the update list
            updateList->append(reg);
         }

         // Apply the update to the DB
         if (status == XmlRpcMethod::OK)
         {
            int timeNow = OsDateTime::getSecsSinceEpoch();
            SipRegistrarServer& registrarServer = getRegistrarServer(userData);
            updateNumber = registrarServer.applyUpdatesToDirectory(timeNow, *updateList);
            UtlLongLongInt updateNumberWrapped(updateNumber);
            response.setResponse(&updateNumberWrapped);
         }

         return true;
      }

private:
   // Compare the binding's updateNumber with the expected number.
   // Return true if they match and false if they don't.
   // If there is a mismatch, then set up fault info in the RPC reponse.
   bool checkUpdateNumber(const RegistrationBinding& reg,
                          intll updateNumber,
                          XmlRpcResponse& response,
                          ExecutionStatus& status)
      {
         if (updateNumber != reg.getUpdateNumber())
         {
            UtlString faultMsg("a registry update contains multiple update numbers: ");
            faultMsg.append(updateNumber);
            faultMsg.append(", ");
            faultMsg.append(reg.getUpdateNumber());
            response.setFault(XmlRpcResponse::IllFormedContents, faultMsg.data());
            status = XmlRpcMethod::FAILED;
            return false;
         }
         else
         {
            return true;
         }
      }

   /// no copy constructor
   SyncRpcPushUpdates(const SyncRpcPushUpdates& nocopy);

   /// no assignment operator
   SyncRpcPushUpdates& operator=(const SyncRpcPushUpdates& noassignment);
};


class SyncRpcReset : public SyncRpcMethodBase
{
public:
   static XmlRpcMethod* get()
      {
         return new SyncRpcReset();
      }

   SyncRpcReset() {}

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status)
      {
         return false;    // :HA: implement this method
      }

private:
   /// no copy constructor
   SyncRpcReset(const SyncRpcReset& nocopy);

   /// no assignment operator
   SyncRpcReset& operator=(const SyncRpcReset& noassignment);
};


// Constructor
SyncRpc::SyncRpc(XmlRpcDispatch& xmlRpcDispatch,
                 SipRegistrar& registrar) :
   mXmlRpcDispatch(xmlRpcDispatch),
   mRegistrar(registrar)
{
}

// Destructor
SyncRpc::~SyncRpc()
{
}

XmlRpcDispatch& SyncRpc::getXmlRpcDispatch()
{
   return mXmlRpcDispatch;
}

void SyncRpc::registerMethod(Method methodEnum)
{
   // register the method class that matches the enum value
   switch(methodEnum)
   {
   case PullUpdates:
      mXmlRpcDispatch.addMethod(PULL_UPDATES, SyncRpcPullUpdates::get, &mRegistrar);
      break;
   case PushUpdates:
      mXmlRpcDispatch.addMethod(PUSH_UPDATES, SyncRpcPushUpdates::get, &mRegistrar);
      break;
   case Reset:
      mXmlRpcDispatch.addMethod(RESET, SyncRpcReset::get, &mRegistrar);
      break;
   default:
      assert(false);
      break;
   }
}
