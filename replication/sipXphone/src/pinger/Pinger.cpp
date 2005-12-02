// $Id: //depot/OPENDEV/sipXphone/src/pinger/Pinger.cpp#8 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES
#include "pinger/Pinger.h"
#include <os/iostream>

#include <assert.h>

#ifdef _WIN32
#include <io.h>
#endif

#ifdef __pingtel_on_posix__
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

// APPLICATION INCLUDES
#include "cp/CallManager.h"
#include "ps/PsPhoneTask.h"

#include "utl/UtlRegex.h"

#include "pinger/PingerMsg.h"
#include "pinger/PingerConfig.h"
#include "pinger/SipConfigDeviceAgent.h"
#include "pinger/PingerConfigEncryption.h"
#include "net/NameValueTokenizer.h"
#include "net/QoS.h"
#include "net/SipConfigServerAgent.h"
#include "net/MimeBodyPart.h"
#include "net/SipLine.h"
#include "utl/UtlTokenizer.h"
#ifdef TEST_SSL
#include "os/OsSSLConnectionSocket.h"
#include "os/OsSSLServerSocket.h"
#endif

#include "licensemanager/BaseLicenseManager.h"
#include "os/OsSysLog.h"

#include "os/OsAssert.h"
#include "os/OsEventMsg.h"
#include "os/OsNameDb.h"
#include "os/OsQueuedEvent.h"
#include "os/OsSysTimer.h"
#include "os/OsSysLog.h"
#include "os/OsTimerTask.h"
#include "os/OsTimeLog.h"
#include "pinger/PingerInfo.h"
#include "ps/PsHookswTask.h"
#include "ps/PsPhoneTask.h"
#include "ps/PsButtonTask.h"
#include "ps/PsButtonId.h"
#include "os/OsConfigDb.h"
#include "os/OsUtil.h"
#include "net/SipUserAgent.h"
#include "net/SipLineMgr.h"
#include "net/HttpServer.h"
#include "net/HttpRequestContext.h"
#include "net/SipNotifyStateTask.h"

#include "tao/TaoServerTask.h"

#include "ptapi/PtCall.h"
#include "ptapi/PtProvider.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtSessionDesc.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneDisplay.h"
#include "ptapi/PtPhoneGraphicDisplay.h"
#include "ptapi/PtPhoneHookswitch.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "ptapi/PtPhoneRinger.h"
#include "ptapi/PtPhoneSpeaker.h"
#include "ptapi/PtComponentGroup.h"
#include "ptapi/PtConnection.h"
#include "ptapi/PtTerminalConnection.h"
#include "mp/dtmflib.h"

#include "pingerjni/ApplicationRegistry.h"
#include "pingerjni/SpeedDialWebDSP.h"
#include "pingerjni/VoicemailNotifyStateListener.h"
#include "pingerjni/JXAPI.h"

#include "cp/CpMediaInterfaceFactoryInterface.h"
#include "cp/CpMediaInterfaceFactory.h"

#include "web/Webui.h"

#ifdef _NONJAVA
#include "pingerjni/JNIStubs.h"
#endif

PtProvider      *Pinger::mpProvider = 0;
PtCall          *Pinger::mpCall = 0;
OsStatus         Pinger::httpStatus = OS_TASK_NOT_STARTED;
static PingerConfigEncryption sConfigEncryption;

// EXTERNAL FUNCTIONS
extern "C" int OpenSSL_add_ssl_algorithms();
extern OsStatus certificateCheck();

extern void PrintCalls(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response);
extern void PrintSipUa(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response);
extern "C" int SetJNIDebugLevel(int level);
void JNI_SoftPhoneReboot(long ms);

// EXTERNAL VARIABLES
extern void* pDhcpcBootCookie;
extern bool g_bRealtimeDebug;
extern int g_iDebugLevel;

// TEMPHACK: The link is not linking in some source that I need from java land
//           so I'm forcing references to it... BOB 03-03-00
extern int FORCE_REFERENCE_PtSessionDesc ;
extern int FORCE_REFERENCE_PtTerminalConnectionListener ;
extern int FORCE_REFERENCE_PtConnectionListener ;
extern int FORCE_REFERENCE_PtTerminalComponentListener ;
extern int FORCE_REFERENCE_PtProvider ;


// CONSTANTS
#define PINGER_STACK_SIZE (24*1024)    // 24K stack for the pinger task

#define CONFIGDB_PREFIX_ON_SERVER "pinger-config-"

#define AUDIO_SPLASH_FILE CONFIG_PREFIX_SHARED_DATA "audioSplash.wav"

#define DEPLOYMENT_HTTP_PORT    80
#define MAX_FILELINE_SIZE       160
#define DEFAULT_SIP_PORT        5060
#define MAX_CALLS               5
// MAX_FORWARDS_LIMIT is the highest value we will allow to be
//                    configured for SIP_MAX_FORWARDS
#define MAX_FORWARDS_LIMIT      100

#ifdef HAVE_GIPS /* [ */
#ifdef _WIN32 /* [ */
#define DEFAULT_CODEC_LIST_STRING \
 "eg711u eg711a pcmu pcma ilbc g729 telephone-event"
#else /* _WIN32 ] [ */
#define DEFAULT_CODEC_LIST_STRING \
 "eg711u eg711a pcmu pcma g729 telephone-event"
#endif /* _WIN32 ] */
#else /* HAVE_GIPS ] [ */
#define DEFAULT_CODEC_LIST_STRING \
 "pcmu pcma telephone-event"
#endif /* HAVE_GIPS ] */

#define OK 0

#define UPGRADE_CURRENT_VERSION "CURRENT_VERSION"
#define UPGRADE_PLATFORM        "PLATFORM"

// Speeddial data
#define MAX_MANAGED_FLOW_GRAPHS 10
#define MAX_SPEEDDIAL_ENTRIES 100
#define MAX_SPEEDDIAL_LENGTH 256
#define SPEED_DIAL_SCHEMA_ID "id"
#define SPEED_DIAL_SCHEMA_LABEL "label"
#define SPEED_DIAL_SCHEMA_PHONE_NUMBER "phone_number"
#define SPEED_DIAL_SCHEMA_URL "url"

// Admin user
#define ADMIN "admin"

/*
#if defined(_WIN32)
 #define HTTPS_PUBLIC_CERTIFICATE_FILE_LOCATION "env\\server.crt"
 #define HTTPS_PRIVATE_KEY_FILE_LOCATION "env\\server.key"
#else
 #define HTTPS_PUBLIC_CERTIFICATE_FILE_LOCATION "..\\share\\sipxphone\\server.crt"
 #define HTTPS_PRIVATE_KEY_FILE_LOCATION "..\\share\\sipxphone\\server.key"
#endif
*/

// New UI HTML

#define HTML_HEADER_1 \
" <html>\n\
<head>\n "

#define HTML_HEADER_2 \
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
<META HTTP-EQUIV = \"PRAGMA\" CONTENT = \"no-cache\">\n\
<META HTTP-EQUIV=\"expires\" CONTENT=\"01 Dec 1994 16:00:00 GMT\">\n\
<link rel=stylesheet href=\"/fonts.css\" type=\"text/css\">\n\
<script language=\"JavaScript\" src=\"/dhtmlMenu.js\"></script>\n\
<script language=\"JavaScript\">function setDialUrl(newUrl) { document.dial.dial_url.value=newUrl; }</script>\n\
</head>\n "

#define HTML_HEADER_3 \
"\n\
<!-- Page Header table -->\n\
<table width=\"750\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n\
  <tr>\n\
    <td width=\"30%\" rowspan=\"2\">\n\
      <table width=\"100%\" border=\"0\">\n\
        <tr>\n\
          <td><a href=http://www.sipfoundry.org><img src=\"/sipfoundry-sipxphone-logo.gif\" width=\"220\" height=\"100\" alt=\"SIPFoundry sipXphone\" border=0></a></td>\n\
         </tr>\n\
      </table>\n\
    </td>\n\
    <td width=\"70%\" align=\"right\" valign=\"top\">\n\
      <div><font class=\"my\">my</font><font class=\"xpressa\">sip softphone&#153;</font></div>\n\
    </td>\n\
    <td>\n\
      <div align=\"right\"></div>\n\
    </td>\n\
  </tr>\n\
  <tr>\n\
    <td>\n\
    </td>\n\
  </tr>\n\
</table>\n\
<!--end of page header table-->\n "

#define HTML_FOOTER \
"   <!--end main body of page table-->\n\
<p align=\"center\" class=\"smallText\">Copyright © 2005 SIPFoundry Corp. Distributed under the LGPL License.</p>\n\
</body>\n\
</html> "

// More HTTP SERVER related definition
#define HTTP_BODY_BEGIN "<HTML>\n<BODY>\n"
#define HTTP_BODY_END   "</BODY>\n</HTML>\n"
#define HTTP_TABLE_BEGIN "<TABLE class=maincontent WIDTH=280 BORDER=0 cellspacing=0 cellpadding=1> \n"
#define HTTP_TABLE_END   "</TABLE>\n"
#define HTTP_CONFIG_TABLE_ROW(COL1, COL2) "<TR><TD>" #COL1 "</TD> <TD>" #COL2 "</TD></TR>"

// STATIC VARIABLE INITIALIZATIONS
Pinger* Pinger::spInstance = 0;
OsBSem  Pinger::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
const int Pinger::PINGER_SIPUSERAGENT_MAX_REQUEST_MSGS = 1000;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
int gSipUdpPort = 0;
int gSipTcpPort = 0;
int gSipTlsPort = 0 ;

/* ============================ CREATORS ================================== */

// Return a pointer to the Pinger task, creating it if necessary
Pinger* Pinger::getPingerTask(void)
{
#ifdef _WIN32  //DWW
try
{
#endif
   UtlBoolean isStarted;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
   {
       spInstance = new Pinger();
   }

   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }
   sLock.release();
#ifdef _WIN32   //DWW
}

catch (...)
{
   char msg[256];
   wsprintf(msg, "  *******************   ERROR Initializing Pinger"
      " ********************** \nLINE: %d %s",__LINE__, __FILE__);
   MessageBox(NULL,msg,"ERROR STARTING sip softphone!",MB_OK);
   osPrintf("%s",msg);
   return NULL;
}

#endif

   return spInstance;
}

// Destructor
Pinger::~Pinger()
{
   stopPingerTask();
}

void Pinger::stopPingerTask()
{
#ifdef INCLUDE_RTCP /* [ */
// Release reference to RTCControl Interface.  This will cause all RTCP
// message processing to terminate and will trigger the deallocation of
// the RTCManager object.
   mpiRTCPControl->Release();
#endif /* INCLUDE_RTCP ] */

   if (mpPhoneTask)
   {
      mpPhoneTask->requestShutdown();    // shut down the phone set task
      delete mpPhoneTask;
      mpPhoneTask = 0;
   }

   if (mpCallMgrTask)
   {
      mpCallMgrTask->requestShutdown();  // shut down the call manager task
      delete mpCallMgrTask;
      mpCallMgrTask = 0;
   }

   if (mpTimerTask)
   {
      mpTimerTask->requestShutdown();    // shut down the timer manager task
      delete mpTimerTask;
      mpTimerTask = 0;
   }

   if (mpHttpServer)
   {
      mpHttpServer->requestShutdown();
      delete mpHttpServer;
      mpHttpServer = NULL;
   }


   if (mpCheckIPEvent)
   {
      delete mpCheckIPEvent;
      mpCheckIPEvent = NULL;
   }

   if (mpCheckIPTimer)
   {
      delete mpCheckIPTimer;
      mpCheckIPTimer = NULL;
   }
/*
   if (mpHttpSecureServer)
   {
      mpHttpSecureServer->requestShutdown();
      delete mpHttpSecureServer;
      mpHttpSecureServer = NULL;
   }
*/

   if (mpRestartTimer != NULL)           // free restart timer
   {
      mpRestartTimer->stop() ;
      delete mpRestartTimer ;
      mpRestartTimer = NULL ;
   }

   if (mpRestartEvent != NULL)          // free restart event
   {
      delete mpRestartEvent ;
      mpRestartEvent = NULL ;
   }

   spInstance = NULL;
}

void Pinger::playStartupSplash(void)
{
#if 0
   //file for testing if file is there.
   FILE *fptr = NULL;

   //determine if we really wanted to play the splash
   UtlString strSplashStatus;

   //load the splashStatus parameter
   mConfigDb.get(CONFIG_PHONESET_SPLASH_SOUND, strSplashStatus);

   //convert to upper for comparison
   strSplashStatus.toUpper();

   //only play if NOT disable
   if (strSplashStatus != "DISABLE")
   {
       // Play the reset audio splash if the file is present
       UtlBoolean bFoundFile = FALSE;

       if ((fptr = fopen(AUDIO_SPLASH_FILE,"r")) != NULL)
       {
          bFoundFile = TRUE;
          fclose(fptr);
       }

       if (bFoundFile == TRUE)
       {
          UtlString locale;
          // get the call progress tones locale setting
          // (represented using an ISO-3166 two letter country code)
          mConfigDb.get("PHONESET_LOCALE_COUNTRY", locale);

          mpSplashEvent = new OsQueuedEvent(*getMessageQueue(), 0);
          mpSplashEvent->setUserData((int) mpSplashEvent);
          mpSplashFlowG = new MpCallFlowGraph(locale.data());
          if(mpSplashFlowG && mpMediaTask)
          {
             mpMediaTask->setFocus(mpSplashFlowG);
             mpPhoneTask->activateGroup(PtComponentGroup::SOUND);
             if(mpSplashFlowG->playFile(AUDIO_SPLASH_FILE, FALSE,
                MpCallFlowGraph::TONE_TO_SPKR, mpSplashEvent) != OS_SUCCESS)
             {
                   osPrintf("Failed to open audio splash file: \"%s\"\n",
                      AUDIO_SPLASH_FILE);
                   doFinishSplash();
             }
             else
             {
                 osPrintf("Opened audio splash file: \"%s\"\n",                                                       AUDIO_SPLASH_FILE);
             }
          }
       }
   } // splashStatus == ENABLE
#endif
}

UtlBoolean Pinger::doFinishSplash(void)
{
#if 0
   mpPhoneTask->deactivateGroup(PtComponentGroup::SOUND);
   if (NULL != mpSplashFlowG) {
      if (mpMediaTask->getFocus() == mpSplashFlowG)
      {
         osPrintf("doFinishSplash: taking flowgraph out of focus\n");
         mpMediaTask->setFocus(NULL);
         mpSplashFlowG->synchronize(); // wait for the focus to change.
      }
      osPrintf("doFinishSplash: tearing down flowgraph\n");
      delete mpSplashFlowG;
      mpSplashFlowG = NULL;
   } else {
      osPrintf("doFinishSplash: no flowgraph\n");
   }
#endif
   return TRUE;
}

UtlBoolean Pinger::finishSplash(const int eventData)
{
#if 0
   switch (eventData) {
   case MprFromFile::PLAY_FINISHED:
      delete mpSplashEvent;
      mpSplashEvent = NULL;
      osPrintf("finishSplash: deleting QueuedEvent object\n");
      return doFinishSplash();
      break;
   case MprFromFile::PLAY_STOPPED:
   case MprFromFile::READ_ERROR:
   case MprFromFile::PLAY_IDLE:
   case MprFromFile::INVALID_SETUP:
      return doFinishSplash();
      break;
   case MprFromFile::PLAYING:
      break;
   default:
      return FALSE;
   }
#endif
   return TRUE;   
}


/* ============================ MANIPULATORS ============================== */

void Pinger::unhold(const char* callId)
{
    if(mpCallMgrTask)
    {
        mpCallMgrTask->unhold(callId);
    }
}

void Pinger::printCalls()
{
    if(mpCallMgrTask)
    {
        mpCallMgrTask->printCalls();
    }
}

void Pinger::printSipUa()
{
    if(mpSipUserAgentTask)
    {
        mpSipUserAgentTask->printStatus();
    }
}

void Pinger::dialString(const char* url)
{
    if(mpCallMgrTask)
    {
        mpCallMgrTask->dialString(url);
    }
}

void Pinger::outGoingCallType(int callType)
{
    if(mpCallMgrTask)
    {
        mpCallMgrTask->setOutGoingCallType(callType);
    }
}

void Pinger::startSipUserAgentLog()
{
    if(mpSipUserAgentTask)
    {
        mpSipUserAgentTask->clearMessageLog();
        mpSipUserAgentTask->startMessageLog();
    }
}

void Pinger::stopSipUserAgentLog()
{
    if(mpSipUserAgentTask)
    {
        mpSipUserAgentTask->stopMessageLog();
    }
}

void Pinger::dumpSipUserAgentLog()
{
    if(mpSipUserAgentTask)
    {
        UtlString logData;
        mpSipUserAgentTask->getMessageLog(logData);
        osPrintf("***************BEGIN SIP LOG***************\n%s",
           logData.data());
    }
}

// Request a phone restart, see pinger.h for details
void Pinger::restart(UtlBoolean confirm, int seconds, const char* reason)
{
   UtlBoolean ignoreState = FALSE ;
   if (seconds < 0)
   {
      ignoreState = TRUE ;
      seconds = 0 ;
   }

   // Always schedule the restart; never block the requestor.
   scheduleRestart(confirm, ignoreState, reason, seconds) ;
}


