// $Id: //depot/OPENDEV/sipXphone/include/pinger/Pinger.h#5 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _Pinger_h_
#define _Pinger_h_

/*
 * Bob 3/21/01:  Uncomment the following #ifdef to work in "deployment server"
 *               mode.  If enabled, a SipConfigServerAgent is started and will
 *               try to handle incoming sip subscribe requests.
 */
//#ifdef _WIN32
//#define PINGTEL_DEPLOYMENT_SERVER
//#endif

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsConfigDb.h"
#include "os/OsCallback.h"
#include "os/OsServerTask.h"
#include "net/HttpServer.h"
#include "net/SdpCodecFactory.h"
#include "mp/MpCallFlowGraph.h"
#include "pinger/PingerMsg.h"
#include "pinger/PingerConfigEncryption.h"
#include "config.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTCManager.h"
#endif /* INCLUDE_RTCP ] */

// DEFINES

#define CONFIGDB_NAME_IN_FLASH CONFIG_PREFIX_USER_DATA "pinger-config"
#define CONFIGDB_USER_IN_FLASH CONFIG_PREFIX_USER_DATA "user-config"
#define CONFIGDB_LOCAL_IN_FLASH CONFIG_PREFIX_USER_DATA "local-config"

#define RESTART_NEW_CONFIG        "new configuration information"
#define RESTART_INCOMING_CONFIG   "incoming configuration information"
#define RESTART_NEW_IP            "NEW IP ADDRESS DETECTED"

#define JAVA_DST_WESTERN_EUROPE   0  // Western Europe Daylight Saving Time Rules
#define JAVA_DST_NORTH_AMERICA    1  // North America Daylight Saving Time Rules
#define JAVA_DST_NONE            -1  // No Daylight Saving Time Rules

// MACROS

// EXTERNAL FUNCTIONS
void hold();
void offhold(const char* callId);
void dialUrl(const char* url);
void outGoingCallType(int callType);
void startSipLog();
void stoopSipLog();
void dumpSipLog();
UtlString httpCreateRow(const char* row, const char* value);
                      

// EXTERNAL VARIABLES
// CONSTANTS
#define CONFIG_PHONESET_DEPLOYMENT_SERVER	"PHONESET_DEPLOYMENT_SERVER"
#define CONFIG_PHONESET_LOGICAL_ID			"PHONESET_LOGICAL_ID" 
#define CONFIG_PHONESET_ADMIN_DOMAIN		"PHONESET_ADMIN_DOMAIN"
#define CONFIG_PHONESET_ADMIN_ID			"PHONESET_ADMIN_ID"
#define CONFIG_PHONESET_ADMIN_PWD			"PHONESET_ADMIN_PWD"
#define CONFIG_PHONESET_AVAILABLE_BEHAVIOR	"PHONESET_AVAILABLE_BEHAVIOR"
#define CONFIG_PHONESET_BUSY_BEHAVIOR		"PHONESET_BUSY_BEHAVIOR"
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
#define PHONESET_SNMP_TRAP_DESTS			"PHONESET_SNMP_TRAP_DESTS"
#define CONFIG_PHONESET_TELNET_ACCESS		"PHONESET_TELNET_ACCESS"
#define CONFIG_PHONESET_MY_PINGTEL_USER		"PHONESET_MY_PINGTEL_USER"

#define CONFIG_PHONESET_LOG_LEVEL         "PHONESET_LOG_LEVEL"
#define CONFIG_PHONESET_LOG_CONSOLE       "PHONESET_LOG_CONSOLE"
#define CONFIG_PHONESET_LOG_TARGET        "PHONESET_LOG_TARGET"
#define CONFIG_PHONESET_LOG_ENTRIES       "PHONESET_LOG_ENTRIES"
#define CONFIG_PHONESET_LOG_FILE          "PHONESET_LOG_FILE"
#define CONFIG_PHONESET_LOG_FILE_FLUSH_PERIOD   "PHONESET_LOG_FILE_FLUSH_PERIOD"
#define LOCAL_DOMAIN                      "LOCAL_DOMAIN"
#define CONFIG_PHONESET_SEND_INBAND_DTMF  "PHONESET_SEND_INBAND_DTMF"
#define CONFIG_PHONESET_DEFAULT_PRIORITY  "PHONESET_DEFAULT_PRIORITY"
#define CONFIG_PHONESET_SPLASH_SOUND      "PHONESET_SPLASH_SOUND"
#define CONFIG_PHONESET_ASSERT_ACTION     "PHONESET_ASSERT_ACTION"

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class CallManager;
class MpMediaTask;
class OsNameDb;
class OsQueuedEvent;
class OsTimer;
class OsTimerTask;
class PsPhoneTask;
class UtlMemCheck;
class SipUserAgent;
class PtMGCP;
class HttpServer;
class TaoServerTask;
class PtProvider;
class PtCall;
class SipNotifyStateTask;
class SipConfigDeviceAgent;
class SipConfigServerAgent;
class SipRefreshMgr;
class SipLineMgr;

