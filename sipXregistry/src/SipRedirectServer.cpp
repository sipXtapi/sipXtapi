// 
// 
// Copyright (C) 2004,2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004,2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlRegex.h>
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "net/SipUserAgent.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "sipdb/PermissionDB.h"
#include "SipRedirectServer.h"
#include "SipRedirectorRegDB.h"
#include "SipRedirectorAliasDB.h"
#include "SipRedirectorMapping.h"
#include "SipRedirectorHunt.h"
#include "SipRedirectorSubscribe.h"
#include "SipRedirectorPickUp.h"
#include "RedirectResumeMsg.h"
#include "RedirectSuspend.h"

// DEFINES

// Names of configuration files.  All are assumed to be in the directory
// 'configDir'.
#define URL_MAPPING_RULES_FILENAME "mappingrules.xml"
#define URL_FALLBACK_RULES_FILENAME "fallbackrules.xml"
#define ORBIT_CONFIG_FILENAME "orbits.xml"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// STATIC VARIABLE INITIALIZATIONS
SipRedirectServer* SipRedirectServer::spInstance = NULL;

// Constructor
SipRedirectServer::SipRedirectServer() :
   OsServerTask("SipRedirectServer-%d", NULL, 2000),
   mMutex(OsMutex::Q_FIFO),
   mIsStarted(FALSE),
   mpSipUserAgent(NULL),
   mNextSeqNo(0)
{
   spInstance = this;
}

// Destructor
SipRedirectServer::~SipRedirectServer()
{
   // Seize the lock that protects the list of suspend objects.
   OsLock lock(mMutex);

   // Cancel all suspended requests.
   UtlHashMapIterator itor(mSuspendList);
   UtlInt* key;
   while (key = dynamic_cast<UtlInt*> (itor()))
   {
      cancelRedirect(*key,
                     dynamic_cast<RedirectSuspend*> (itor.value()));
   }

   // Finalize and delete all the redirectors.
   for (int i = 0; i < MREDIRECTORCOUNT; i++)
   {
      mRedirectors[i]->finalize();
      delete mRedirectors[i];
      mRedirectors[i] = NULL;
   }

   spInstance = NULL;
}

// Get the unique instance, if it exists.  Will not create it, though.
SipRedirectServer*
SipRedirectServer::getInstance()
{
    return spInstance;
}

