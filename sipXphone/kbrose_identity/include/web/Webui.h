// $Id: //depot/OPENDEV/sipXphone/include/web/Webui.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _Webui_h_
#define _Webui_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "pinger/Pinger.h"
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsConfigDb.h"
#include "os/OsServerTask.h"
#include "net/HttpServer.h"
#include "net/SdpCodecFactory.h"



// DEFINES

// MACROS

// EXTERNAL FUNCTIONS
UtlString httpCreateRow(const char* row, const char* value);
void processGetConfig(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response);
                      
// EXTERNAL VARIABLES
// CONSTANTS

#define CONFIG_PHONESET_DEPLOYMENT_SERVER	"PHONESET_DEPLOYMENT_SERVER"
#define CONFIG_PHONESET_LOGICAL_ID			"PHONESET_LOGICAL_ID" 
#define CONFIG_PHONESET_ADMIN_DOMAIN		"PHONESET_ADMIN_DOMAIN"
#define CONFIG_PHONESET_ADMIN_ID			"PHONESET_ADMIN_ID"
#define CONFIG_PHONESET_ADMIN_PWD			"PHONESET_ADMIN_PWD"
#define CONFIG_PHONESET_AVAILABLE_BEHAVIOR	"PHONESET_AVAILABLE_BEHAVIOR"
#define CONFIG_PHONESET_BUSY_BEHAVIOR		"PHONESET_BUSY_BEHAVIOR"
#define CONFIG_PHONESET_FORWARD_UNCONDITIONAL "PHONESET_FORWARD_UNCONDITIONAL"
#define	CONFIG_PHONESET_DIALPLAN_LENGTH		"PHONESET_DIALPLAN_LENGTH"
#define	CONFIG_PHONESET_DIGITMAP			"PHONESET_DIGITMAP"
#define CONFIG_PHONESET_EXTENSION			"PHONESET_EXTENSION"
#define	CONFIG_PHONESET_EXTERNAL_IP_ADDRESS	"PHONESET_EXTERNAL_IP_ADDRESS"
#define CONFIG_PHONESET_HTTP_AUTH_DB		"PHONESET_HTTP_AUTH_DB"
#define	CONFIG_PHONESET_HTTP_PORT			"PHONESET_HTTP_PORT"
#define	CONFIG_PHONESET_LOGICAL_ID			"PHONESET_LOGICAL_ID"
#define CONFIG_PHONESET_NETWORK_DUPLEX      "PHONESET_NETWORK_DUPLEX" 
#define	CONFIG_PHONESET_OUTGOING_CALL_PROTOCOL "PHONESET_OUTGOING_CALL_PROTOCOL"
#define	CONFIG_PHONESET_RTP_PORT_START		"PHONESET_RTP_PORT_START"
#define	CONFIG_PHONESET_TIME_DST_RULE		"PHONESET_TIME_DST_RULE"
#define	CONFIG_PHONESET_TIME_OFFSET			"PHONESET_TIME_OFFSET"
#define	CONFIG_PHONESET_TIME_SERVER			"PHONESET_TIME_SERVER"
#define CONFIG_PHONESET_CALL_WAITING_BEHAVIOR "PHONESET_CALL_WAITING_BEHAVIOR"
#define CONFIG_PHONESET_RINGER				"PHONESET_RINGER"
#define CONFIG_PHONESET_DND					"PHONESET_DND"
#define CONFIG_PHONESET_DND_METHOD			"PHONESET_DND_METHOD"
#define CONFIG_PHONESET_LOGO_URL			"PHONESET_LOGO_URL"
#define	SIP_ADDRESS							"SIP_ADDRESS"
#define	SIP_AUTHENTICATE_DB					"SIP_AUTHENTICATE_DB"
#define	SIP_AUTHENTICATE_REALM				"SIP_AUTHENTICATE_REALM"
#define	SIP_AUTHENTICATE_SCHEME				"SIP_AUTHENTICATE_SCHEME"
#define	SIP_AUTHORIZE_PASSWORD				"SIP_AUTHORIZE_PASSWORD"
#define	SIP_AUTHORIZE_USER					"SIP_AUTHORIZE_USER"
#define	SIP_CONFIG_SERVER_ADDRESS			"SIP_CONFIG_SERVER_ADDRESS"				
#define	SIP_CONFIG_SERVER_PORT				"SIP_CONFIG_SERVER_PORT"
#define	SIP_DIRECTORY_SERVERS				"SIP_DIRECTORY_SERVERS"
#define	SIP_FORWARD_ON_BUSY					"SIP_FORWARD_ON_BUSY"
#define	SIP_FORWARD_ON_NO_ANSWER			"SIP_FORWARD_ON_NO_ANSWER"
#define	SIP_FORWARD_UNCONDITIONAL			"SIP_FORWARD_UNCONDITIONAL"
#define	SIP_PROXY_SERVERS					"SIP_PROXY_SERVERS"
#define	SIP_REGISTER_PERIOD					"SIP_REGISTER_PERIOD"
#define	SIP_REGISTRY_SERVERS				"SIP_REGISTRY_SERVERS"
#define	SIP_SESSION_REINVITE_TIMER			"SIP_SESSION_REINVITE_TIMER"
#define	SIP_TCP_PORT						"SIP_TCP_PORT"
#define	SIP_UDP_PORT						"SIP_UDP_PORT"
#define	CONFIG_PHONESET_NO_ANSWER_TIMEOUT	"PHONESET_NO_ANSWER_TIMEOUT"
#define CONFIG_PHONESET_HTTP_PROXY_PORT		"PHONESET_HTTP_PROXY_PORT"
#define CONFIG_PHONESET_HTTP_PROXY_HOST		"PHONESET_HTTP_PROXY_HOST"
#define ADDITIONAL_PARAMETERS				"ADDITIONAL_PARAMETERS"
#define PHONESET_MSG_WAITING_SUBSCRIBE		"PHONESET_MSG_WAITING_SUBSCRIBE"
#define PHONESET_VOICEMAIL_RETRIEVE			"PHONESET_VOICEMAIL_RETRIEVE"
#define PHONESET_SNMP_TRAP_DESTS			"PHONESET_SNMP_TRAP_DESTS"
#define CONFIG_PHONESET_TELNET_ACCESS		"PHONESET_TELNET_ACCESS"
#define PHONESET_VERSIONCHECK_URL     "PHONESET_VERSIONCHECK_URL"
#define PHONESET_VERSIONCHECK_PERIOD  "PHONESET_VERSIONCHECK_PERIOD"

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
//:Webui class
// This is the main Webui task. It is a singleton task and is responsible
// for initializing the Webui device and starting up any other tasks that
// are needed.
class Url;