// Store the local config DB to flash
void Pinger::saveLocalConfigDb()
{
   if (mLocalConfigDb.storeToFile(CONFIGDB_LOCAL_IN_FLASH) != OS_SUCCESS)
   {
      osPrintf("Could not create %s config file", CONFIGDB_LOCAL_IN_FLASH);
   }
}


/* ============================ ACCESSORS ================================= */

// Return a pointer to the configuration database
OsConfigDb* Pinger::getConfigDb(void)
{
   return &mConfigDb;
}

PingerConfigEncryption* Pinger::getConfigEncryption()
{
    return &sConfigEncryption;
}

SipConfigDeviceAgent* Pinger::getConfigAgent()
{
    return mpConfigAgent;
}


// Return a pointer to the local configuration database
OsConfigDb* Pinger::getLocalConfigDb()
{
   return &mLocalConfigDb ;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getPingerTask())
Pinger::Pinger()
:  OsServerTask("Pinger", NULL, DEF_MAX_MSGS, DEF_PRIO, DEF_OPTIONS, PINGER_STACK_SIZE)
{
#ifdef INCLUDE_RTCP /* [ */
   mpiRTCPControl = NULL ;
#endif /* INCLUDE_RTCP ] */
   mpTaoServerTask = NULL ;
   mpTimerTask = NULL ;
   mpSipUserAgentTask = NULL ;
   mpLineMgrTask = NULL ;
   mpCodecFactory = NULL ;
   mpRefreshMgrTask = NULL ;
   mpMemCheck = NULL ;
   mpCallMgrTask = NULL ;
   mpPhoneTask = NULL ;
   mpHttpServer = NULL ;
   mpSplashEvent = NULL ;
   mpRestartTimer = NULL ;
   mpRestartEvent = NULL ;
   mpCheckIPEvent = NULL ;
   mpCheckIPTimer = NULL ;

   mNetworkRetryCnt = 0 ;
   mNetworkRetryTimer = NETWORK_RETRY_INIT_TIMEOUT_SECS ;

#ifdef DISPLAY_STARTUP_STATUS
    displayStartupStatus(4, "Starting App") ;
#endif

   // Play the startup splash, once the pinger task has started
   PingerMsg splashMsg(PingerMsg::PLAY_SPLASH);
   postMessage(splashMsg);

   msLogicalId.remove(0);

   // handling of encrypted config files
   OsConfigDb::setStaticEncryption(&sConfigEncryption);

   // Set up the local database from the local-config file name
   initLocalConfigFile();
   srand(mRestartCount);

   // handling of encrypted config files
   OsConfigDb::setStaticEncryption(&sConfigEncryption);

   // If not already present, add the "admin" user with the default
   // password.  This has to happen before initConfigFile() is called.
   addUserToConfig("admin","") ;

   // Set up the configuration database from the default file name
   initConfigFile(&mConfigDb);

   // Initialize the system logger as early as possible
   initSysLog(&mConfigDb) ;

   //sets the priority of the process as defined by PHONESET_DEFAULT_PRIORITY
   initDefaultProcessPriority(&mConfigDb);

   // it is possible that the files are unencrypted, even though they should be.
   // make sure now
   encryptExistingProfiles();

#ifdef TEST
   // if "TEST" is defined, wait a couple of seconds after each task is
   // started in order to give that task a chance to finish its self tests
   // without interference from other Pinger activities.
#define TEST_DELAY_LONG  OsTask::delay(2000)
#define TEST_DELAY_SHORT OsTask::delay(500)
#else
#define TEST_DELAY_LONG
#define TEST_DELAY_SHORT

   // Allow the OsSysTimer tests to run before we start the OsTimerTask
   OsSysTimer::getSysTimer();
#endif //TEST

   mpTimerTask = OsTimerTask::getTimerTask(); // init the timer manager task
   TEST_DELAY_LONG;

   // create the timer that is used to periodically check the IP address for a change
   initIPCheckTimer(&mConfigDb);   

   int ret = initDeploymentServer(&mConfigDb);
   if (ret != ERR_CONFIG_OK)
   {
      osPrintf("Pinger::Pinger: initDeploymentServer: returns: %d\n", ret);
   }

   // Init the http server and mappings
   initHttpServer(&mConfigDb);

   mpPhoneTask = PsPhoneTask::getPhoneTask(); // init the phone set task

   UtlString extensionNum;

   mConfigDb.get("PHONESET_EXTENSION", extensionNum);
   osPrintf("Extension num: %s\n", extensionNum.data());

   // Initialize the audio codec definitions
   initCodecs(&mConfigDb);

   //init Line manager
   initLineMgr(&mConfigDb);

   // initialize the SIP user agent
   initSipUserAgent(&mConfigDb);

   //This was the second part of the epoch time clean up
   mpSipUserAgentTask->start();

   PingerMsg sipWaitMsg(PingerMsg::WAIT_FOR_SIP_UA);
   postMessage(sipWaitMsg);

   // initialize the Refresh Manager
   initRefreshMgr(&mConfigDb);
   PingerMsg sipStartRefreshMsg(PingerMsg::START_SIP_REFRESH_MGR);
   postMessage(sipStartRefreshMsg);

   // start line manager after refresh manager has been initialised
   PingerMsg sipStartLineMgrMsg(PingerMsg::START_SIP_LINE_MGR);
   postMessage(sipStartLineMgrMsg);

   initConfigAgent(&mConfigDb);

#if defined(ENABLE_MGCP)
   // initialize the MGCP stack
   initMgcpStack(&mConfigDb);
#endif

   // init the call processing subsystem
   initCallManager(&mConfigDb);

// temporarily disable tao server
   initTaoServer(&mConfigDb);

// Start WebUI CGIs to HttpServer
   Webui::getWebuiTask()->initWebui(mpHttpServer) ;
   // Webui::getWebuiTask()->initWebui(mpHttpSecureServer) ;
}
/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Handle an incoming message.
// For now, we always return FALSE (thereby passing responsibility for
// handling the message to the OsServerTask).
UtlBoolean Pinger::handleMessage(OsMsg& rMsg)
{
   int         eventData;
   int         userData;
   UtlBoolean   returnValue = TRUE;
   OsEventMsg* pEventMsg;
   PingerMsg*  pPingerMsg;

   if (rMsg.getMsgType() == OsMsg::PINGER_MSG)
   {
      pPingerMsg = (PingerMsg*) &rMsg;
      switch (rMsg.getMsgSubType())
      {
        case PingerMsg::PLAY_SPLASH:
             playStartupSplash();
             break;
         break;

     case PingerMsg::WAIT_FOR_SIP_UA:
        {
         mpSipUserAgentTask->waitUntilReady();
         break;
        }

      case PingerMsg::START_SIP_REFRESH_MGR:
         mpRefreshMgrTask->StartRefreshMgr();
         break;

      case PingerMsg::START_SIP_LINE_MGR:
         {
            mpLineMgrTask->StartLineMgr();
            mpLineMgrTask->initializeRefreshMgr(mpRefreshMgrTask);
            UtlString contactUri;
            // Sip user agent has no concept of lines, it only
            // knows about ip addresses etc as it should do
            mpSipUserAgentTask->getContactUri(&contactUri);

            // set the contact info in the lineManager
            mpLineMgrTask->setDefaultContactUri(Url(contactUri));
            // Attempt to read the line defs
            if (!SipLineMgrSerialize(*mpLineMgrTask, false))
            {
               LoadLinesOnStartup(&mConfigDb);
               SipLineMgrSerialize(*mpLineMgrTask, true);
            }
            // using the line manager, set the default outbound
            // line in the call manager
            UtlString outboundLine;
            mpLineMgrTask->getDefaultOutboundLine( outboundLine );
            mpCallMgrTask->setOutboundLine( outboundLine.data() );
         }
         break;

      case PingerMsg::JAVA_INITIALIZED:

            // Do not initalize the line manager lines until after the JVM
            // is started and had a chance to add line listeners.  Otherwise,
            // the GUI may miss line events.
            mpLineMgrTask->enableAllLines();

            // Likewise, do not send out the MWI subscribe until after the
            // the application layer is ready.  Otherwise, we may miss the
            // MWI notification.
            {
               UtlString subscribeStr;

               mConfigDb.get( "PHONESET_MSG_WAITING_SUBSCRIBE", subscribeStr );
               if (!subscribeStr.isNull())
               {
                  UtlString userId;
                  UtlString contactUri;
                  UtlString outboundLine;
                  Url subscribeUrl(subscribeStr);
                  subscribeUrl.getUserId(userId);

                  mpLineMgrTask->getDefaultOutboundLine(outboundLine);

                  // If the PHONESET_MSG_WAITING_SUBSCRIBE setting does
                  // not have a userid field specified, get the one
                  // from the default outbound line and use that
                  if( userId.isNull() )
                  {
                     // get default outbound line from the line manager
                     Url outbound(outboundLine);
                     outbound.getUserId(userId);
                     subscribeUrl.setUserId(userId);
                     subscribeStr = subscribeUrl.toString();
                  }

                  // make sure we associate a user with the line
                  mpSipUserAgentTask->getContactUri( &contactUri );
                  Url contactForLine ( contactUri );
                  contactForLine.setUserId( userId );

                  UtlString contactforLineStr = contactForLine.toString();

                  SipMessage* mwiSubscribeRequest = new SipMessage();

                  int sipRegisterPeriodSeconds;

                  // Make sure we have a sensible registration period for voicemail subscriptions
                  if ( (mConfigDb.get( "SIP_REGISTER_PERIOD", sipRegisterPeriodSeconds ) != OS_SUCCESS) ||
                       (sipRegisterPeriodSeconds <= 0) )
                  {
                     sipRegisterPeriodSeconds = 3600;
                  }

                  mwiSubscribeRequest->setVoicemailData (
                     outboundLine.data(),        // fromField
                     outboundLine.data(),        // toField
                     subscribeStr.data(),        // uri
                     contactforLineStr.data(),   // contactUrl
                     NULL,                       // callId, do not set here
                     1,                          // cseq
                     sipRegisterPeriodSeconds);  // expiresInSeconds

                  // send the subscribe request to the refresh manager
                  if ( mpRefreshMgrTask )
                      mpRefreshMgrTask->newSubscribeMsg( *mwiSubscribeRequest );
               }
            }

            /*
              check that network started ok
            */
            if (mpSipUserAgentTask && OsUtil::getProductType() == OsUtil::PRODUCT_INSTANT_XPRESSA)
               {
                   UtlString strErrorMsg;
                   char buf[80];
                   if (gSipTcpPort > 0 && !mpSipUserAgentTask->isOk(OsSocket::TCP))
                   {
                        sprintf(buf,"The TCPServer failed to initialize port %d\n",gSipTcpPort);
                        strErrorMsg += buf;
                   }
                   if (gSipUdpPort > 0 && !mpSipUserAgentTask->isOk(OsSocket::UDP))
                   {
                        sprintf(buf,"The UDP Server failed to initialize port %d\n",gSipUdpPort);
                        strErrorMsg += buf;
                   }
                   if (gSipTlsPort > 0 && !mpSipUserAgentTask->isOk(OsSocket::SSL_SOCKET))
                        strErrorMsg += "The TLS Server failed to initialize.\n";

                   //display error message to java layer
                   if (strErrorMsg.length())
                        JXAPI_MessageBox(MBT_ERROR, "Socket Error", strErrorMsg.data(), false) ;

           }

            if (sConfigEncryption.handleIncomingProfiles())
            {
                // User just started, but we have no choice, we have new profiles to
                // load. Reason is not RESTART_NEW_ENCRYPTED_CONFIG because we've already
                // decrypted them above
                handleRestart(false, true, RESTART_NEW_CONFIG);
            }

            break ;

      default:
         returnValue = FALSE;
         break;
      }
   }
   else if (rMsg.getMsgType() == OsMsg::OS_EVENT)
   {
      pEventMsg = (OsEventMsg*) &rMsg;
      pEventMsg->getEventData(eventData);
      pEventMsg->getUserData(userData);
      
      if (eventData == (int) mpCheckIPTimer)
      {
         checkIP();
      }
      else if (userData == ((int) mpSplashEvent))
      {
         returnValue = finishSplash(eventData);
      }
      else if (eventData == ((int) mpRestartTimer))
      {
         // Restart Event: handle the event and clean up the temp data
         struct tagRestartEventInfo* pInfo =
               (struct tagRestartEventInfo*) userData ;
         if (pInfo != NULL)
         {
            handleRestart(pInfo->confirm, pInfo->ignoreState, pInfo->reason) ;
            if (pInfo->reason != NULL)
            {
               free(pInfo->reason) ;
            }
            delete pInfo ;
         }
      }
      else
      {
         assert(FALSE);
      }
   }
   else
   {
      assert(FALSE);
   }

   return(returnValue);
}

// Handle a restart
UtlBoolean Pinger::handleRestart(UtlBoolean confirm, UtlBoolean bIgnoreState, const char *reason)
{
   // If the phone is busy (and NOT ignoring state), reschedule in the future
   if (!bIgnoreState && JXAPI_IsPhoneBusy())
   {
      scheduleRestart(confirm, bIgnoreState, reason, RESTART_BUSY_TIMEOUT_SECS) ;
   }
   else
   {
       // OPINION: Do not check for incoming files unless this was the reason for the 
       // restart.  If the restart was not because of incoming config files user will
       // be asked for user id and password after restart and can hit cancel if nec.
       UtlBoolean restart = TRUE;
       if (reason != NULL && strcmp(reason, RESTART_INCOMING_CONFIG) == 0)
       {
           restart = sConfigEncryption.handleIncomingProfiles();
       }

       // this will encrypt the existing profiles, if the ENCRYPT_PROFILES property is set to ENABLE
       encryptExistingProfiles();

      // Confirm with the user if required
      if (restart && ((!confirm) || (JXAPI_RequestRestart(60, reason))))
      {
         // Add to syslog, and flush everything we can before proceeding.
         syslog(FAC_KERNEL, PRI_NOTICE, "Restarting: %s",
               (reason == NULL) ? "Unknown reason" : reason) ;

         OsSysLog::flush() ;
         UtlString restartDisplay;
         restartDisplay = UtlString(reason) + ": Restarting...";
         JXAPI_DisplayStatus(restartDisplay) ;         // JXAPI_FlushCache() ;

         // Lastly, reboot.
         JNI_SoftPhoneReboot(0) ;
      }
   }

   return TRUE ;
}

// ****************************************************************************
// Pinger::encryptExistingProfiles
//
// This method will determine if profiles need to be encrypted, and will 
// encrypt them.  The encryption decision is determined by looking at the
// ENCRYPT_PROFILES property IN ONLY the pinger-config file.  (it was decided
// that ENCRYPT_PROFILES is a device level property.
// 
// MDC - 12/8/2004
// ****************************************************************************
UtlBoolean Pinger::encryptExistingProfiles()
{
    // MDC - 12/06/04 - added the following for XPH-24
    // load the profile into a ConfigDB object
    UtlBoolean profileSaved = FALSE;
    OsConfigDb configDb;
    configDb.loadFromFile(CONFIGDB_NAME_IN_FLASH);
            
    // query the ENCRYPT_PROFILES key
    UtlString key("ENCRYPT_PROFILES");
    UtlString value;
    configDb.get(key, value);
    if (value == "ENABLE")
    {
        PingerConfigEncryption* cryptObj = NULL;

        cryptObj = static_cast<PingerConfigEncryption*>(configDb.getEncryption());
        if (cryptObj)
        {
            cryptObj->initWriteEnabled(&configDb);
        }
        // store the encrypted pinger-config file
        profileSaved = configDb.storeToFile(CONFIGDB_NAME_IN_FLASH);

        // now load and write the user-config file
        configDb.loadFromFile(CONFIGDB_USER_IN_FLASH);
        profileSaved = profileSaved && configDb.storeToFile(CONFIGDB_USER_IN_FLASH);

    }
    return profileSaved;
    // MDC - 12/06/04 - added the above for XPH-24
}

// Checks for a change of IP address.  If a change is detected,
// then a restart is requested.
void Pinger::checkIP()
{
    static UtlString lastIP = "";
    UtlString currentIP;
    UtlBoolean isDHCP = TRUE;

    if (lastIP == "")
    {
        PingerInfo::getCurrentIPAddress(lastIP, isDHCP);
        return;
    }
    PingerInfo::getCurrentIPAddress(currentIP, isDHCP);
    if (lastIP != currentIP)
    {
          lastIP = currentIP;
          Pinger::getPingerTask()->restart(TRUE, 0, RESTART_NEW_IP) ;    
    }
    
}


// Schedule a restart sometime in the future
UtlBoolean Pinger::scheduleRestart(UtlBoolean confirm, UtlBoolean ignoreState, const char* reason, int seconds)
{
   // Free the restart timer and event
   if (mpRestartTimer != NULL)
   {
      mpRestartTimer->stop() ;
      delete mpRestartTimer ;
      mpRestartTimer = NULL ;
   }
   if (mpRestartEvent != NULL)
   {
      delete mpRestartEvent ;
      mpRestartEvent = NULL ;
   }

   // Create a temp data structure to communicate with the handler
   struct tagRestartEventInfo* pInfo = new struct tagRestartEventInfo ;
   pInfo->confirm = confirm ;
   pInfo->ignoreState = ignoreState ;
   if (reason == NULL)
      pInfo->reason = NULL ;
   else
      pInfo->reason = strdup(reason) ;

   // Create the event/timer
   mpRestartEvent = new OsQueuedEvent(*getMessageQueue(), 0) ;
   mpRestartEvent->setUserData((int) pInfo) ;
   mpRestartTimer = new OsTimer(*mpRestartEvent) ;

   // Force a min of 1 seconds
   if (seconds <= 0)
      seconds = 1 ;

   // Finally, set the timer
   mpRestartTimer->oneshotAfter(OsTime(seconds, 0)) ;

   return OS_SUCCESS ;
}