// Eventually, the redirector objects will not be created by special
// code but rather by a generalized initialization routine driven by
// an XML file that loads .so's for the SipRedirector* classes.  The
// XML will look something like this:
//    <redirectors>
//        <redirector name="RegDB" library="SipRedirectorRegDB.so">
//        </redirector>
//        <redirector name="AliasDB" library="SipRedirectorAliasDB.so">
//        </redirector>
//        <redirector name="MappingRules" library="SipRedirectorMapping.so">
//            <param name="fallback" value="false"/>
//            <param name="mappingRulesFilename" value="mappingrules.xml"/>
//            <param name="configDir" value="SIPX_CONFDIR"/>
//            <param name="mediaServer" value="..."/>
//            <param name="voicemailServer" value="..."/>
//            <param name="localDomainHost" value="..."/>
//        </redirector>
//        <redirector name="FallbackRules" library="SipRedirectorMapping.so">
//            <param name="fallback" value="true"/>
//            <param name="mappingRulesFilename" value="fallbackrules.xml"/>
//            <param name="configDir" value="SIPX_CONFDIR"/>
//            <param name="mediaServer" value="..."/>
//            <param name="voicemailServer" value="..."/>
//            <param name="localDomainHost" value="..."/>
//        </redirector>
//        <redirector name="Hunt" library="SipRedirectorHunt.so">
//        </redirector>
//        <redirector name="Subscribe" library="SipRedirectorSubscribe.so">
//        </redirector>
//    </redirectors>
UtlBoolean
SipRedirectServer::initialize(
   SipUserAgent* pSipUserAgent,
   const UtlString& configDir,
   const UtlString& mediaServer,
   const UtlString& voicemailServer,
   const UtlString& localDomainHost,
   const char* configFileName)
{
   if (!mIsStarted)
   {
      //start the thread
      start();
   }

   mpSipUserAgent = pSipUserAgent;

   if (!mpSipUserAgent)
   {
      return false;
   }

   // Load the configuration file.

   OsConfigDb configDb;

   if (configDb.loadFromFile(configFileName) == OS_SUCCESS)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectServer::initialize "
                    "Loaded config file '%s'", configFileName);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectServer::initialize "
                    "Error loading config file '%s'", configFileName);
      return false;
   }

   // Initialize the list of redirect processors.

   // Make a hash map to convey the configuration parameters to the
   // redirector objects.
   // This really ought to be simpler, but UtlHashMap demands that its
   // arguments be pointers to writable objects.
   UtlHashMap configParameters;
   UtlString k1 = "configDir";
   UtlString v1 = configDir;
   configParameters.insertKeyAndValue(&k1, &v1);
   UtlString k2 = "mediaServer";
   UtlString v2 = mediaServer;
   configParameters.insertKeyAndValue(&k2, &v2);
   UtlString k3 = "voicemailServer";
   UtlString v3 = voicemailServer;
   configParameters.insertKeyAndValue(&k3, &v3);
   UtlString k4 = "localDomainHost";
   UtlString v4 = localDomainHost;
   configParameters.insertKeyAndValue(&k4, &v4);

   // Create and initialize the SipRedirectorRegDB object.
   mRedirectors[0] = new SipRedirectorRegDB;
   mRedirectors[0]->initialize(configParameters, configDb, mpSipUserAgent, 0);

   // Create and initialize the SipRedirectorAliasDB object.
   mRedirectors[1] = new SipRedirectorAliasDB;
   mRedirectors[1]->initialize(configParameters, configDb, mpSipUserAgent, 1);

   // Create and initialize the mRedirectorMappingRules object.
   UtlString k5 = "mappingRulesFilename";
   UtlString v5 = URL_MAPPING_RULES_FILENAME;
   configParameters.insertKeyAndValue(&k5, &v5);
   UtlString k6 = "reportingName";
   UtlString v6 = "mapping rules";
   configParameters.insertKeyAndValue(&k6, &v6);
   UtlString k7 = "fallback";
   UtlString v7 = "false";
   configParameters.insertKeyAndValue(&k7, &v7);
   mRedirectors[2] = new SipRedirectorMapping;
   mRedirectors[2]->initialize(configParameters, configDb, mpSipUserAgent, 2);

   // Create and initialize the mRedirectorFallbackRules object.
   v5 = URL_FALLBACK_RULES_FILENAME;
   configParameters.insertKeyAndValue(&k5, &v5);
   v6 = "fallback rules";
   configParameters.insertKeyAndValue(&k6, &v6);
   v7 = "true";
   configParameters.insertKeyAndValue(&k7, &v7);
   mRedirectors[3] = new SipRedirectorMapping;
   mRedirectors[3]->initialize(configParameters, configDb, mpSipUserAgent, 3);

   // Create and initialize the mRedirectorHunt object.
   mRedirectors[4] = new SipRedirectorHunt;
   mRedirectors[4]->initialize(configParameters, configDb, mpSipUserAgent, 4);

   // Create and initialize the mRedirectorSubscribe object.
   mRedirectors[5] = new SipRedirectorSubscribe;
   mRedirectors[5]->initialize(configParameters, configDb, mpSipUserAgent, 5);

   // Create and initialize the mRedirectorPickUp object.
   UtlString k8 = "orbitConfigFilename";
   UtlString v8 = ORBIT_CONFIG_FILENAME;
   configParameters.insertKeyAndValue(&k8, &v8);
   mRedirectors[6] = new SipRedirectorPickUp;
   mRedirectors[6]->initialize(configParameters, configDb, mpSipUserAgent, 6);

// Update this test with the index of the last redirector loaded.
#if 6 != (MREDIRECTORCOUNT-1)
#error MREDIRECTORCOUNT does not match SipRedirectServer::initialize.
#endif

   return true;
}

/**
 * Report a response that we are about to send.
 */
static void logResponse(UtlString& messageStr)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "----------------------------------"
                 "Sending final response%s", messageStr.data());
}

