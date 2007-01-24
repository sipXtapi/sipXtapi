// 
// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <signal.h>

// APPLICATION INCLUDES
#include <os/OsConfigDb.h>
#include <os/OsSysLog.h>
#include <utl/UtlString.h>
#include <os/OsFS.h>
#include <net/SipUserAgent.h>
#include <os/OsSocket.h>
#include <net/SipSubscriptionMgr.h>
#include <net/SipSubscribeServer.h>
#include <net/NameValueTokenizer.h>
#include <net/SipUserAgent.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipDialogEvent.h>
#include <net/SipSubscribeClient.h>
#include <os/OsSysLog.h>
#include "RlsSubscribePolicy.h"
#include "ResourceList.h"

// DEFINES
#ifndef SIPX_VERSION
#  include "sipxpbx-buildstamp.h"
#  define SIPXCHANGE_VERSION          SipXpbxVersion
#  define SIPXCHANGE_VERSION_COMMENT  SipXpbxBuildStamp
#else
#  define SIPXCHANGE_VERSION          SIPX_VERSION
#  define SIPXCHANGE_VERSION_COMMENT  ""
#endif

#define CONFIG_SETTINGS_FILE          "sipxrls-config"
#define CONFIG_ETC_DIR                SIPX_CONFDIR

#define CONFIG_LOG_FILE               "sipxrls.log"
#define CONFIG_LOG_DIR                SIPX_LOGDIR

#define CONFIG_SETTING_LOG_DIR        "SIP_RLS_LOG_DIR"
#define CONFIG_SETTING_LOG_LEVEL      "SIP_RLS_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_RLS_LOG_CONSOLE"
#define CONFIG_SETTING_UDP_PORT       "SIP_RLS_UDP_PORT"
#define CONFIG_SETTING_TCP_PORT       "SIP_RLS_TCP_PORT"

#define CONFIG_SETTING_RLS_FILE       "SIP_RLS_FILE_NAME"
#define CONFIG_SETTING_DOMAIN_NAME    "SIP_RLS_DOMAIN_NAME"

#define LOG_FACILITY                  FAC_RLS

#define RLS_DEFAULT_UDP_PORT          5120       // Default UDP port
#define RLS_DEFAULT_TCP_PORT          5120       // Default TCP port

#define RESUBSCRIBE_PERIOD            3600

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*sighandler_t)(int);

// FUNCTIONS
extern "C" {
    void sigHandler(int sig_num);
    sighandler_t pt_signal(int sig_num, sighandler_t handler);
}

// FORWARD DECLARATIONS

void subscription_state_callback(SipSubscribeClient::SubscriptionState newState,
                                 const char* earlyDialogHandle,
                                 const char* dialogHandle,
                                 void* applicationData,
                                 int responseCode,
                                 const char* responseText,
                                 long expiration,
                                 const SipMessage* subscribeResponse);

void notify_event_callback(const char* earlyDialogHandle,
                           const char* dialogHandle,
                           void* applicationData,
                           const SipMessage* notifyRequest);

// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS
UtlBoolean    gShutdownFlag = FALSE;



/* ============================ FUNCTIONS ================================= */

/**
 * Description:
 * This is a replacement for signal() which registers a signal handler but sets
 * a flag causing system calls (namely read() or getchar()) not to bail out
 * upon recepit of that signal. We need this behavior, so we must call
 * sigaction() manually.
 */
sighandler_t pt_signal(int sig_num, sighandler_t handler)
{
#if defined(__pingtel_on_posix__)
    struct sigaction action[2];
    action[0].sa_handler = handler;
    sigemptyset(&action[0].sa_mask);
    action[0].sa_flags = 0;
    sigaction (sig_num, &action[0], &action[1]);
    return action[1].sa_handler;
#else
    return signal(sig_num, handler);
#endif
}


/**
 * Description:
 * This is the signal handler, When called this sets the
 * global gShutdownFlag allowing the main processing
 * loop to exit cleanly.
 */
void sigHandler(int sig_num)
{
    // set a global shutdown flag
    gShutdownFlag = TRUE;

    // Unregister interest in the signal to prevent recursive callbacks
    pt_signal(sig_num, SIG_DFL);

    // Minimize the chance that we loose log data
    OsSysLog::flush();
    if (SIGTERM == sig_num)
    {
       OsSysLog::add(LOG_FACILITY, PRI_INFO,
                     "sigHandler: terminate signal received.");
    }
    else
    {
       OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                     "sigHandler: caught signal: %d", sig_num);
    }
    OsSysLog::flush();
}