// Set up the system configuration database from the default file name
void Pinger::initLocalConfigFile()
{
   UtlString strSerialNumber;
   mRestartCount = 0;
   osPrintf("---- Pinger::initLocalConfigFile: %s\n", CONFIGDB_LOCAL_IN_FLASH);

   if(mLocalConfigDb.loadFromFile(CONFIGDB_LOCAL_IN_FLASH) == OS_SUCCESS)
   {
      osPrintf("Found config file: %s\n", CONFIGDB_LOCAL_IN_FLASH);
      mLocalConfigDb.get("LOCAL_RESTART_COUNT", mRestartCount);
      mLocalConfigDb.get("SERIAL_NUMBER", strSerialNumber);

      mLocalConfigDb.get("PHONESET_AUDIO_DEVICE_CALL", mCallDevice);
      mLocalConfigDb.get("PHONESET_AUDIO_DEVICE_RINGER", mRingDevice);
      mLocalConfigDb.get("PHONESET_AUDIO_DEVICE_MIC", mMicDevice);
   }


   /*
     The follow code allows the local-config to override the serial number
     for a given phone.  Since the serial can be generated in some 
     circumstances, the current value is stored to the local-config when 
     not found -- this helps keeps the serial number sticky in cases 
     where the serial number is generated on volatile data (e.g. mac 
     address on a laptop).
    */
   BaseLicenseManager * pLM = BaseLicenseManager::getInstance();
   assert(pLM != NULL) ;
   if (strSerialNumber.length())
   {
       UtlBoolean bRC ;
       bRC = pLM->setSerialNumber(strSerialNumber);
       assert(bRC == TRUE) ;
   }
   else
   {
       pLM->getSerialNumber(strSerialNumber);
       mLocalConfigDb.set("SERIAL_NUMBER", strSerialNumber.data());
   }   

   sConfigEncryption.initLocalConfig(&mLocalConfigDb);

   mRestartCount++;
   char Buffer[64];
   sprintf(Buffer, "%d", mRestartCount);
   mLocalConfigDb.set("LOCAL_RESTART_COUNT",Buffer);

   saveLocalConfigDb() ;
}

// Set up the configuration database from the default file name
void Pinger::initConfigFile(OsConfigDb* config)
{
   UtlString configFileName;

   configFileName.insert(0, CONFIGDB_NAME_IN_FLASH);

   if(config->loadFromFile((char *)configFileName.data()) == OS_SUCCESS)
   {
      osPrintf("Found config file: %s\n", configFileName.data());
   }
   else
   {
      OsSocket::getHostIp(&configFileName);
      osPrintf("---- Pinger::initConfigFile: %s\n", configFileName.data());
      configFileName.insert(0, CONFIGDB_PREFIX_ON_SERVER);

      if(config->loadFromFile((char *)configFileName.data()) == OS_SUCCESS)
      {
         osPrintf("Found config file: %s\n", configFileName.data());
      }
      else
      {
         config->set("PHONESET_MAX_ACTIVE_CALLS_ALLOWED", "10");
         config->set("PHONESET_DIALPLAN_LENGTH", "12");
         config->set("PHONESET_AVAILABLE_BEHAVIOR", "RING");
         config->set("PHONESET_BUSY_BEHAVIOR", "BUSY");
         config->set("PHONESET_DND_METHOD", "FORWARD_ON_BUSY") ;
         config->set("PHONESET_FORWARD_UNCONDITIONAL", "DISABLE") ;
         config->set("PHONESET_TIME_DST_RULE", "");
         config->set("PHONESET_TIME_OFFSET", "");
         config->set("SIP_UDP_PORT", "5060");
         config->set("SIP_TCP_PORT", "5060");
         config->set("SIP_TLS_PORT", "5061");
         config->set("PHONESET_RTP_PORT_START", "8766");

         config->set(CONFIG_PHONESET_CALL_WAITING_BEHAVIOR, "ALERT");
         config->set(CONFIG_PHONESET_RINGER, "BOTH");
         config->set("SIP_PROXY_SERVERS", "");
         config->set("SIP_DIRECTORY_SERVERS", "");
         config->set("SIP_REGISTER_PERIOD", "3600");
         config->set("SIP_FORWARD_UNCONDITIONAL", "");
         config->set("SIP_FORWARD_ON_BUSY", "");
         config->set("SIP_FORWARD_ON_NO_ANSWER", "");
         config->set("SIP_AUTHENTICATE_SCHEME", "");
         config->set("SIP_AUTHENTICATE_REALM", "");

         config->set("SIP_SESSION_REINVITE_TIMER", "");

     // Try to create a default config file in flash

         osPrintf("Creating config file: %s\n", CONFIGDB_NAME_IN_FLASH);

         if (config->storeToFile(CONFIGDB_NAME_IN_FLASH) != OS_SUCCESS)
         {
             if (OS_SUCCESS!=config->storeToFile((char *)configFileName.data()))
             {
                 osPrintf("Could not create %s config file",
                    configFileName.data());
             }
             else
                  osPrintf("Write file %s",configFileName.data());
         }
         else
             osPrintf("Write file %s",CONFIGDB_NAME_IN_FLASH);
      }
   }

   configFileName.remove(0);
   configFileName.insert(0, CONFIGDB_USER_IN_FLASH);

   if(config->loadFromFile((char *)configFileName.data()))
   {
      osPrintf("Found user config file: %s\n", configFileName.data());
   }
   else
   {
      osPrintf("User config file %s not found.\n", configFileName.data());
   }

   //here's a value which may be used to set the default adapter
   UtlString mac_address;

   config->get("PHONESET_BIND_MAC_ADDRESS", mac_address);
   unsigned long address = OsSocket::initDefaultAdapterID(mac_address);
   OsSocket::setDefaultBindAddress(address);

   UtlString enableJNIDebugging;
   if (config->get("PHONESET_ENABLE_JNI_DEBUGGING", enableJNIDebugging) == OS_SUCCESS)
   {

        g_bRealtimeDebug = true;
        SetJNIDebugLevel(atoi(enableJNIDebugging.data()));
   }

   // During startup, we will suspend on an assertion failure.
   // Now, we check to see whether we are to continue in that mode for
   // debugging, or if not specified, we switch to rebooting on an
   // assertion failure.  Note that this is only done on VxWorks,
   // the function called below is defined as nothing on other platforms.

   OsAssert_SetFailureAction(OS_ASSERT_REBOOT);
   UtlString strRebootOnAssert;
   if (config->get("PHONESET_ASSERT_ACTION", strRebootOnAssert) == OS_SUCCESS)
   {
      strRebootOnAssert.toUpper();

   // if param is found and is set to SUSPEND, set assert action to SUSPEND
      if (strRebootOnAssert == "SUSPEND") {
         OsAssert_SetFailureAction(OS_ASSERT_SUSPEND);
      }
   }

   //
   UtlString strHandsetEQSetting;
   if (config->get("PHONESET_HANDSET_EQ_SETTING", strHandsetEQSetting) == OS_SUCCESS)
   {
      strHandsetEQSetting.toUpper();
      PingerInfo::setHandsetEQSetting(strHandsetEQSetting);     
   }

   sConfigEncryption.initConfig(config);
}


 
// Start the timer for periodically checking the IP adress
void Pinger::initIPCheckTimer(OsConfigDb* config)
{
   int      secs;
   OsStatus res;

   res = config->get("PHONESET_IPCHECK_PERIOD", secs);
   if ((res == OS_NOT_FOUND) || secs < 10)
   {
      secs = DEFAULT_IPCHECK_PERIOD_SECS;
   }

   mpCheckIPEvent = new OsQueuedEvent(mIncomingQ, 0);
   mpCheckIPTimer = new OsTimer(*mpCheckIPEvent);

   // start the timer running
   // it will fire immediately and then every ipCheckPeriod thereafter
  {
      OsTime ipCheckPeriod(secs, 0);

      res = mpCheckIPTimer->periodicEvery(OsTime(10, 0),
                                             ipCheckPeriod);
      assert(res == OS_SUCCESS);
   }
}
 
// initialize the Deployment server
int Pinger::initDeploymentServer(OsConfigDb* config)
{
   UtlString   deployServer;
   PingerInfo pingerInfo;
   int ret = ERR_CONFIG_NOTPRESENT;

   // Get the deployment server parameter from the config file
   config->get(CONFIG_PHONESET_DEPLOYMENT_SERVER, deployServer);

   osPrintf("Pinger::initDeploymentServer: deployment server name: %s\n",
      deployServer.data());
   if (!deployServer.isNull())
   {
      if (deployServer.compareTo("-"))
      {
#ifdef DISPLAY_STARTUP_STATUS
         displayStartupStatus(3, "Request Config") ;
#endif

         // check the configuration, get new config file from deployment
         // server if needed
         UtlString adminDomain;

         if (msLogicalId.isNull())
            config->get(CONFIG_PHONESET_LOGICAL_ID, msLogicalId);
         config->get(CONFIG_PHONESET_ADMIN_DOMAIN, adminDomain);

         if (msLogicalId.isNull() || adminDomain.isNull())
         {
            osPrintf("Pinger::initDeploymentServer: msLogicalId %s or"
               " adminDomain %s null.\n",
               msLogicalId.data(), adminDomain.data());
            return ERR_CONFIG_INCORRECT;
         }

         UtlString request;
         UtlString server = deployServer;
         UtlString path;

         if (deployServer.compareTo(MY_PINGTEL_SERVER)==0) {
         } else {
             // Using other deployment server
             PingerInfo::setupGetConfigUrl(request,
                                              adminDomain, msLogicalId);

             unsigned int pos = deployServer.first('/');
             if (pos != UTL_NOT_FOUND)
             {
                server = UtlString(deployServer.data(), pos);
                path = deployServer(pos, (deployServer.length() - pos));
                request = path + request;
             }

             ret = getConfigFromDeploymentServer(server, request);
             osPrintf("Pinger::initDeploymentServer:"
                " getConfigFromDeploymentServer server %s returns: %d\n",
                server.data(), ret);
         }

         if (ret == ERR_CONFIG_OK)
         {
            initConfigFile(config);  // use the new config file
         }
      }
      else
      {
         //  standalone mode, no deployment server set
         osPrintf("Pinger::initDeploymentServer: standalone mode.\n");
         ret = ERR_CONFIG_STANDALONE;
      }
   }
   // else, need to use Java GUI to setup delpoyment server options
   // with user, which is done in JPingerStart.

   return ret;
}

// Initialize default priority 0-255 (default is 128)
void Pinger::initDefaultProcessPriority(OsConfigDb* pConfig)
{
    int priority;

   // If configured, set the PHONESET_SNMP_SYS_CONTACT
   if (pConfig->get(CONFIG_PHONESET_DEFAULT_PRIORITY, priority)
       != OS_SUCCESS)
        priority = 64; //default

    //this function should set the "process priority" not the thread priority
    //all thread priorities willl be based on this level
#ifdef WIN32
    int priorityClass = NORMAL_PRIORITY_CLASS;

    if (priority >= 0 && priority <= 63)
        priorityClass = REALTIME_PRIORITY_CLASS;
    else
    if (priority >= 64 && priority <= 127)
        priorityClass = HIGH_PRIORITY_CLASS;
    else
    if (priority >= 128 && priority <= 191)
        priorityClass = NORMAL_PRIORITY_CLASS;
    else
    if (priority >= 192 && priority <= 255)
        priorityClass = IDLE_PRIORITY_CLASS;


    if (!SetPriorityClass(GetCurrentProcess(),priorityClass))
        osPrintf("!!!!!!! Could not set process priority!!!!!!!\n");
    else
        osPrintf("Successfully change priority to %d.\n",priority);

#endif

}

// Initialize the OsSysLog
void Pinger::initSysLog(OsConfigDb* pConfig)
{
   UtlString logLevel;
   int      logEntries ;
   UtlString consoleLogging;
   UtlString fileTarget;
   UtlString fileFlushPeriod;
   UtlString remoteTarget;
   UtlString serialNumber;

   struct tagPrioriotyLookupTable
   {
      const char*      pIdentity;
      OsSysLogPriority ePriority;
   } ;

   struct tagPrioriotyLookupTable lkupTable[] =
   {
      { "DEBUG",   PRI_DEBUG},
      { "INFO",    PRI_INFO},
      { "NOTICE",  PRI_NOTICE},
      { "WARNING", PRI_WARNING},
      { "ERR",     PRI_ERR},
      { "CRIT",    PRI_CRIT},
      { "ALERT",   PRI_ALERT},
      { "EMERG",   PRI_EMERG},
   } ;

   //
   // Get/Apply LogEntries
   //
   if (pConfig->get(CONFIG_PHONESET_LOG_ENTRIES, logEntries) != OS_SUCCESS)
      logEntries = 100 ;
   if (logEntries < 0)
      logEntries = 0 ;
   if (logEntries > 9999)
      logEntries = 9999 ;
   PingerInfo::getSerialNum(serialNumber) ;
   OsSysLog::initialize(logEntries, serialNumber.data()) ;

   //
   // Get/Apply Log Level
   //
   if ((pConfig->get(CONFIG_PHONESET_LOG_LEVEL, logLevel) != OS_SUCCESS) ||
         logLevel.isNull())
   {
      logLevel = "ERR" ;
   }
   logLevel.toUpper();
   OsSysLogPriority priority = PRI_ERR ;
   int iEntries = sizeof(lkupTable)/sizeof(struct tagPrioriotyLookupTable) ;
   for (int i=0; i<iEntries; i++)
   {
      if (logLevel == lkupTable[i].pIdentity)
      {
         priority = lkupTable[i].ePriority ;
         break ;
      }
   }
   OsSysLog::setLoggingPriority(priority) ;

   //
   // Get/Apply console logging
   //
   if ((pConfig->get(CONFIG_PHONESET_LOG_CONSOLE, consoleLogging) == OS_SUCCESS))
   {
      consoleLogging.toUpper() ;
      if (consoleLogging == "ENABLE")
      {
         OsSysLog::enableConsoleOutput(true) ;
      }
   }

   //
   // Get/Apply Remote Target
   //
   if ((pConfig->get(CONFIG_PHONESET_LOG_TARGET, remoteTarget) == OS_SUCCESS) && !remoteTarget.isNull())
   {
      OsSysLog::addOutputSocket(remoteTarget.data());
   }

   //
   // Get/Apply Flush and File
   //
   if ((pConfig->get(CONFIG_PHONESET_LOG_FILE_FLUSH_PERIOD, fileFlushPeriod) != OS_SUCCESS) || fileFlushPeriod.isNull())
      fileFlushPeriod = "14400" ;
   int flushPeriod = atoi(fileFlushPeriod.data());
   if (flushPeriod < 0)
      flushPeriod = 0 ;
   if (flushPeriod > 86400)
      flushPeriod = 86400 ;
   if ((pConfig->get(CONFIG_PHONESET_LOG_FILE, fileTarget) != OS_SUCCESS) || fileTarget.isNull())
   {
      PingerInfo::getFlashFileSystemLocation(fileTarget) ;
      fileTarget.append("/syslog.txt") ;
   }
   if (fileTarget != "-")
   {
      OsSysLog::setOutputFile(flushPeriod, fileTarget.data());
   }
}


#ifdef HEARTBEATS_ARE_READY_FOR_PRIME_TIME /* [ */
 // init the heartbeat task
void Pinger::initHeartbeat(OsConfigDb* config)
{
   mpHeartbeat = HeartbeatTask::getHeartbeatTask(config);
} 
#endif /* HEARTBEATS_ARE_READY_FOR_PRIME_TIME ] */

