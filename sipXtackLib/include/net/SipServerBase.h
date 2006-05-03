//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SipServerBase_h_
#define _SipServerBase_h_

// SYSTEM INCLUDES
//#include <...>

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
class OsConfigDb;
class SipUserAgent;
class SipMessage;
//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipServerBase : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   virtual ~SipServerBase();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
    virtual UtlBoolean handleMessage(OsMsg& eventMessage) = 0;
    virtual void initialize() = 0;

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        virtual UtlBoolean isAuthorized(const SipMessage* message, SipMessage * responseMessage) = 0;
   virtual UtlBoolean isValidDomain(const SipMessage* message, SipMessage * responseMessage) = 0;
   virtual UtlBoolean isPermitted(const SipMessage* message, SipMessage * responseMessage) = 0;

   SipUserAgent* mSipUserAgent;
   UtlString mDefaultDomain;

   SipServerBase(const SipServerBase& rSipServerBase);
   //:Copy constructor (disabled)
   SipServerBase& operator=(const SipServerBase& rhs);
   //:Assignment operator (disabled)
   SipServerBase( SipUserAgent* sipUserAgent,
                 const UtlString& defaultDomain = "");

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipServerBase_h_
