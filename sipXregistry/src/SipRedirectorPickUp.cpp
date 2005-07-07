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

// SYSTEM INCLUDES
#include <stdlib.h>
#include <limits.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/CredentialDB.h"
#include "SipRedirectorPickUp.h"
#include "SipRedirectServer.h"
#include "SipRedirector.h"
#include "os/OsProcess.h"
#include "net/NetMd5Codec.h"
#include "net/Url.h"

// DEFINES

// The parameter giving the directed call pick-up feature code.
#define CONFIG_SETTING_DIRECTED_CODE \
    "SIP_REGISTRAR_DIRECTED_CALL_PICKUP_CODE"
// The parameter giving the global call pick-up feature code.
#define CONFIG_SETTING_GLOBAL_CODE \
    "SIP_REGISTRAR_GLOBAL_CALL_PICKUP_CODE"
// The parameter giving the call retrieve feature code.
#define CONFIG_SETTING_RETRIEVE_CODE \
    "SIP_REGISTRAR_CALL_RETRIEVE_CODE"
// The parameter giving the call pick-up wait time.
#define CONFIG_SETTING_WAIT \
    "SIP_REGISTRAR_CALL_PICKUP_WAIT"
// The default call pick-up wait time, in seconds and microseconds.
#define DEFAULT_WAIT_TIME_SECS        1
#define DEFAULT_WAIT_TIME_USECS       0
// The minimum and maximum call pick-up wait time allowed (floating-point).
#define MIN_WAIT_TIME            0.001
#define MAX_WAIT_TIME            100.0

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Constructor
SipRedirectorPickUp::SipRedirectorPickUp() :
   mpSipUserAgent(NULL),
   mTask(NULL)
{
}

// Destructor
SipRedirectorPickUp::~SipRedirectorPickUp()
{
}

// A zero timezone value, to be used when calling gettimeofday().
struct timezone SipRedirectorPickUp::mTZ = { 0, 0 };