// get config info from the Deployment server
int Pinger::getConfigFromDeploymentServer(UtlString& deployServer,
                                          UtlString& requestCmd)
{
   int ret = ERR_CONFIG_OK;

   osPrintf("Pinger:: getConfigFromDeploymentServer server: %s, request: %s\n",
      deployServer.data(), requestCmd.data());
   HttpMessage *pResquest = new HttpMessage();
   pResquest->setFirstHeaderLine("GET",
                              requestCmd.data(), HTTP_PROTOCOL_VERSION);
   pResquest->addHeaderField("Accept", "*/*");
   pResquest->addHeaderField("Accept-Language", "en-us;q=0.5");
   pResquest->addHeaderField("User-Agent", "Mozilla/4.0 (compatible)");
   pResquest->addHeaderField("Host", CONFIG_DEPLOYMENT_SERVER);
   pResquest->addHeaderField("Connection", "Keep-Alive");

   ret = sendRequestToDeploymentServer(pResquest, deployServer);
   if (ret != ERR_CONFIG_OK)
   {
      delete pResquest;
      return ret;
   }

   UtlString buffer;
   int bufferLen = 0;
   HttpBody *pBody = (HttpBody *)pResquest->getBody();
   if (pBody)
   {
      pBody->getBytes(&buffer, &bufferLen);
   }

   UtlString configFileName;
   if (bufferLen > 0 && buffer.length() > 0)
   {
      // Create the config html page on the flash

      configFileName.insert(0, CONFIGDB_NAME_IN_FLASH);

      FILE* fp = fopen(configFileName.data(), "w+");

      if(!fp)
      {
         OsSocket::getHostIp(&configFileName);
         osPrintf("---- Pinger::getConfigFromDeploymentServer: %s\n",
            configFileName.data());
         configFileName.insert(0, CONFIGDB_PREFIX_ON_SERVER);
         fp = fopen(configFileName.data(), "w+");
      }

      if(fp)
      {
         int bytes = fwrite(buffer.data(), sizeof(char), buffer.length(), fp);

         osPrintf("Pinger::getConfigFromDeploymentServer: %d bytes written"
            " into config  file %s.\n", bytes, configFileName.data());
         fclose(fp);
      }
      else
      {
         ret = ERR_CONFIG_INCORRECT;
         osPrintf("ERROR: Pinger::getConfigFromDeploymentServer: failed to"
            " open config file %s.\n", configFileName.data());
      }
   }
   else
   {
      ret = ERR_CONFIG_INCORRECT;
      osPrintf("ERROR: Pinger::getConfigFromDeploymentServer:"
         " %d bytes, Config file %s is not changed.\n",
         bufferLen, configFileName.data());
   }


   delete pResquest;
   return ret;
}

// This method sends a request to getDeviceId on the my.pingtel site
// It is called on startup, to send device information to my.pingtel for
//   the current user

int Pinger::sendDeviceId(UtlString& strServer, UtlString& strRequest)
{
   int ret = ERR_CONFIG_OK;

   osPrintf("Pinger::getDeviceId from DeploymentServer server:"
      " %s, request: %s\n", strServer.data(), strRequest.data());
   HttpMessage *pRequest = new HttpMessage();
   pRequest->setFirstHeaderLine("GET",
                                    strRequest.data(), HTTP_PROTOCOL_VERSION);
   pRequest->addHeaderField("Accept", "*/*");
   pRequest->addHeaderField("Accept-Language", "en-us;q=0.5");
   pRequest->addHeaderField("User-Agent", "Mozilla/4.0 (compatible)");
   pRequest->addHeaderField("Host", MY_PINGTEL_SERVER);
   pRequest->addHeaderField("Connection", "Keep-Alive");

   ret = sendRequestToDeploymentServer(pRequest, strServer);
   if (ret != ERR_CONFIG_OK)
   {
      delete pRequest;
      return ret;
   }

   UtlString buffer;
   int bufferLen = 0;
   HttpBody *pBody = (HttpBody *)pRequest->getBody();
   if (pBody)
   {
      pBody->getBytes(&buffer, &bufferLen);
   }

   UtlString configFileName;
   if (bufferLen > 0 && buffer.length() > 0)
   {
      if (buffer.contains("SERVICE_SET")) {
         // service is set - need to get config file
         ret = ERR_CONFIG_SERVICE_SET ;
      }
   }

   delete pRequest;
   return ret;
}

// get the logical id from the deployment server
int Pinger::getDeploymentLogicalId(OsConfigDb* config, UtlString& logicalId)
{
   int ret = ERR_CONFIG_NOTPRESENT;
   UtlString deployServer = CONFIG_DEPLOYMENT_SERVER;
   osPrintf("deployServer: %s\n", deployServer.data());

   UtlString adminDomain;
   UtlString adminId;
   UtlString adminPwd;
   config->get(CONFIG_PHONESET_ADMIN_DOMAIN, adminDomain);
   config->get(CONFIG_PHONESET_ADMIN_ID, adminId);
   config->get(CONFIG_PHONESET_ADMIN_PWD, adminPwd);

   UtlString request;
   PingerInfo::setupGetLogicalIdUrl(request,
                     adminDomain.data(), adminId.data(), adminPwd.data());
   request = CONFIG_DEPLOYMENT_PATH + request;
   osPrintf("\nrequest: %s\n", request.data());

   HttpMessage *pResquest = new HttpMessage();
   pResquest->setFirstHeaderLine("GET", request.data(), HTTP_PROTOCOL_VERSION);
   pResquest->addHeaderField("Accept", "*/*");
   pResquest->addHeaderField("Accept-Language", "en-us;q=0.5");
   pResquest->addHeaderField("User-Agent", "Mozilla/4.0 (compatible)");
   pResquest->addHeaderField("Host", CONFIG_DEPLOYMENT_SERVER);
   pResquest->addHeaderField("Connection", "Keep-Alive");

   ret = sendRequestToDeploymentServer(pResquest, deployServer);

   int bufferLen;
   HttpBody *pBody = (HttpBody *)pResquest->getBody();
   if (pBody && ret == ERR_CONFIG_OK)
   {
      pBody->getBytes(&logicalId, &bufferLen);
      logicalId.strip(UtlString::both, '\r');
      logicalId.strip(UtlString::both, '\n');
      int pos = logicalId.index("\n");
      if (pos > 0)
      {
         logicalId.remove(pos);
      }
      osPrintf("Pinger:: getDeploymentLogicalId single id: %s\n",
         logicalId.data());
   }
   else if (pBody && ret == ERR_CONFIG_MULTIPLE)
   {
      pBody->getBytes(&logicalId, &bufferLen);
      logicalId.strip(UtlString::both, '\r');
      logicalId.strip(UtlString::both, '\n');
      int pos = logicalId.index("\n");
      if (pos > 0)
      {
         logicalId.remove(pos);
      }
      osPrintf("Pinger:: getDeploymentLogicalId multiple id: %s\n",
         logicalId.data());
   }
   return ret;
}

int Pinger::sendRequestToDeploymentServer(HttpMessage*& pResquest,
                                          UtlString& deployServer)
{
   OsConnectionSocket *pSocket;
   pSocket = new OsConnectionSocket(DEPLOYMENT_HTTP_PORT, deployServer.data());

   int recvd = 0;
   if (pSocket && pSocket->isOk())
   {
      if (pResquest->write(pSocket))
      {
         recvd = pResquest->read(pSocket);
      }
      pSocket->close();
      delete pSocket;
   }
   else
   {
      return ERR_CONFIG_CANNOTREACH;  // couldn't talk to the deployment server
            // need error handling!!
   }

   char *header = (char*)pResquest->getFirstHeaderLine();

   int ret = -1;
   sscanf(header, "HTTP/1.1 %d ", &ret);
   if (ret == -1)
      sscanf(header, "HTTP/1.0 %d ", &ret);

   osPrintf("Pinger:: return code from deployment server: %d\n", ret);
   int err;

   switch (ret)
   {
   case 200:
      err = ERR_CONFIG_OK;
      break;
   case 401:
      err = ERR_CONFIG_UNAUTHORIZED;
      break;
   case 303:
      err = ERR_CONFIG_MULTIPLE;
      break;
   case 409:
      err = ERR_CONFIG_CONFLICT;
      break;
   default:
      err = ERR_CONFIG_INCORRECT;
      break;
   }

   return err;
}

OsStatus Pinger::getHttpStatus()
{
   return httpStatus;
}

void Pinger::initHttpRequestProcessor(HttpServer *pHttpServer)
{
  // Method to post files to phone file system
  pHttpServer->addRequestProcessor("/cgi/putFile.cgi",
      processPostFile);

  // Method to get SIP information
  pHttpServer->addRequestProcessor("/cgi/sip.cgi", processSipPage);
  pHttpServer->addRequestProcessor("/cgi/toggleSipLog.cgi",
     processToggleSipLog);

  pHttpServer->addRequestProcessor("/cgi/cgi-test.cgi",
     HttpServer::testCgiRequest);

  // Added web based dialing CGIs
  pHttpServer->addRequestProcessor("/cgi/dial.cgi", processDialCgi);
  pHttpServer->addRequestProcessor("/cgi/drop.cgi", processDropCgi);
  pHttpServer->addRequestProcessor("/cgi/answer.cgi", processAnswerCgi);
  pHttpServer->addRequestProcessor("/cgi/transfer.cgi", processTransferCgi);
  pHttpServer->addRequestProcessor("/cgi/hold.cgi", processHoldCgi);
  pHttpServer->addRequestProcessor("/cgi/offhold.cgi", processOffHoldCgi);



  // Added get callIds cgi
  pHttpServer->addRequestProcessor("/cgi/getCallIds.cgi",
     processGetCallIdsCgi);

  // Pingtel.net service
  pHttpServer->addRequestProcessor("/servlets/getConfig.cgi",
     processGetDeployConfigFile);
  pHttpServer->addRequestProcessor("/servlets/getLogicalId.cgi",
     processGetLogicalId);

  // Method to get call state information
  pHttpServer->addRequestProcessor("/cgi/stateInfo.cgi",
     processCallStateInfo);
  pHttpServer->addRequestProcessor("/cgi/toggleStateLog.cgi",
     processToggleCallStateLog);

#ifdef DEBUG_VERSIONCHECK
  // Method to get installation url
  pHttpServer->addRequestProcessor("/cgi/upgrade.cgi",
     processInstallUrl);
  pHttpServer->addRequestProcessor("/cgi/upgradeComplete.cgi",
     processGetInstallUrl);

 // get the latest file
  pHttpServer->addRequestProcessor("/cgi/vc.cgi",
     processVersionCheckForm);
 // do version check
  pHttpServer->addRequestProcessor("/cgi/dovc.cgi",
     processDoVersionCheck);
#endif // DEBUG_VERSIONCHECK

  // Call control debug:
  pHttpServer->addRequestProcessor("/cgi/PrintCalls.cgi", PrintCalls);
  pHttpServer->addRequestProcessor("/cgi/PrintSipUa.cgi", PrintSipUa);


}



// Initializes the HTTP server depending on config setting in the
// pinger-config or user-config.  Doing this is slightly painful, as
// the defaults are slightly different depending on the platform that
// you are on.
//
// Hardphone:
//
// A PHONESET_HTTP_PORT setting of <= 0 indicates that the web server should
// be disabled.
//
// A PHONESET_HTTP_PORT of > 0 or a blank value or missing key indicates
// that the web server should be enabled.  A blank value/key should default
// to using port of 80.
//
// Softphone:
//
// Same as above, except a blank value and/or key indicates that the web
// server should be disabled.
//
void Pinger::initHttpServer(OsConfigDb* config)
{
   int      httpSecureServerPort = 0;
   int      httpNonSecureServerPort = 0;
   UtlString osNonSecurePort ;
   UtlString osSecurePort ;
   UtlString osBaseUriDirectory ;
   OsStatus secureStatus ;
   OsStatus nonsecureStatus ;

   httpStatus = OS_TASK_NOT_STARTED;

   // Get the port settings
   //they return 0 if string is empty or -1 if tag is missing completely
   nonsecureStatus = config->get("PHONESET_HTTP_PORT", osNonSecurePort) ;
   secureStatus = config->get("PHONESET_HTTPS_PORT", osSecurePort) ;
   config->get("PHONESET_HTTP_PORT", httpNonSecureServerPort) ;
   config->get("PHONESET_HTTPS_PORT", httpSecureServerPort) ;

   // Get the webserver authentication database
   OsConfigDb* userPasswordDigestDb = new OsConfigDb() ;
   config->getSubHash("PHONESET_HTTP_AUTH_DB.", *userPasswordDigestDb) ;

   // Determine whether we should start the web server or not.  Use the port
   // value as the decision point.  Anything > 0 means enable.
#if defined(_WIN32) || defined(__pingtel_on_posix__)

   osBaseUriDirectory = CONFIG_PREFIX_USER_DATA ;

   if (nonsecureStatus == OS_NOT_FOUND)
      httpNonSecureServerPort = -1;

   if (nonsecureStatus == OS_NOT_FOUND)
      httpSecureServerPort = -1;

   if (nonsecureStatus == OS_SUCCESS && httpNonSecureServerPort <=0)
      httpNonSecureServerPort = -1;

   if (secureStatus == OS_SUCCESS && httpSecureServerPort <=0)
      httpSecureServerPort = -1;
#endif

   // Start up webserver if port is > 0
   if(httpSecureServerPort > 0 || httpNonSecureServerPort > 0)
   {

      osPrintf("Starting HTTP  Embedded Web Server on port %d...\n",
            httpNonSecureServerPort) ;

      OsServerSocket *pServerSocket = NULL;

#if 0
/* DWW disable HTTPS server until we finish it */

      if (httpSecureServerPort > 0)
      {
            osPrintf("Starting HTTPS Embedded Web Server on port %d...\n",
                    httpSecureServerPort) ;

            OsSSLServerSocket::setSSLPublicCertificateLocation(
                HTTPS_PUBLIC_CERTIFICATE_FILE_LOCATION);
            OsSSLServerSocket::setSSLPrivateKeyLocation(
                HTTPS_PRIVATE_KEY_FILE_LOCATION);
            OsSSLServerSocket::setSSLPrivateKeyPassword("PingMe");

          pServerSocket = new OsSSLServerSocket(50, httpSecureServerPort);
          mpHttpSecureServer = new HttpServer(pServerSocket, userPasswordDigestDb,
                "sipXphone");
          // Set the web server root to the current directory
          mpHttpSecureServer->addUriMap("/", osBaseUriDirectory.data()) ;
      }
#endif
      if (httpNonSecureServerPort > 0)
      {
          pServerSocket = new OsServerSocket(50, httpNonSecureServerPort);
          mpHttpServer = new HttpServer(pServerSocket, userPasswordDigestDb,
                "sipXphone");
          // Set the web server root to the current directory
          mpHttpServer->addUriMap("/", osBaseUriDirectory.data()) ;
      }

   }
   else
   {
      delete userPasswordDigestDb;
      userPasswordDigestDb = NULL;
   }

   //before we go and start anything secure, lets check our license
   //situation.
//   certificateCheck();

   if(mpHttpServer)
   {
       initHttpRequestProcessor(mpHttpServer);
       mpHttpServer->start();
       httpStatus = mpHttpServer->getStatus();
   }
/*
   if(mpHttpSecureServer)
   {
#ifndef NO_SSL_LIB
       initHttpRequestProcessor(mpHttpSecureServer);
       mpHttpSecureServer->start();
       httpStatus = mpHttpSecureServer->getStatus();
#endif
   }
*/
}

static UtlString codecOverrideList("");
static int useCodecOverride = 0;
extern "C" {
   int overrideCodecs(char* list);
}

int overrideCodecs(char* list)
{
   UtlString save(codecOverrideList);

   if (NULL == list) {
      codecOverrideList = "";
      osPrintf("Replacing list '%s' with <NOTHING>\n", save.data());
      useCodecOverride = 0;
   } else {
      codecOverrideList = list;
      osPrintf("Replacing list '%s' with '%s'\n",
         save.data(), codecOverrideList.data());
      useCodecOverride = 1;
   }
   return useCodecOverride;
}

// initialize the set of codecs to be available for use
// Set the codecs from the config. parameter

void Pinger::initCodecs(OsConfigDb* config)
{
   UtlString codecListParameter;
   UtlString oneCodec;
   int codecStringIndex = 0;
   SdpCodec::SdpCodecTypes internalCodecId;
   BaseLicenseManager *lm = NULL;

   {
      config->get("PHONESET_RTP_CODECS", codecListParameter);
   }

   if (codecListParameter.isNull()) {
      codecListParameter = DEFAULT_CODEC_LIST_STRING;
   }

   if (!mpCodecFactory) {
      mpCodecFactory = new SdpCodecFactory();
   }

   NameValueTokenizer::getSubField(codecListParameter, codecStringIndex,
                                        ", \n\r\t", &oneCodec);

   lm = BaseLicenseManager::getInstance();

   while(!oneCodec.isNull())
   {
      if (lm->isLicensed(oneCodec))
      {
         internalCodecId = SdpCodecFactory::getCodecType(oneCodec.data());
         if (internalCodecId != SdpCodec::SDP_CODEC_UNKNOWN)
         {
            mpCodecFactory->buildSdpCodecFactory(1, &internalCodecId);
         } else {
            osPrintf("\n\n** Unknown codec ID: %s **\n\n",
               oneCodec.data());
         }
      } else {
         osPrintf("\n\n** This product is not licensed for codec: %s **\n\n",
            oneCodec.data());
      }

      codecStringIndex++;
      NameValueTokenizer::getSubField(codecListParameter, codecStringIndex,
                                     ", \n\r\t", &oneCodec);
   }
}