class Webui
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   static Webui* getWebuiTask(void);
     //:Return a pointer to the Webui task, creating it if necessary

   virtual
   ~Webui();
     //:Destructor

   void initWebui(HttpServer* pHttpServer);

/* ============================ MANIPULATORS ============================== */


   static void processHomePage(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processApplications(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void getPingerConfigAdmin(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void getPingerConfigAdminMiddle(UtlString& html, 
	                  OsConfigDb& dbValues) ;
   
   static void getPingerConfigUser(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;
  
   static void setPingerConfigUser(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void setPingerConfigAdmin(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processLogin(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processChangePassword(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processSetPassword(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response);

   static void processDeleteUser(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processConfig(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processConfigEnroll(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response);

   static void processRestart(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response);

   static void processFonts(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;
   
   static void processLogo(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processBackgroundImage(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void displayMenu(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void createJsConfigAdmin(UtlString& html, UtlString strParamName) ;

   static void setConfigValuesUser(OsConfigDb *dbPage) ;
 
   static void setConfigValuesAdmin(OsConfigDb *dbPage) ;

   static void setMultipleValues(const char* paramKey, UtlString paramValue, OsConfigDb& changeDb) ;

   static void addAdditionalEntries(UtlString paramValue, OsConfigDb& changeDb) ;

   static int validateChangePassword(UtlString userName, UtlString password1, UtlString password2, UtlString &errorMessage) ;

   static int deleteUser(UtlString userId, UtlString password) ;

   static int changeUserPassword(UtlString userId, UtlString password) ;

   static void getAdditionalValues(OsConfigDb& dbAdditional) ;

   static void insertHeader(UtlString& html, UtlBoolean bIsSecure) ;

   static void processAppManager(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response);

   static void processAddSpeeddial(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processViewSpeeddial(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processViewSpeeddialMiddle(UtlString& html) ;

   static void processDeleteSpeeddial(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processViewSyslog(const HttpRequestContext& requestContext,
                                 const HttpMessage& request,
                                 HttpMessage*& response);
   static void processConfigSyslog(const HttpRequestContext& requestContext,
                                   const HttpMessage& request,
                                   HttpMessage*& response);
   static void appendSysLogEntry(const char *szSourceEntry, UtlString& target) ;


   //Multiline
   static void processViewLinesCgi(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processAddLinesCgi(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;
   static void processAddCredentialsCgi(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processDhtmlMenu(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processSpeeddialJar(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;


   static UtlBoolean isContainedInPingerConfig(UtlString strKey) ;

   static UtlBoolean isAdminMultiValueConfig(UtlString strKey) ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
	

   Webui();
     //:Constructor (called only indirectly via getWebuiTask())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
  

   //HttpServer*       mpHttpServer;       // Http Server

   // Static data members used to enforce Singleton behavior
   static Webui*    spInstance;    // pointer to the single instance of
                                    //  the Webui class
   static OsBSem     sLock;         // semaphore used to ensure that there
                                    //  is only one instance of this class   

   // device profile (pinger-config) parameters
   static const char* deviceConfig[] ;

   // all parameters managed by the Web UI
   static const char* allConfig[] ;

private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _Webui_h_


