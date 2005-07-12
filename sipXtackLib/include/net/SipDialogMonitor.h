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

#ifndef _SIPDIALOGMONITOR_H_
#define _SIPDIALOGMONITOR_H_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsBSem.h>
#include <net/SipUserAgent.h>
#include <net/SipDialogMgr.h>
#include <net/SipSubscriptionMgr.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipSubscribeServer.h>
#include <net/SipRefreshManager.h>
#include <net/SipSubscribeClient.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipDialogEvent.h>
#include <utl/UtlSList.h>
#include <utl/UtlHashMap.h>

// DEFINES
// MACROS   SipDialogMgr dialogManager;

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * A SipDialogMonitor is an object that is used for monitoring all the extensions
 * based on the dialog event.
 *
 */

class SipDialogMonitor
{
  public:

   SipDialogMonitor(SipUserAgent* userAgent,
                    UtlString& domainName,
                    UtlString& hostAddress,
                    int refreshTimeout);

   virtual ~SipDialogMonitor();

   /// Add an extension to a group
   void addExtension(UtlString& groupName, UtlString& contact);

   /// Remove an extension from a group
   void removeExtension(UtlString& groupName, UtlString& contact);
   
   
  
   /// Add the contact and dialog event to the subscribe list
   void addDialogEvent(UtlString* contact, SipDialogEvent* dialogEvent);

   /// Publish the dialog event package to the resource list
   void publishContent(UtlString& contact, SipDialogEvent* dialogEvent);

   static void subscriptionStateCallback(SipSubscribeClient::SubscriptionState newState,
                                         const char* earlyDialogHandle,
                                         const char* dialogHandle,
                                         void* applicationData,
                                         int responseCode,
                                         const char* responseText,
                                         long expiration,
                                         const SipMessage* subscribeResponse);

   static void notifyEventCallback(const char* earlyDialogHandle,
                                   const char* dialogHandle,
                                   void* applicationData,
                                   const SipMessage* notifyRequest);
                                   
   void handleNotifyMessage(const SipMessage* notifyMessage);

  protected:

  private:

   SipUserAgent* mpUserAgent;  
   UtlString mDomainName;
   UtlString mHostAddress;
   int mRefreshTimeout;
   
   OsBSem mLock;
   
   SipDialogMgr mDialogManager;
   SipRefreshManager* mpRefreshMgr;
   SipSubscribeClient* mpSipSubscribeClient;
      
   SipDialogMgr mDialogMgr;
   SipSubscriptionMgr* mpSubscriptionMgr;
   SipSubscribeServerEventHandler mPolicyHolder;
   SipPublishContentMgr mSipPublishContentMgr;
   SipSubscribeServer* mpSubscribeServer;

   UtlHashMap mSubscribeList;   
   UtlHashMap mMonitoredLists;
};

#endif // _SIPDIALOGMONITOR_H_
