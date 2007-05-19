//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

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
#include <cp/SipPresenceMonitor.h>


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
   LinePresenceMonitor(int userAgentPort,     ///< user agent port
                       UtlString& domainName, ///< sipX domain name
                       UtlString& groupName,  ///< name of the group to be monitored
                       bool local,            ///< option for using local or remote monitor
                       Url& remoteServerUrl); ///< remote server url
  
   /// Destructor
   virtual ~LinePresenceMonitor();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   /// Set the state value.
   virtual bool setStatus(const Url& aor, const Status value);

   /// Subscribe a line in the list
   OsStatus subscribe(LinePresenceBase* line);

   /// Unsubscribe a line from the list
   OsStatus unsubscribe(LinePresenceBase* line);
   
   /// Subscribe a list
   OsStatus subscribe(UtlSList& list);
   
   /// Unsubscribe a list
   OsStatus unsubscribe(UtlSList& list);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipUserAgent* mpUserAgent;
   UtlString mGroupName;
   bool mLocal;
   
   SipDialogMonitor* mpDialogMonitor;
   SipPresenceMonitor* mpPresenceMonitor;

   SipDialogMgr mDialogManager;
   SipRefreshManager* mpRefreshMgr;
   SipSubscribeClient* mpSipSubscribeClient;
   
   Url mRemoteServer;

   UtlHashMap mSubscribeList;

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