/**
 * Cancel a suspended redirection.
 *
 * Caller must hold mMutex.
 *
 * containableSeqNo - UtlInt containing the sequence number.
 *
 * suspendObject - pointer to the suspense object.
 */
void SipRedirectServer::cancelRedirect(UtlInt& containableSeqNo,
                                       RedirectSuspend* suspendObject)
{
   RequestSeqNo seqNo = containableSeqNo.getValue();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectServer::cancelRedirect "
                 "Canceling suspense of request %d", seqNo);
   // Call cancel for redirectors that need it.
   for (int i = 0; i < MREDIRECTORCOUNT; i++)
   {
      if (suspendObject->mRedirectors[i].needsCancel)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectServer::cancelRedirect "
                       "Calling cancel(%d) for redirector %d", seqNo, i);
         mRedirectors[i]->cancel(seqNo);
      }
   }
   // Remove the entry from mSuspendList.
   // Also deletes the suspend object.
   // Deleting the suspend object frees the array of information about
   // the redirectors, and the private storage for each redirector.
   // (See RedirectSuspend::~RedirectSuspend().)
   mSuspendList.destroy(&containableSeqNo);
}

/**
 * Process a redirection.  The caller sets up our processing, but we
 * carry it through to the generation of the response, queuing of the
 * suspend object, etc.  We send the response if we finish processing.
 *
 * message is the message to be processed.  Its memory is owned by our
 * caller, or is attached to the suspend object.
 *
 * method is the request's SIP method.  Its memory is owned by our caller.
 *
 * seqNo is the sequence number to be used for this request.  (If this
 * request is to be suspended and seqNo == mNextSeqNo, we must increment
 * mNextSeqNo.)
 *
 * suspendObject is the suspend object for this request (if it already
 * exists) or NULL.  It is passed as an argument to avoid attempting to
 * look it up if the caller knows that it does not exist (because this
 * is a first processing attempt).
 */
