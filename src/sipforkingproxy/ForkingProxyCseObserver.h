// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
//////////////////////////////////////////////////////////////////////////////

#ifndef _ForkingProxyCseObserver_h_
#define _ForkingProxyCseObserver_h_

// SYSTEM INCLUDES
#include "utl/UtlString.h"

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include <os/OsFS.h>
#include <os/OsTimer.h>
#include "CallStateEventBuilder_XML.h"
#include "CallStateEventBuilder_DB.h"
#include "CallStateEventWriter_XML.h"
#include "CallStateEventWriter_DB.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;

/// Observe and record Call State Events in the Forking Proxy
class ForkingProxyCseObserver : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   ForkingProxyCseObserver(SipUserAgent&         sipUserAgent,
                           const UtlString&      dnsName,
                           CallStateEventWriter* pWriter
                           );
     //:Default constructor

   virtual
   ~ForkingProxyCseObserver();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SipUserAgent*              mpSipUserAgent;
   CallStateEventBuilder*     mpBuilder;
   CallStateEventWriter*      mpWriter;
   
   int                        mSequenceNumber;
   OsTimer*                   mFlushTimer;
   
   /// no copy constructor or assignment operator
   ForkingProxyCseObserver(const ForkingProxyCseObserver& rForkingProxyCseObserver);
   ForkingProxyCseObserver operator=(const ForkingProxyCseObserver& rForkingProxyCseObserver);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ForkingProxyCseObserver_h_
