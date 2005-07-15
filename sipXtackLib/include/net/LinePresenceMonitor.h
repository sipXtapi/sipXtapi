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
#include <net/LinePresenceBase.h>
#include <net/SipDialogMonitor.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * A LinePresenceMonitor is an object that is used in the ACD to receive the
 * presence state change notifitcation from the dialog monitor.
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
   LinePresenceMonitor(int userAgentPort, ///< user agent port
                       UtlString& domainName); ///< sipX domain name
  
   /// Destructor
   virtual ~LinePresenceMonitor();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   /// Set the state value.
   virtual void setStatus(const Url& aor, const Status value);

   OsStatus subscribe(LinePresenceBase* line);

   OsStatus unsubscribe(LinePresenceBase* line);


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipUserAgent* mpUserAgent;
   SipDialogMonitor* mpDialogMonitor;
   
   UtlHashMap mSubscribeMap;

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