// Initializer
OsStatus
SipRedirectorPickUp::initialize(const UtlHashMap& configParameters,
                                OsConfigDb& configDb,
                                SipUserAgent* pSipUserAgent,
                                int redirectorNo)
{
   // The return status.
   // It will be OS_SUCCESS if this redirector is configured to do any work,
   // and OS_FAILED if not.
   OsStatus r = OS_FAILED;

   // Fetch the call pick-up festure code from the config file.
   // If it is null, it doesn't count.
   if ((configDb.get(CONFIG_SETTING_DIRECTED_CODE, mCallPickUpCode) !=
        OS_SUCCESS) ||
       mCallPickUpCode.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "SipRedirectorPickUp::initialize "
                    "No call pick-up feature code specified");
   }
   else
   {
      // Call pick-up feature code is configured.
      // Initialize the system.
      OsSysLog::add(FAC_SIP, PRI_INFO, "SipRedirectorPickUp::initialize "
                    "Call pick-up feature code is '%s'",
                    mCallPickUpCode.data());
      r = OS_SUCCESS;

      // Record the two user-names that are excluded as being pick-up requests.
      mExcludedUser1 = mCallPickUpCode;
      mExcludedUser1.append("*");
      mExcludedUser2 = mCallPickUpCode;
      mExcludedUser2.append("#");
   }

   // Fetch the global call pick-up username from the config file.
   // If it is null, it doesn't count.
   if ((configDb.get(CONFIG_SETTING_GLOBAL_CODE, mGlobalPickUpCode) !=
        OS_SUCCESS) ||
       mGlobalPickUpCode.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "SipRedirectorPickUp::initialize "
                    "No global call pick-up code specified");
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "SipRedirectorPickUp::initialize "
                    "Global call pick-up code is '%s'",
                    mGlobalPickUpCode.data());
      r = OS_SUCCESS;
   }

   // Fetch the call retrieve username from the config file.
   // If it is null, it doesn't count.
   if ((configDb.get(CONFIG_SETTING_RETRIEVE_CODE, mCallRetrieveCode) !=
        OS_SUCCESS) ||
       mCallRetrieveCode.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "SipRedirectorPickUp::initialize "
                    "No call retrieve code specified");
   }
   else
   {
      UtlString s = "configDir";
      const UtlString* configDir =
         dynamic_cast<UtlString*> (configParameters.findValue(&s));
      s = "orbitConfigFilename";
      const UtlString* orbitConfigFilename =
         dynamic_cast<UtlString*> (configParameters.findValue(&s));
      // Check that an orbit description file exists.
      if (orbitConfigFilename == NULL ||
          orbitConfigFilename->length() == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_INFO, "SipRedirectorPickUp::initialize "
                       "No orbit file name specified");
      }
      else
      {
         UtlString fileName =
            *configDir + OsPathBase::separator + *orbitConfigFilename;

         OsSysLog::add(FAC_SIP, PRI_INFO, "SipRedirectorPickUp::initialize "
                       "Call retrieve code is '%s', orbit file is '%s'",
                       mCallRetrieveCode.data(), fileName.data());
         // Read the orbit file to get the list of orbit numbers.
         if (parseOrbitFile(fileName) == OS_SUCCESS)
         {
            r = OS_SUCCESS;
         }
         else
         {
            // Parsing the file failed, so do not do retrieval processing.
            // (parseOrbitFile output appropriate error messages.)
            mCallRetrieveCode = "";
         }
      }
   }

   // If any of the pick-up redirections are active, set up the machinery
   // to execute them.
   if (r == OS_SUCCESS)
   {
      // Get and save our domain name.
      UtlString temp("localDomainHost");
      mDomain =
         *(dynamic_cast<UtlString*> (configParameters.findValue(&temp)));

      // Create a SIP user agent to generate SUBSCRIBEs and receive NOTIFYs,
      // and save a pointer to it.
      // Having a separate user agent ensures that the NOTIFYs are not
      // processed for redirection, but rather we can act as a UAS to
      // process them.
      mpSipUserAgent = new SipUserAgent(
         // Let the system choose the port numbers.
         0, // sipTcpPort
         0, // sipUdpPort
         0, // sipTlsPort
         NULL, // publicAddress
         NULL, // defaultUser
         NULL, // defaultSipAddress
         NULL, // sipProxyServers
         NULL, // sipDirectoryServers
         NULL, // sipRegistryServers
         NULL, // authenticationScheme
         NULL, // authenicateRealm
         NULL, // authenticateDb
         NULL, // authorizeUserIds
         NULL, // authorizePasswords
         NULL, // natPingUrl
         0, // natPingFrequency
         "PING", // natPingMethod
         NULL, // lineMgr
         SIP_DEFAULT_RTT, // sipFirstResendTimeout
         TRUE, // defaultToUaTransactions
         -1, // readBufferSize
         OsServerTask::DEF_MAX_MSGS // queueSize
         );
      mpSipUserAgent->start();

      // Get the wait time for NOTIFYs in response to our SUBSCRIBEs.
      // Set the default value, to be overridden if the user specifies a valud
      // value.
      mWaitSecs = DEFAULT_WAIT_TIME_SECS;
      mWaitUSecs = DEFAULT_WAIT_TIME_USECS;
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorPickUp::initialize "
                    "Default wait time is %d.%06d", mWaitSecs, mWaitUSecs);
      // Fetch the parameter value.
      UtlString waitUS;
      float waitf;
      if (configDb.get(CONFIG_SETTING_WAIT, waitUS) == OS_SUCCESS)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectorPickUp::initialize "
                       CONFIG_SETTING_WAIT " is '%s'",
                       waitUS.data());
         // Parse the value, checking for errors.
         unsigned int char_count;
         sscanf(waitUS.data(), " %f %n", &waitf, &char_count);
         if (char_count != waitUS.length())
         {
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipRedirectorPickUp::initialize "
                          "Invalid format for "
                          CONFIG_SETTING_WAIT
                          " '%s'", 
                          waitUS.data());
         }
         else if (
            // Check that the value is in range.
            !(waitf >= MIN_WAIT_TIME && waitf <= MAX_WAIT_TIME))
         {
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipRedirectorPickUp::initialize "
                          CONFIG_SETTING_WAIT
                          " (%f) outside allowed range (%f to %f)",
                          waitf, MIN_WAIT_TIME, MAX_WAIT_TIME);
         }
         else
         {
            // Extract the seconds and microseconds, being careful to round
            // because the conversion from character data may have
            // been inexact.
            // Since waitf <= 100, usecs <= 100,000,000.
            int usecs = (int)((waitf * 1000000) + 0.0000005);
            mWaitSecs = usecs / 1000000;
            mWaitUSecs = usecs % 1000000;
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRedirectorPickUp::initialize "
                          "Wait time is %d.%06d",
                          mWaitSecs, mWaitUSecs);
         }
      }

      // Initialize the CSeq counter to an arbitrary acceptable value.
      mCSeq = 4711;

      // Create and start the task to receive NOTIFYs.
      mTask = new SipRedirectorPickUpTask(mpSipUserAgent, redirectorNo);
      mTask->start();
   }

   return r;
}

