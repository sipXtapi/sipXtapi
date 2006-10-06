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

#include <stdio.h>

// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlHashBagIterator.h>
#include <net/SipResourceList.h>
#include <net/SipUserAgent.h>
#include <net/NetMd5Codec.h>
#include <net/SipMessage.h>
#include <net/SipDialogMgr.h>
#include <net/SipRefreshManager.h>
#include <net/SipSubscribeClient.h>


#define OUTPUT_PREFIX "[start of body]\n"
#define OUTPUT_SUFFIX "\n[end of body]\n"

void subscriptionStateCallback(SipSubscribeClient::SubscriptionState newState,
                               const char* earlyDialogHandle,
                               const char* dialogHandle,
                               void* applicationData,
                               int responseCode,
                               const char* responseText,
                               long expiration,
                               const SipMessage* subscribeResponse)
{
   fprintf(stderr,
           "subscriptionStateCallback is called with responseCode = %d (%s)\n",
           responseCode, responseText); 
}                                            


// Callback to handle incoming NOTIFYs.
void notifyEventCallback(const char* earlyDialogHandle,
                         const char* dialogHandle,
                         void* applicationData,
                         const SipMessage* notifyRequest)
{
   fprintf(stderr,
           "notifyEventCallback called with early handle '%s' handle '%s' message:\n",
           earlyDialogHandle, dialogHandle);
   if (notifyRequest)
   {
      const HttpBody* notifyBody = notifyRequest->getBody();
      fprintf(stdout, OUTPUT_PREFIX);
      if (notifyBody)
      {
         UtlString messageContent;
         int bodyLength;
         notifyBody->getBytes(&messageContent, &bodyLength);
         fprintf(stdout, "%s", messageContent.data());
      }
      fprintf(stdout, OUTPUT_SUFFIX);
      // Make sure the event notice is written promptly.
      fflush(stdout);
   }
}


int main(int argc, char* argv[])
{
   // Initialize logging.
   OsSysLog::initialize(0, "test");
   OsSysLog::setOutputFile(0, "log");
   OsSysLog::setLoggingPriority(PRI_DEBUG);

   if (argc == 1)
   {
      fprintf(stderr, "Usage: %s target-URI\n", argv[0]);
      exit(1);
   }

   // The URI to subscribe to.
   UtlString resourceId = argv[1];

   // Seconds to set for subscription.
   int refreshTimeout = 300;

   // The domain name to call myself.
   UtlString myDomainName = "example.com";

   // Create the SIP Subscribe Client

   SipUserAgent* pSipUserAgent = new SipUserAgent(PORT_DEFAULT, PORT_DEFAULT, PORT_NONE);

   SipDialogMgr dialogManager;

   SipRefreshManager refreshMgr(*pSipUserAgent, dialogManager);
   refreshMgr.start();

   SipSubscribeClient sipSubscribeClient(*pSipUserAgent, dialogManager,
                                         refreshMgr);
   sipSubscribeClient.start();  

   UtlString toUri(resourceId);
   UtlString fromUri = "dialogwatch@" + myDomainName;
   UtlString earlyDialogHandle;
            
   fprintf(stderr, "resourceId '%s' fromUri '%s' toUri '%s'\n",
           resourceId.data(), fromUri.data(), toUri.data());

   UtlBoolean status =
      sipSubscribeClient.addSubscription(resourceId.data(),
                                         DIALOG_EVENT_TYPE,
                                         DIALOG_EVENT_CONTENT_TYPE,
                                         fromUri.data(),
                                         toUri.data(),
                                         NULL,
                                         refreshTimeout,
                                         (void *) NULL,
                                         subscriptionStateCallback,
                                         notifyEventCallback,
                                         earlyDialogHandle);
               
   if (!status)
   {
      fprintf(stderr, "Subscription attempt failed.\n");
   }
   else
   {
      fprintf(stderr, "Subscription attempt succeeded.  Handle: '%s'\n",
              earlyDialogHandle.data());
   }
   while (1)
   {
      sleep(1000);
   }
}