// Initialize the OsSysLog
void initSysLog(OsConfigDb* pConfig)
{
   UtlString logLevel;               // Controls Log Verbosity
   UtlString consoleLogging;         // Enable console logging by default?
   UtlString fileTarget;             // Path to store log file.
   UtlBoolean bSpecifiedDirError ;   // Set if the specified log dir does not
                                    // exist
   struct tagPrioriotyLookupTable
   {
      const char*      pIdentity;
      OsSysLogPriority ePriority;
   };

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
   };

   OsSysLog::initialize(0, "sipxrls");


   //
   // Get/Apply Log Filename
   //
   fileTarget.remove(0);
   if ((pConfig->get(CONFIG_SETTING_LOG_DIR, fileTarget) != OS_SUCCESS) ||
      fileTarget.isNull() || !OsFileSystem::exists(fileTarget))
   {
      bSpecifiedDirError = !fileTarget.isNull();

      // If the log file directory exists use that, otherwise place the log
      // in the current directory
      OsPath workingDirectory;
      if (OsFileSystem::exists(CONFIG_LOG_DIR))
      {
         fileTarget = CONFIG_LOG_DIR;
         OsPath path(fileTarget);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s",
                       CONFIG_SETTING_LOG_DIR, workingDirectory.data());
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s",
                       CONFIG_SETTING_LOG_DIR, workingDirectory.data());
      }

      fileTarget = workingDirectory +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   else
   {
      bSpecifiedDirError = false;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data());
      OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s",
                    CONFIG_SETTING_LOG_DIR, fileTarget.data());

      fileTarget = fileTarget +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   OsSysLog::setOutputFile(0, fileTarget);


   //
   // Get/Apply Log Level
   //
   if ((pConfig->get(CONFIG_SETTING_LOG_LEVEL, logLevel) != OS_SUCCESS) ||
         logLevel.isNull())
   {
      logLevel = "ERR";
   }
   logLevel.toUpper();
   OsSysLogPriority priority = PRI_ERR;
   int iEntries = sizeof(lkupTable) / sizeof(struct tagPrioriotyLookupTable);
   for (int i = 0; i < iEntries; i++)
   {
      if (logLevel == lkupTable[i].pIdentity)
      {
         priority = lkupTable[i].ePriority;
         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_LEVEL,
                  lkupTable[i].pIdentity);
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s",
                       CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity);
         break;
      }
   }
   OsSysLog::setLoggingPriority(priority);

   //
   // Get/Apply console logging
   //
   UtlBoolean bConsoleLoggingEnabled = false;
   if ((pConfig->get(CONFIG_SETTING_LOG_CONSOLE, consoleLogging) == OS_SUCCESS))
   {
      consoleLogging.toUpper();
      if (consoleLogging == "ENABLE")
      {
         OsSysLog::enableConsoleOutput(true);
         bConsoleLoggingEnabled = true;
      }
   }

   osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE,
            bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE,
                 bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                    "Cannot access %s directory; please check configuration.",
                    CONFIG_SETTING_LOG_DIR);
   }
}


