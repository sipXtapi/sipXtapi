// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTSERVER_H
#define SIPREDIRECTSERVER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "digitmaps/UrlMapping.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;

class SipRedirectServer : public OsServerTask  
{
public:
    SipRedirectServer();
    virtual ~SipRedirectServer();
    UtlBoolean initialize(
        SipUserAgent* mSipUserAgent,
        const UtlString& configDir,
        const UtlString& mediaServer,
        const UtlString& voicemailServer,
        const UtlString& localDomainHost);

    int registrationDBLookUp (
        const SipMessage& message, 
        SipMessage& response);

    int aliasDBLookUp (
        const SipMessage& message, 
        SipMessage& response);

    int urlMappingDBLookUp (
        const SipMessage& message, 
        const int& passNumber,
        SipMessage& response);
   
protected:

    UtlBoolean mIsStarted;
    SipUserAgent* mSipUserAgent;
    UrlMapping mMap;
    UrlMapping mFallback;
    OsStatus mUrlMappingRulesLoaded;
    OsStatus mFallbackRulesLoaded;

    // functions
    UtlBoolean handleMessage( OsMsg& eventMessage );
};
#endif // SIPREDIRECTSERVER_H

