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

#ifndef _SipAaa_h_
#define _SipAaa_h_

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include <net/SipNonceDb.h>


// DEFINES
#define CONFIG_LOG_FILE       "sipauthproxy.log"
#define CONFIG_LOG_DIR        SIPX_LOGDIR
#define CONFIG_ETC_DIR        SIPX_CONFDIR
#define CONFIG_SETTINGS_FILE  "authproxy-config"
#define AUTH_RULES_FILENAME   "authrules.xml"

// Configuration names pulled from config-file
#define CONFIG_SETTING_LOG_LEVEL      "SIP_AUTHPROXY_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_AUTHPROXY_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_AUTHPROXY_LOG_DIR"
#define LOG_FACILITY                  FAC_SIP

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class OsConfigDb;
class UrlMapping;
class SipMessage;
class ResultSet;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipAaa : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipAaa(SipUserAgent& sipUserAgent,
          const char* authenticationRealm,
          UtlString& routeName);
     //:Default constructor

   SipAaa(const SipAaa& rSipAaa);
     //:Copy constructor

   virtual
   ~SipAaa();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   SipAaa& operator=(const SipAaa& rhs);
     //:Assignment operator

   virtual UtlBoolean handleMessage(OsMsg& rMsg);
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlBoolean isAuthenticated(
        const SipMessage& sipRequest,
        UtlString& authUser,
        SipMessage& authResponse );

    UtlBoolean isAuthorized (
        const SipMessage& sipRequest,
        const ResultSet& permissions, 
        const char* authUser,
        SipMessage& authResponse,
        UtlString& matchedPermission );

    void calcRouteSignature(UtlString& matchedPermission,
                           UtlString& callId, 
                           UtlString& fromTag,
                           UtlString& signature);

    SipUserAgent* mpSipUserAgent;
    UrlMapping* mpAuthorizationRules;
    UtlString mRealm;
    UtlString mSignatureSecret;
    SipNonceDb mNonceDb;
    long mNonceExpiration;
    UtlString mRouteName;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipAaa_h_