//
// The main entry point to sipXrls.
//
int main(int argc, char* argv[])
{
   // Configuration Database (used for OsSysLog)
   OsConfigDb configDb;

   // Register Signal handlers so we can perform graceful shutdown
   pt_signal(SIGINT,   sigHandler);    // Trap Ctrl-C on NT
   pt_signal(SIGILL,   sigHandler);
   pt_signal(SIGABRT,  sigHandler);    // Abort signal 6
   pt_signal(SIGFPE,   sigHandler);    // Floading Point Exception
   pt_signal(SIGSEGV,  sigHandler);    // Address access violations signal 11
   pt_signal(SIGTERM,  sigHandler);    // Trap kill -15 on UNIX
#if defined(__pingtel_on_posix__)
   pt_signal(SIGHUP,   sigHandler);    // Hangup
   pt_signal(SIGQUIT,  sigHandler);
   pt_signal(SIGPIPE,  SIG_IGN);    // Handle TCP Failure
   pt_signal(SIGBUS,   sigHandler);
   pt_signal(SIGSYS,   sigHandler);
   pt_signal(SIGXCPU,  sigHandler);
   pt_signal(SIGXFSZ,  sigHandler);
   pt_signal(SIGUSR1,  sigHandler);
   pt_signal(SIGUSR2,  sigHandler);
#endif

   UtlString argString;
   for (int argIndex = 1; argIndex < argc; argIndex++)
   {
      osPrintf("arg[%d]: %s\n", argIndex, argv[argIndex]);
      argString = argv[argIndex];
      NameValueTokenizer::frontBackTrim(&argString, "\t ");
      if (argString.compareTo("-v") == 0)
      {
         osPrintf("Version: %s (%s)\n", SIPXCHANGE_VERSION,
                  SIPXCHANGE_VERSION_COMMENT);
         return 1;
      }
      else
      {
         osPrintf("usage: %s [-v]\nwhere:\n -v provides the software version\n",
                  argv[0]);
         return 1;
      }
   }

   // Load configuration file file
   OsPath workingDirectory;
   if (OsFileSystem::exists(CONFIG_ETC_DIR))
   {
      workingDirectory = CONFIG_ETC_DIR;
      OsPath path(workingDirectory);
      path.getNativePath(workingDirectory);
   }
   else
   {
      OsPath path;
      OsFileSystem::getWorkingDirectory(path);
      path.getNativePath(workingDirectory);
   }

   UtlString fileName =  workingDirectory +
      OsPathBase::separator +
      CONFIG_SETTINGS_FILE;

   configDb.loadFromFile(fileName);

   // Initialize log file
   initSysLog(&configDb);

   // Read the user agent parameters from the config file.
   int udpPort;
   if (configDb.get(CONFIG_SETTING_UDP_PORT, udpPort) != OS_SUCCESS)
   {
      udpPort = RLS_DEFAULT_UDP_PORT;
   }

   int tcpPort;
   if (configDb.get(CONFIG_SETTING_TCP_PORT, tcpPort) != OS_SUCCESS)
   {
      tcpPort = RLS_DEFAULT_TCP_PORT;
   }

   UtlString resourceListFile;
   if ((configDb.get(CONFIG_SETTING_RLS_FILE, resourceListFile) !=
        OS_SUCCESS) ||
       resourceListFile.isNull())
   {
      OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                    "Resource list file name is not configured");
      return 1;
   }

   UtlString domainName;
   if ((configDb.get(CONFIG_SETTING_DOMAIN_NAME, domainName) !=
        OS_SUCCESS) ||
       domainName.isNull())
   {
      OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                    "Resource domain name is not configured");
      return 1;
   }

   // Instantiate the call processing subsystem
   UtlString localAddress;
   OsSocket::getHostIp(&localAddress);
   // Construct our host-part.
   UtlString localHostPart;
   // Construct an address to be used in outgoing requests (primarily SUBSCRIBEs)
   UtlString outgoingAddress;
   {
      char buffer[100];
      sprintf(buffer, "%s:%d", localAddress.data(),
              portIsValid(udpPort) ? udpPort : tcpPort);
      localHostPart = buffer;
      sprintf(buffer, "sip:sipXrls@%s:%d", localAddress.data(),
              portIsValid(udpPort) ? udpPort : tcpPort);
      outgoingAddress = buffer;
   }

   // Create the user agent.
   SipUserAgent userAgent(
      // Let the system choose the port numbers.
      tcpPort, // sipTcpPort
      udpPort, // sipUdpPort
      tcpPort, // sipTlsPort
      NULL, // publicAddress
      NULL, // defaultUser
      NULL, // defaultSipAddress
      NULL, // sipProxyServers
      NULL, // sipDirectoryServers
      NULL, // sipRegistryServers
      NULL, // authenticationScheme
      NULL, // authenicateRealm
      NULL, // authenticateDb
      NULL, // authorizeUserIds
      NULL, // authorizePasswords
      NULL, // natPingUrl
      0, // natPingFrequency
      "PING", // natPingMethod
      NULL, // lineMgr
      SIP_DEFAULT_RTT, // sipFirstResendTimeout
      TRUE, // defaultToUaTransactions
      -1, // readBufferSize
      OsServerTask::DEF_MAX_MSGS, // queueSize
      FALSE // bUseNextAvailablePort
      );
   // Set the user-agent string.
   userAgent.setUserAgentHeaderProperty("sipX/rls");
   // Allow the "eventlist" extension.
   userAgent.allowExtension("eventlist");
   userAgent.start();

   // Create the SIP Subscribe Server
   // Component for holding the subscription data
   SipSubscriptionMgr subscriptionMgr; 
   // Component for granting subscription rights
   RlsSubscribePolicy policyHolder;
   // Component for publishing the event content
   // This will contain the event content for every resource list URI
   // and event type that the RLS services.
   SipPublishContentMgr eventPublisher;

   // Start the SIP Subscribe Server.
   SipSubscribeServer subscribeServer(userAgent, eventPublisher,
                                      subscriptionMgr, policyHolder);
   subscribeServer.enableEventType(DIALOG_EVENT_TYPE);
   subscribeServer.start();

   // Set up the SIP Subscribe Client
   SipDialogMgr dialogManager;
   SipRefreshManager refreshMgr(userAgent, dialogManager);
   refreshMgr.start();
   SipSubscribeClient subscribeClient(userAgent, dialogManager, refreshMgr);
   subscribeClient.start();  

   // Set up resource list.