//initialize RefreshManager
void Pinger::initRefreshMgr(OsConfigDb* config)
{
   UtlString sipProxies;
   UtlString sipDirectories;
   UtlString sipRegistries;
   UtlString publicAddress;
   int sipUdpPort;
   int sipTcpPort;
   int sipTlsPort;
   int sipRegisterPeriodSeconds;
   UtlString extension;
   UtlString sipAddress;
   UtlString Subscribe;
   int sipSubscribePeriodSeconds;

   sipTcpPort = mpSipUserAgentTask->getTcpPort() ;
   sipUdpPort = mpSipUserAgentTask->getUdpPort() ;
   sipTlsPort = mpSipUserAgentTask->getTlsPort() ;


   //save these off...we'll need them later for error reporting
   gSipUdpPort = sipUdpPort;
   gSipTcpPort = sipTcpPort;
   gSipTlsPort = sipTlsPort;

   // initialize the SIP user agent
   config->get("PHONESET_EXTENSION", extension);
   config->get("PHONESET_EXTERNAL_IP_ADDRESS", publicAddress);

   if(publicAddress.isNull())
      OsSocket::getHostIp(&publicAddress);

   config->get("SIP_ADDRESS", sipAddress);
   config->get("SIP_DIRECTORY_SERVERS", sipDirectories);
   config->get("SIP_REGISTRY_SERVERS", sipRegistries);
   config->get("SIP_REGISTER_PERIOD", sipRegisterPeriodSeconds);
   config->get("SIP_SUBSCRIBE_PERIOD", sipSubscribePeriodSeconds);

   UtlString macAddress;
   PingerInfo::getMacAddress(macAddress);

   mpRefreshMgrTask = new SipRefreshMgr();
   mpRefreshMgrTask->init(mpSipUserAgentTask,
      sipTcpPort,
      sipUdpPort,
      extension,
      publicAddress.data(),
      sipAddress.data(),
      sipDirectories.data(),
      sipRegistries.data(),
      sipRegisterPeriodSeconds,
      60*60*24,
      mRestartCount,
      macAddress.data());

}
//initialize configuration agent
void Pinger::initConfigAgent(OsConfigDb* config)

{
   // Setup the configuration agent
   UtlString fileSystemRoot;
   UtlString sipConfigServerAddress;
   int      sipConfigServerPort;

   PingerInfo::getRootDirectory(fileSystemRoot);
   UtlString model;
   PingerInfo::getPlatformType(model) ;
   UtlString version;
   PingerInfo::getVersion(version);
   UtlString macAddress;
   PingerInfo::getMacAddress(macAddress);
   UtlString serial;
   PingerInfo::getSerialNum(serial);
#ifdef PINGTEL_DEPLOYMENT_SERVER /* [ */
   mpConfigServerAgent = new SipConfigServerAgent(mpSipUserAgentTask) ;
   mpConfigServerAgent->start();
#else /* ] [ */

   config->get("SIP_CONFIG_SERVER_ADDRESS", sipConfigServerAddress);
   config->get("SIP_CONFIG_SERVER_PORT",    sipConfigServerPort);
   if (sipConfigServerPort <= 0)
      sipConfigServerPort = 0;

   mpConfigAgent = new SipConfigDeviceAgent(model.data(),
       version.data(),
       macAddress.data(),
       serial.data(),
       mpSipUserAgentTask,
       mpRefreshMgrTask,
       fileSystemRoot.data(),
       sipConfigServerAddress.data(),
       sipConfigServerPort,
       mRestartCount);

   mpConfigAgent->start();
   mpConfigAgent->discover();
   mpConfigAgent->enroll();

#endif /* ] */
}

void Pinger::initLineMgr( OsConfigDb* config)
{
   UtlString authScheme;

   // set if access is restricted
   config->get("SIP_AUTHENTICATE_SCHEME", authScheme);
   mpLineMgrTask = new SipLineMgr( authScheme);
}

void Pinger::LoadLinesOnStartup(OsConfigDb* config)
{
   UtlString enableLog;
   UtlString extension;
   UtlString sipAddress;
   UtlString publicAddress;
   UtlString sipRegistries;
   OsConfigDb* authorizeUserIds = new OsConfigDb();
   OsConfigDb* authorizePasswords = new OsConfigDb();

   // Get parameters defining how/whether access is retricted to this phone
   config->getSubHash("SIP_AUTHORIZE_USER.", *authorizeUserIds);
   config->getSubHash("SIP_AUTHORIZE_PASSWORD.", *authorizePasswords);

   // initialize the get line information
   // initialize the SIP user agent
   config->get("PHONESET_EXTENSION", extension);
   config->get("PHONESET_EXTERNAL_IP_ADDRESS", publicAddress);
   if(publicAddress.isNull())
      OsSocket::getHostIp(&publicAddress);
   config->get("SIP_ADDRESS", sipAddress);
   config->get("SIP_REGISTRY_SERVERS", sipRegistries);

   // Create REGISTER messages that get sent upon start to register at each
   // of the registration servers

   UtlBoolean isProvisioned = FALSE;
   UtlBoolean isDynamicIpAddress = FALSE;
   Url userEnteredUrl;
   Url identity;

   // Define the line from PHONESET_EXTENSION & SIP_REGISTRY_SERVERS
   // if present
   if(!sipRegistries.isNull())
   {
        UtlTokenizer tokenizer(sipRegistries);

        // url
        UtlString addr ;
        tokenizer.next(addr, ",") ;
        Url sipRegFromAddress(addr);
        sipRegFromAddress.setUserId(extension);
        identity = sipRegFromAddress;
   }

   // Define the line from SIP_ADDRESS if present
   else if(!sipAddress.isNull())
   {
       identity = sipAddress;
   }

   // Define the line from the local IP address
   else
   {
      isDynamicIpAddress = TRUE;
      Url defaultUrl;
      defaultUrl.setUserId(extension);
      defaultUrl.setHostAddress(publicAddress);
      identity = defaultUrl;
      identity.setUserId(extension);
      isProvisioned = TRUE;
   }

   // Make sure the userid is set
   UtlString urlUserId;
   identity.getUserId(urlUserId);
   if(urlUserId.isNull())
   {
       identity.setUserId("4444");
   }
   if(isDynamicIpAddress)
   {
      UtlString urlUserId;
      identity.getUserId(urlUserId);
      userEnteredUrl.setUserId(urlUserId);
   }
   else
   {
      userEnteredUrl = identity;
   }

    UtlString nextKey("");
    UtlString nextRealm;
    UtlString nextUserId;
    UtlString nextPassword;

    SipLine newLine;
    newLine.setIdentityAndUrl(identity, userEnteredUrl);
    newLine.setUser("Device");
    while ( authorizeUserIds->getNext(nextKey, nextRealm,
                               nextUserId) != OS_NO_MORE_DATA )
    {
      if (authorizePasswords->get(nextRealm.data(), nextPassword)
                                                   == OS_NOT_FOUND)
      {
         osPrintf("No password Token for %s realm: ", nextRealm.data());
      }
      else
      {
          UtlString passwordToken;
          HttpMessage::buildMd5UserPasswordDigest(nextUserId, nextRealm,
              nextPassword, passwordToken);
          newLine.addCredentials( nextRealm, nextUserId , passwordToken,
             HTTP_DIGEST_AUTHENTICATION);
      }
      nextKey = nextRealm;
    }
    if (isProvisioned)
    {
        newLine.setState(SipLine::LINE_STATE_PROVISIONED);
    }
    //when upgrading by default the Call forwarding is enabled
    newLine.setCallHandling(TRUE);
    mpLineMgrTask->addLine(newLine);
    Url canonicalUrl = newLine.getCanonicalUrl();
    mpLineMgrTask->setDefaultOutboundLine(canonicalUrl);
    mpCallMgrTask->setOutboundLine(canonicalUrl.toString());
}

// initialize the SIP user agent
void Pinger::initSipUserAgent(OsConfigDb* config)
{
   UtlString sipProxies;
   UtlString sipDirectories;
   UtlString publicAddress;
   UtlString sipRegistries;
   int sipUdpPort;
   int sipTcpPort;
   int sipTlsPort;
   UtlString extension;
   UtlString sipAddress;
   UtlString authScheme;
   UtlString authRealm;
   UtlString enableLog;
   OsConfigDb* authenticateDb = new OsConfigDb();
   OsConfigDb* authorizeUserIds = new OsConfigDb();
   OsConfigDb* authorizePasswords = new OsConfigDb();

   // Get parameters defining how/whether access is retricted to this phone

   // set if access is restricted
   config->get("SIP_AUTHENTICATE_SCHEME", authScheme);

   config->get("SIP_AUTHENTICATE_REALM", authRealm);
   config->getSubHash("SIP_AUTHENTICATE_DB.", *authenticateDb);

   // Get user ids and passwords for access to other phones
   config->getSubHash("SIP_AUTHORIZE_USER.", *authorizeUserIds);
   config->getSubHash("SIP_AUTHORIZE_PASSWORD.", *authorizePasswords);

   // initialize the SIP user agent
   config->get("PHONESET_EXTENSION", extension);
   sipTcpPort = config->getPort("SIP_TCP_PORT") ;
   sipUdpPort = config->getPort("SIP_UDP_PORT") ;
   sipTlsPort = config->getPort("SIP_TLS_PORT") ;

   // Temp hack: Don't allow all ports to be disabled
   if ((sipTcpPort == -1) && (sipUdpPort == -1))
   {
       sipUdpPort = 5060 ;
       sipTcpPort = 5060 ;
   }

   int maxNumSrvRecords = -1;
   config->get("SIP_DNSSRV_MAX_DESTS", maxNumSrvRecords);
   // If explicitly set to a valid number
  OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_DNSSRV_MAX_DESTS : %d",
              maxNumSrvRecords);
   if(maxNumSrvRecords > 0)
   {
      osPrintf("SIP_DNSSRV_MAX_DESTS : %d\n", maxNumSrvRecords);
   }
   else
   {
       maxNumSrvRecords = 4;
   }

   int dnsSrvTimeout = -1; //seconds
   config->get("SIP_DNSSRV_TIMEOUT", dnsSrvTimeout);
   // If explicitly set to a valid number
   OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_DNSSRV_TIMEOUT : %d",
             dnsSrvTimeout);
   if(dnsSrvTimeout > 0)
   {
      osPrintf("SIP_DNSSRV_TIMEOUT : %d\n", dnsSrvTimeout);
   }
   else
   {
      dnsSrvTimeout = 4;
   }


   int dnsSrvResolveTimeout = 0;
   config->get("SIP_DNSSRV_RESOLVE_TIMEOUT", dnsSrvResolveTimeout);
   // If explicitly set to a valid number
   OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_DNSSRV_RESOLVE_TIMEOUT : %d",
             dnsSrvResolveTimeout);
   if(dnsSrvResolveTimeout <= 0)
   {
      dnsSrvResolveTimeout = 1;
   }
   
   int dnsSrvResolveRetries = 0;
   config->get("SIP_DNSSRV_RESOLVE_RETRIES", dnsSrvResolveRetries);
   // If explicitly set to a valid number
   OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_DNSSRV_RESOLVE_RETRIES : %d",
             dnsSrvResolveRetries);
   if(dnsSrvResolveRetries <= 0)
   {
      dnsSrvResolveRetries = 2;
   }
   // now set the timeout and retries
   SipSrvLookup::setDnsSrvTimeouts(dnsSrvResolveTimeout, dnsSrvResolveRetries);

   config->get("PHONESET_EXTERNAL_IP_ADDRESS", publicAddress);
   // if(publicAddress.isNull()) OsSocket::getHostIp(&publicAddress);
   config->get("SIP_ADDRESS", sipAddress);
   config->get("SIP_PROXY_SERVERS", sipProxies);
   config->get("SIP_DIRECTORY_SERVERS", sipDirectories);
   config->get("SIP_REGISTRY_SERVERS", sipRegistries);
   config->get("SIP_LOG", enableLog); // ENABLE or DISABLE
   enableLog.toUpper();

   int sipFirstResendTimeout;
   config->get("SIP_RTT", sipFirstResendTimeout);

   // NAT ping parameters
   UtlString natPingUrl;
   UtlString natPingMethod;
   int natPingPeriodSeconds;
   config->get("SIP_NAT_PING_URL", natPingUrl);
   config->get("SIP_NAT_PING_PERIOD", natPingPeriodSeconds);
   config->get("SIP_NAT_PING_METHOD", natPingMethod);

   mpSipUserAgentTask = new SipUserAgent(sipTcpPort,
                                         sipUdpPort,
                                         sipTlsPort,
                                         publicAddress.data(),
                                         extension.data(),
                                         sipAddress.data(),
                                         sipProxies.data(),
                                         sipDirectories.data(),
                                         sipRegistries.data(),
                                         authScheme.data(),
                                         authRealm.data(),
                                         authenticateDb,
                                         authorizeUserIds,
                                         authorizePasswords,
                                         natPingUrl.data(),
                                         natPingPeriodSeconds,
                                         natPingMethod.data(),
                                         mpLineMgrTask,
                                         sipFirstResendTimeout,
                                         TRUE,
                                         -1,
                                         PINGER_SIPUSERAGENT_MAX_REQUEST_MSGS);

    UtlString symSignaling ;
    config->get("SIP_SYMMETRIC_SIGNALING", symSignaling) ;
    symSignaling.toUpper() ;
    if (symSignaling.compareTo("DISABLE") == 0)
    {
        mpSipUserAgentTask->setUseRport(FALSE) ;
    }
    else
    {
        mpSipUserAgentTask->setUseRport(TRUE) ;
    }

   // STUN parameters
   UtlString stunServer ;
   int stunRefreshPeriod ;
   config->get("SIP_STUN_SERVER", stunServer) ;
   if (stunServer.length())
   {
       UtlString strRefreshPeriod ;
       if ((config->get("SIP_STUN_REFRESH_PERIOD", strRefreshPeriod) == OS_NOT_FOUND) ||
               (strRefreshPeriod.length() == 0))
       {
           stunRefreshPeriod = 28 ;
       }
       else
       {
            config->get("SIP_STUN_REFRESH_PERIOD", stunRefreshPeriod) ;
            if (stunRefreshPeriod < 0)
                stunRefreshPeriod = 0 ;
       }
      
       mpSipUserAgentTask->enableStun(stunServer.data(), stunRefreshPeriod) ;
   }
   

   // Call timeout period (maximum amount of time to
   // allow the phone wait for the other/called end to ring.
   int defaultExpires = 180;
   config->get("SIP_DEFAULT_EXPIRES", defaultExpires);
   if(defaultExpires <= 0 ||
       defaultExpires > 180) defaultExpires = 180;
   mpSipUserAgentTask->setDefaultExpiresSeconds(defaultExpires);

   int maxForwards;
   config->get("SIP_MAX_FORWARDS", maxForwards);
   if(   maxForwards > 0
      && maxForwards <= MAX_FORWARDS_LIMIT
      )
   {
       mpSipUserAgentTask->setMaxForwards(maxForwards);
   }
   else if ( maxForwards != 0 )
   {
       OsSysLog::add( FAC_SIP, PRI_ERR, "SIP_MAX_FORWARDS %d out of range [1..%d]"
                     ,maxForwards, MAX_FORWARDS_LIMIT);
   }

   mpSipUserAgentTask->setDnsSrvTimeout(dnsSrvTimeout);
   mpSipUserAgentTask->setMaxSrvRecords(maxNumSrvRecords);

   if(enableLog.compareTo("ENABLE") == 0)
   {
       mpSipUserAgentTask->startMessageLog();
   }
   mpSipUserAgentTask->allowExtension(SIP_CALL_CONTROL_EXTENSION);
   mpSipUserAgentTask->allowExtension("sip-cc-01");
   mpSipUserAgentTask->allowExtension(SIP_SESSION_TIMER_EXTENSION);
   mpSipUserAgentTask->allowExtension(SIP_REPLACES_EXTENSION);

   // Initialize the Temporary SIP NOTIFY message consumer
   UtlString checkSyncPolicy ;
   config->get("PHONESET_CHECK_SYNC", checkSyncPolicy) ;
   if (checkSyncPolicy.isNull())
   {
      config->get("PHONESET_CHECK-SYNC", checkSyncPolicy) ;
   }
   mpNotifyStateTask = new SipNotifyStateTask(checkSyncPolicy, mpSipUserAgentTask);

   // Setup the default NOTIFY call backs
   mpNotifyStateTask->setRebootFunction(VoicemailNotifyStateListener::doReboot);
   mpNotifyStateTask->setBinaryMessageWaitingFunction(
                       VoicemailNotifyStateListener::doBinaryMessageWaiting);
   mpNotifyStateTask->setDetailMessageWaitingFunction(
                       VoicemailNotifyStateListener::doDetailMessageWaiting);

   // Add the NOTIFY catcher task to the list of
   mpSipUserAgentTask->addMessageObserver(
       *(mpNotifyStateTask->getMessageQueue()), "NOTIFY",
       TRUE,   // want to get requests
       FALSE,  // No responses thank you
       TRUE,   // Incoming messages
       FALSE); // Don't want to see out going messages
   mpNotifyStateTask->start();
}

// init the MGCP stack
void Pinger::initMgcpStack(OsConfigDb* config)
{
#if defined(ENABLE_MGCP)

   UtlString MgcpAgentHostname;
   UtlString MgcpPort;
   UtlString MgcpKeyboardHackPort;

   // initialize the MGCP gateway
   config->get("MGCP_AGENT_HOSTNAME", MgcpAgentHostname);
   config->get("MGCP_PORT", MgcpPort);
   config->get("MGCP_KEYBOARD_HACK_PORT", MgcpKeyboardHackPort);

   // This "Hack Keyboard Port" is a TCP port used for the MGCP thread
   // handling the phone set hardware interface to talk to the
   // main processing loop of the MGCP stack. It's icky, but it
   // works :)

   if (MgcpKeyboardHackPort.length() == 0)
   {
      // If there isn't a port specified, then set a default
      MgcpKeyboardHackPort = "3333";
   }

   mpMgcpStackTask =
      new PtMGCP(MgcpAgentHostname, MgcpPort, MgcpKeyboardHackPort);
#endif
}