// Finalizer
void
SipRedirectorPickUp::finalize()
{
   // Close down the SipUserAgent.
   if (mpSipUserAgent)
   {
      // Do not block, as any incomplete work is useless anyway.
      mpSipUserAgent->shutdown(FALSE);
      delete mpSipUserAgent;
      mpSipUserAgent = NULL;
   }

   // Terminate the task to receive NOTIFYs.
   if (mTask)
   {
      mTask->requestShutdown();
      delete mTask;
      mTask = NULL;
   }
}

SipRedirector::LookUpStatus
SipRedirectorPickUp::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   SipMessage& response,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage)
{
   UtlString userId;
   requestUri.getUserId(userId);

   if (!mCallPickUpCode.isNull() &&
       userId.length() > mCallPickUpCode.length() &&
       userId.index(mCallPickUpCode.data()) == 0 &&
       userId.compareTo(mExcludedUser1) != 0 &&
       userId.compareTo(mExcludedUser2) != 0)
   {
      // Check if directed call pick-up is active, and this is a
      // request for directed call pick-up.
      // Because the default directed pick-up feature code is "*78" and
      // the default global pick-up feature code is "*78*", we can't just
      // match all strings with the directed pick-up feature code as a
      // prefix, we also require that the suffix not be "*" or "#".
      return lookUpDialog(requestString,
                          response,
                          requestSeqNo,
                          redirectorNo,
                          privateStorage,
                          // The suffix of the request URI after the
                          // directed call pick-up code.
                          userId.data() + mCallPickUpCode.length(),
                          // Only examine early dialogs.
                          stateEarly);
   }
   else if (!mGlobalPickUpCode.isNull() &&
            userId.compareTo(mGlobalPickUpCode) == 0)
   {
      // Process the global call pick-up code.
      return lookUpDialog(requestString,
                          response,
                          requestSeqNo,
                          redirectorNo,
                          privateStorage,
                          // The all-exetnsions user.
                          ALL_CREDENTIALS_USER,
                          // Only examine early dialogs.
                          stateEarly);
   }
   else if (!mGlobalPickUpCode.isNull() &&
            userId.compareTo(ALL_CREDENTIALS_USER) == 0 &&
            method.compareTo("SUBSCRIBE", UtlString::ignoreCase) == 0)
   {
      // Process the "*allcredentials" user for global call pick-up.
      // But only for SUBSCRIBE, to prevent "INVITE *allcredentials"
      // from ringing every phone!
      ResultSet credentials;
      CredentialDB::getInstance()->getAllRows(credentials);

      // Loop through the result set, looking at each credentials
      // entry in turn.
      int numGlobalPickUpContacts = credentials.getSize();
      for (int i = 0; i < numGlobalPickUpContacts; i++)
      {
         static UtlString uriKey("uri");

         UtlHashMap record;
         if (credentials.getIndex(i, record))
         {
            // Extract the "uri" element from the credential.
            UtlString contactStr;
            UtlString uri = *((UtlString*)record.findValue(&uriKey));
            Url contactUri(uri);

            // Add the contact to the response.
            addContact(response, requestString, contactUri,
                       "global call pick-up");
         }
      }
      return SipRedirector::LOOKUP_SUCCESS;
   }
   else if (!mCallRetrieveCode.isNull() &&
            userId.length() > mCallRetrieveCode.length() &&
            userId.index(mCallRetrieveCode.data()) == 0)
   {
      // Check if call retrieve is active, and this is a request for
      // call retrieve.

      // Extract the putative orbit number.
      UtlString orbit(userId.data() + mCallRetrieveCode.length());
      // Look it up in the orbit list.
      if (mOrbitList.find(&orbit))
      {
         return lookUpDialog(requestString,
                             response,
                             requestSeqNo,
                             redirectorNo,
                             privateStorage,
                             // The orbit number.
                             orbit.data(),
                             // Only examine confirmed dialogs.
                             stateConfirmed);
      }
      else
      {
         // It appears to be a call retrieve, but the orbit number is invalid.
         // Return LOOKUP_ERROR_REQUEST.
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectorPickUp::lookUp Invalid orbit number '%s'",
                       orbit.data());
         return SipRedirector::LOOKUP_ERROR_REQUEST;
      }
   }
   else
   {
      // We do not recognize the user, so we do nothing.
      return SipRedirector::LOOKUP_SUCCESS;
   }
}

