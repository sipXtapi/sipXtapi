// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
//////////////////////////////////////////////////////////////////////////////

#ifndef _AuthProxyCseObserver_h_
#define _AuthProxyCseObserver_h_

// SYSTEM INCLUDES
#include "utl/UtlString.h"

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include <os/OsFS.h>
#include "CallStateEventBuilder_XML.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class OsTimer;

/// Observe and record Call State Events in the Forking Proxy
class AuthProxyCseObserver : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   AuthProxyCseObserver(SipUserAgent&    sipUserAgent,
                           const UtlString& dnsName,
                           OsFile*          eventFile
                           );
     //:Default constructor

   virtual
   ~AuthProxyCseObserver();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SipUserAgent*             mpSipUserAgent;
   CallStateEventBuilder_XML mBuilder;
   OsFile*                   mEventFile;
   int                       mSequenceNumber;
   OsTimer*                  mFlushTimer;
   
   /// no copy constructor or assignment operator
   AuthProxyCseObserver(const AuthProxyCseObserver& rAuthProxyCseObserver);
   AuthProxyCseObserver operator=(const AuthProxyCseObserver& rAuthProxyCseObserver);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _AuthProxyCseObserver_h_