void SipRedirectServer::processRedirect(const SipMessage* message,
                                        UtlString& method,
                                        RequestSeqNo seqNo,
                                        RedirectSuspend* suspendObject)
{
   // The response we will compose and, hopefully, send.
   SipMessage response;

   // Extract the request URI.
   UtlString stringUri;
   message->getRequestUri(&stringUri);
   // The requestUri is an addr-spec, not a name-addr.
   const Url requestUri(stringUri, TRUE);
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectServer::processRedirect "
                 "Starting to process request URI '%s'",
                 stringUri.data());

   // Seize the lock that protects the list of suspend objects.
   OsLock lock(mMutex);

   // Process with the redirectors.
   // Set to TRUE if any of the redirectors requests suspension.
   UtlBoolean willSuspend = FALSE;
   // Set to TRUE if any of the redirectors requests an error response.
   UtlBoolean willError = FALSE;
   // If willError is set, this is the response code to give.
   int responseCode = 0;        // Initialize to avoid warning.
   // Cycle through the redirectors in order.
   for (int i = 0; i < MREDIRECTORCOUNT; i++)
   {
      // Place to store the private storage pointer.
      SipRedirectorPrivateStorage* privateStorageP;
      // Initialize it.
      privateStorageP = (suspendObject ?
                         suspendObject->mRedirectors[i].privateStorage :
                         NULL);

      // Call the redirector to process the request.
      SipRedirector::LookUpStatus status =
         mRedirectors[i]->lookUp(*message, stringUri, requestUri, method,
                                 response, seqNo, i, privateStorageP);

      // Create the suspend object if it does not already exist and we need it.
      if (!suspendObject &&
          (status == SipRedirector::LOOKUP_SUSPEND || privateStorageP))
      {
         suspendObject = new RedirectSuspend(MREDIRECTORCOUNT);
         // Insert it into mSuspendList, keyed by seqNo.
         UtlInt* containableSeqNo = new UtlInt(seqNo);
         mSuspendList.insertKeyAndValue(containableSeqNo, suspendObject);
         // Save in it a copy of the message.  (*message is
         // dependent on the OsMsg bringing the message to us, and
         // will be freed when we are done with that OsMsg.)
         suspendObject->mMessage = *message;
         // Use the next sequence number for the next request.
         if (seqNo == mNextSeqNo)
         {
            // Increment to the next value (and roll over if necessary).
            mNextSeqNo++;
         }
      }
      // Store the private storage pointer.
      if (suspendObject)
      {
         suspendObject->mRedirectors[i].privateStorage = privateStorageP;
      }

      // Dispatch on status.
      switch (status)
      {
      case SipRedirector::LOOKUP_SUCCESS:
         // Processing was successful.
         break;

      case SipRedirector::LOOKUP_ERROR_REQUEST:
         // Processing detected an error.  Processing of this request
         // should end immediately and a 403 Forbidden response returned.
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRedirectServer::processRedirect "
                       "LOOKUP_ERROR_REQUEST returned by redirector "
                       "%d while processing method '%s' URI '%s'",
                       status, method.data(), stringUri.data());
         willError = TRUE;
         responseCode = 403;
         break;

      case SipRedirector::LOOKUP_ERROR_SERVER:
         // Processing detected an error.  Processing of this request
         // should end immediately and a 500 response returned.
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRedirectServer::processRedirect "
                       "LOOKUP_ERROR_SERVER returned by redirector "
                       "%d while processing method '%s' URI '%s'",
                       status, method.data(), stringUri.data());
         willError = TRUE;
         responseCode = 500;
         break;

      case SipRedirector::LOOKUP_SUSPEND:
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRedirectServer::processRedirect "
                       "LOOKUP_SUSPEND returned by redirector "
                       "%d while processing method '%s' URI '%s'",
                       i, method.data(), stringUri.data());
         willSuspend = TRUE;
         // Mark that this redirector has requested suspension.
         suspendObject->mRedirectors[i].suspended = TRUE;
         suspendObject->mRedirectors[i].needsCancel = TRUE;
         suspendObject->mSuspendCount++;
         break;

      default:
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRedirectServer::processRedirect "
                       "Invalid status value %d returned by redirector "
                       "%d while processing method '%s' URI '%s'",
                       status, i, method.data(), stringUri.data());
         break;
      }
   }

   if (willError || !willSuspend)
   {
      // Send a response and terminate processing now if an error has
      // been found or if no redirector has requested suspension.
      if (willError)
      {
         // Send 403 or 500 response.
         switch (responseCode)
         {
         case 403:
            response.setResponseData(message,
                                     SIP_FORBIDDEN_CODE,
                                     SIP_FORBIDDEN_TEXT);
            break;

         default:
            // Ugh, responseCode should never have an unknown value.
            // Log a message and return 500.
            OsSysLog::add(FAC_SIP, PRI_CRIT,
                          "SipRedirectServer::processRedirect "
                          "Invalid responseCode %d",
                          responseCode);
            /* Fall through to next case. */
         case 500:
            response.setResponseData(message,
                                     SIP_SERVER_INTERNAL_ERROR_CODE,
                                     SIP_SERVER_INTERNAL_ERROR_TEXT);
            break;
         }
         // Remove all Contact: headers.
         SipRedirector::removeAllContacts(response);
      }
      else
      {
         // If request processing is finished, construct a response,
         // either 302 or 404.
         if (response.getCountHeaderFields(SIP_CONTACT_FIELD) > 0)
         {
            // There are contacts, so send a 302 Moved Temporarily.
            response.setResponseData(message,
                                     SIP_TEMPORARY_MOVE_CODE,
                                     SIP_TEMPORARY_MOVE_TEXT);
         }
         else
         {
            // There are no contacts, send back a 404 Not Found.
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRedirectServer::processRedirect "
                          "No contacts added, sending 404 response");
            response.setResponseData(message,
                                     SIP_NOT_FOUND_CODE,
                                     SIP_NOT_FOUND_TEXT);
         }
      }

      // Now that we've set the right code into the response, send the
      // response.
      UtlString finalMessageStr;
      int finalMessageLen;

      response.getBytes(&finalMessageStr, &finalMessageLen);
      // Log the response.
      logResponse(finalMessageStr);
      mpSipUserAgent->send(response);

      // If the suspend object exists, remove it from mSuspendList
      // and delete it.
      if (suspendObject)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectServer::processRedirect "
                       "Cleaning up suspense of request %d", seqNo);
         UtlInt containableSeqNo(seqNo);
         cancelRedirect(containableSeqNo, suspendObject);
      }
   }      
   else
   {
      // Request is suspended.
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectServer::processRedirect "
                    "Suspending request %d", seqNo);
   }
}

