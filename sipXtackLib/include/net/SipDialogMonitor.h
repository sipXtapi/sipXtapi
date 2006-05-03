//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SIPDIALOGMONITOR_H_
#define _SIPDIALOGMONITOR_H_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsBSem.h>
#include <net/StateChangeNotifier.h>
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
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * A SipDialogMonitor is an object that is used for monitoring the on/off hook
 * status of all the SIP user agents for the given extensions based on the dialog
 * events. When the dialog event changes, this information will be composed in a
 * resource list and stored in a NOTIFIER so that other clients can subcribe for
 * the same information based on the resource list. If a StateChangeNotifier is
 * registered with SipDialogMonitor, the state change will also be sent out via
 * the StateChangeNotifier.
 *
 */

class SipDialogMonitor
{
  public:

   SipDialogMonitor(SipUserAgent* userAgent, /**<
                                               * Sip user agent for sending out
                                               * SUBSCRIBEs and receiving NOTIFYs
                                               */
                    UtlString& domainName,   ///< sipX domain name
                    int hostPort,            ///< Host port
                    int refreshTimeout,      ///< refresh timeout for SUBSCRIBEs
                    bool toBePublished);     ///< option to publish for other subscriptions

   virtual ~SipDialogMonitor();

   /// Add an extension to a group to be monitored
   bool addExtension(UtlString& groupName, Url& contactUrl);

   /// Remove an extension from a group to be monitored
   bool removeExtension(UtlString& groupName, Url& contactUrl);
  
   /// Registered a StateChangeNotifier
   void addStateChangeNotifier(const char* fileUrl, StateChangeNotifier* notifier);

   /// Unregistered a StateChangeNotifier
   void removeStateChangeNotifier(const char* fileUrl);

  protected:

   /// Add the contact and dialog event to the subscribe list
   void addDialogEvent(UtlString& contact, SipDialogEvent* dialogEvent);

   /// Publish the dialog event package to the resource list
   void publishContent(UtlString& contact, SipDialogEvent* dialogEvent);

   /// Send the state change to the notifier
   void notifyStateChange(UtlString& contact, SipDialogEvent* dialogEvent);

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

  private:

   SipUserAgent* mpUserAgent;  
   UtlString mDomainName;
   UtlString mContact;
   int mRefreshTimeout;
   bool mToBePublished;
   
   OsBSem mLock;
   
   SipDialogMgr mDialogManager;
   SipRefreshManager* mpRefreshMgr;
   SipSubscribeClient* mpSipSubscribeClient;
      
   SipDialogMgr mDialogMgr;
   SipSubscriptionMgr* mpSubscriptionMgr;
   SipSubscribeServerEventHandler mPolicyHolder;
   SipPublishContentMgr mSipPublishContentMgr;
   SipSubscribeServer* mpSubscribeServer;

   UtlHashMap mMonitoredLists;
   UtlHashMap mDialogEventList;
   UtlHashMap mStateChangeNotifiers;   

   /// Disabled copy constructor
   SipDialogMonitor(const SipDialogMonitor& rSipDialogMonitor);

   /// Disabled assignment operator
   SipDialogMonitor& operator=(const SipDialogMonitor& rhs);
};

#endif // _SIPDIALOGMONITOR_H_
