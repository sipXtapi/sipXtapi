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
#include "os/OsDateTime.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlLongLongInt.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlString.h"
#include "net/XmlRpcDispatch.h"
#include "net/XmlRpcMethod.h"
#include "net/XmlRpcRequest.h"
#include "sipdb/RegistrationBinding.h"
#include "sipdb/RegistrationDB.h"
#include "SipRegistrar.h"
#include "SipRegistrarServer.h"
#include "SyncRpc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/*****************************************************************
 **** SyncRpcMethod contains common code for SyncRpc methods
 *****************************************************************/
const char* SyncRpcMethod::METHOD_NAME = "SyncRpcMethod.BASE";

XmlRpcMethod* SyncRpcMethod::get()
{
   assert(false);  // this should have been overridden in the subclass

   return NULL;
}

/// Get the name of the XML-RPC method.
const char* SyncRpcMethod::name()
{
   assert(false);  // this should have been overridden in the subclass

   return METHOD_NAME;
}

/// Register this method with the XmlRpcDispatch object so it can be called.
void SyncRpcMethod::registerSelf(SipRegistrar&   registrar)
{
   assert(false);  // this should have been overridden in the subclass
}

/// Constructor
SyncRpcMethod::SyncRpcMethod()
{
}

/// Common method for registration with the XML-RPC dispatch mechanism
void SyncRpcMethod::registerMethod(const char*        methodName,
                                   XmlRpcMethod::Get  getMethod,
                                   SipRegistrar&      registrar
                                   )
{
   registrar.getXmlRpcDispatch()->addMethod(methodName, getMethod, &registrar);
}

/// The execute method called by XmlRpcDispatch
bool SyncRpcMethod::execute(const HttpRequestContext& requestContext, ///< request context
                            UtlSList& params,                         ///< request param list
                            void* userData,                           ///< user data
                            XmlRpcResponse& response,                 ///< request response
                            ExecutionStatus& status
                            )
{
   assert(false); // this should have been overridden in the subclass

   return false;
}

/// Common method to do peer authentication
XmlRpcMethod::ExecutionStatus SyncRpcMethod::authenticateCaller(
   const HttpRequestContext& requestContext, ///< request context
   const UtlString&          peerName,       ///< name of the peer who is calling
   XmlRpcResponse&           response        ///< response to put fault in
                                                  )
{
   // :TODO: Authenticate: Make sure that the RPC call is from a peer that is on
   // our list of configured peers, and that this request is that peer.

   return XmlRpcMethod::OK;               // return dummy success value for now
}


/*****************************************************************
 **** SyncRpcReset
 *****************************************************************/

const char* SyncRpcReset::METHOD_NAME = "registerSync.reset";

XmlRpcMethod* SyncRpcReset::get()
{
   return new SyncRpcReset();
}

/// Get the name of the XML-RPC method.
const char* SyncRpcReset::name()
{
   return METHOD_NAME;
}

/// Register this method with the XmlRpcDispatch object so it can be called.
void SyncRpcReset::registerSelf(SipRegistrar&   registrar)
{
   registerMethod(METHOD_NAME, SyncRpcReset::get, registrar);
}

/// Constructor
SyncRpcReset::SyncRpcReset()
{
}

bool SyncRpcReset::execute(const HttpRequestContext& requestContext, ///< request context
                           UtlSList& params,                         ///< request param list
                           void* userData,                           ///< user data
                           XmlRpcResponse& response,                 ///< request response
                           ExecutionStatus& status)
{
   return false;                // :TODO: implement this method
}