UtlBoolean
SipRedirectServer::handleMessage(OsMsg& eventMessage)
{
   int msgType = eventMessage.getMsgType();

   switch (msgType)
   {
   case OsMsg::PHONE_APP:
   {      
      // An incoming request to be redirected.
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectServer::handleMessage "
                    "Start processing redirect message %d", mNextSeqNo);

      // Get a pointer to the SIP message.
      const SipMessage* message =
         ((SipMessageEvent&) eventMessage).getMessage();

      // Extract the request method.
      UtlString method;
      message->getRequestMethod(&method);

      if (method.compareTo(SIP_ACK_METHOD, UtlString::ignoreCase) == 0)
      {
         /* ACKs require no action. */ ;
      }
      else if (method.compareTo(SIP_CANCEL_METHOD, UtlString::ignoreCase) == 0)
      {
         // For CANCEL.
         // If we have a suspended request with this Call-Id, cancel it.
         // Send a 200 response.

         // Cancel the suspended request.

         // Get a pointer to the SIP message.
         const SipMessage* message =
            ((SipMessageEvent&) eventMessage).getMessage();
         UtlString cancelCallId;
         message->getCallIdField(&cancelCallId);

         {
            // Seize the lock that protects the list of suspend objects.
            OsLock lock(mMutex);

            // Look for a suspended request that had this Call-Id.
            UtlHashMapIterator itor(mSuspendList);

            // Fetch a pointer to each suspend object.
            while (itor())
            {
               RedirectSuspend* suspend_object =
                  dynamic_cast<RedirectSuspend*> (itor.value());

               // Is this a request to which the CANCEL applies?
               if (suspend_object->mMessage.isInviteFor(message))
               {
                  // Send a 487 response to the original request.
                  SipMessage response;
                  response.setResponseData(&suspend_object->mMessage,
                                           SIP_REQUEST_TERMINATED_CODE,
                                           SIP_REQUEST_TERMINATED_TEXT);
                  UtlString finalMessageStr;
                  int finalMessageLen;
                  response.getBytes(&finalMessageStr, &finalMessageLen);
                  logResponse(finalMessageStr);
                  mpSipUserAgent->send(response);

                  // Cancel the redirection.
                  // (After we are done using suspend_object->mMessage
                  // to generate the response.)
                  UtlInt requestNo = *dynamic_cast<UtlInt*> (itor.key());
                  cancelRedirect(requestNo, suspend_object);
               }
            }
         }

         // We do not need to send a 200 for the CANCEL, as the stack does that
         // for us.  (And will eat a 200 that we generate, it seems!)
      }
      else
      {
         // For all methods other than CANCEL or ACK:
         // Call processRedirect to call the redirectors, and handle
         // their results, send a response or suspend processing of
         // the request.

         // Assign mNextSeqNo as the sequence number for this request.
         // If this request needs to be suspended, processRedirect will
         // increment mNextSeqNo so that value will not be reused (soon).
         // Initially, the suspendObject is NULL.
         processRedirect(message, method, mNextSeqNo, (RedirectSuspend*) 0);
      }
   }
   break;

   case REDIRECT_RESTART:
   {
      // A message saying that a redirector is now willing to resume
      // processing of a request.
      // Get the redirector and sequence number.
      const RedirectResumeMsg* msg =
         dynamic_cast<RedirectResumeMsg*> (&eventMessage);
      RequestSeqNo seqNo = msg->getRequestSeqNo();
      int redirectorNo = msg->getRedirectorNo();
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectServer::handleMessage "
                    "Resume for redirector %d request %d",
                    redirectorNo, seqNo);

      // Look up the suspend object.
      UtlInt containableSeqNo(seqNo);
      RedirectSuspend* suspendObject =
         dynamic_cast<RedirectSuspend*>
         (mSuspendList.findValue(&containableSeqNo));

      // If there is no request with this sequence number, ignore the message.
      if (!suspendObject)
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipRedirectServer::handleMessage No suspended request "
                       "with seqNo %d",
                       seqNo);
         break;
      }

      // Check that this redirector is suspended.
      if (redirectorNo < 0 || redirectorNo >= MREDIRECTORCOUNT)
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRedirectServer::handleMessage "
                       "Invalid redirector %d",
                       redirectorNo);
         break;
      }         
      if (!suspendObject->mRedirectors[redirectorNo].suspended)
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipRedirectServer::handleMessage Redirector %d is "
                       "not suspended for seqNo %d",
                       redirectorNo, seqNo);
         break;
      }

      // Mark this redirector as no longer wanting suspension.
      suspendObject->mRedirectors[redirectorNo].suspended = FALSE;
      suspendObject->mSuspendCount--;
      // If no more redirectors want suspension, reprocess the request.
      if (suspendObject->mSuspendCount == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectServer::handleMessage "
                       "Start reprocessing request %d", seqNo);

         // Get a pointer to the message.
         const SipMessage* message = &suspendObject->mMessage;

         // Extract the request method.
         UtlString method;
         message->getRequestMethod(&method);

         processRedirect(message, method, seqNo, suspendObject);
      }
   }
      break;
   }

   return TRUE;
}

