// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Jason Fischl, Dan Petrie
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <signal.h>
#if defined(_WIN32)
#   include <winsock2.h>
#endif

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "net/NameValueTokenizer.h"
#include "resiprocateLib/sipXexternalLogger.h"
#include "net/XmlRpcDispatch.h"
#include "configrpc/ConfigRPC.h"
#include "ConferenceUserAgent.h"

#include "bbridge.h"

// DEFINES

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* ConfigurationDatasetName = CONFIG_SETTINGS_FILE;

#define CONFIG_SETTING_PREFIX "BOSTON_BRIDGE"

const char* CONFIG_SETTING_LOG_DIR           = CONFIG_SETTING_PREFIX "_LOG_DIR";
const char* CONFIG_SETTING_LOG_LEVEL         = CONFIG_SETTING_PREFIX "_LOG_LEVEL";
const char* CONFIG_SETTING_RESIP_LOG_LEVEL   = CONFIG_SETTING_PREFIX "_RESIP_LOG_LEVEL";
const char* CONFIG_SETTING_LOG_CONSOLE       = CONFIG_SETTING_PREFIX "_LOG_CONSOLE";
const char* CONFIG_SETTING_UDP_PORT          = CONFIG_SETTING_PREFIX "_UDP_PORT";
const char* CONFIG_SETTING_TCP_PORT          = CONFIG_SETTING_PREFIX "_TCP_PORT";
const char* CONFIG_SETTING_TLS_PORT          = CONFIG_SETTING_PREFIX "_TLS_PORT";
const char* CONFIG_SETTING_XMLRPC_PORT       = CONFIG_SETTING_PREFIX "_XMLRPC_PORT";
const char* CONFIG_SETTING_MEDIA_IP_ADDRESS  = CONFIG_SETTING_PREFIX "_MEDIA_IP_ADDRESS";
const char* CONFIG_SETTING_RTP_START         = CONFIG_SETTING_PREFIX "_RTP_START";
const char* CONFIG_SETTING_RTP_END           = CONFIG_SETTING_PREFIX "_RTP_END";
const char* CONFIG_SETTING_ENTER_SOUND       = CONFIG_SETTING_PREFIX "_ENTER_SOUND";
const char* CONFIG_SETTING_EXIT_SOUND        = CONFIG_SETTING_PREFIX "_EXIT_SOUND";
const char* CONFIG_SETTING_HOLD_MUSIC        = CONFIG_SETTING_PREFIX "_HOLD_MUSIC";
const char* CONFIG_SETTING_PROXY             = CONFIG_SETTING_PREFIX "_PROXY";
const char* CONFIG_SETTING_MAX_LEGS          = CONFIG_SETTING_PREFIX "_MAX_LEGS";
const char* CONFIG_SETTING_SIP_DOMAIN        = CONFIG_SETTING_PREFIX "_SIP_DOMAIN";
const char* CONFIG_SETTING_SIP_ALIASES       = CONFIG_SETTING_PREFIX "_SIP_ALIASES";
const char* CONFIG_SETTING_COMMAND_MUTE      = CONFIG_SETTING_PREFIX "_COMMAND_MUTE";
const char* CONFIG_SETTING_COMMAND_END       = CONFIG_SETTING_PREFIX "_COMMAND_END";

const char* CONFIG_SETTING_CONFERENCE_STATUS = CONFIG_SETTING_PREFIX "_CONFERENCE_STATUS.";
const char* CONFIG_SETTING_CONFERENCE_ACCESS = CONFIG_SETTING_PREFIX "_CONFERENCE_ACCESS.";

const char* CONFERENCE_STATUS_CONFIGURE      = "CONFIGURE";
const char* CONFERENCE_STATUS_AVAILABLE      = "AVAILABLE";

const char* CONFERENCE_PARAM                 = CONFIG_SETTING_PREFIX "_CONFERENCE.";

const char* CONFERENCE_AOR                   = ".AOR";

const char* CONFERENCE_ACCESS                = ".ACCESS";
const char* CONFERENCE_ACCESS_OPEN           = "OPEN";
const char* CONFERENCE_ACCESS_REMOTE_ADMIT   = "REMOTE_ADMIT";

const char* CONFERENCE_MAX_LEGS              = ".MAX_LEGS";
const char* CONFERENCE_REGISTER_USERNAME     = ".REGISTER_USERNAME";
const char* CONFERENCE_REGISTER_REALM        = ".REGISTER_REALM";
const char* CONFERENCE_REGISTER_PASSWORD     = ".REGISTER_PASSWORD";