SipRedirector::LookUpStatus
SipRedirectorPickUp::lookUpDialog(
   const UtlString& requestString,
   SipMessage& response,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage,
   const char* subscribeUser,
   State stateFilter)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorPickUp::lookUpDialog requestString = '%s', "
                 "requestSeqNo = %d, redirectorNo = %d, privateStorage = %p, "
                 "subscribeUser = '%s', stateFilter = %d",
                 requestString.data(), requestSeqNo, redirectorNo,
                 privateStorage, subscribeUser, stateFilter);

   // If the private storage is already allocated, then this is a
   // reprocessing cycle, and the dialog to pick up (if any) is
   // recorded in private storage.
   if (privateStorage)
   {
      // Cast the private storage pointer to the right type so we can
      // access the needed dialog information.
      SipRedirectorPrivateStoragePickUp* dialog_info =
         dynamic_cast<SipRedirectorPrivateStoragePickUp*> (privateStorage);

      if (dialog_info->mTargetDialogDuration !=
          SipRedirectorPrivateStoragePickUp::TargetDialogDurationAbsent)
      {
         // A dialog has been recorded.  Construct a contact for it.
         Url contact_URI(dialog_info->mTargetDialogRemoteURI);

         // Construct the Replaces: header value the caller should use.
         UtlString header_value(dialog_info->mTargetDialogCallId);
         header_value.append(";to-tag=");
         header_value.append(dialog_info->mTargetDialogLocalTag);
         header_value.append(";from-tag=");
         header_value.append(dialog_info->mTargetDialogRemoteTag);
         // If the state filtering is "early", add "early-only", so we
         // don't pick up a call that has just been answered.
         if (dialog_info->mStateFilter == stateEarly)
         {
            header_value.append(";early-only");
         }

         // Add a header parameter to specify the Replaces: header.
         contact_URI.setHeaderParameter("Replaces", header_value.data());

         // We do not add a header parameter to cause the redirection
         // to include a "Require: replaces" header.  If we did, then
         // if the caller phone did not support INVITE/Replaces:, the
         // pick-up would fail entirely.  This way, if the caller
         // phone does not support INVITE/Replaces:, the caller will
         // get a simultaneous incoming call from the executing phone.

         // Record the URI as a contact.
         addContact(response, requestString, contact_URI, "pick-up");
      }

      // We do not need to suspend this time.
      return SipRedirector::LOOKUP_SUCCESS;
   }
   else
   {
      // Construct the SUBSCRIBE.
      SipMessage subscribe;
      UtlString subscribeRequestUri("sip:");
      // The user of the request URI is our subscribeUser parameter.
      subscribeRequestUri.append(subscribeUser);
      subscribeRequestUri.append("@");
      subscribeRequestUri.append(mDomain);
      // Construct a Call-Id on the plan:
      //   Pickup-process-time-counter@domain
      // The process number has at most 8 characters, the time has at most 8
      // characters, and the counter has at most 8 characters, so 28 characters
      // suffice for the buffer to assemble the "process-time-counter" part
      // of the Call-Id..  (Using the time ensures that the Ids remain unique
      // when the registrar is restarted.)
      char buffer[32];
      sprintf(buffer, "%x-%x-%x@", (unsigned int) OsProcess::getCurrentPID(),
              (unsigned int) OsDateTime::getSecsSinceEpoch(), mCSeq);
      UtlString callId("Pickup-");
      callId.append(buffer);
      callId.append(mDomain);
      // Construct the From: value.
      UtlString fromUri;
      {
         // Get the local address and port.
         UtlString address;
         int port;
         mpSipUserAgent->getLocalAddress(&address, &port);
         // Use the first 8 chars of the MD5 of the Call-Id as the from-tag.
         NetMd5Codec encoder;
         UtlString tag;
         encoder.encode(callId.data(), tag);
         tag.remove(8);
         // Assemble the URI.
         subscribe.buildSipUrl(&fromUri,
                               address.data(),
                               port,
                               NULL, // protocol
                               NULL, // user
                               NULL, // userLabel,
                               tag.data());
      }
       
      // Set the standard request headers.
      // Allow the SipUserAgent to fill in Contact:.
      subscribe.setRequestData(
         SIP_SUBSCRIBE_METHOD,
         subscribeRequestUri.data(), // request URI
         fromUri, // From:
         subscribeRequestUri.data(), // To:
         callId,
         mCSeq);
      // Increment CSeq and roll it over if necessary.
      mCSeq++;
      mCSeq &= 0x0FFFFFFF;
      // Set the "Expires: 0" header.
      subscribe.setExpiresField(0);
      // Set the "Event: dialog" header.
      subscribe.setEventField("dialog");
      // Set the "Accept: application/dialog-info+xml" header.
      // Not strictly necessary (per the I-D), but it makes the SUBSCRIBE
      // more strictly compliant.
      subscribe.setHeaderValue(SIP_ACCEPT_FIELD,
                               "application/dialog-info+xml");

      // Send the SUBSCRIBE.
      mpSipUserAgent->send(subscribe);

      // Allocate private storage.
      SipRedirectorPrivateStoragePickUp *storage =
         new SipRedirectorPrivateStoragePickUp(requestSeqNo,
                                               redirectorNo);
      privateStorage = storage;

      // Record the Call-Id of the SUBSCRIBE, so we can correlated the
      // NOTIFYs with it.
      storage->mSubscribeCallId = callId;
      // Record the state filtering criterion.
      storage->mStateFilter = stateFilter;

      // If we are printing debug messages, record when the SUBSCRIBE
      // was sent, so we can report how long it took to get the NOTIFYs.
      if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
      {
         gettimeofday(&storage->mSubscribeSendTime, &SipRedirectorPickUp::mTZ);
      }

      // Set the timer to resume.
      storage->mTimer.oneshotAfter(OsTime(mWaitSecs, mWaitUSecs));

      // Suspend processing the request.
      return SipRedirector::LOOKUP_SUSPEND;
   }
}