// init the call processing subsystem
void Pinger::initCallManager(OsConfigDb* config)
{
   int noAnswerTimeoutSeconds;
   UtlString outboundLine;
   int rtpPortStart;
   int rtpPortEnd;
   UtlString localAddress;
   UtlString publicAddress;
   UtlString busyBehaviorString;
   UtlString availableBehaviorString;
   int busyBehavior;
   int availableBehavior;
   UtlString phonesetOutgoingCallProtocolString;
   CallManager::CallTypes phonesetOutgoingCallProtocolType;
   int numDialPlanDigits;
   int sipSessionReinviteTimer = 0;
   UtlString holdTypeString;
   int holdType;
   UtlString earlyMediaFor180;
   UtlString isRequireUserId;
   int offeringExpireSeconds;
   int ringingExpireSeconds;

   //by default early medai is enabled for 180 response code
   UtlBoolean isEarlyMediaFor180Enabled  = TRUE;
   config->get("SIP_180_ALLOW_EARLY_MEDIA", earlyMediaFor180);
   if ((earlyMediaFor180.compareTo("DISABLED") == 0) ||
       (earlyMediaFor180.compareTo("DISABLE") == 0))
   {
      isEarlyMediaFor180Enabled = FALSE;
   }
   //by default isRequredUserIdMatch is FALSE to that you can call the phone by just the IP address
   UtlBoolean isRequredUserIdMatch  = FALSE;
   config->get("SIP_REQUIRE_USERID_MATCH", isRequireUserId);
   if ((isRequireUserId.compareTo("ENABLED") == 0) ||
       (isRequireUserId.compareTo("ENABLE") == 0))
   {
      isRequredUserIdMatch = TRUE;
   }

   config->get("PHONESET_OFFERING_EXPIRE", offeringExpireSeconds);
   if(offeringExpireSeconds < 1) offeringExpireSeconds = 3;
   osPrintf("Offering expiration time is %d seconds\n", offeringExpireSeconds);

   config->get("PHONESET_RINGING_EXPIRE", ringingExpireSeconds);
   if(ringingExpireSeconds < 1 || ringingExpireSeconds > CP_MAXIMUM_RINGING_EXPIRE_SECONDS) ringingExpireSeconds = CP_MAXIMUM_RINGING_EXPIRE_SECONDS; /*180*/
   osPrintf("Ringing expiration time is %d seconds\n", ringingExpireSeconds);

   config->get("PHONESET_RTP_PORT_START", rtpPortStart);
   if(rtpPortStart < 1) rtpPortStart = 8766;
   config->get("PHONESET_RTP_PORT_END", rtpPortEnd);
   config->get("PHONESET_EXTERNAL_IP_ADDRESS", publicAddress);
   OsSocket::getHostIp(&localAddress);
   osPrintf("---- Pinger::initCallManager: %s\n", localAddress.data());
   if(publicAddress.isNull()) publicAddress.append(localAddress);
   config->get("SIP_SESSION_REINVITE_TIMER", sipSessionReinviteTimer);
   config->get("PHONESET_NO_ANSWER_TIMEOUT", noAnswerTimeoutSeconds);
   if(noAnswerTimeoutSeconds < 1) noAnswerTimeoutSeconds = -1;
   //config->get("PHONESET_EXTENSION", extension);
   config->get("PHONESET_DIALPLAN_LENGTH", numDialPlanDigits);
   if(numDialPlanDigits < 1) numDialPlanDigits = 12;
   config->get("PHONESET_BUSY_BEHAVIOR", busyBehaviorString);
   config->get("PHONESET_AVAILABLE_BEHAVIOR", availableBehaviorString);
   busyBehaviorString.toUpper();
   osPrintf("PHONESET_BUSY_BEHAVIOR: %s\n", busyBehaviorString.data());
   availableBehaviorString.toUpper();
   if(availableBehaviorString.compareTo("FORWARD") == 0)
   {
       availableBehavior = Connection::FORWARD_UNCONDITIONAL;
   }
   else if(availableBehaviorString.compareTo("FORWARD_ON_NO_ANSWER") == 0)
   {
       availableBehavior = Connection::FORWARD_ON_NO_ANSWER;
   }
   else
   {
       availableBehavior = Connection::RING;
   }

   if(busyBehaviorString.compareTo("QUEUE") == 0)
   {
       busyBehavior = Connection::QUEUE_ALERT;
       osPrintf("Enabling queuing of calls\n");
   }
   else if(busyBehaviorString.compareTo("FORWARD") == 0)
   {
       busyBehavior = Connection::FORWARD_ON_BUSY;
       osPrintf("Enabling Forward On Busy\n");
   }
   else
   {
       busyBehavior = Connection::BUSY;
       osPrintf("Enabling Busy\n");
   }

   // Get the outbound line
   mpLineMgrTask->getDefaultOutboundLine(outboundLine);

   // Determine which protocol to use as a default, MGCP or SIP

   phonesetOutgoingCallProtocolString = "SIP";

   if (phonesetOutgoingCallProtocolString.compareTo("SIP") == 0)
   {
       osPrintf("SIP is the outgoing protocol\n");
           phonesetOutgoingCallProtocolType = CallManager::SIP_CALL;
   }
   else
   {
       osPrintf("MGCP is the outgoing protocol\n");
           phonesetOutgoingCallProtocolType = CallManager::MGCP_CALL;
   }

   // Get default hold type handling (i.e. NEAR/FAR SIDE
   config->get("PHONESET_HOLD_TYPE", holdTypeString);
   holdTypeString.toUpper();
   if(holdTypeString.compareTo("FAR_END") == 0)
   {
       holdType = CallManager::FAR_END_HOLD;
   }
   else
   {
       holdType = CallManager::NEAR_END_HOLD;
   }

   UtlString locale;
   // get the call progress tones locale setting
   // (represented using an ISO-3166 two letter country code)
   mConfigDb.get("PHONESET_LOCALE_COUNTRY", locale);

   // get the layer 2 (802.1p priority) setting for expedited forwarding
   UtlString prioStr;
   int expedited8021pPrio;
   mConfigDb.get("PHONESET_EXPEDITED_8021P_PRIORITY", prioStr);
   if ((OsUtil::convertUtlStringToInt(prioStr, expedited8021pPrio) != OS_SUCCESS) ||
       expedited8021pPrio < 0 || expedited8021pPrio > 7)
   {
       expedited8021pPrio = QOS_LAYER2_LOW_DELAY_PRIO;
   }
   
   // get the layer 3 (IP ToS) setting for expedited forwarding
   UtlString ipTosStr;
   int expeditedIpTos;
   mConfigDb.get("PHONESET_EXPEDITED_IP_TOS", ipTosStr);
   if ((OsUtil::convertUtlStringToInt(ipTosStr, expeditedIpTos) != OS_SUCCESS)  ||
       expeditedIpTos < 0 || expeditedIpTos > 255)
   {
       expeditedIpTos = QOS_LAYER3_LOW_DELAY_IP_TOS;
   }

   mpCallMgrTask = new CallManager(isRequredUserIdMatch,
                                   mpLineMgrTask,
                                   isEarlyMediaFor180Enabled,
                                   mpCodecFactory,
                                   rtpPortStart,
                                   rtpPortEnd,
                                   localAddress.data(),
                                   publicAddress.data(),
                                   mpSipUserAgentTask,
                                   sipSessionReinviteTimer,
                                   NULL,
                                   outboundLine.data(),
                                   availableBehavior,
                                   NULL,
                                   noAnswerTimeoutSeconds,
                                   NULL,
                                   busyBehavior,
                                   NULL,
                                   &mConfigDb,
                                   phonesetOutgoingCallProtocolType,
                                   numDialPlanDigits,
                                   holdType,
                                   offeringExpireSeconds*1000,// milliseconds offering delay
                                   locale.data(),
                                   ringingExpireSeconds,
                                   expeditedIpTos,
                                   10,
                                   NULL);

   // STUN parameters
   UtlString stunServer ;
   int stunRefreshPeriod ;
   config->get("SIP_STUN_SERVER", stunServer) ;
   if (stunServer.length())
   {
       UtlString strRefreshPeriod ;
       if ((config->get("SIP_STUN_REFRESH_PERIOD", strRefreshPeriod) == OS_NOT_FOUND) ||
               (strRefreshPeriod.length() == 0))
       {
           stunRefreshPeriod = 28 ;
       }
       else
       {
            config->get("SIP_STUN_REFRESH_PERIOD", stunRefreshPeriod) ;
            if (stunRefreshPeriod < 0)
                stunRefreshPeriod = 0 ;
       }
      
       mpCallMgrTask->enableStun(stunServer.data(), stunRefreshPeriod) ;
   }

   // create the call manager task
   mpCallMgrTask->start();         // start the task running

   CpMediaInterfaceFactoryInterface* pInterface = 
         mpCallMgrTask->getMediaInterfaceFactory()->getFactoryImplementation() ;

   if (pInterface)
   {
      // Set Microphone bindings
      pInterface->setMicrophoneDevice(mMicDevice) ;
      pInterface->setSpeakerDevice(mCallDevice) ;
      // DmaTask::setRingDevice(mRingDevice) ;

      // Enable/Disable AEC
      UtlString enableAEC ;
      config->get("PHONESET_AEC", enableAEC) ;
      enableAEC.toUpper() ;
      if (enableAEC == "ENABLE")
      {
         pInterface->enableAEC(true) ;
      }
      else
      {
         pInterface->enableAEC(false) ;
      }
   }


#if defined(_WIN32) || defined(__pingtel_on_posix__)
#ifdef _NONJAVA
      // register for phone set events
      mpPhoneTask->addListener(mpCallMgrTask);
#endif
#endif
}

// init the tao subsystem
void Pinger::initTaoServer(OsConfigDb* config)
{
   mpTaoServerTask = TaoServerTask::getTaoServerTask(mpCallMgrTask);
}

/* ============================ FUNCTIONS ================================= */

// Functions easily callable from the VxWorks shell to start and stop the task

extern "C" {extern void pingerStart(int startType);}
extern "C" {extern void pingerStartOnly(int startType);}
extern "C" {extern void pingerIsStarting(int flag);}

#define resetPriorities(delta)
#define pingerDosFsTweaks()

void pingerStartOnly(int startType)
{
   Pinger::getPingerTask() ;
}

static int pingerStarted = 0;

void pingerStart(int startType)
{
   if (pingerStarted) return;
   pingerStarted = 1;

   pingerStartOnly(startType);
}

void pingerStop(void)
{
   OsNameDb* pNameDb = OsNameDb::getNameDb();
   Pinger* pPinger   = Pinger::getPingerTask();

   pPinger->requestShutdown();
   delete pPinger;

   delete pNameDb;
}

// Until we get the hardware keypad working, we use the following helper
// function to send digits to the phone set
void di(char* str, int toneMs)
{
   int          btnId;
   int          btnIndex;
   PsMsg        msg(PsMsg::BUTTON_DOWN, NULL, 0, 0);
   PsPhoneTask* pPhoneTask;
   OsStatus     res;

   if (toneMs < 50) toneMs = 50;
   pPhoneTask = PsPhoneTask::getPhoneTask();

   while (*str)
   {
      btnId = *str++;
      switch (btnId)
      {
      case '0': btnIndex = 0;  break;
      case '1': btnIndex = 1;  break;
      case '2': btnIndex = 2;  break;
      case '3': btnIndex = 3;  break;
      case '4': btnIndex = 4;  break;
      case '5': btnIndex = 5;  break;
      case '6': btnIndex = 6;  break;
      case '7': btnIndex = 7;  break;
      case '8': btnIndex = 8;  break;
      case '9': btnIndex = 9;  break;
      case '*': btnIndex = 10; break;
      case '#': btnIndex = 11; break;
      case '+': btnIndex = 12; break;
      case '-': btnIndex = 13; break;
      case '<': btnIndex = 14; break;
      case '>': btnIndex = 15; break;
      //case 'h': btnIndex = 16; break;
      default:  btnIndex = -1; break;
      }

      if (btnIndex < 0)
         continue;

      msg.setMsgSubType(PsMsg::BUTTON_DOWN);   // signal button down
      msg.setParam1(btnIndex);
      msg.setParam2(btnId);
      res = pPhoneTask->postMessage(msg);
      assert(res == OS_SUCCESS);

      OsTask::delay(toneMs);                   // wait at least 50 milliseconds

      msg.setMsgSubType(PsMsg::BUTTON_UP);     // signal button up
      res = pPhoneTask->postMessage(msg);
      assert(res == OS_SUCCESS);

      OsTask::delay(50);                       // wait 50 milliseconds
   }
}

int OffHook() {

   PsPhoneTask* pPhoneTask;
   PsHookswTask* pHookswTask;

         pPhoneTask = PsPhoneTask::getPhoneTask();
         pHookswTask = PsHookswTask::getHookswTask();
         pPhoneTask->postEvent(PsMsg::HOOKSW_STATE,       // msg type
                             pHookswTask,                 // source
                             PsHookswTask::OFF_HOOK,      // hooksw state
                             0);                          // not used
        return 0;
}

int OnHook() {

   PsPhoneTask* pPhoneTask;
   PsHookswTask* pHookswTask;

         pPhoneTask = PsPhoneTask::getPhoneTask();
         pHookswTask = PsHookswTask::getHookswTask();
         pPhoneTask->postEvent(PsMsg::HOOKSW_STATE,    // msg type
                             pHookswTask,              // source
                             PsHookswTask::ON_HOOK,    // hooksw state
                             0);                       // not used
        return 0;
}

void PrintCalls(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    Pinger* pPinger   = Pinger::getPingerTask();
    if(pPinger)
    {
        pPinger->printCalls();
    }
}

void PrintSipUa(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    Pinger* pPinger   = Pinger::getPingerTask();
    if(pPinger)
    {
        pPinger->printSipUa();
    }
}

void hold()
{

   PsMsg        msg(PsMsg::BUTTON_DOWN, NULL, 0, 0);
   PsPhoneTask* pPhoneTask = PsPhoneTask::getPhoneTask();
   PsButtonTask* pButtonTask = PsButtonTask::getButtonTask();
   OsStatus     res;
   int btnIndex = pButtonTask->getButtonIndex(FKEY_HOLD);

   msg.setMsgSubType(PsMsg::BUTTON_DOWN);   // signal button down
   msg.setParam1(btnIndex);
   msg.setParam2(FKEY_HOLD);
   res = pPhoneTask->postMessage(msg);
   assert(res == OS_SUCCESS);

   msg.setMsgSubType(PsMsg::BUTTON_UP);     // signal button up
   res = pPhoneTask->postMessage(msg);
   assert(res == OS_SUCCESS);
}

void offhold(const char* callId)
{
    Pinger* pPinger   = Pinger::getPingerTask();
    if(pPinger)
    {
        pPinger->unhold(callId);
    }
}

void startSipLog()
{
    Pinger* pPinger   = Pinger::getPingerTask();
    if(pPinger)
    {
        pPinger->startSipUserAgentLog();
    }
}

void stopSipLog()
{
    Pinger* pPinger   = Pinger::getPingerTask();
    if(pPinger)
    {
        pPinger->stopSipUserAgentLog();
    }
}

void dumpSipLog()
{
    Pinger* pPinger   = Pinger::getPingerTask();
    if(pPinger)
    {
        pPinger->dumpSipUserAgentLog();
    }
}

void dialUrl(const char* url)
{

   char callId[128];

   JXAPI_SimpleDial(url, callId, sizeof(callId));

}

void outGoingCallType(int callType)
{
    Pinger* pPinger   = Pinger::getPingerTask();
    if(pPinger)
    {
        pPinger->outGoingCallType(callType);
    }
}

UtlString httpCreateRow(const char* row, const char* value)
{
    UtlString tempString;

    tempString.append("<TR>\n<TD width=\"9\"><IMG SRC=\"http://my.pingtel.com"
       "/images/spacer.gif\" WIDTH=\"9\" HEIGHT=\"5\" BORDER=\"0\" ALT=\"\">"
       "</TD>\n");
    tempString.append("<TD class=\"maincontent\" width=\"130\" align=left"
       " valign=\"top\" nowrap>");
    tempString.append(row);
    tempString.append("</TD>\n<TD class=\"maincontent\" width=\"150\""
       " align=left valign=\"bottom\">");
    tempString.append(value);
    tempString.append("</TD>\n</TR>\n");

    return tempString;
}

void Pinger::processPostFile(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
   JXAPI_setUploadingFile(TRUE);
   UtlString status;
   int res = HttpServer::doPostFile(requestContext, request, response, status);
   if (!res) 
   {
      JXAPI_ClearStatus();
      JXAPI_MessageBox(MBT_ERROR, "Error", "Upload failed. Check the filename"
         " and try again.\nThis may be a possible out of memory condition."
         "  Restart and try again.", false) ;
   }
   JXAPI_setUploadingFile(FALSE);
}