#define RESOURCE_URI "sip:700@maine.pingtel.com"
   ResourceList rl("~~rl~1", domainName, localHostPart,
                   DIALOG_EVENT_TYPE,
                   DIALOG_EVENT_CONTENT_TYPE,
                   eventPublisher);
   ResourceListResource* rlr = rl.addResource(RESOURCE_URI, "<name>foobar</name>");

   // Wait 60 seconds to allow the Park Server to come up.
   OsTask::delay(60000);

   // Set up subscriptions to the park orbit.
   UtlString client700_early_handle;
   UtlBoolean ret;
   ret = subscribeClient.addSubscription(RESOURCE_URI,
                                         DIALOG_EVENT_TYPE,
                                         DIALOG_EVENT_CONTENT_TYPE,
                                         outgoingAddress,
                                         RESOURCE_URI,
                                         outgoingAddress,
                                         RESUBSCRIBE_PERIOD,
                                         rlr,
                                         subscription_state_callback,
                                         notify_event_callback,
                                         client700_early_handle);
   if (ret)
   {
      OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                    "addSubscription succeeded");
   }
   else
   {
      OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                    "addSubscription failed");
   }

   // Loop forever until signaled to shut down
   while (!gShutdownFlag)
   {
      OsTask::delay(2000);
   }

   // Shut down userAgent
   userAgent.shutdown(FALSE);

   while(!userAgent.isShutdownDone())
   {
      OsTask::delay(100);
   }

   // Flush the log file
   OsSysLog::flush();

   // Say goodnight Gracie...
   return 0;
}


void subscription_state_callback(SipSubscribeClient::SubscriptionState newState,
                                 const char* earlyDialogHandle,
                                 const char* dialogHandle,
                                 void* applicationData,
                                 int responseCode,
                                 const char* responseText,
                                 long expiration,
                                 const SipMessage* subscribeResponse)
{
   OsSysLog::add(LOG_FACILITY, PRI_DEBUG,
                 "subscription_state_callback newState = %d, applicationData = %p, dialogHandle = '%s'",
                 newState, applicationData, dialogHandle);
#if 0
   UtlString b;
   int l;
   subscribeResponse->getBytes(&b, &l);
   OsSysLog::add(LOG_FACILITY, PRI_DEBUG,
                 "subscription_state_callback subscribeResponse = '%s'",
                 b.data());
#endif // 0
   ResourceListResource* rlr = (ResourceListResource*) applicationData;

   switch (newState)
   {
   case SipSubscribeClient::SUBSCRIPTION_UNKNOWN:
      break;
   case SipSubscribeClient::SUBSCRIPTION_INITIATED:
      break;
   case SipSubscribeClient::SUBSCRIPTION_SETUP:
      // We should be able to add an instance here (with subscription
      // state "pending", as we have no content yet).  But
      // subscription handles are not returned consistently (see XSL-146).
      // So we don't add the instance here, but rather when the first NOTIFY
      // comes in and the matching instance can't be found.
#if 0
   {
      // Beware that this callback may be due to a NOTIFY that was
      // received before the response to the SUBSCRIBE, so the information
      // from the response (responseCode, subscribeResponse) may not be
      // available.
      OsSysLog::add(LOG_FACILITY, PRI_DEBUG,
                    "subscription_state_callback SUBSCRIPTION_SETUP rlr = %p",
                    rlr);
      // Put the subscription into pending state, as we have no
      // content for it yet.
      rlr->addInstancePublish(dialogHandle, "pending");
   }
#endif // 0
      break;
   case SipSubscribeClient::SUBSCRIPTION_FAILED:
   case SipSubscribeClient::SUBSCRIPTION_TERMINATED:
   {
      // Get the subscription state, from which we can extract the
      // reason for termination.
      const char* subscription_state =
         subscribeResponse->getHeaderValue(0, SIP_SUBSCRIPTION_STATE_FIELD);
      rlr->deleteInstancePublish(dialogHandle, "terminated", subscription_state);
   }
      break;
   }
}

void notify_event_callback(const char* earlyDialogHandle,
                           const char* dialogHandle,
                           void* applicationData,
                           const SipMessage* notifyRequest)
{
   if (OsSysLog::willLog(LOG_FACILITY, PRI_DEBUG))
   {
      UtlString b;
      int l;
      notifyRequest->getBytes(&b, &l);
      OsSysLog::add(LOG_FACILITY, PRI_DEBUG,
                    "notify_event_callback applicationData = %p, dialogHandle = '%s', notify = '%s'",
                    applicationData, dialogHandle, b.data());
   }
   ResourceListResource* rlr = (ResourceListResource*) applicationData;

   const char* content_bytes;
   int content_length;
   notifyRequest->getBody()->getBytes(&content_bytes, &content_length);
   rlr->updateInstancePublish(dialogHandle, content_bytes, content_length);
}

// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
   return 0;
}