OsStatus SipRedirectorPickUp::parseOrbitFile(UtlString& fileName)
{
   // Initialize Tiny XML document object.
   TiXmlDocument document;
   TiXmlNode* orbits_element;
   if (
      // Load the XML into it.
      document.LoadFile(fileName.data()) &&
      // Find the top element, which should be an <orbits>.
      (orbits_element = document.FirstChild("orbits")) != NULL &&
      orbits_element->Type() == TiXmlNode::ELEMENT)
   {
      // Find all the <orbit> elements.
      for (TiXmlNode* orbit_element = 0;
           orbit_element = orbits_element->IterateChildren("orbit",
                                                           orbit_element);
         )
      {
         // Process each <orbit> element.
         TiXmlNode* extension_element =
            orbit_element->FirstChild("extension");
         if (extension_element)
         {
            // Process the <extension> element.
            UtlString *user = new UtlString;
            SipRedirectorPickUp::textContentShallow(
               *user,
               extension_element->ToElement());
            if (user->length() > 0)
            {
               // Insert the user into the orbit list (which now owns *user).
               if (!mOrbitList.insert(user))
               {
                  // Insertion failed, presumably because the extension was
                  // already in there.
                  OsSysLog::add(FAC_SIP, PRI_ERR,
                                "SipRedirectorPrivateStoragePickUp::parseOrbitFile "
                                "Extension '%s' specified as an orbit twice?",
                                user->data());
                  free(user);
               }
            }
            else
            {
               // Extension had zero length
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "SipRedirectorPrivateStoragePickUp::parseOrbitFile "
                             "Extension was null.");
               free(user);
            }
         }
         else
         {
            // No <extension> child of <orbit>.
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipRedirectorPrivateStoragePickUp::parseOrbitFile "
                          "<orbit> element did not have <extension>.");
         }
      }

      if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
      {
         // Output the list of orbits.
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectorPrivateStoragePickUp::parseOrbitFile "
                       "Valid orbits are:");
         UtlHashMapIterator itor(mOrbitList);
         while (itor())
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRedirectorPrivateStoragePickUp::parseOrbitFile "
                          "Orbit '%s'",
                          (dynamic_cast<UtlString*> (itor.key()))->data());
         }
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectorPrivateStoragePickUp::parseOrbitFile "
                       "End of list");
      }

      // In any of these cases, attempt to do call retrieval.
      return OS_SUCCESS;
   }
   else
   {
      // Report error parsing file.
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SipRedirectorPrivateStoragePickUp::parseOrbitFile "
                    "Orbit file '%s' could not be parsed.", fileName.data());
      // No hope of doing call retrieval.
      return OS_FAILED;
   } 
}

SipRedirectorPrivateStoragePickUp::SipRedirectorPrivateStoragePickUp(
   RequestSeqNo requestSeqNo,
   int redirectorNo) :
   mNotification(requestSeqNo, redirectorNo),
   mTimer(mNotification),
   // Set special value to show no dialog has been recorded.
   mTargetDialogDuration(TargetDialogDurationAbsent)
{
}

SipRedirectorPrivateStoragePickUp::~SipRedirectorPrivateStoragePickUp()
{
}

void SipRedirectorPrivateStoragePickUp::processNotify(const char* body)
{
   // Initialize Tiny XML document object.
   TiXmlDocument document;
   TiXmlNode* dialog_info;
   if (
      // Load the XML into it.
      document.Parse(body) &&
      // Find the top element, which should be a <dialog-info>.
      (dialog_info = document.FirstChild("dialog-info")) != NULL &&
      dialog_info->Type() == TiXmlNode::ELEMENT)
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipRedirectorPrivateStoragePickUp::processNotify "
                    "Body parsed, <dialog-info> found");
      // Find all the <dialog> elements.
      for (TiXmlNode* dialog = 0;
           dialog = dialog_info->IterateChildren("dialog", dialog); )
      {
         // Process each <dialog> element.
         processNotifyDialogElement(dialog->ToElement());
      }
   }
   else
   {
      // Report error.
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipRedirectorPrivateStoragePickUp::processNotify "
                    "NOTIFY body invalid: '%s'", body);
   } 
}

