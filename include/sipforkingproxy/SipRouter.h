// $Id: //depot/OPENDEV/sipXproxy/include/sipforkingproxy/SipRouter.h#7 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipRouter_h_
#define _SipRouter_h_

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <os/OsServerTask.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class ForwardRules;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipRouter : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipRouter(SipUserAgent& sipUserAgent, 
             ForwardRules& forwardingRules,
             OsBoolean useAuthServer,
             const char* authServer,
             OsBoolean shouldRecordRoute);
     //:Default constructor

   SipRouter(const SipRouter& rSipRouter);
     //:Copy constructor

   virtual
   ~SipRouter();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   SipRouter& operator=(const SipRouter& rhs);
     //:Assignment operator

   virtual OsBoolean handleMessage(OsMsg& rMsg);
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    SipUserAgent* mpSipUserAgent;
    ForwardRules* mpForwardingRules;
    OsBoolean mShouldRecordRoute;
    OsBoolean mAuthEnabled;
    UtlString mAuthServer;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipRouter_h_