void
SipRedirectServer::resumeRequest(RequestSeqNo requestSeqNo,
                                 int redirectorNo)
{
   // Create the appropriate message.
   RedirectResumeMsg message = RedirectResumeMsg(requestSeqNo, redirectorNo);

   // Send the message to the redirect server.
   // Note that send() copies its argument.
   getMessageQueue()->send(message);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectServer::resumeRequest "
                 "Redirector %d sent message to resume request %d",
                 redirectorNo, requestSeqNo);
}

SipRedirectorPrivateStorage*
SipRedirectServer::getPrivateStorage(
   RequestSeqNo requestSeqNo,
   int redirectorNo)
{
   // Turn the request number into a UtlInt.
   UtlInt containableSeqNo(requestSeqNo);
   // Look up the suspend object.
   RedirectSuspend* suspendObject =
      dynamic_cast<RedirectSuspend*>
      (mSuspendList.findValue(&containableSeqNo));
   // Get the private storage pointer.
   return suspendObject->mRedirectors[redirectorNo].privateStorage;
}

SipRedirectServerPrivateStorageIterator::
SipRedirectServerPrivateStorageIterator(int redirectorNo) :
   UtlHashMapIterator(SipRedirectServer::getInstance()->mSuspendList),
   mRedirectorNo(redirectorNo)
{
}

UtlContainable*
SipRedirectServerPrivateStorageIterator::operator()()
{
   SipRedirectorPrivateStorage* pStorage = NULL;
   // Step the iterator until we find a member which has a non-NULL pointer
   // to private storage for redirector mRedirectorNo.
// :BUG: Known to be incorrect
//   while ((*((UtlIterator*) this))() &&
//          (pStorage =
//           (dynamic_cast<RedirectSuspend*> (this->value()))->
//           mRedirectors[mRedirectorNo].privateStorage))
//   {
//   }
   while (1)
   {
      // Step the iterator using UtlHashMapIterator's step function.
      if (this->UtlHashMapIterator::operator()() ==
          NULL)
      {
         break;
      }
      pStorage =
         (dynamic_cast<RedirectSuspend*> (this->value()))->
         mRedirectors[mRedirectorNo].privateStorage;
      if ( pStorage != NULL )
      {
         break;
      }
   }
   // Return the pointer to the storage, or NULL if none was found.
   return pStorage;
}

RequestSeqNo SipRedirectServerPrivateStorageIterator::requestSeqNo() const
{
   // The key is a UtlInt which is the request sequence number.
   return (dynamic_cast<UtlInt*> (this->key()))->getValue();
}