void Pinger::processSipPage(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

    SipUserAgent* sipUserAgent = (Pinger::getPingerTask())->mpSipUserAgentTask;
    UtlString sipLog;
    UtlString toggleSipLogLabel;
    if(sipUserAgent->isMessageLoggingEnabled())
    {
        toggleSipLogLabel.append("Disable SIP Logging");
    }
    else
    {
        toggleSipLogLabel.append("Enable SIP Logging");
    }

    UtlString messageBody("<TITLE>SIP Control Page</TITLE>\n");
    messageBody.append(HTTP_BODY_BEGIN);

    // Toggle SIP logging on/off button
    messageBody.append("<FORM ACTION=\"/cgi/toggleSipLog.cgi\" METHOD=POST>\n");
    messageBody.append("<INPUT TYPE=SUBMIT NAME=TOGGLE_SIP_LOG VALUE=\"");
    messageBody.append(toggleSipLogLabel.data());
    messageBody.append("\">\n</FORM>\n");

    // Reload SIP Log
    messageBody.append("<BR>\n");
    messageBody.append("<FORM ACTION=\"/cgi/sip.cgi\" METHOD=POST>\n");
    messageBody.append("<INPUT TYPE=SUBMIT NAME=TOGGLE_SIP_LOG VALUE=\"Reload"
       " SIP Log\">\n</FORM>\n");

    // Insert the SIP log
    sipUserAgent->getMessageLog(sipLog);
    // Escape "<" so that they are not interpreted as HTML tags
    int lessThanIndex = sipLog.index('<');
    while(lessThanIndex >= 0)
    {
        sipLog.remove(lessThanIndex, 1);
        sipLog.insert(lessThanIndex, "&lt;");

        lessThanIndex = sipLog.index('<');
    }
    messageBody.append("<HR>\n<H3>SIP Message Log</H3>\n<PRE>\n");
    messageBody.append(sipLog);
    messageBody.append("</PRE>\n<HR>\n");

    messageBody.append(HTTP_BODY_END);

    // Added by Harippriya on 07/01/2002
    // Made the SIP Logs available only to the admin user.

    // Retrieve the username of the user who is logged
    UtlString userName ;
    requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER, userName);

    // Check if the user is "admin".
    if (userName.compareTo("admin")!=0) {
        const char * text = "<HTML><BODY>User Not Authorized<br><br>Must be user \"admin\" to access this page.</BODY></HTML>\n" ;
        HttpServer::processUserNotAuthorized(requestContext,request,response, text) ;
    } else {

        // Build the response message
        HttpBody* body = new HttpBody(messageBody.data(),
            messageBody.length(), CONTENT_TYPE_TEXT_HTML);
        response = new HttpMessage();
        response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
            HTTP_OK_CODE, HTTP_OK_TEXT);
        response->setBody(body);
        response->setContentType(CONTENT_TYPE_TEXT_HTML);
        response->setContentLength(messageBody.length());
    }
}


void Pinger::processToggleSipLog(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    SipUserAgent* sipUserAgent = (Pinger::getPingerTask())->mpSipUserAgentTask;
    if(sipUserAgent->isMessageLoggingEnabled())
    {
        sipUserAgent->stopMessageLog();
    }
    else
    {
        sipUserAgent->clearMessageLog();
        sipUserAgent->startMessageLog();
    }

    // Relocate to the sip control page
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);
    response->setLocationField("/cgi/sip.cgi");
}


void Pinger::processAnswerCgi(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    UtlString strCallID;
    requestContext.getCgiVariable("CALL_ID", strCallID);
    if( ! strCallID.isNull() ){
         JXAPI_Answer( strCallID.data() );
    }

    UtlString messageBody("<h2>processAnswerCgi was successfully called.</h2>");
    HttpBody* body = new HttpBody(messageBody.data(),
        messageBody.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    response->setContentLength(messageBody.length());


}


void Pinger::processTransferCgi(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    UtlString dialUrl;
    requestContext.getCgiVariable("DIAL_URL", dialUrl);
    UtlString strCallID;
    requestContext.getCgiVariable("CALL_ID", strCallID);
    if( ! strCallID.isNull() ){
      if (parseDialString(dialUrl) >=0 ) {
         JXAPI_Transfer( dialUrl.data(), strCallID.data());
      }
    }

    UtlString messageBody("<h2>processTransferCgi was successfully called.</h2>");
    HttpBody* body = new HttpBody(messageBody.data(),
        messageBody.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    response->setContentLength(messageBody.length());


}

void Pinger::processHoldCgi(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    UtlString strCallID;
    requestContext.getCgiVariable("CALL_ID", strCallID);
    bool    bEnable = true;
    if( ! strCallID.isNull() ){
        JXAPI_Hold( strCallID.data(), bEnable);
    }

    UtlString messageBody("<h2>processHoldCgi was successfully called.</h2>");
    HttpBody* body = new HttpBody(messageBody.data(),
        messageBody.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    response->setContentLength(messageBody.length());
}


void Pinger::processOffHoldCgi(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    UtlString strCallID;
    requestContext.getCgiVariable("CALL_ID", strCallID);
    bool    bEnable = false;
    if( ! strCallID.isNull() ){
        JXAPI_Hold( strCallID.data(), bEnable);
    }

    UtlString messageBody("<h2>processOffHoldCgi was successfully called.</h2>");
    HttpBody* body = new HttpBody(messageBody.data(),
        messageBody.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    response->setContentLength(messageBody.length());


}

void Pinger::processDialCgi(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
   UtlString dialUrl;
   requestContext.getCgiVariable("DIAL_URL", dialUrl);
   UtlString strCallID;
   requestContext.getCgiVariable("CALL_ID", strCallID);

   response = new HttpMessage();

   if( ! strCallID.isNull() ){
        if (parseDialString(dialUrl) >=0 ) {
            JXAPI_AddParty( dialUrl.data(), strCallID.data() );
        }

        UtlString messageBody("<h2>processDialCgi was successfully called.</h2>");
        HttpBody* body = new HttpBody(messageBody.data(),
        messageBody.length(), CONTENT_TYPE_TEXT_HTML);
        response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
        response->setBody(body);
        response->setContentType(CONTENT_TYPE_TEXT_HTML);
        response->setContentLength(messageBody.length());

   }else{
       osPrintf("Pinger::processDialCgi \nDial URL digits: \"%s\"\n", dialUrl.data());

       // Valid urls are all digits or sip:...
       if (parseDialString(dialUrl) >=0 ) {

          // Do the call setup stuff

          char callId[128];

          JXAPI_SimpleDial(dialUrl.data(), callId, sizeof(callId)) ;

            // Redirect to the call status cgi
            response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
            HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

          UtlString location("/cgi/homepage.cgi");
          if (callId[0])
          {
             location.append("?MESSAGE=Dialing&CALL_ID=");
             UtlString callIdEscape(dialUrl.data()) ;
             HttpMessage::escape(callIdEscape) ;
             location.append(callIdEscape);
          }
          response->setLocationField(location);
       }

       // Dialing failed
       else
       {
          // Build the response message
          response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
              HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

          UtlString location("/cgi/homepage.cgi");
          location.append("?MESSAGE=");
          location.append("Dial+failed.++Invalid+address.");
          response->setLocationField(location);
       }
   }
}

// Utility function to determine if a dial String is
// valid, and whether it is a URL or digits.
// Returns:
// 1 - all digits
// 2 - URL
// -1 - invalid string

int Pinger::parseDialString(UtlString dialUrl)
{
   osPrintf("parseDialString entered = %s\n", dialUrl.data()) ;

   RegEx digitsExpression("^\\d+$");

   if (dialUrl.isNull()) {
      return -1 ;
   }
   else if (digitsExpression.Search(dialUrl.data())) {
      osPrintf("Dial URL digits: \"%s\"\n", dialUrl.data());
      return 1;
   }
   else {
      RegEx americanNumberExpression("^\\((\\d{3})\\) ?(\\d{3})-(\\d{4})$");

      if(americanNumberExpression.Search(dialUrl.data())) {
         UtlString allDigits;
         allDigits.append(americanNumberExpression.Match(1));
         allDigits.append(americanNumberExpression.Match(2));
         allDigits.append(americanNumberExpression.Match(3));
         osPrintf("Dial URL digits: \"%s\"\n", allDigits.data());
         return 1 ;
      } else {
         //:TODO: 
         //  This method should use a better way of determining a valid SIP URL.
         //  Currently just assume that this is a SIP URL, if not null
         return 2 ;
      }
   }
}

void Pinger::processGetCallIdsCgi(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response)
{
   int numCalls = 0;
   UtlString callIdArray[MAX_CALLS];

   Pinger* pinger = Pinger::getPingerTask();
   if(pinger && pinger->mpCallMgrTask)
   {
      pinger->mpCallMgrTask->getCalls(MAX_CALLS, numCalls, callIdArray);
   }

   response = new HttpMessage();
   UtlString messageBody;
   messageBody.remove(0);
   for (int i = 0; i < numCalls; i++)
   {
      messageBody.append(callIdArray[i]);
      if (numCalls > 1)
         messageBody.append(",");
   }
   // Build the response message
   HttpBody* body = new HttpBody(messageBody.data(),
      messageBody.length(), CONTENT_TYPE_TEXT_PLAIN);
   response->setBody(body);
   response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
   response->setContentLength(messageBody.length());
}

void Pinger::processDropCgi(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response)
{
   const char* badCallId = "call-id1234@5.5.5.5.5";
   UtlString callId;
   requestContext.getCgiVariable("CALL_ID", callId);

   UtlString dialUrl;
   requestContext.getCgiVariable("DIAL_URL", dialUrl);



   //UtlString html("<H3>Drop Call</H3>\n");
   UtlString message;

   osPrintf("CALL_ID: \"%s\"\n", callId.data());

   // Error bad callId cannot drop call
   if(callId.isNull() || callId.compareTo(badCallId) == 0)
   {
      message.append("Call+does+not+exist.");
   }

   // Good call id drop the call
   else
   {
      if (parseDialString(dialUrl) >=0 ) {
        JXAPI_RemoveParty( dialUrl.data(), callId.data());
        message.append("party+removed.");
      }else{
          JXAPI_DropCall(callId.data()) ;

          // create feed back html
          message.append("Call+hung+up.");
      }
   }

   // Build the response message
   response = new HttpMessage();
   response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
      HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);

   UtlString location("/cgi/homepage.cgi");
   location.append("?MESSAGE=");
   location.append(message);
   response->setLocationField(location);
}

void Pinger::processGetDeployConfigFile(
                      const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

   UtlString phoneId;
   requestContext.getCgiVariable("LOGICAL_ID", phoneId);
   UtlString phonePwd;
   requestContext.getCgiVariable("LOGICAL_PWD", phonePwd);
   UtlString userId;
   requestContext.getCgiVariable("PHYSICAL_ID", userId);
   UtlString adminDomain;
   requestContext.getCgiVariable("ADMIN_DOMAIN", adminDomain);
   UtlString version;
   requestContext.getCgiVariable("KERNEL_VERSION", version);

   response = new HttpMessage();
   UtlString messageBody;
   messageBody.remove(0);
   if(!phoneId.isNull() && !phonePwd.isNull() &&
      !userId.isNull() && !adminDomain.isNull() && !version.isNull())
   {
      UtlString configFileName;

      configFileName.insert(0, CONFIGDB_NAME_IN_FLASH);

      FILE* fp = fopen(configFileName.data(), "r");

      if(!fp)
      {
        OsSocket::getHostIp(&configFileName);
        osPrintf("---- Pinger::processGetDeployConfigFile: %s\n",
           configFileName.data());
        configFileName.insert(0, CONFIGDB_PREFIX_ON_SERVER);
        fp = fopen(configFileName.data(), "r");
      }
      if(fp)
      {
         char fileLine[MAX_FILELINE_SIZE + 1];

         while (!feof(fp))
         {
            if(fgets(fileLine, MAX_FILELINE_SIZE, fp))
            {
               messageBody.append(fileLine);
            }
         }
         fclose(fp);
      }
      response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
         HTTP_OK_CODE, HTTP_OK_TEXT);
   }
   else
   {
      response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
         HTTP_UNAUTHORIZED_CODE, HTTP_UNAUTHORIZED_TEXT);
   }

   // Build the response message
   HttpBody* body = new HttpBody(messageBody.data(),
                            messageBody.length(), CONTENT_TYPE_TEXT_PLAIN);
   response->setBody(body);
   response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
   response->setContentLength(messageBody.length());
}

void Pinger::processGetLogicalId(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{

   UtlString phoneId;
   requestContext.getCgiVariable("ADMIN_ID", phoneId);
   UtlString phonePwd;
   requestContext.getCgiVariable("PHYSICAL_PWD", phonePwd);
   UtlString userId;
   requestContext.getCgiVariable("PHYSICAL_ID", userId);
   UtlString adminDomain;
   requestContext.getCgiVariable("ADMIN_DOMAIN", adminDomain);
   UtlString adminPwd;
   requestContext.getCgiVariable("ADMIN_PWD", adminPwd);
   UtlString version;
   requestContext.getCgiVariable("KERNEL_VERSION", version);

   osPrintf("getLogicalId: ADMIN_DOMAIN = %s, ADMIN_ID = %s, ADMIN_PWD = %s,"
      " PHYSICAL_ID = %s, PHYSICAL_PWD = %s, KERNEL_VERSION = %s\n",
      adminDomain.data(), phoneId.data(), adminPwd.data(),
      userId.data(), phonePwd.data(), version.data());

   response = new HttpMessage();
   UtlString messageBody;
   messageBody.remove(0);
   if(!phoneId.isNull() && !phonePwd.isNull() &&
      !userId.isNull() && !adminDomain.isNull() && !version.isNull())
   {
      UtlString configFileName;

      configFileName.insert(0, CONFIGDB_NAME_IN_FLASH);

      FILE* fp = fopen(configFileName.data(), "r");

      if(!fp)
      {
        OsSocket::getHostIp(&configFileName);
        osPrintf("---- Pinger::processGetLogicalId: %s\n",
           configFileName.data());
        configFileName.insert(0, CONFIGDB_PREFIX_ON_SERVER);
        fp = fopen(configFileName.data(), "r");
      }
      if(fp)
      {
         char fileLine[MAX_FILELINE_SIZE + 1];

         while (!feof(fp))
         {
            if(fgets(fileLine, MAX_FILELINE_SIZE, fp))
            {
               messageBody.append(fileLine);
            }
         }
         fclose(fp);
      }
      response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
         HTTP_OK_CODE, HTTP_OK_TEXT);
   }
   else
   {
      response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
         HTTP_UNAUTHORIZED_CODE, HTTP_UNAUTHORIZED_TEXT);
   }

   // Build the response message
   HttpBody* body = new HttpBody(messageBody.data(),
      messageBody.length(), CONTENT_TYPE_TEXT_PLAIN);
   response->setBody(body);
   response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
   response->setContentLength(messageBody.length());
}

void Pinger::processCallStateInfo(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    UtlString callStateLog;
    UtlString toggleCallStateLogLabel;
    Pinger* pinger = Pinger::getPingerTask();
    if (pinger && pinger->mpCallMgrTask &&
           pinger->mpCallMgrTask->isCallStateLoggingEnabled())
    {
        toggleCallStateLogLabel.append("Disable Call State Logging");
    }
    else
    {
        toggleCallStateLogLabel.append("Enable Call State Logging");
    }

    UtlString messageBody("<TITLE>Call State Control Page</TITLE>\n");
    messageBody.append(HTTP_BODY_BEGIN);

    // Toggle SIP logging on/off button
    messageBody.append(
       "<FORM ACTION=\"/cgi/toggleStateLog.cgi\" METHOD=POST>\n");
    messageBody.append(
       "<INPUT TYPE=SUBMIT NAME=TOGGLE_CALL_STATE_LOG VALUE=\"");
    messageBody.append(toggleCallStateLogLabel.data());
    messageBody.append("\">\n</FORM>\n");

    // Reload SIP Log
    messageBody.append("<BR>\n");
    messageBody.append("<FORM ACTION=\"/cgi/stateInfo.cgi\" METHOD=POST>\n");
    messageBody.append("<INPUT TYPE=SUBMIT NAME=TOGGLE_CALL_STATE_LOG"
       " VALUE=\"Reload Call State Log\">\n</FORM>\n");

    // Insert the SIP log
    pinger->mpCallMgrTask->getCallStateLog(callStateLog);
    // Escape "<" so that they are not interpreted as HTML tags
    int lessThanIndex = callStateLog.index('<');
    while(lessThanIndex >= 0)
    {
        callStateLog.remove(lessThanIndex, 1);
        callStateLog.insert(lessThanIndex, "&lt;");

        lessThanIndex = callStateLog.index('<');
    }
    messageBody.append("<HR>\n<H3>Call State Message Log</H3>\n<PRE>\n");
    messageBody.append(callStateLog);
    messageBody.append("</PRE>\n<HR>\n");

    messageBody.append(HTTP_BODY_END);

    // Build the response message
    HttpBody* body = new HttpBody(messageBody.data(),
        messageBody.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    response->setContentLength(messageBody.length());
}


void Pinger::processToggleCallStateLog(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    Pinger* pinger = Pinger::getPingerTask();
    if (pinger && pinger->mpCallMgrTask &&
              pinger->mpCallMgrTask->isCallStateLoggingEnabled())
    {
         pinger->mpCallMgrTask->stopCallStateLog();
    }
    else
    {
         pinger->mpCallMgrTask->clearCallStateLog();
         pinger->mpCallMgrTask->startCallStateLog();
    }

    // Relocate to the sip control page
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_MOVED_TEMPORARILY_CODE, HTTP_MOVED_TEMPORARILY_TEXT);
    response->setLocationField("/cgi/stateInfo.cgi");
}