/// Reset the SynchronizationState and update numbers with respect to some peer.
RegistrarPeer::SynchronizationState
SyncRpcReset::invoke(const char*    myName, ///< primary name of the caller
                     RegistrarPeer& peer    ///< the peer to be reset
                     )
{
   RegistrarPeer::SynchronizationState resultState; 

   // check inputs
   assert(myName && *myName != '\000');

   // get the target URI
   Url targetURI;
   peer.rpcURL(targetURI);

   // construct the request to be sent
   XmlRpcRequest request(targetURI, METHOD_NAME);

   // first parameter is my own name
   UtlString callerName(myName);
   request.addParam(&callerName);

   // second parameter is our PeerReceivedDbUpdateNumber for the peer
   UtlLongLongInt receivedUpdate(peer.receivedFrom());
   request.addParam(&receivedUpdate);
   
   // make the request
   XmlRpcResponse response;
   if (request.execute(response)) // blocks; returns false for any fault
   {
      // Apparently successfull, so update the PeerSentDbUpdateNumber for this peer
      UtlContainable* value;
      response.getResponse(value);

      // response is the PeerSentDbUpdateNumber for the peer
      UtlLongLongInt* sentTo = dynamic_cast<UtlLongLongInt*>(value);
      if ( sentTo )
      {
         peer.setSentTo(*sentTo);

         // all is well
         resultState = RegistrarPeer::Reachable;
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "SyncRpcReset::invoke : no update number returned : "
                       " %s marked incompatible for replication",
                       peer.name()
                       );
         assert(false); // bad xmlrpc response
         resultState = RegistrarPeer::Incompatible;
         peer.markIncompatible();
      }
   }
   else
   {
      // fault returned - we are now unsynchronized
      int faultCode;
      UtlString faultText;

      response.getFault(&faultCode, faultText);
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SyncRpcReset::invoke : fault %d %s\n"
                    " %s is now marked UnReachable",
                    faultCode, faultText.data(), peer.name()
                    );
      peer.markUnReachable();
      resultState = RegistrarPeer::UnReachable;
   }

   return resultState;
}


/*****************************************************************
 **** SyncRpcPullUpdates implements XML-RPC registrarSync.pullUpdates
 *****************************************************************/

const char*     SyncRpcPullUpdates::METHOD_NAME = "registerSync.pullUpdates";
const UtlString SyncRpcPullUpdates::NUM_UPDATES("num_updates");
const UtlString SyncRpcPullUpdates::UPDATES("updates");
   

XmlRpcMethod* SyncRpcPullUpdates::get()
{
   return new SyncRpcPullUpdates();
}

/// Get the name of the XML-RPC method.
const char* SyncRpcPullUpdates::name()
{
   return METHOD_NAME;
}

/// Register this method with the XmlRpcDispatch object so it can be called.
void SyncRpcPullUpdates::registerSelf(SipRegistrar&   registrar)
{
   registerMethod(METHOD_NAME, SyncRpcPullUpdates::get, registrar);
}

/// Constructor
SyncRpcPullUpdates::SyncRpcPullUpdates()
{
}

