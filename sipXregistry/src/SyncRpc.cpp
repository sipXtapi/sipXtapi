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
#include "RegistrarPeer.h"
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
RegistrarPeer* SyncRpcMethod::authenticateCaller(
   const HttpRequestContext& requestContext, ///< request context
   const UtlString&          peerName,       ///< name of the peer who is calling
   XmlRpcResponse&           response,       ///< response to put fault in
   SipRegistrar&             registrar       ///< registrar
                                                  )
{
   // Authenticate: Make sure that the RPC call is from a peer that is on
   // our list of configured peers and that SSL confirms that the connection
   // is from that peer.
   assert(peerName);
   RegistrarPeer* peer = NULL;
   
   if (requestContext.isTrustedPeer(peerName))
   {
      // ssl says the connection is from the named host
      peer = registrar.getPeer(peerName); // so look it up in the peer table
      if (peer)
      {
         // all is well
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SyncRpc::authenticateCaller '%s' peer authenticated",
                       peerName.data()
                       );
      }
      else
      {
         // this peer is authenticated, but not configured, so provide a good error response
         UtlString faultMsg;
         faultMsg.append("Unconfigured peer '");
         faultMsg.append(peerName);
         faultMsg.append("'");
         response.setFault(SyncRpcMethod::UnconfiguredPeer, faultMsg.data());
            
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SyncRpcMethod::authenticateCaller '%s' not a configured sync peer",
                       peerName.data()
                       );
      }
   }
   else
   {
      // ssl says not authenticated - provide only a generic error
      UtlString faultMsg;
      faultMsg.append("Authentication Failure");
      response.setFault(XmlRpcResponse::AuthenticationRequired, faultMsg.data());
            
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SyncRpcMethod::authenticateCaller '%s' failed SSL authentication",
                    peerName.data()
                    );
   }
   
   return peer;
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
   bool result = false;

   UtlString* callingRegistrar = dynamic_cast<UtlString*>(params.at(0));
   if (callingRegistrar && !callingRegistrar->isNull())
   {
      SipRegistrar* registrar = static_cast<SipRegistrar*>(userData);
      RegistrarPeer* peer = authenticateCaller(requestContext, *callingRegistrar,
                                               response, *registrar);
      if (peer)
      {
            UtlLongLongInt* updateNumber = dynamic_cast<UtlLongLongInt*>(params.at(1));
            if (updateNumber)
            {
               peer->setSentTo(updateNumber->getValue());
               UtlLongLongInt returnedUpdateNumber(peer->receivedFrom());
               peer->markReachable();
               response.setResponse(&returnedUpdateNumber);
               result = true;
            }
            else
            {
               handleMissingExecuteParam(METHOD_NAME, "updateNumber", response, status, peer);
            }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SyncRpcReset::execute '%s' not authenticated by SSL",
                       callingRegistrar->data()
                       );
      }
   }
   else
   {
      handleMissingExecuteParam(METHOD_NAME, "callingRegistrar", response, status);
   }


   return result;
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
      }
   }
   else
   {
      // fault returned - we are now unsynchronized
      int faultCode;
      UtlString faultText;

      response.getFault(&faultCode, faultText);
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SyncRpcReset::invoke : fault %d %s"
                    " %s is now marked UnReachable",
                    faultCode, faultText.data(), peer.name()
                    );
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