void SipRedirectorPrivateStoragePickUp::processNotifyDialogElement(
   TiXmlElement* dialog)
{
   // Variables to record the data items that we find in the dialog.
   const char* call_id = NULL;
   const char* local_tag = NULL;
   const char* remote_tag = NULL;
   const char* incoming_string;
   UtlBoolean incoming = FALSE;
   // Duration defaults to 0 if none is given.
   int duration = 0;
   SipRedirectorPickUp::State state = SipRedirectorPickUp::stateUnknown;
   UtlString local_identity;
   UtlString local_target;
   UtlString remote_identity;
   UtlString remote_target;

   // Get attribute values.
   call_id = dialog->Attribute("call-id");
   local_tag = dialog->Attribute("local-tag");
   remote_tag = dialog->Attribute("remote-tag");
   incoming_string = dialog->Attribute("direction");
   // Beware that the direction attribute might be missing.
   incoming =
      incoming_string != NULL &&
      strcasecmp(incoming_string, "recipient") == 0;

   // Get values from children.
   for (TiXmlNode* child = dialog->FirstChild(); child;
        child = child->NextSibling())
   {
      if (child->Type() == TiXmlNode::ELEMENT &&
          strcmp(child->Value(), "duration") == 0)
      {
         // duration element
         // Get the content and convert to an integer.
         UtlString duration_s;
         SipRedirectorPickUp::textContentShallow(duration_s, child->ToElement());
         const char* startptr = duration_s.data();
         char** endptr;
         long int temp = strtol(startptr, endptr, 10);
         // If the duration value passes sanity checks, use it.
         if (((const char*) endptr) == startptr + duration_s.length() &&
             temp >= 0 &&
             temp <= INT_MAX)
         {
            duration = temp;
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipRedirectorPrivateStoragePickUp::"
                          "processNotifyDialogElement "
                          "Invalid <duration> '%s'",
                          duration_s.data());
         }
      }
      else if (child->Type() == TiXmlNode::ELEMENT &&
          strcmp(child->Value(), "state") == 0)
      {
         // state element
         UtlString state_string;
         SipRedirectorPickUp::textContentShallow(state_string, child->ToElement());
         if (state_string.compareTo("early", UtlString::ignoreCase) == 0)
         {
            state = SipRedirectorPickUp::stateEarly;
         }
         else if (state_string.compareTo("confirmed",
                                         UtlString::ignoreCase) == 0)
         {
            state = SipRedirectorPickUp::stateConfirmed;
         }
         else
         {
            /* Other values leave 'state' set to stateUnknown. */ ;
         }
      }
      else if (child->Type() == TiXmlNode::ELEMENT &&
          strcmp(child->Value(), "local") == 0)
      {
         // local element
         processNotifyLocalRemoteElement(child->ToElement(),
                                         local_identity, local_target);
      }
      else if (child->Type() == TiXmlNode::ELEMENT &&
          strcmp(child->Value(), "remote") == 0)
      {
         // remote element
         processNotifyLocalRemoteElement(child->ToElement(),
                                         remote_identity, remote_target);
      }
      else
      {
         /* Ignore unknown elements. */ ;
      } 
   }
   // Report all the information we have on the dialog.
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorPrivateStoragePickUp::"
                 "processNotifyDialogElement "
                 "Dialog values: call_id = '%s', "
                 "local_tag = '%s', remote_tag = '%s', "
                 "incoming = %d, duration = %d, state = %d, "
                 "local_identity = '%s', local_target = '%s', "
                 "remote_identity = '%s', remote_target = '%s'",
                 call_id ? call_id : "[NULL]",
                 local_tag ? local_tag : "[NULL]",
                 remote_tag ? remote_tag : "[NULL]",
                 incoming,
                 duration,
                 state,
                 local_identity.data() ? local_identity.data() : "[NULL]",
                 local_target.data() ? local_target.data() : "[NULL]",
                 remote_identity.data() ? remote_identity.data() : "[NULL]",
                 remote_target.data() ? remote_target.data() : "[NULL]");

   // Check whether the element has enough information to be usable, and
   // ignore it if not.
   if (!(call_id && call_id[0] != '\0' &&
         local_tag && local_tag[0] != '\0' &&
         remote_tag && remote_tag[0] != '\0' &&
         // Must have at least one remote URI, so we can contact the caller.
         (!remote_identity.isNull() || !remote_target.isNull())))
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorPrivateStoragePickUp::"
                    "processNotifyDialogElement "
                    "Dialog element unusable");
      return;
   }

   // Check whether this is a dialog that qualifies for our consideration:
   //   direction is incoming, and
   //   state matches mStateFilter, and
   //   its duration is > the duration of the currently recorded dialog
   if (!(incoming &&
         // Currently, for the state to match, it must be the same as
         // mStateFilter, or mStateFilter is stateDontCare.
         (mStateFilter == SipRedirectorPickUp::stateDontCare ||
          state == mStateFilter) &&
         duration > mTargetDialogDuration))
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorPrivateStoragePickUp::"
                    "processNotifyDialogElement "
                    "Dialog does not qualify");
      return;
   }

   // Save information about this dialog.
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorPrivateStoragePickUp::"
                 "processNotifyDialogElement "
                 "Dialog element saved");
   // Note that the members here that are strings are UtlStrings, so
   // these assignments copy the contents of these char*'s into the
   // UtlStrings, and thus will save the values when we discard the TiXml
   // document tree.
   mTargetDialogCallId = call_id;
   mTargetDialogLocalTag = local_tag;
   mTargetDialogRemoteTag = remote_tag;
   mTargetDialogDuration = duration;
   // Record the URI at which to contact the calling phone.
   // Use the remote target if it exists, otherwise the remote identity.
   mTargetDialogRemoteURI =
      !remote_target.isNull() ? remote_target : remote_identity;
}