// STRUCTSCONFIG_SETTING_RTP_START
// TYPEDEFS
typedef void (*sighandler_t)(int);

// FUNCTIONS
extern "C" {
    void  sigHandler( int sig_num );
    sighandler_t pt_signal( int sig_num, sighandler_t handler );
}

// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS
UtlBoolean    gShutdownFlag = FALSE;

/* ============================ FUNCTIONS ================================= */

/**
 * Description:
 * This is a replacement for signal() which registers a signal handler but sets
 * a flag causing system calls ( namely read() or getchar() ) not to bail out
 * upon recepit of that signal. We need this behavior, so we must call
 * sigaction() manually.
 */
sighandler_t pt_signal( int sig_num, sighandler_t handler)
{
#if defined(__pingtel_on_posix__)
    struct sigaction action[2];
    action[0].sa_handler = handler;
    sigemptyset(&action[0].sa_mask);
    action[0].sa_flags = 0;
    sigaction ( sig_num, &action[0], &action[1] );
    return action[1].sa_handler;
#else
    return signal( sig_num, handler );
#endif
}


/**
 * Description:
 * This is the signal handler, When called this sets the
 * global gShutdownFlag allowing the main processing
 * loop to exit cleanly.
 */
void sigHandler( int sig_num )
{
    // set a global shutdown flag
    gShutdownFlag = TRUE;

    // Unregister interest in the signal to prevent recursive callbacks
    pt_signal( sig_num, SIG_DFL );

    // Minimize the chance that we loose log data   configDb.loadFromFile(fileName);
    
    OsSysLog::flush();
    if (SIGTERM == sig_num)
    {
       OsSysLog::add( LOG_FACILITY, PRI_INFO, "sigHandler: terminate signal received.");
    }
    else
    {
       OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
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

   OsSysLog::initialize(0, "bbridge");


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
      OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, fileTarget.data());

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
         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity);
         OsSysLog::add(LOG_FACILITY, PRI_INFO,
                       "%s : %s", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity);
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

   osPrintf("%s : %s\n",
            CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s",
                 CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      OsSysLog::add(FAC_LOG, PRI_CRIT, "Cannot access %s directory; please check configuration.",
                    CONFIG_SETTING_LOG_DIR);
   }
}


//
// The main entry point to the Boston Bridge
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
   pt_signal(SIGPIPE,  SIG_IGN);       // Force TCP failure to return an error.
   pt_signal(SIGBUS,   sigHandler);
   pt_signal(SIGSYS,   sigHandler);
   pt_signal(SIGXCPU,  sigHandler);
   pt_signal(SIGXFSZ,  sigHandler);
   pt_signal(SIGUSR1,  sigHandler);
   pt_signal(SIGUSR2,  sigHandler);