//:Pinger class
// This is the main Pinger task. It is a singleton task and is responsible
// for initializing the Pinger device and starting up any other tasks that
// are needed.
class Pinger : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum ConfigError
   {
      ERR_CONFIG_OK,            // no error
      ERR_CONFIG_MULTIPLE,      // no error, multiple logical ids are returned
      ERR_CONFIG_CONFLICT,      // physical id in use
      ERR_CONFIG_CANNOTREACH,   // error occurred, cannot get config file from deployment server
      ERR_CONFIG_UNAUTHORIZED,  // id or pwd incorrect, cannot get config file from deployment server
      ERR_CONFIG_INCORRECT,     // error occurred, cannot get config file from deployment server
      ERR_CONFIG_STANDALONE,    // standalone mode, deployment server parameter set to '-' in config file
      ERR_CONFIG_NOTPRESENT,    // deployment server parameter not present in config file  
	  ERR_CONFIG_SERVICE_SET  	// no error, service is set so config file to be retreived
   };



/* ============================ CREATORS ================================== */

   static Pinger* getPingerTask(void);
     //:Return a pointer to the Pinger task, creating it if necessary

   virtual
   ~Pinger();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
   void stopPingerTask();

   void setPtProvider(PtProvider* pProvider);

   void unhold(const char* callId);
   void printCalls();
   void printSipUa();
   void dialString(const char* url);
   void outGoingCallType(int callType);

   CallManager* getCallManager() { return mpCallMgrTask; };
   TaoServerTask* getTaoServerTask() { return mpTaoServerTask; };

   void startSipUserAgentLog();
   void stopSipUserAgentLog();
   void dumpSipUserAgentLog();
    
   static void initHttpRequestProcessor(HttpServer *pHttpServer);

   static void processPostFile(const HttpRequestContext& requestContext,
                          const HttpMessage& request,
                          HttpMessage*& response);

   static void processSipPage(const HttpRequestContext& requestContext,
                          const HttpMessage& request,
                          HttpMessage*& response);
   static void processToggleSipLog(const HttpRequestContext& requestContext,
                          const HttpMessage& request,
                          HttpMessage*& response);

   static void processAnswerCgi(const HttpRequestContext& requestContext,
                              const HttpMessage& request,
                              HttpMessage*& response);
   static void processTransferCgi(const HttpRequestContext& requestContext,
                              const HttpMessage& request,
                              HttpMessage*& response);



   static void processDialCgi(const HttpRequestContext& requestContext,
                              const HttpMessage& request,
                              HttpMessage*& response);

   static void Pinger::processUpgradeLog(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

	static void Pinger::processGetCallIdsCgi(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response);

    static void processDropCgi(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response);

    static void processHoldCgi(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response);

	static void processOffHoldCgi(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response);


   static void processGetDeployConfigFile(const HttpRequestContext& requestContext,
                          const HttpMessage& request,
                          HttpMessage*& response);

   static void processGetLogicalId(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response);
    

   static void processFilelist(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;

   static void processCallStateInfo(const HttpRequestContext& requestContext,
                          const HttpMessage& request,
                          HttpMessage*& response);
   static void processToggleCallStateLog(const HttpRequestContext& requestContext,
                          const HttpMessage& request,
                          HttpMessage*& response);

   static void processMyXViewSpeeddial(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;
#ifdef DEBUG_VERSIONCHECK
   static void processInstallUrl(const HttpRequestContext& requestContext,
                          const HttpMessage& request,
                          HttpMessage*& response);

   static void processGetInstallUrl(const HttpRequestContext& requestContext,
                          const HttpMessage& request,
                          HttpMessage*& response);


	static void processVersionCheckForm(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response);

	static void processDoVersionCheck(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response);
#endif // DEBUG_VERSIONCHECK  

#ifdef INCLUDE_SNMP
   static void usrSnmpInitStart(void);
     //:Function invoked during SNMP agent initialization to set up the 
     //:trap destination table.

   static void usrSnmpInitFinish(void);
     //:Function invoked during SNMP agent initialization to set MIB 
     //:variables whose value is stored in the application config.
#endif

   static int Pinger::authenticateUser(UtlString userId, UtlString password) ;

    //: Installs upgrade kernel and jar files

   int sendRequestToDeploymentServer(HttpMessage*& pResquest,UtlString& deployServer);

   static OsStatus getHttpServerStatus();

   static int addUserToConfig(UtlString userId, UtlString password) ;

   static int addUser(UtlString userId, UtlString password) ;

   static void userLogin(UtlString userId, UtlString password) ;

   static int Pinger::parseDialString(UtlString dialUrl) ;
     //: returns if a valid dial string

   void checkConsoleOutputEnable(OsConfigDb* config);

   void restart(UtlBoolean confirm = TRUE, int seconds = 0, const char* reason = NULL)  ;
     //: Request a restart of the xpressa phone or instant xpressa softphone
     //
     //!param confirm - Does this restart require confirmation from the end
     //       user?  If the end user fails to respond within a sensible of 
     //       time, the software will automatically accept the request.
     //!param seconds - When the restart should take place.  A value of less 
     //       than zero indicates that the restart should happened now, 
     //       regardless of phone's busy state.  A value of zero indicates 
     //       that the restart should take place as soon as the phone is not
     //       busy.  A value greater than zero indicates that the restart 
     //       should occur in that many seconds.
     //!param reason - The reason for the restart.  Presently, this is only
     //       used for logging purposes.

   void saveLocalConfigDb() ;
     //:Store the local config DB to flash


/* ============================ ACCESSORS ================================= */

   int getDeploymentLogicalId(OsConfigDb* config, UtlString& logicalId);

   int sendDeviceId(UtlString& strServer, UtlString& strRequest) ;
     // sends deviceID to my.pingtel.com on startup, if user is registered with service

   CallManager* getCallMgrTask() { return mpCallMgrTask; };

   SipUserAgent* getSipUserAgent() { return mpSipUserAgentTask; };
     //: get a pointer to the sip user agent

   SipRefreshMgr* getRefreshManager() { return mpRefreshMgrTask; };
     //: get a pointer to the refresh manager

   SipLineMgr* getLineManager() { return mpLineMgrTask; };
     //: get a pointer to the Line manager

   HttpServer* getHttpServer() { return mpHttpServer; } ;

   PingerConfigEncryption *getConfigEncryption(void);
     //:Return a pointer to the configuration encryption handler

   OsConfigDb* getConfigDb(void);
     //:Return a pointer to the configuration database

   OsConfigDb* getLocalConfigDb() ;
     //:Return a pointer to the local configuration database

   SipConfigDeviceAgent* getConfigAgent();
     //: Return the pointer to the SipConfigDeviceAgent

#ifdef PINGTEL_DEPLOYMENT_SERVER
   SipConfigServerAgent* getSipConfigServerAgent() { return mpConfigServerAgent ;} ;
#endif


/* ============================ INQUIRY =================================== */

   static OsStatus  getHttpStatus() ;
     //: current state of http server

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   Pinger();
     //:Constructor (called only indirectly via getPingerTask())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    static const int PINGER_SIPUSERAGENT_MAX_REQUEST_MSGS;  
    // Maximum number of request messages sip user agent processes.

   enum PrivateConsts
   {
      NETWORK_RETRY_INIT_TIMEOUT_SECS  = 4,     // 4 seconds
      NETWORK_RETRY_MAX_CNT            = 5,     // max network retries
      DEFAULT_NETTIME_SYNC_PERIOD_SECS = 6000,  // 100 minutes
      RESTART_BUSY_TIMEOUT_SECS        = 90,    // timeout between busy checks
      DEFAULT_IPCHECK_PERIOD_SECS      = 15     // period between IP address checks
   };
  
   static PtProvider *mpProvider ;
   static PtCall     *mpCall ;
   static OsStatus   httpStatus;

   UtlMemCheck*      mpMemCheck;         // mem leak detection checkpoint
   CallManager*      mpCallMgrTask;      // call manager task
   SipNotifyStateTask* mpNotifyStateTask; // Task to catch SIP NOTIFY messages
   MpMediaTask*      mpMediaTask;        // media processing task
   PsPhoneTask*      mpPhoneTask;        // phone set task
   OsTimerTask*      mpTimerTask;        // timer manager task
   SipUserAgent*     mpSipUserAgentTask; // sip stack
   SipRefreshMgr*	 mpRefreshMgrTask;   //Refersh Manager
   SipLineMgr*       mpLineMgrTask;     //Line manager
   SipConfigDeviceAgent* mpConfigAgent;  // Config agent

#ifdef PINGTEL_DEPLOYMENT_SERVER
   SipConfigServerAgent* mpConfigServerAgent;	// Server Config Agent
#endif
   HttpServer*       mpHttpServer;       // Http Server
   // HttpServer*       mpHttpSecureServer; // Https Server
   TaoServerTask*    mpTaoServerTask;    // Tao server
   OsConfigDb        mConfigDb;          // Configuration database
   OsConfigDb        mLocalConfigDb;     // Local Configuration Database
   UtlString          msLogicalId;
   SdpCodecFactory*  mpCodecFactory;     // default collection of codecs to be used
   int               mNetworkRetryCnt;   // number of network retries performed
   int               mNetworkRetryTimer; // time to wait (in secs) between network retries
   int               mRestartCount;      // number of times the phone has been rebooted after setting to 
                                         // factory defaults

#ifdef INCLUDE_RTCP /* [ */
   IRTCPControl*     mpiRTCPControl;     // Realtime Control Interface
#endif /* INCLUDE_RTCP ] */

   // Static data members used to enforce Singleton behavior
   static Pinger*    spInstance;    // pointer to the single instance of
                                    //  the Pinger class
   static OsBSem     sLock;         // semaphore used to ensure that there
public:
   OsQueuedEvent*    mpSplashEvent; // Signal to clean up splash player
   OsQueuedEvent*    mpRestartEvent; // Event marking time to restart
   OsTimer*          mpRestartTimer; // Timer evaluating time to restart
   OsQueuedEvent*    mpCheckIPEvent; // Signal to check the IP address
   OsTimer*          mpCheckIPTimer; // Timer to check the IP address

   MpCallFlowGraph*  mpSplashFlowG;

   struct tagRestartEventInfo
   {
      UtlBoolean confirm ;    // Should the phone restart w/ user confirmation
      UtlBoolean ignoreState; // Should the state of the phone be ignored?
      char* reason ;         // User supplied reason for restarting
   } ;


private:
   virtual UtlBoolean handleMessage(OsMsg& rMsg);
     //:Handle an incoming message
     // Return TRUE if the message was handled, otherwise FALSE.   

   virtual void initHttpServer(OsConfigDb* config);
     //:Initialize http management server

   virtual void initCodecs(OsConfigDb* config);
     //: Initialize the default set of codecs to be used

   virtual void initSipUserAgent(OsConfigDb* config);
     //:Initialize the SIP user agent

   virtual void initRefreshMgr(OsConfigDb* config);
	//initialize refresh manager

   virtual void checkIP(void);
     //:Checks that the IP address has not changed.  If it has, a restart is requested

   virtual void initIPCheckTimer(OsConfigDb* config);
      //:Start the timer for periodically checking the system IP address

   virtual void initLineMgr(OsConfigDb* config);
   virtual void LoadLinesOnStartup(OsConfigDb* config);
   virtual void SaveLinesOnExit(OsConfigDb* config){};
	//initialize Line manager and load all lines

   virtual void initLocalConfigFile();
   // initialize system-config file
   
   virtual void initConfigAgent(OsConfigDb* config);
	//initialize config agent

   virtual void initMgcpStack(OsConfigDb* config);
     //:Initialize the MGCP stack

   virtual void initCallManager(OsConfigDb* config);
     //:Init the call processing subsystem

   virtual void initTaoServer(OsConfigDb* config);
     //:Init the tao subsystem

   virtual int initDeploymentServer(OsConfigDb* config);
     //:Init the tao subsystem

   UtlBoolean encryptExistingProfiles();
     //:Encrypt profile files, if needed

   virtual void initDefaultProcessPriority(OsConfigDb* config);
     //:Initialise the default process priority

   virtual void initSysLog(OsConfigDb* config);
     //:Initialise the OsSysLog

   virtual int getConfigFromDeploymentServer(UtlString& deployServer, UtlString& requestCmd);
     //:Get configuration info from the deployment server

   virtual void initConfigFile(OsConfigDb* config);
     //:Set up the configuration database from the default file name


   Pinger(const Pinger& rPinger);
     //:Copy constructor (not implemented for this class)

   Pinger& operator=(const Pinger& rhs);
     //:Assignment operator (not implemented for this class)

   void playStartupSplash(void);

   UtlBoolean doFinishSplash(void);

   UtlBoolean finishSplash(const int eventData);

   UtlBoolean handleRestart(UtlBoolean confirm, UtlBoolean ignoreState, const char *reason);
     //:Handles the processing of a restart requeust
   UtlBoolean scheduleRestart(UtlBoolean confirm, UtlBoolean ignoreState, const char* reason, int seconds);
     //:Handles the scheduling/rescheduling or a restart request.

};

/* ============================ INLINE METHODS ============================ */

#endif  // _Pinger_h_