void SipRedirectorPrivateStoragePickUp::processNotifyLocalRemoteElement(
   TiXmlElement* element,
   UtlString& identity,
   UtlString& target)
{
   // Get values from children.
   for (TiXmlNode* child = element->FirstChild(); child;
        child = child->NextSibling())
   {
      if (child->Type() == TiXmlNode::ELEMENT &&
          strcmp(child->Value(), "target") == 0)
      {
         // target element
         const char* target_string = child->ToElement()->Attribute("uri");
         target = target_string != NULL ? target_string : "";

         // According to section 4.1.6.2 of draft-ietf-sipping-dialog-package-06,
         // the <target> element can carry URI parameter values in <param>
         // children.  The design of this feature seems to be quite bad,
         // but we recover the parameters for fear that phones use them to
         // identify logical "lines" in a way that helps access the correct
         // calls.
         for (TiXmlNode* param_child = 0;
              param_child = child->IterateChildren("param", param_child);
            )
         {
            // This could probably be done better with a Url object.
            const char* pname = param_child->ToElement()->Attribute("pname");
            const char* pval = param_child->ToElement()->Attribute("pval");
            // According to section 4.1.6.2, a pval of "true" means that
            // the parameter is present but has no value, and "false" means
            // the parameter is explicitly negated.
            // This code also treats a missing pval as the presence
            // of a binary parameter, which isn't provided for in 4.1.6.2,
            // but would be a better way to represent boolean parameters.
            // Check that pname is present and non-null.
            if (pname != NULL && pname[0] != '\0')
            {
               // pval absent or "true" means the presence of a
               // boolean parameter.
               if (pval == NULL || strcmp(pval, "true") == 0)
               {
                  target.append(";");
                  target.append(pname);
               }
               else if (strcmp(pval, "false") == 0)
               {
                  /* Add nothing. */
               }
               else
               {
                  target.append(";");
                  target.append(pname);
                  target.append("=");
                  target.append(pval);
               }
            }
         }
      }
      else if (child->Type() == TiXmlNode::ELEMENT &&
               strcmp(child->Value(), "identity") == 0)
      {
         // identity element
         SipRedirectorPickUp::textContentShallow(identity, child->ToElement());
      }
   }
}

SipRedirectorPickUpNotification::SipRedirectorPickUpNotification(
   RequestSeqNo requestSeqNo,
   int redirectorNo) :
   mRequestSeqNo(requestSeqNo),
   mRedirectorNo(redirectorNo)
{
}

OsStatus SipRedirectorPickUpNotification::signal(const int eventData)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorPickUpNotification::signal "
                 "Fired mRequestSeqNo %d, mRedirectorNo %d",
                 mRequestSeqNo, mRedirectorNo);
   SipRedirectServer::getInstance()->
      resumeRequest(mRequestSeqNo, mRedirectorNo);
   return OS_SUCCESS;
}

SipRedirectorPickUpTask::SipRedirectorPickUpTask(SipUserAgent* pSipUserAgent,
                                                 int redirectorNo) :
   OsServerTask("SipRedirectorPickUpTask-%d"),
   mpSipUserAgent(pSipUserAgent),
   mRedirectorNo(redirectorNo)
{
   // Set up listening for NOTIFYs.
   pSipUserAgent->addMessageObserver(*(this->getMessageQueue()),
                                     SIP_NOTIFY_METHOD,
                                     TRUE, // want to get requests
                                     FALSE, // no responses
                                     TRUE, // Incoming messages
                                     FALSE); // No outgoing messages
}