/// Fetch all unfetched updates for a given primary from a peer
RegistrarPeer::SynchronizationState SyncRpcPullUpdates::invoke(
   RegistrarPeer* source,  ///< peer to pull from
   const char*    myName,  ///< primary name of this registrar
   RegistrarPeer* primary, ///< whose records to pull
   UtlSList*      bindings ///< list of RegistrationBinding 
                                                               )
{
   RegistrarPeer::SynchronizationState resultState; 

   // check inputs
   assert(source);
   assert(primary);
   assert(myName && *myName != '\000');
   assert(bindings->isEmpty());

   // get the target URI
   Url peer;
   source->rpcURL(peer);

   // construct the request to be sent
   XmlRpcRequest request(peer, METHOD_NAME);

   // first parameter is my own name
   UtlString callerName(myName);
   request.addParam(&callerName);

   // second parameter is the name of the registrar whose records we want
   request.addParam(primary);

   // third parameter is the PeerReceivedDbUpdateNumber from the primary we want
   UtlLongLongInt received(primary->receivedFrom());
   request.addParam(&received);
   
   // make the request
   XmlRpcResponse response;
   if (request.execute(response)) // blocks; returns false for any fault
   {
      // Apparently successful, so update the PeerSentDbUpdateNumber for this peer
      UtlContainable* value;
      response.getResponse(value);
      UtlHashMap* responseStruct = dynamic_cast<UtlHashMap*>(value);
      UtlInt*     numUpdates;
      if (   responseStruct
          && (numUpdates
              = dynamic_cast<UtlInt*>(responseStruct->find(&NUM_UPDATES))
              )
          )
      {
         resultState = RegistrarPeer::Reachable;
         if (numUpdates->getValue())
         {
            UtlSList* responseUpdates =
               dynamic_cast<UtlSList*>(responseStruct->find(&UPDATES));
            if (responseUpdates)
            {
               int actualUpdateCount = responseUpdates->entries();
               if (actualUpdateCount == numUpdates->getValue())
               {
                  UtlSListIterator updates(*responseUpdates);
                  UtlHashMap* update;
                  while ((update = dynamic_cast<UtlHashMap*>(updates())))
                  {
                     bindings->append(new RegistrationBinding(*update));
                  }
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_CRIT,
                                "SyncRpcPullUpdates::invoke : "
                                "inconsistent number of updates %d != %d : "
                                " %s marked incompatible for replication",
                                numUpdates->getValue(),
                                responseUpdates->entries(),
                                source->name()
                                );
                  assert(false); // bad xmlrpc response
                  resultState = RegistrarPeer::Incompatible;
                  source->markIncompatible();
               }
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_CRIT,
                             "SyncRpcPullUpdates::invoke : no updates element found  "
                             " %s marked incompatible for replication",
                             source->name()
                             );
               assert(false); // bad xmlrpc response
               resultState = RegistrarPeer::Incompatible;
               source->markIncompatible();
            }
         }
         else
         {
            // no updates - all is well
            resultState = RegistrarPeer::Reachable;
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "SyncRpcPullUpdates::invoke : no numUpdates element found : "
                       " %s marked incompatible for replication",
                       source->name()
                       );
         assert(false); // bad xmlrpc response
         resultState = RegistrarPeer::Incompatible;
         source->markIncompatible();
      }
   }
   else
   {
      // fault returned - we are now unsynchronized
      int faultCode;
      UtlString faultText;

      response.getFault(&faultCode, faultText);
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SyncRpcPullUpdates::invoke : fault %d %s\n"
                    " %s is now marked UnReachable",
                    faultCode, faultText.data(), source->name()
                    );
      source->markUnReachable();
      resultState = RegistrarPeer::UnReachable;
   }

   return resultState;
}

/// The execute method called by XmlRpcDispatch
bool SyncRpcPullUpdates::execute(
   const HttpRequestContext& requestContext, ///< request context
   UtlSList& params,                         ///< request param list
   void* userData,                           ///< user data
   XmlRpcResponse& response,                 ///< request response
   ExecutionStatus& status
                                 )
{
   // Verify that the callingRegistrar is a configured peer registrar.
   UtlString* callingRegistrar = dynamic_cast<UtlString*>(params.at(0));
   status = authenticateCaller(requestContext, *callingRegistrar, response);
         
   // Retrieve all updates for the primaryRegistrar whose update number is greater 
   // than updateNumber.
   UtlString* primaryRegistrar = dynamic_cast<UtlString*>(params.at(1));
   UtlLongLongInt* updateNumber = dynamic_cast<UtlLongLongInt*>(params.at(2));
   UtlSList updates;    // list of RegistrationBindings
   SipRegistrar* registrar = static_cast<SipRegistrar*>(userData);
   SipRegistrarServer& registrarServer = registrar->getRegistrarServer();
   int numUpdates = registrarServer.pullUpdates(*primaryRegistrar, *updateNumber, updates);

   // Fill in the response

   UtlHashMap output;
   output.insertKeyAndValue(new UtlString(NUM_UPDATES),
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
   output.insertKeyAndValue(new UtlString(UPDATES),
                            &updateMaps);

   return true;
}



/*****************************************************************
 **** SyncRpcPushUpdates
 *****************************************************************/

const char* SyncRpcPushUpdates::METHOD_NAME = "registerSync.pushUpdates";

XmlRpcMethod* SyncRpcPushUpdates::get()
{
   return new SyncRpcPushUpdates();
}

/// Get the name of the XML-RPC method.
const char* SyncRpcPushUpdates::name()
{
   return METHOD_NAME;
}

/// Register this method with the XmlRpcDispatch object so it can be called.
void SyncRpcPushUpdates::registerSelf(SipRegistrar&   registrar)
{
   registerMethod(METHOD_NAME, SyncRpcPushUpdates::get, registrar);
}

/// Constructor
SyncRpcPushUpdates::SyncRpcPushUpdates()
{
}

bool SyncRpcPushUpdates::execute(
   const HttpRequestContext& requestContext, ///< request context
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
   // :TODO: do this check - UtlLongLongInt* lastSentUpdateNumber = dynamic_cast<UtlLongLongInt*>(params.at(1));

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
      SipRegistrar* registrar = static_cast<SipRegistrar*>(userData);
      SipRegistrarServer& registrarServer = registrar->getRegistrarServer();
      updateNumber = registrarServer.applyUpdatesToDirectory(timeNow, *updateList);
      UtlLongLongInt updateNumberWrapped(updateNumber);
      response.setResponse(&updateNumberWrapped);
   }

   return true;
}