// Fetch all unfetched updates for a given primary from a peer.
// Pulling updates happens during startup, when peers are in the Uninitialized state.
// Peers can transition to UnReachable or Incompatible if there is an error, but the
// transition to Reachable cannot happen until later.
RegistrarPeer::SynchronizationState SyncRpcPullUpdates::invoke(
   RegistrarPeer* source,       ///< peer to pull from
   const char*    myName,       ///< primary name of this registrar
   const char*    primaryName,  ///< name of registrar whose updates we want
   intll          updateNumber, ///< pull updates starting after this number
   UtlSList*      bindings      ///< list of RegistrationBinding 
                                                               )
{
   RegistrarPeer::SynchronizationState resultState = RegistrarPeer::Uninitialized; 

   // check inputs
   assert(source);
   assert(primaryName);
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
   UtlString primary(primaryName);
   request.addParam(&primary);

   // third parameter is the update number after which we want all updates
   UtlLongLongInt updateNumberBoxed(updateNumber);
   request.addParam(&updateNumberBoxed);
   
   // make the request
   XmlRpcResponse response;
   if (request.execute(response)) // blocks; returns false for any fault
   {
      // Apparently successful, so validate the response and extract and return updates.
      UtlContainable* value;
      response.getResponse(value);
      UtlHashMap* responseStruct = dynamic_cast<UtlHashMap*>(value);
      if ( responseStruct )
      {
         UtlInt* numUpdates = dynamic_cast<UtlInt*>(responseStruct->findValue(&NUM_UPDATES));
         if ( numUpdates )
         {
            if (numUpdates->getValue())
            {
               UtlSList* responseUpdates =
                  dynamic_cast<UtlSList*>(responseStruct->findValue(&UPDATES));
               if (responseUpdates)
               {
                  int actualUpdateCount = responseUpdates->entries();
                  if (actualUpdateCount == numUpdates->getValue())
                  {
                     OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                   "SyncRpcPullUpdates::invoke : '%s' returned %d updates",
                                   source->name(), actualUpdateCount
                                   );

                     UtlSListIterator updates(*responseUpdates);
                     UtlHashMap* update;
                     while ((update = dynamic_cast<UtlHashMap*>(updates())))
                     {
                        // add this to the returned bindings list
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
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SyncRpcPullUpdates::invoke : no updates returned by '%s'",
                             source->name()
                             );
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
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "SyncRpcPullUpdates::invoke : no response data returned : "
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
                    "SyncRpcPullUpdates::invoke : fault %d %s"
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
   ExecutionStatus& status)
{
   bool result = false;

   UtlString* callingRegistrar = dynamic_cast<UtlString*>(params.at(0));
   if (callingRegistrar && !callingRegistrar->isNull())
   {
      // Verify that the callingRegistrar is a configured peer registrar
      SipRegistrar* registrar = static_cast<SipRegistrar*>(userData);
      RegistrarPeer* peer = authenticateCaller(requestContext, *callingRegistrar, response, *registrar);
      if (peer)
      {
         // Retrieve all updates for the primaryRegistrar whose update number is greater 
         // than updateNumber
         UtlString* primaryRegistrar = dynamic_cast<UtlString*>(params.at(1));
         if (primaryRegistrar && !primaryRegistrar->isNull())
         {
            UtlLongLongInt* updateNumber = dynamic_cast<UtlLongLongInt*>(params.at(2));
            if (updateNumber != NULL)
            {
               UtlSList updates;    // list of RegistrationBindings
               SipRegistrarServer& registrarServer = registrar->getRegistrarServer();
               int numUpdates =
                  registrarServer.pullUpdates(*primaryRegistrar, *updateNumber, updates);

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
               response.setResponse(&output);
               result = true;
            }
            else
            {
               handleMissingExecuteParam(METHOD_NAME, "updateNumber", response, status, peer);
            }
         }
         else
         {
            handleMissingExecuteParam(METHOD_NAME, "primaryRegistrar", response, status, peer);
         }
      }
      else
      {
         /*
          * Either authentication or peer lookup failed
          *   errors already logged, and response is set up
          *
          * Note - we don't use REQUIRE_AUTHENTICATION
          * because that requests HTTP digest authentication,
          * which won't help
          */
         status = XmlRpcMethod::FAILED;
      }
   }
   else
   {
      handleMissingExecuteParam(METHOD_NAME, "primaryRegistrar", response, status);
   }

   return result;
}

void SyncRpcMethod::handleMissingExecuteParam(const char* methodName,
                                              const char* paramName,
                                              XmlRpcResponse& response,
                                              ExecutionStatus& status,
                                              RegistrarPeer* peer)
{
   UtlString faultMsg;
   faultMsg += methodName;
   faultMsg += " '";
   faultMsg += paramName;
   faultMsg += "' parameter is missing or invalid type";
   status = XmlRpcMethod::FAILED;
   if (peer != NULL)
   {
      // Mark the peer incompatible since we can't understand what it's saying.
      // Log that as part of the fault message.
      peer->markIncompatible();

      faultMsg += ": '";
      faultMsg += peer->name();
      faultMsg += "' marked incompatible for replication";
   }
   response.setFault(SyncRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_SIP, PRI_CRIT, faultMsg);
   assert(false);    // bad XML-RPC response
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
   bool result = false;

   UtlString* callingRegistrar = dynamic_cast<UtlString*>(params.at(0));
   if (callingRegistrar && !callingRegistrar->isNull())
   {
      // Verify that the caller is a configured peer registrar.
      SipRegistrar* registrar = static_cast<SipRegistrar*>(userData);
      RegistrarPeer* peer =
         authenticateCaller(requestContext, *callingRegistrar, response, *registrar);
      if (peer)
      {
         UtlLongLongInt* lastSentUpdateNumber = dynamic_cast<UtlLongLongInt*>(params.at(1));
         if (lastSentUpdateNumber != NULL)
         {
            // check lastSentUpdateNumber <= peerReceivedDbUpdateNumber
            // sets status and handles any errors
            checkLastSentUpdateNumber(*lastSentUpdateNumber, *peer, response, status);

            if (status == XmlRpcMethod::OK)
            {
               UtlSList* updateMaps = dynamic_cast<UtlSList*>(params.at(2));
               if (updateMaps != NULL)
               {
                  result = applyPushedUpdates(*updateMaps, response, status, *peer, *registrar);
               }
               else
               {
                  handleMissingExecuteParam(METHOD_NAME, "updates", response, status, peer);
               }
            }
         }
         else
         {
            handleMissingExecuteParam(
               METHOD_NAME, "lastSentUpdateNumber", response, status, peer);
         }
      }
      else
      {
         /*
          * Either authentication or peer lookup failed
          *   errors already logged, and response is set up
          *
          * Note - we don't use REQUIRE_AUTHENTICATION
          * because that requests HTTP digest authentication,
          * which won't help
          */
         status = XmlRpcMethod::FAILED;
      }
   }     
   else
   {
      handleMissingExecuteParam(METHOD_NAME, "callingRegistrar", response, status);
   }

   return result;
}

// This method is the guts of SyncRpcPushUpdates::execute, separated from all the
// error-checking noise.
bool SyncRpcPushUpdates::applyPushedUpdates(UtlSList&        updateMaps,
                                            XmlRpcResponse&  response,
                                            ExecutionStatus& status,
                                            RegistrarPeer&   peer,
                                            SipRegistrar&    registrar)
{
   bool result = false;

   if (updateMaps.entries() == 0)
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipRegistrarServer::applyPushedUpdates empty updates list");
      return result;
   }

   UtlSListIterator updateIter(updateMaps);

   // Iterate over the updates and convert RPC params to RegistrationBindings
   intll updateNumber = 0;
   UtlHashMap* update;
   UtlSList* updateList = new UtlSList();    // collect all the updates
   status = XmlRpcMethod::OK;
   while ((update = dynamic_cast<UtlHashMap*>(updateIter())) &&
          (status == XmlRpcMethod::OK))
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
         // on error, sets status to XmlRpcMethod::FAILED and does other error handling
         checkUpdateNumber(*reg, updateNumber, peer, response, status);
      }

      if (status == XmlRpcMethod::OK)
      {
         // Add the row to the update list
         updateList->append(reg);
      }
   }

   // Apply the update to the DB
   if (status == XmlRpcMethod::OK)    // if there were no errors earlier
   {
      int timeNow = OsDateTime::getSecsSinceEpoch();
      SipRegistrarServer& registrarServer = registrar.getRegistrarServer();
      UtlString errorMsg;
      updateNumber = registrarServer.applyUpdatesToDirectory(timeNow, *updateList, &errorMsg);
      UtlLongLongInt updateNumberWrapped(updateNumber);
      response.setResponse(&updateNumberWrapped);
      if (updateNumber > 0)
      {
         result = true;    // success
      }
      else
      {
         status = XmlRpcMethod::FAILED;
         errorMsg.insert(0, 
                         "SyncRpcPushUpdates::applyPushedUpdates error applying updates ");
         OsSysLog::add(FAC_SIP, PRI_ERR, errorMsg.data());
         peer.markUnReachable();
         response.setFault(SyncRpcMethod::UpdateFailed, errorMsg);
      }
   }

   return result;
}

// Check lastSentUpdateNumber <= peerReceivedDbUpdateNumber, otherwise updates are missing
// If everything is OK, set status to XmlRpcMethod::OK.
// Otherwise mark the response and set status to an error.
void SyncRpcPushUpdates::checkLastSentUpdateNumber(intll lastSentUpdateNumber,
                                                   RegistrarPeer& peer,
                                                   XmlRpcResponse& response,
                                                   ExecutionStatus& status)
{
   intll peerReceivedDbUpdateNumber = peer.receivedFrom();
   if (lastSentUpdateNumber <= peerReceivedDbUpdateNumber)
   {
      status = XmlRpcMethod::OK;
   }
   else
   {
      // Updates are missing.  This should be rare, but is possible under normal
      // operation.  Log an error and mark the peer unreachable.  The reset machinery
      // will get us back in sync.
      status = XmlRpcMethod::FAILED;
      char buf[1024];
      sprintf(buf, 
              "SyncRpcPushUpdates::checkLastSentUpdateNumber "
              "lastSentUpdateNumber = %lld but peerReceivedDbUpdateNumber = %lld",
              lastSentUpdateNumber, peerReceivedDbUpdateNumber);
      OsSysLog::add(FAC_SIP, PRI_WARNING, buf);
      peer.markUnReachable();
      response.setFault(SyncRpcMethod::UpdateOutOfOrder, buf);
   }
}

// Compare the binding's updateNumber with the expected number.
// Return true if they match and false if they don't.
// If there is a mismatch, then set up fault info in the RPC reponse.
bool SyncRpcPushUpdates::checkUpdateNumber(
   const RegistrationBinding& reg,
   intll updateNumber,
   RegistrarPeer& peer,
   XmlRpcResponse& response,
   ExecutionStatus& status
                                           )
{
   if (updateNumber != reg.getUpdateNumber())
   {
      const char* msg = 
         "SyncRpcPushUpdates::execute: "
         "a registry update contains multiple update numbers: %lld and %lld";
      char buf[1024];
      int msgLen = sprintf(buf, msg, updateNumber, reg.getUpdateNumber());
      assert(msgLen > 0);
      peer.markIncompatible();    // this peer is confused, don't talk to him
      response.setFault(SyncRpcMethod::MixedUpdateNumbers, buf);
      status = XmlRpcMethod::FAILED;
      assert(false);              // should never happen
      return false;
   }
   else
   {
      return true;
   }
}


/// Push one update to a given peer
RegistrarPeer::SynchronizationState
SyncRpcPushUpdates::invoke(RegistrarPeer* peer,       ///< peer to push to
                           const char*    myName,     ///< primary name of this registrar
                           UtlSList*      bindings    ///< list of RegistrationBinding 
                           )
{
   RegistrarPeer::SynchronizationState resultState; 

   // check inputs
   assert(peer);
   assert(myName && *myName != '\000');
   assert(!bindings->isEmpty());

   // get the target URI
   Url peerUrl;
   peer->rpcURL(peerUrl);

   // construct the request to be sent
   XmlRpcRequest request(peerUrl, METHOD_NAME);

   // first parameter is my own name
   UtlString primaryName(myName);
   request.addParam(&primaryName);

   // second parameter is the updateNumber of the last update we sent to the peer
   UtlLongLongInt lastSentUpdateNumber(peer->sentTo());
   request.addParam(&lastSentUpdateNumber);

   // third parameter is a list of registration bindings
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
   if (request.execute(response))    // blocks; returns false for any fault
   {
      // The request succeeded, so increase peerSentDbUpdateNumber accordingly.
      // :LATER: Consider moving this code out of here (into SipRegistrarServer?
      // into RegistrarSync) since the RPC method should really just be handling comms.
      UtlContainable* value;
      response.getResponse(value);
      UtlLongLongInt* newSentUpdateNumber = dynamic_cast<UtlLongLongInt*>(value);
      if (newSentUpdateNumber)
      {
         peer->setSentTo(newSentUpdateNumber->getValue());
         resultState = RegistrarPeer::Reachable;
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "SyncRpcPushUpdates::invoke : invalid response "
                       " %s marked incompatible for replication",
                       peer->name()
                       );
         assert(false);    // bad XML-RPC response
         resultState = RegistrarPeer::Incompatible;
      }
   }
   else
   {
      // fault returned - we are now unsynchronized
      peer->markUnReachable();
      resultState = RegistrarPeer::UnReachable;
   }

   return resultState;
}