SipRedirectorPickUpTask::~SipRedirectorPickUpTask()
{
   // Stop listening for NOTIFYs.
   mpSipUserAgent->removeMessageObserver(*(this->getMessageQueue()));
}

UtlBoolean
SipRedirectorPickUpTask::handleMessage(OsMsg& eventMessage)
{
   int msgType = eventMessage.getMsgType();

   switch (msgType)
   {
   case OsMsg::PHONE_APP:
   {      
      // Get a pointer to the message.
      const SipMessage* message =
         ((SipMessageEvent&)eventMessage).getMessage();

      // Extract the request method.
      UtlString method;
      message->getRequestMethod(&method);

      if (method.compareTo(SIP_NOTIFY_METHOD, UtlString::ignoreCase) == 0)
      {
         // Get the Call-Id.
         UtlString callId;
         message->getCallIdField(&callId);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectorPickUpTask::handleMessage "
                       "Start processing NOTIFY CallID '%s'", callId.data());

         {
            // This block holds SipRedirectServer::mMutex.
            OsLock lock(SipRedirectServer::getInstance()->mMutex);

            // Look for a suspended request whose SUBSCRIBE had this Call-Id.
            SipRedirectServerPrivateStorageIterator itor(mRedirectorNo);
            // Fetch a pointer to each element of myContentSource into
            // pStorage.
            SipRedirectorPrivateStoragePickUp* pStorage;
            while (pStorage =
                   dynamic_cast<SipRedirectorPrivateStoragePickUp*> (itor()))
            {
               // Does this request have the same Call-Id?
               if (callId.compareTo(pStorage->mSubscribeCallId) == 0)
               {
                  // This is the request to which this NOTIFY is a response.
                  // Process the NOTIFY and store its information in
                  // *pStorage.
                  const char* body;
                  int length;
                  message->getBody()->getBytes(&body, &length);
                  if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
                  {
                     // Calculate the response delay.
                     struct timeval tv;
                     gettimeofday(&tv, &SipRedirectorPickUp::mTZ);
                     tv.tv_sec -= pStorage->mSubscribeSendTime.tv_sec;
                     tv.tv_usec -= pStorage->mSubscribeSendTime.tv_usec;
                     if (tv.tv_usec < 0)
                     {
                        tv.tv_sec--;
                        tv.tv_usec += 1000000;
                     }
                     OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                   "SipRedirectorPickUpTask::handleMessage "
                                   "NOTIFY for request %d, delay %d.%06d, "
                                   "body '%s'",
                                   itor.requestSeqNo(),
                                   (int) tv.tv_sec, (int) tv.tv_usec,
                                   body);
                  }
                  // Parse this NOTICE and store the needed
                  // information in private storage.
                  pStorage->processNotify(body);

                  // Don't bother checking for a match with any other request.
                  break;
               }
            }
         }

         // Return a 200 response.
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectServer::handleMessage "
                       "Sending 200 OK response to NOTIFY");
         SipMessage response;
         response.setOkResponseData(message);
         mpSipUserAgent->send(response);
      }
   }
   break;
   }

   return TRUE;
}

void
SipRedirectorPickUp::textContentShallow(UtlString& string,
                                        TiXmlElement *element)
{
   // Clear the string.
   string.remove(0);

   // Iterate through all the children.
   for (TiXmlNode* child = element->FirstChild(); child;
        child = child->NextSibling())
   {
      // Examine the text nodes.
      if (child->Type() == TiXmlNode::TEXT)
      {
         // Append the content to the string.
         string.append(child->Value());
      }
   }
}

void SipRedirectorPickUp::textContentDeep(UtlString& string,
                                          TiXmlElement *element)
{
   // Clear the string.
   string.remove(0);

   // Recurse into the XML.
   textContentDeepRecursive(string, element);
}

void
SipRedirectorPickUp::textContentDeepRecursive(UtlString& string,
                                              TiXmlElement *element)
{
   // Iterate through all the children.
   for (TiXmlNode* child = element->FirstChild(); child;
        child = child->NextSibling())
   {
      // Examine the text nodes.
      if (child->Type() == TiXmlNode::TEXT)
      {
         // Append the content to the string.
         string.append(child->Value());
      }
      else if (child->Type() == TiXmlNode::ELEMENT)
      {
         // Recurse on this element.
         textContentDeepRecursive(string, child->ToElement());
      }
   }
}