#endif

   UtlString argString;
   for(int argIndex = 1; argIndex < argc; argIndex++)
   {
      osPrintf("arg[%d]: %s\n", argIndex, argv[argIndex]);
      argString = argv[argIndex];
      NameValueTokenizer::frontBackTrim(&argString, "\t ");
      if(argString.compareTo("-v") == 0)
      {
         osPrintf("Version: %s (%s)\n", SIPX_BBRIDGE_VERSION, SIPX_BBRIDGE_VERSION_COMMENT);
         exit(0);
      }
      else
      {
         osPrintf("usage: %s [-v]\nwhere:\n -v provides the software version\n",
         argv[0]);
         exit(1);
      }
   }

   // Load configuration file 
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

   UtlString configFileName =  workingDirectory + OsPathBase::separator + ConfigurationDatasetName;

   if (configDb.loadFromFile(configFileName) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_LOG_LEVEL, "INFO");
      configDb.set(CONFIG_SETTING_RESIP_LOG_LEVEL, "INFO");
      configDb.set(CONFIG_SETTING_LOG_CONSOLE, "");
      configDb.set(CONFIG_SETTING_LOG_DIR, "");

      configDb.set(CONFIG_SETTING_UDP_PORT, DEFAULT_UDP_PORT);
      configDb.set(CONFIG_SETTING_TCP_PORT, DEFAULT_TCP_PORT);
      configDb.set(CONFIG_SETTING_TLS_PORT, DEFAULT_TLS_PORT);
      configDb.set(CONFIG_SETTING_XMLRPC_PORT, DEFAULT_XMLRPC_PORT);
      configDb.set(CONFIG_SETTING_MEDIA_IP_ADDRESS, "");
      configDb.set(CONFIG_SETTING_RTP_START, DEFAULT_RTP_START);
      configDb.set(CONFIG_SETTING_RTP_END, DEFAULT_RTP_END);

      configDb.set(CONFIG_SETTING_PROXY, "");
      configDb.set(CONFIG_SETTING_MAX_LEGS, "0");

      configDb.set(CONFIG_SETTING_ENTER_SOUND, "");
      configDb.set(CONFIG_SETTING_EXIT_SOUND, "");
      configDb.set(CONFIG_SETTING_HOLD_MUSIC, "");

      configDb.set(CONFIG_SETTING_SIP_DOMAIN, "");
      configDb.set(CONFIG_SETTING_SIP_ALIASES, "");

      configDb.set(CONFIG_SETTING_COMMAND_MUTE, "");
      configDb.set(CONFIG_SETTING_COMMAND_END, "");

      configDb.set(CONFIG_SETTING_CONFERENCE_ACCESS, "REMOTE_ADMIT");

      configDb.storeToFile(configFileName);
   }
   
   UtlString level;
   configDb.get(CONFIG_SETTING_RESIP_LOG_LEVEL, level);
   SipXExternalLogger resipLogger;
   resip::Log::initialize(resip::Log::Cout, 
                          resip::Log::toLevel(level.data()), 
                          resip::Data(argv[0]), 
                          resipLogger);

   // Initialize log file
   initSysLog(&configDb);

   // Read the user agent parameters from the config file.
   int UdpPort;
   if (configDb.get(CONFIG_SETTING_UDP_PORT, UdpPort) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_UDP_PORT, DEFAULT_UDP_PORT);
   }
   
   int TcpPort;
   if (configDb.get(CONFIG_SETTING_TCP_PORT, TcpPort) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_TCP_PORT, DEFAULT_TCP_PORT);
   }

   int TlsPort;
   if (configDb.get(CONFIG_SETTING_TLS_PORT, TlsPort) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_TLS_PORT, DEFAULT_TLS_PORT);
   }

   UtlString ipAddress;
   if (configDb.get(CONFIG_SETTING_MEDIA_IP_ADDRESS, ipAddress) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_MEDIA_IP_ADDRESS, "");
   }

   int RtpStart;
   if (configDb.get(CONFIG_SETTING_RTP_START, RtpStart) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_RTP_START, DEFAULT_RTP_START);
   }

   int RtpEnd;
   if (configDb.get(CONFIG_SETTING_RTP_END, RtpEnd) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_RTP_END, DEFAULT_RTP_END);
   }

   UtlString sipDomain;
   if (configDb.get(CONFIG_SETTING_SIP_DOMAIN, sipDomain) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "No value configured for %s", CONFIG_SETTING_SIP_DOMAIN
                    );
      osPrintf("No value configured for %s\n", CONFIG_SETTING_SIP_DOMAIN );
      exit(1);
   }

   int XmlRpcPort = configDb.getPort(CONFIG_SETTING_XMLRPC_PORT);
   if (PORT_DEFAULT == XmlRpcPort) 
   {
      XmlRpcPort = DEFAULT_XMLRPC_PORT;
   }

   ConfigRPC_Callback* defaultCallbacks;
   ConfigRPC*          configRPC;
   XmlRpcDispatch*     rpc;
   
   if (XmlRpcPort != PORT_NONE)
   {
      // start an XmlRpc interface
      rpc = new XmlRpcDispatch(XmlRpcPort, true /* use https */);

      // create the connector between the OsConfigDb and XmlRpc
      defaultCallbacks = new ConfigRPC_InDomainCallback(sipDomain);
      configRPC        = new ConfigRPC( ConfigurationDatasetName
                                       ,bbridgeVersion
                                       ,configFileName
                                       ,defaultCallbacks
                                       );
      // enter the connector RPC methods in the XmlRpcDispatch table
      ConfigRPC::registerMethods(*rpc);
   }

   try
   {
      bbridge::ConferenceUserAgent ua(configDb);
      while (!gShutdownFlag)
      {
         OsTask::delay(1000);
      }
   }
   catch (resip::BaseException& e)
   {
      std::cerr << "Uncaught resip exception: " << e << std::endl;
   }
   catch (...)
   {
      std::cerr << "Uncaught exception: " << std::endl;
   }
   
   // Flush the log file
   OsSysLog::flush();

   // Say goodnight Gracie...
   return 0;
}