// Compare the binding's updateNumber with the expected number.
// Return true if they match and false if they don't.
// If there is a mismatch, then set up fault info in the RPC reponse.
bool SyncRpcPushUpdates::checkUpdateNumber(
   const RegistrationBinding& reg,
   intll updateNumber,
   XmlRpcResponse& response,
   ExecutionStatus& status
                                           )
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


/// Push one update to a given peer
RegistrarPeer::SynchronizationState
SyncRpcPushUpdates::invoke(RegistrarPeer* replicated, ///< peer to push to
                           const char*    myName,     ///< primary name of this registrar
                           UtlSList*      bindings    ///< list of RegistrationBinding 
                           )
{
   RegistrarPeer::SynchronizationState resultState; 

   // check inputs
   assert(replicated);
   assert(myName && *myName != '\000');
   assert(!bindings->isEmpty());

   // get the target URI
   Url peer;
   replicated->rpcURL(peer);

   // construct the request to be sent
   XmlRpcRequest request(peer, METHOD_NAME);

   // first parameter is my own name
   UtlString primaryName(myName);
   request.addParam(&primaryName);

   // second parameter is a list of registration bindings
   UtlSListIterator bindingIterator(*bindings);
   UtlSList bindingParamList;
   RegistrationBinding* binding;
   while ((binding = dynamic_cast<RegistrationBinding*>(bindingIterator())))
   {
      UtlHashMap* toMap = new UtlHashMap;
      binding->copy(*toMap);
      bindingParamList.append(toMap);
   }
   request.addParam(&bindingParamList);
   
   // make the request
   XmlRpcResponse response;
   if (request.execute(response)) // blocks; returns false for any fault
   {
      // Apparently successful, so update the PeerSentDbUpdateNumber for this peer
      UtlContainable* value;
      response.getResponse(value);
      UtlLongLongInt* newSentUpdateNumber = dynamic_cast<UtlLongLongInt*>(value);
      if (newSentUpdateNumber)
      {
         replicated->setSentTo(newSentUpdateNumber->getValue());
         resultState = RegistrarPeer::Reachable;
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "SyncRpcPushUpdates::invoke : invalid response "
                       " %s marked incompatible for replication",
                       replicated->name()
                       );
         assert(false); // bad xmlrpc response
         resultState = RegistrarPeer::Incompatible;
      }
   }
   else
   {
      // fault returned - we are now unsynchronized
      replicated->markUnReachable();
      resultState = RegistrarPeer::UnReachable;
   }

   return resultState;
}
