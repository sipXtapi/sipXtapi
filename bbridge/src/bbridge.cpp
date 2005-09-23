// 
// 
// Copyright (C) 2005 Jason Fischl, Dan Petrie
//
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <os/OsConfigDb.h>
#include <net/NameValueTokenizer.h>
#include "ConferenceUserAgent.h"
#include "config/bbridge-buildstamp.h"

#include "rutil/Log.hxx"

// DEFINES
#ifndef SIPX_VERSION
#  define SIPX_BBRIDGE_VERSION          bbridgeVersion
#  define SIPX_BBRIDGE_VERSION_COMMENT  bbridgeBuildStamp
#else
#  define SIPX_BBRIDGE_VERSION          SIPX_VERSION
#  define SIPX_BBRIDGE_VERSION_COMMENT  ""
#endif

#ifndef SIPX_LOGDIR
#  define SIPX_LOGDIR "."
#endif

#ifndef SIPX_CONFDIR
#  define SIPX_CONFDIR "."
#endif

#define CONFIG_SETTINGS_FILE          "bbridge.conf"
#define CONFIG_ETC_DIR                SIPX_CONFDIR

#define CONFIG_LOG_FILE               "bbridge.log"
#define CONFIG_LOG_DIR                SIPX_LOGDIR

#define LOG_FACILITY                  FAC_CONFERENCE

#define CONFIG_SETTING_LOG_DIR        "BOSTON_BRIDGE_LOG_DIR"
#define CONFIG_SETTING_LOG_LEVEL      "BOSTON_BRIDGE_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "BOSTON_BRIDGE_LOG_CONSOLE"
#define CONFIG_SETTING_UDP_PORT       "BOSTON_BRIDGE_UDP_PORT"
#define CONFIG_SETTING_TCP_PORT       "BOSTON_BRIDGE_TCP_PORT"
#define CONFIG_SETTING_TLS_PORT       "BOSTON_BRIDGE_TLS_PORT"
#define CONFIG_SETTING_RTP_START      "BOSTON_BRIDGE_RTP_START"
#define CONFIG_SETTING_RTP_END        "BOSTON_BRIDGE_RTP_END"

#define DEFAULT_UDP_PORT              5060       // Default UDP port
#define DEFAULT_TCP_PORT              5060       // Default TCP port
#define DEFAULT_TLS_PORT              5061       // Default TLS port
#define DEFAULT_RTP_START             15000
#define DEFAULT_RTP_END               20000

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
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
    OsSysLog::add(LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num);
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
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
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
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity);
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

   osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      OsSysLog::add(FAC_LOG, PRI_CRIT, "Cannot access %s directory; please check configuration.", CONFIG_SETTING_LOG_DIR);
   }
}

class SipXExternalLogger : public resip::ExternalLogger
{
   public:
      virtual bool operator()(resip::Log::Level level,
                              const resip::Subsystem& subsystem, 
                              const resip::Data& appName,
                              const char* file,
                              int line,
                              const resip::Data& message,
                              const resip::Data& messageWithHeaders)
      {
         OsSysLog::add(FAC_CONFERENCE, toPriority(level), "%s", message.c_str());
         return false;
      }
      
      static OsSysLogPriority toPriority(resip::Log::Level level)
      {
         switch (level)
         {
            case resip::Log::Crit:
               return PRI_CRIT;
            case resip::Log::Err:
               return PRI_ERR;
            case resip::Log::Warning:
               return PRI_WARNING;
            case resip::Log::Info:
               return PRI_INFO;
            case resip::Log::Debug:
            case resip::Log::Stack:
            default:
               return PRI_DEBUG;
         }
      }
      
};

   

//
// The main entry point to the sipXpark
//
int main(int argc, char* argv[])
{
   SipXExternalLogger resipLogger;
   resip::Log::initialize(resip::Log::Cout, resip::Log::Info, resip::Data(argv[0]), resipLogger);

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
   pt_signal(SIGPIPE,  sigHandler);    // Handle TCP Failure
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
         osPrintf("Version: %s (%s)\n", SIPX_BBRIDGE_VERSION_COMMENT, SIPX_BBRIDGE_VERSION_COMMENT);
         exit(0);
      }
      else
      {
         osPrintf("usage: %s [-v]\nwhere:\n -v provides the software version\n",
         argv[0]);
         exit(1);
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

   if (configDb.loadFromFile(fileName) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_LOG_DIR, "");
      configDb.set(CONFIG_SETTING_LOG_LEVEL, "INFO");
      configDb.set(CONFIG_SETTING_LOG_CONSOLE, "");
      configDb.set(CONFIG_SETTING_UDP_PORT, DEFAULT_UDP_PORT);
      configDb.set(CONFIG_SETTING_TCP_PORT, DEFAULT_TCP_PORT);
      configDb.set(CONFIG_SETTING_TLS_PORT, DEFAULT_TLS_PORT);
      configDb.set(CONFIG_SETTING_RTP_START, DEFAULT_RTP_START);
      configDb.set(CONFIG_SETTING_RTP_END, DEFAULT_RTP_END);
      configDb.storeToFile(fileName);
   }
   
   // Initialize log file
   initSysLog(&configDb);

   // Read the user agent parameters from the config file.
   int UdpPort;
   if (configDb.get(CONFIG_SETTING_UDP_PORT, UdpPort) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_UDP_PORT, DEFAULT_UDP_PORT);;
   }
   
   int TcpPort;
   if (configDb.get(CONFIG_SETTING_TCP_PORT, TcpPort) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_TCP_PORT, DEFAULT_TCP_PORT);;
   }

   int TlsPort;
   if (configDb.get(CONFIG_SETTING_TLS_PORT, TlsPort) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_TLS_PORT, DEFAULT_TLS_PORT);;
   }

   int RtpStart;
   if (configDb.get(CONFIG_SETTING_RTP_START, RtpStart) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_RTP_START, DEFAULT_RTP_START);;
   }

   int RtpEnd;
   if (configDb.get(CONFIG_SETTING_RTP_END, RtpEnd) != OS_SUCCESS)
   {
      configDb.set(CONFIG_SETTING_RTP_END, DEFAULT_RTP_END);;
   }
   
   bbridge::ConferenceUserAgent ua(configDb);
   
   while (!gShutdownFlag)
   {
      OsTask::delay(1000);
   }

   // Flush the log file
   OsSysLog::flush();

   // Say goodnight Gracie...
   return 0;
}