#ifdef DEBUG_VERSIONCHECK
void Pinger::processVersionCheckForm(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    UtlString messageBody(HTTP_BODY_BEGIN);
    messageBody.append("<TITLE>Version Check</TITLE>\n");

    messageBody.append(
       "<FORM ACTION=\"/cgi/dovc.cgi\" METHOD=POST>\n");

    messageBody.append("<p><b>Version Check</b>\n");

    messageBody.append("<p>Enter the URL indicating the latest file:<br>\n");
    messageBody.append("<INPUT TYPE=TEXT NAME=URL SIZE=58"
       " VALUE=\"http://updates.pingtel.com/VersionCheck/latest\">\n");

    messageBody.append("<p><INPUT TYPE=SUBMIT VALUE=\"Check Now\">\n");
    messageBody.append("<INPUT TYPE=RESET VALUE=Reset>\n</FORM>\n");

    messageBody.append(HTTP_BODY_END);

    // Build the response message
    HttpBody* body = new HttpBody(messageBody.data(),
        messageBody.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    response->setContentLength(messageBody.length());
}

void Pinger::processDoVersionCheck(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
   UtlString latestURL;
   OsStatus rc = OS_VERSIONCHECK_NO_LATESTURL;  // success;

   requestContext.getCgiVariable("URL", latestURL);

   if (latestURL.length() == 0)
   {
      OsConfigDb config;
      UtlString configFileName;
      configFileName = CONFIGDB_NAME_IN_FLASH;

      if(config.loadFromFile((char *)configFileName.data()))
      {
        osPrintf("Found user config file: %s\n", configFileName.data());
      }
      config.get("PHONESET_VERSIONCHECK_URL", latestURL);
   }

   if (latestURL.length() != 0)
   {
      VersionCatalog vc;
      UtlString runningv;
      UtlString catalogUrl;
      UtlString platform;
      UtlString tgtv;
      UtlBoolean confirmUpgrade;

      PingerInfo::getVersion(runningv);
      PingerInfo::getPlatformType(platform);
      //platform = "xpressa_strongarm_vxworks";
      rc = vc.findLatestVersion(latestURL.data(),
                                platform.data(),
                                tgtv,
                                catalogUrl,
                                confirmUpgrade);
      if (OS_SUCCESS == rc)
      {
         UtlString upver;
         UtlString url;
         rc = vc.findUpdateScript(catalogUrl,
                     platform.data(),
                     runningv.data(),
                     tgtv.data(),
                     upver,
                     url);
         if (OS_SUCCESS == rc)
         {
            int reboot = 0;
            rc = vc.executeUpdateScript(url.data(), reboot);
         }
      }
   }

  // Build the response message
   UtlString messageBody(HTTP_BODY_BEGIN);
   messageBody.append("<TITLE>Version Check Complete</TITLE>\r\n");
   if (rc == OS_SUCCESS)
   {
      messageBody.append("<b>Version check is completed. </b><p>");
   }
   else
   {
      messageBody.append("<b>Version check failed:  </b><p>");
      switch (rc)
      {
      case OS_VERSIONCHECK_NO_PLATFORMTYPE:
         messageBody.append("<b> - platform type not provided while calling findLatestVersion. </b><p>");
         break;

      case OS_VERSIONCHECK_NO_LATESTURL:
         messageBody.append("<b> - latest url not provided while calling findLatestVersion. </b><p>");
         break;

      case OS_VERSIONCHECK_NO_SCRIPTURL:
         messageBody.append("<b> - script url not found in the catalog file. </b><p>");
         break;

      case OS_VERSIONCHECK_NO_CATALOGURL:
         messageBody.append("<b> - catalog_url not found in the latest file. </b><p>");
         break;

      case OS_VERSIONCHECK_NO_LATESTVERSION:
         messageBody.append("<b> - catalog_url not found in the latest file. </b><p>");
         break;

      case OS_VERSIONCHECK_FAILURE_CONNECT_TO_SERVER:
         messageBody.append("<b> - cannot connect to the web server for the latest or catalog url. </b><p>");
         break;

      case OS_VERSIONCHECK_FAILURE_GET_LATESTFILE:
         messageBody.append("<b> - failed to get the specified latest file. </b><p>");
         break;

      case OS_VERSIONCHECK_FAILURE_GET_CATALOGFILE:
         messageBody.append("<b> - failed to get the specified catalog file. </b><p>");
         break;

      case OS_INVALID_ARGUMENT:
         messageBody.append("<b> - The platform type, running version, target version, or catalog URL parameters provided are incorrect. </b><p>");
         break;

      case OS_COMMAND_NOT_FOUND:
         messageBody.append("<b> - The specified cmd is not supported. </b><p>");
         break;

      case OS_COMMAND_AMBIGUOUS:
         messageBody.append("<b> - The specified cmd is ambiguous, i.e., more than one cmd can be assigned to it. </b><p>");
         break;

      case OS_COMMAND_BAD_SYNTAX:
         messageBody.append("<b> - The arguments do not match specified the cmd. </b><p>");
         break;

      case OS_HTTP_MOVED_PERMANENTLY_CODE:
         messageBody.append("<b> - Moved Permanently. </b><p>");
         break;

      case OS_HTTP_MOVED_TEMPORARILY_CODE:
         messageBody.append("<b> - Moved Temporatily. </b><p>");
         break;

      case OS_HTTP_UNAUTHORIZED_CODE:
         messageBody.append("<b> - Unauthorized. </b><p>");
         break;

      case OS_HTTP_FILE_NOT_FOUND_CODE:
         messageBody.append("<b> - File Not Found. </b><p>");
         break;

      case OS_HTTP_PROXY_UNAUTHORIZED_CODE:
         messageBody.append("<b> - Proxy Authentication Required. </b><p>");
         break;

      case OS_HTTP_UNSUPPORTED_METHOD_CODE:
         messageBody.append("<b> - Not Implemented. </b><p>");
         break;

      case OS_NO_MEMORY:
         messageBody.append("<b> - Out of memory. </b><p>");
         break;

      default:
         messageBody.append("<b> - Unknown error. </b><p>");
         break;
      }
   }

   messageBody.append(
   "<FORM ACTION=\"/upgrade-log\" METHOD=GET>\n");
   messageBody.append("Check the upgrade-log file on the platform for results.<p>");
   messageBody.append("<p><INPUT TYPE=SUBMIT VALUE=\"Check Log\">\n");
   messageBody.append("</FORM>\n");
   messageBody.append(HTTP_BODY_END);

   HttpBody* body = new HttpBody(messageBody.data(),
      messageBody.length(), CONTENT_TYPE_TEXT_HTML);
   response = new HttpMessage();
   response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
      HTTP_OK_CODE, HTTP_OK_TEXT);
   response->setBody(body);
   response->setContentType(CONTENT_TYPE_TEXT_HTML);
   response->setContentLength(messageBody.length());
}

void Pinger::processInstallUrl(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
    UtlString messageBody(HTTP_BODY_BEGIN);
    messageBody.append("<TITLE>Software Installation Page</TITLE>\n");

    messageBody.append(
       "<FORM ACTION=\"/cgi/upgradeComplete.cgi\" METHOD=POST>\n");

    messageBody.append("<p><b>Software Installation</b>\n");

    messageBody.append("<p>Enter the URL indicating the location and name of"
       " the software:<br>\n");
    messageBody.append("<INPUT TYPE=TEXT NAME=URL SIZE=58"
       " VALUE=\"http://appdev.pingtel.com/upgrades/080upgrade.txt\">\n");

    messageBody.append("<p><INPUT TYPE=SUBMIT VALUE=Submit>\n");
    messageBody.append("<INPUT TYPE=RESET VALUE=Reset>\n</FORM>\n");

    messageBody.append(HTTP_BODY_END);

    // Build the response message
    HttpBody* body = new HttpBody(messageBody.data(),
        messageBody.length(), CONTENT_TYPE_TEXT_HTML);
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_OK_CODE, HTTP_OK_TEXT);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    response->setContentLength(messageBody.length());
}

void Pinger::processGetInstallUrl(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response)
{
   UtlString installURL;
   UtlString logText;

   requestContext.getCgiVariable("URL", installURL);

   if (installURL.length() != 0)
   {
      UtlString version;
      PingerInfo::getVersion(version);
      HttpMessage::escape(version);

      char buf[16];
      sprintf(buf, "%d", OsUtil::getPlatformType());


      installURL += UtlString("?") + UPGRADE_CURRENT_VERSION + "=" + version;
      installURL += UtlString("&") + UPGRADE_PLATFORM + "=" + UtlString(buf);

      VersionCatalog::installUrlScripts(installURL.data(), &logText);
   }

   // Build the response message
   UtlString messageBody(HTTP_BODY_BEGIN);
   messageBody.append("<TITLE>Installation Complete Page</TITLE>\r\n");
   messageBody.append("<b>Software Installation Log </b><p>");
   if (logText.isNull())
      messageBody.append("<p>The information is being processed. The new"
         " software will be installed.\n");
   else
      messageBody.append(logText);

   messageBody.append(HTTP_BODY_END);

   HttpBody* body = new HttpBody(messageBody.data(),
      messageBody.length(), CONTENT_TYPE_TEXT_HTML);
   response = new HttpMessage();
   response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
      HTTP_OK_CODE, HTTP_OK_TEXT);
   response->setBody(body);
   response->setContentType(CONTENT_TYPE_TEXT_HTML);
   response->setContentLength(messageBody.length());
}
#endif // DEBUG_VERSIONCHECK

void Pinger::setPtProvider(PtProvider* pProvider)
{
   mpProvider = pProvider;
}


#define AUTHERR_NONE                0
#define AUTHERR_PASSWORD_MISMATCH   1
#define AUTHERR_USER_NOT_FOUND      2
#define AUTHERR_CONFIG_MISSING     -1
int Pinger::authenticateUser(UtlString userId, UtlString password)
{
   // Create digest of password
   UtlString realm(PASSWORD_SECRET) ;
   UtlString digestPassword ;
   HttpMessage::buildMd5UserPasswordDigest((const char*) userId.data(),
                                 (const char*) realm.data(),
                                 (const char*) password.data(),
                                 digestPassword) ;
#ifdef TEST_PRINT
   osPrintf("Pinger:userLogin digest = %s", digestPassword.data()) ;
#endif

   int iReturn = AUTHERR_CONFIG_MISSING;
    OsConfigDb dbUserConfig ;
   UtlString currentPassword("") ;

    //
    // First look in user-config
    //
   if (dbUserConfig.loadFromFile(CONFIGDB_USER_IN_FLASH))
   {
      UtlString strKey("PHONESET_HTTP_AUTH_DB.") ;
      strKey.append(userId.data()) ;

      dbUserConfig.get(strKey, currentPassword) ;
#ifdef TEST_PRINT
      osPrintf("Pinger::authenticateUser user:%s, currentpass:%s.\n",
         strKey.data(), currentPassword.data()) ;
#endif
      if (currentPassword.compareTo(digestPassword) == 0) {
#ifdef TEST_PRINT
         osPrintf("Pinger::authenticateUser passwords match\n") ;
#endif
         iReturn = AUTHERR_NONE ;
      } else if (currentPassword.compareTo("") == 0) {
#ifdef TEST_PRINT
         osPrintf("Pinger::authenticateUser user not found\n") ;
         // Doesn't compile...???? - DLH
         //iReturn = AUTHERR_BAD_USER ;
#endif

      } else {
         iReturn = AUTHERR_PASSWORD_MISMATCH ;
#ifdef TEST_PRINT
         osPrintf("Pinger::authenticateUser passwords do not match\n") ;
#endif
      }
   }
   else
   {
      iReturn = AUTHERR_CONFIG_MISSING ;
   }

   // Next Try device config
   if ((iReturn == AUTHERR_USER_NOT_FOUND) ||
               (iReturn == AUTHERR_CONFIG_MISSING))
   {
      OsConfigDb dbDeviceConfig ;

      if (dbDeviceConfig.loadFromFile(CONFIGDB_NAME_IN_FLASH))
      {
         UtlString strKey("PHONESET_HTTP_AUTH_DB.") ;
         strKey.append(userId.data()) ;

         dbDeviceConfig.get(strKey, currentPassword) ;
#ifdef TEST_PRINT
         osPrintf("Pinger::authenticateUser user:%s, currentpass:%s.\n",
            strKey.data(), currentPassword.data()) ;
#endif
         if (currentPassword.compareTo(digestPassword) == 0) {
#ifdef TEST_PRINT
            osPrintf("Pinger::authenticateUser passwords match\n") ;
#endif
            iReturn = AUTHERR_NONE ;
         } else if (currentPassword.compareTo("") == 0) {
#ifdef TEST_PRINT
            osPrintf("Pinger::authenticateUser user not found\n") ;
            // Doesn't compile ???? - DLH
            //iReturn = AUTHERR_BAD_USER ;
#endif

         } else {
            iReturn = AUTHERR_PASSWORD_MISMATCH ;
#ifdef TEST_PRINT
            osPrintf("Pinger::authenticateUser passwords do not match\n") ;
#endif
         }
      }
      else
      {
          iReturn = AUTHERR_CONFIG_MISSING ;
      }
   }
   return iReturn ;
}


// Adds a user to the config pro file, and to the web server digest database

int Pinger::addUser(UtlString userId, UtlString password)
{
   //Add user to user-config file
   int iReturn = addUserToConfig(userId, password) ;

   // if user was successfully added to the config files,
   // add user to HTTP server
   if( iReturn == 0 )
   {
        userLogin(userId, password) ;
   }

   return iReturn ;
}

int Pinger::addUserToConfig(UtlString userId, UtlString password)
{
   OsConfigDb deviceProfile;
   OsConfigDb userProfile;
   UtlString   key("PHONESET_HTTP_AUTH_DB.");
   UtlString   realm(PASSWORD_SECRET) ;
   UtlString   responseHash;
   UtlBoolean  userAlreadyExists = FALSE;

   //because OsConfigDb get() will whack the current value we have
   //lets not pass responseHash in.  Lets pass a copy.
   UtlString   tmpHash;

   // Create digest of password
   HttpMessage::buildMd5UserPasswordDigest((const char*) userId.data(),
                                           (const char*) realm.data(),
                                             (const char*) password.data(),
                                           responseHash) ;

#ifdef TEST_PRINT
   osPrintf("Pinger::addUserToConfig userId:%s, realm:%s, password:%s, digestPassword:%s\n",
            userId.data(), realm.data(), password.data(), responseHash.data()) ;
#endif

   // The deployment server stores user login information in the device
   // profile.  The browser-based UI stores user login information in the
   // user profile.  Because of this difference, adding a user behaves as
   // follows:
   //
   //    IF the user is already defined in the device profile THEN
   //       return 1 (user already exists)
   //    ELIF the user is already defined in the user profile THEN
   //       return 1 (user already exists)
   //    ELSE
   //       add the entry to the user profile
   //       write the user profile to FLASH
   //    ENDIF

   key.append(userId.data()) ;

   // check for userId in the device profile
   deviceProfile.loadFromFile(CONFIGDB_NAME_IN_FLASH);

   if (deviceProfile.get(key, tmpHash) == OS_SUCCESS)
   {
      userAlreadyExists = TRUE;
   }

   // check for userId in the user profile
   if (!userAlreadyExists)
   {
      userProfile.loadFromFile(CONFIGDB_USER_IN_FLASH);
      if (userProfile.get(key, tmpHash) == OS_SUCCESS)
      {
         userAlreadyExists = TRUE;
      }
   }

   // if user not found, add to the user profile
   if (!userAlreadyExists)
   {
      userProfile.set(key, responseHash) ;

      // must do this in order to preserve file remains encrypted or unencrypted
      OsConfigDb pingerConfig;
      if (pingerConfig.loadFromFile(CONFIGDB_NAME_IN_FLASH) == OS_SUCCESS)
      {
          sConfigEncryption.initConfig(&pingerConfig);
      }

      userProfile.storeToFile(CONFIGDB_USER_IN_FLASH);
      return 0;
   }
   else
   {
      osPrintf("Pinger::addUserToConfig userId:%s already exists\n",
               userId.data());
      return 1;
   }
}


// Adds a user to the basic authentication HTTP database for session

void Pinger::userLogin(UtlString userId, UtlString password)
{
   UtlString userPasswordDigest;
   // Create digest of password
   UtlString realm(PASSWORD_SECRET) ;
   UtlString digestPassword ;
   HttpMessage::buildMd5UserPasswordDigest((const char*) userId.data(),
                                           (const char*) realm.data(),
                                           (const char*) password.data(),
                                           digestPassword) ;

   HttpServer* pHttpServer   = Pinger::getPingerTask()->getHttpServer();
   pHttpServer->setPasswordDigest(userId.data(), password.data(),
      userPasswordDigest);
#ifdef TEST_PRINT
   osPrintf("Webui::userLogin userId:%s, realm:%s, password:%s"
      " digestPassword:%s\n", userId.data(), realm.data(), password.data(),
      digestPassword.data()) ;
#endif
}

// Tester method to invoke restarts
void doRestart(UtlBoolean confirm, int seconds, char* reason)
{
   Pinger::getPingerTask()->restart(confirm, seconds, reason) ;
}
