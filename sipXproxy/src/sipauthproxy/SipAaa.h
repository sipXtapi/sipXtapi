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

// The period of time in seconds that nonces are valid, in seconds.
#define NONCE_EXPIRATION_PERIOD    (60 * 5)     // five minutes

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


/**
 * SipAaa is the core of the auth proxy
 */
class SipAaa : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   SipAaa(SipUserAgent& sipUserAgent,
          const char* authenticationRealm,
          UtlString& routeName);
     //:Default constructor

   SipAaa(SipUserAgent& sipUserAgent,
          const char* authenticationRealm,
          const UtlString& routeName,
          const char* authRules);
     //: Test constructor

   SipAaa(const SipAaa& rSipAaa);
     //:Copy constructor

   virtual ~SipAaa();
     //:Destructor

   SipAaa& operator=(const SipAaa& rhs);
     //:Assignment operator

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

   /// Modify the message as needed to be proxied
   bool proxyMessage(SipMessage& sipRequest);
   ///< @returns true if message should be sent, false if not

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    // Empty constructor, for use by unit tests only
    SipAaa() {}

    /**
     * Check the credentials in sipRequest for valid credentials for a
     * user in mRealm.
     * If valid credentials found, put the user in authUser and return TRUE.
     * If not, compose a suitable authentication challenge response in
     * authResponse and return FALSE.
     */
    UtlBoolean isAuthenticated(
        const SipMessage& sipRequest,
        UtlString& authUser,
        SipMessage& authResponse);

    /**
     * Compare the permissions required for the user (requiredPermissions) with the
     * permissions granted to the user (grantedPermissions).  Return true in two
     * cases:
     *  # If the user has *any* (not all) of the required permissions
     *  # If requiredPermissions contains one or more of the special permissions "ValidUser" or "RecordRoute".
     * and false otherwise.
     * In the second case (special permissions) the grantedPermissions don't matter, only
     * the requiredPermissions matter.
     * Return the first matched permission in matchedPermission.
     * Return unmatched permissions in unmatchedPermissions as a single string, using "+"
     * as a delimiter between permission names.
     * One would expect permissions to be logically AND'd not OR'd, that is, *all*
     * requiredPermissions should be required, not just one of them.  This is probably
     * a bug, but since many installations may now depend on this behavior, we can't change
     * it without including a configuration option that provides backward compatibility.
     */
    UtlBoolean isAuthorized(
       const ResultSet& requiredPermissions,
       const ResultSet& grantedPermissions,
       UtlString& matchedPermission,
       UtlString& unmatchedPermissions);

    /**
     * Similar to the previous form of isAuthorized(), but takes the user
     * authUser and looks up all his permissions.  These are then fed to the
     * previous form of isAuthorized as the grantedPermissions.
     * If it returns TRUE, this method copies matchedPermission and returns TRUE.
     * If it returns FALSE, this method constructs an appropriate authorization
     * challenge in authResponse and returns FALSE.
     */
    UtlBoolean isAuthorized(
        const SipMessage& sipRequest,
        const ResultSet& requiredPermissions, 
        const char* authUser,
        SipMessage& authResponse,
        UtlString& matchedPermission);

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

    friend class SipAaaTest;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipAaa_h_
