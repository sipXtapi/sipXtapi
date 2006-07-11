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

#ifndef _LINEPRESENCEMONITOR_H_
#define _LINEPRESENCEMONITOR_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsBSem.h>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>
#include <net/StateChangeNotifier.h>
#include <net/SipDialogMonitor.h>
#include <cp/LinePresenceBase.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * A LinePresenceMonitor is an object that is used in the ACD-like clients to
 * receive the presence state change notifitcation from the dialog/presence
 * monitor. It has an option to use either local or remote dialog/presence
 * monitor. If using the remote monitor, subscription is sent out via XML-RPC.
 * 
 * This class is derived from StateChangeNotifier class.
 *
 */

class LinePresenceMonitor : public StateChangeNotifier
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Constructor
   LinePresenceMonitor(int userAgentPort,       ///< user agent port
                       UtlString& domainName,   ///< sipX domain name
                       UtlString& groupName,    ///< name of the group to be monitored
                       bool local,              ///< option for using local or remote monitor
                       Url& remoteServerUrl,    ///< remote monitor server url
                       Url& presenceServerUrl); ///< presence server url
  
   /// Destructor
   virtual ~LinePresenceMonitor();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   /// Set the state value.
   virtual bool setStatus(const Url& aor, const Status value);

   /// Subscribe the dialog on a specific line in the list
   OsStatus subscribeDialog(LinePresenceBase* line);

   /// Unsubscribe the dialog on a specific line from the list
   OsStatus unsubscribeDialog(LinePresenceBase* line);
   
   /// Subscribe the dialogs for a list
   OsStatus subscribeDialog(UtlSList& list);
   
   /// Unsubscribe the dialogs for a list
   OsStatus unsubscribeDialog(UtlSList& list);

   /// Subscribe the presence on a specific line in the list
   OsStatus subscribePresence(LinePresenceBase* line);

   /// Unsubscribe the presence on a specific line from the list
   OsStatus unsubscribePresence(LinePresenceBase* line);
   
   /// Subscribe the presence for a list
   OsStatus subscribePresence(UtlSList& list);
   
   /// Unsubscribe the presence for a list
   OsStatus unsubscribePresence(UtlSList& list);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipUserAgent* mpUserAgent;
   UtlString mGroupName;
   bool mLocal;
   UtlString mDomainName;
   UtlString mContact;
   
   SipDialogMonitor* mpDialogMonitor;

   SipDialogMgr mDialogManager;
   SipRefreshManager* mpRefreshMgr;
   SipSubscribeClient* mpSipSubscribeClient;
   
   Url mRemoteServer;
   UtlString mPresenceServer;

   UtlHashMap mDialogSubscribeList;
   UtlHashMap mPresenceSubscribeList;
   UtlHashMap mDialogHandleList;

   OsBSem mLock;                  /**<
                                    * semaphore used to ensure that there
                                    * is only one instance of this class
                                    */
                                    
   /// Disabled copy constructor
   LinePresenceMonitor(const LinePresenceMonitor& rLinePresenceMonitor);

   /// Disabled assignment operator
   LinePresenceMonitor& operator=(const LinePresenceMonitor& rhs); 
};

/* ============================ INLINE METHODS ============================ */

#endif  // _LINEPRESENCEMONITOR_H_


