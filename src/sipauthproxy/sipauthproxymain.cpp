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
// SYSTEM INCLUDES
#include <stdio.h>
#include <signal.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__pingtel_on_posix__)
#include <unistd.h>
#endif
#include <iostream.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsConfigDb.h"
#include "os/OsTask.h"
#include "net/SipUserAgent.h"
#include "net/NameValueTokenizer.h"
#include "sipdb/SIPDBManager.h"
#include "SipAaa.h"
#include "AuthProxyCseObserver.h"

// DEFINES

#define CONFIG_SETTING_CALL_STATE     "SIP_AUTHPROXY_CALL_STATE"
#define CONFIG_SETTING_CALL_STATE_LOG "SIP_AUTHPROXY_CALL_STATE_LOG"
#define CALL_STATE_LOG_FILE_DEFAULT SIPX_LOGDIR "/sipauthproxy_callstate.log"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*sighandler_t)(int);

#ifndef _WIN32
using namespace std ;
#endif

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
OsServerTask* pServerTask   = NULL;
UtlBoolean     gShutdownFlag = FALSE;
UtlBoolean     gClosingIMDB  = FALSE;
OsMutex       gLockMutex (OsMutex::Q_FIFO);

/* ============================ FUNCTIONS ================================= */

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 */
void
closeIMDBConnections ()
{
    // Critical Section here
    OsLock lock( gLockMutex );

    // now deregister this process's database references from the IMDB
    // and also ensure that we do not cause this code recursively
    // specifically SIGABRT or SIGSEGV could cause problems here
    if ( !gClosingIMDB )
    {
        gClosingIMDB = TRUE;
        // if deleting this causes another problem in this process
        // the gClosingIMDB flag above will protect us
        delete SIPDBManager::getInstance();
    }
}

/**
 * Description:
 * This is a replacement for signal() which registers a signal handler but sets
 * a flag causing system calls ( namely read() or getchar() ) not to bail out
 * upon recepit of that signal. We need this behavior, so we must call
 * sigaction() manually.
 */
sighandler_t
pt_signal( int sig_num, sighandler_t handler)
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
void 
sigHandler( int sig_num )
{
    // set a global shutdown flag
    gShutdownFlag = TRUE;

    // Unregister interest in the signal to prevent recursive callbacks
    pt_signal( sig_num, SIG_DFL );

    // Minimize the chance that we loose log data
    OsSysLog::flush();
    OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
    OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: closing IMDB connections" );
    OsSysLog::flush();

    // Finally close the IMDB connections
    closeIMDBConnections();
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
   OsSysLog::initialize(0, "SipAuthProxy");


   //
   // Get/Apply Log Filename
   //
   fileTarget.remove(0) ;
   if ((pConfig->get(CONFIG_SETTING_LOG_DIR, fileTarget) != OS_SUCCESS) ||
      fileTarget.isNull() || !OsFileSystem::exists(fileTarget))
   {
      bSpecifiedDirError = !fileTarget.isNull() ;

      // If the log file directory exists use that, otherwise place the log
      // in the current directory
      OsPath workingDirectory;
      if (OsFileSystem::exists(CONFIG_LOG_DIR))
      {
         fileTarget = CONFIG_LOG_DIR;
         OsPath path(fileTarget);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }

      fileTarget = workingDirectory +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   else
   {
      bSpecifiedDirError = false ;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;
      OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;

      fileTarget = fileTarget +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   OsSysLog::setOutputFile(0, fileTarget) ;


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
   int iEntries = sizeof(lkupTable)/sizeof(struct tagPrioriotyLookupTable);
   for (int i=0; i<iEntries; i++)
   {
      if (logLevel == lkupTable[i].pIdentity)
      {
         priority = lkupTable[i].ePriority;
         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity) ;
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity) ;
         break;
      }
   }
   OsSysLog::setLoggingPriority(priority);

   //
   // Get/Apply console logging
   //
   UtlBoolean bConsoleLoggingEnabled = false ;
   if ((pConfig->get(CONFIG_SETTING_LOG_CONSOLE, consoleLogging) ==
         OS_SUCCESS))
   {
      consoleLogging.toUpper();
      if (consoleLogging == "ENABLE")
      {
         OsSysLog::enableConsoleOutput(true);
         bConsoleLoggingEnabled = true ;
      }
   }

   osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
   OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      OsSysLog::add(FAC_LOG, PRI_CRIT, "Cannot access %s directory; please check configuration.", CONFIG_SETTING_LOG_DIR);
   }
}

int
main( int argc, char* argv[] )
{
    // Register Signal handlers to close IMDB
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

   UtlBoolean interactiveSet = false;
   UtlString argString;
   for(int argIndex = 1; argIndex < argc; argIndex++)
   {
       osPrintf("arg[%d]: %s\n", argIndex, argv[argIndex]);
       argString = argv[argIndex];
       NameValueTokenizer::frontBackTrim(&argString, "\t ");
       if(argString.compareTo("-v") == 0)
       {
           osPrintf("Version: %s\n", SIPX_VERSION);
           return(1);
       } else if( argString.compareTo("-i") == 0)
       {
          interactiveSet = true;
          osPrintf("Entering Interactive Mode\n");
       } else
       {
            osPrintf("usage: %s [-v] [-i]\nwhere:\n -v provides the software version\n"
               " -i starts the server in an interactive mode\n",
               argv[0]);
           return(1);
       }
   }

   //Config files which are specific to a component
   //(e.g. mappingrules.xml is to sipregistrar) Use the
   //following logic:
   //1) If  directory ../etc exists:
   //   The path to the data file is as follows
   //   ../etc/<data-file-name>
   //
   //2) Else the path is assumed to be:
   //   ./<data-file-name>
   //
   OsPath workingDirectory ;
   if ( OsFileSystem::exists( CONFIG_ETC_DIR ) )
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

   UtlString ConfigfileName =  workingDirectory +
      OsPathBase::separator +
      "authproxy-config"  ;

    int proxyTcpPort;
    int proxyUdpPort;
    UtlString routeName;
    //UtlString proxyRecordRoute;
    //int maxForwards;
    OsConfigDb configDb;
    UtlString ipAddress;

    OsSocket::getHostIp(&ipAddress);

    if(configDb.loadFromFile(ConfigfileName) == OS_SUCCESS)
    {
      osPrintf("Found config file: %s\n", ConfigfileName.data());
    }
    else
    {
        configDb.set("SIP_AUTHPROXY_AUTHENTICATE_ALGORITHM", "");
        configDb.set("SIP_AUTHPROXY_AUTHENTICATE_QOP", "");
        configDb.set("SIP_AUTHPROXY_AUTHENTICATE_REALM", "");
        configDb.set("SIP_AUTHPROXY_UDP_PORT", "");
        configDb.set("SIP_AUTHPROXY_TCP_PORT", "");
        configDb.set("SIP_AUTHPROXY_ROUTE_NAME", "");
        configDb.set("SIP_AUTHPROXY_HOST_ALIASES", "");
        //configDb.set("SIP_AUTHPROXY_RECORD_ROUTE", "DISABLE");
        //configDb.set("SIP_AUTHPROXY_MAX_FORWARDS", "");
        configDb.set("SIP_AUTHPROXY_STALE_TCP_TIMEOUT", "");
        configDb.set(CONFIG_SETTING_LOG_DIR, "");
        configDb.set(CONFIG_SETTING_LOG_LEVEL, "");
        configDb.set(CONFIG_SETTING_LOG_CONSOLE, "");
        configDb.set(CONFIG_SETTING_CALL_STATE, "DISABLE");
        configDb.set(CONFIG_SETTING_CALL_STATE_LOG, "");

        if (configDb.storeToFile(ConfigfileName) != OS_SUCCESS)
        {
          osPrintf("Could not write config file: %s\n", ConfigfileName.data());
        }
    }

    initSysLog(&configDb) ;

    UtlString algorithm;
    configDb.get("SIP_AUTHPROXY_AUTHENTICATE_ALGORITHM", algorithm);
    algorithm.toUpper();
    if( TRUE )
       // algorithm.compareTo("MD5") != 0 ||
       // algorithm.compareTo("MD5-SESS") != 0)
    {
        algorithm = "MD5";
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_AUTHENTICATE_ALGORITHM : %s", algorithm.data());
    osPrintf("SIP_AUTHPROXY_AUTHENTICATE_ALGORITHM : %s\n", algorithm.data());

    UtlString qop;
    configDb.get("SIP_AUTHPROXY_AUTHENTICATE_QOP", qop);
    qop.toUpper();

    if( TRUE )
      // qop.compareTo("none") != 0 ||
      // qop.compareTo("AUTH") != 0 ||
      // qop.compareTo("AUTH-INT") != 0)
    {
        qop = "none";
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_AUTHENTICATE_QOP : %s", qop.data());
    osPrintf("SIP_AUTHPROXY_AUTHENTICATE_QOP : %s\n", qop.data());

    UtlString realm;
    configDb.get("SIP_AUTHPROXY_AUTHENTICATE_REALM", realm);
    if(realm.isNull())
    {
        realm = ipAddress;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_AUTHENTICATE_REALM : %s", realm.data());
    osPrintf("SIP_AUTHPROXY_AUTHENTICATE_REALM : %s\n", realm.data());

    UtlBoolean authEnabled;
    UtlString authScheme;
    configDb.get("SIP_AUTHPROXY_AUTHENTICATE_SCHEME", authScheme);
    authScheme.toLower();
    if(authScheme.compareTo("none") == 0)
    {
        authEnabled = FALSE;
        OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_AUTHENTICATE_SCHEME : NONE");
        osPrintf("SIP_AUTHPROXY_AUTHENTICATE_SCHEME : NONE\n");
    }
    //else
    //  osPrintf("SIP_AUTHPROXY_AUTHENTICATE_SCHEME : DIGEST\n");

    configDb.get("SIP_AUTHPROXY_ROUTE_NAME", routeName);
    if(routeName.isNull())
    {
        routeName = ipAddress;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_ROUTE_NAME : %s", routeName.data());
    osPrintf("SIP_AUTHPROXY_ROUTE_NAME : %s\n", routeName.data());

    configDb.get("SIP_AUTHPROXY_UDP_PORT", proxyUdpPort);
    if(proxyUdpPort <=0) proxyUdpPort = 5080;
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_UDP_PORT : %d", proxyUdpPort);
    osPrintf("SIP_AUTHPROXY_UDP_PORT : %d\n", proxyUdpPort);

    configDb.get("SIP_AUTHPROXY_TCP_PORT", proxyTcpPort);
    if(proxyTcpPort <=0) proxyTcpPort = 5080;
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_TCP_PORT : %d", proxyTcpPort);
    osPrintf("SIP_AUTHPROXY_TCP_PORT : %d\n", proxyTcpPort);

    UtlString hostAliases;
    configDb.get("SIP_AUTHPROXY_HOST_ALIASES", hostAliases);
    if(hostAliases.isNull())
    {
        hostAliases = ipAddress;
        char portBuf[20];
        sprintf(portBuf, ":%d", proxyUdpPort);
        hostAliases.append(portBuf);

        if(!routeName.isNull())
        {
            hostAliases.append(" ");
            hostAliases.append(routeName);
            char portBuf[20];
            sprintf(portBuf, ":%d", proxyUdpPort);
            hostAliases.append(portBuf);
        }
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_AUTHHOST_ALIASES : %s",
                  hostAliases.data());
    osPrintf("SIP_AUTHPROXY_HOST_ALIASES : %s\n", hostAliases.data());

    UtlString enableCallStateObserverSetting;
    configDb.get(CONFIG_SETTING_CALL_STATE, enableCallStateObserverSetting);

    bool enableCallStateObserver;
    if (   (enableCallStateObserverSetting.isNull())
        || (0== enableCallStateObserverSetting.compareTo("disable", UtlString::ignoreCase))
        )
    {
       enableCallStateObserver = false;
    }
    else if (0 == enableCallStateObserverSetting.compareTo("enable", UtlString::ignoreCase))
    {
       enableCallStateObserver = true;
    }
    else
    {
       enableCallStateObserver = false;
       OsSysLog::add(FAC_SIP, PRI_ERR, "SipAuthProxyMain:: invalid configuration value for "
                     CONFIG_SETTING_CALL_STATE " '%s' - should be 'enable' or 'disable'",
                     enableCallStateObserverSetting.data()
                     );
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, CONFIG_SETTING_CALL_STATE " : %s",
                  enableCallStateObserver ? "ENABLE" : "DISABLE" );

    UtlString callStateLogFileName;
    OsFile* callStateLog = NULL;
    if (enableCallStateObserver)
    {
       configDb.get(CONFIG_SETTING_CALL_STATE_LOG, callStateLogFileName);
       if (callStateLogFileName.isNull())
       {
          callStateLogFileName = CALL_STATE_LOG_FILE_DEFAULT;
       }
       OsSysLog::add(FAC_SIP, PRI_INFO, CONFIG_SETTING_CALL_STATE_LOG " : %s",
                     callStateLogFileName.data());
    }

    // Set the maximum amount of time that TCP connections can
    // stay around when they are not used.
    int      staleTcpTimeout = 3600;
    UtlString staleTcpTimeoutStr;

    // Check for missing parameter or empty value
    configDb.get("SIP_AUTHPROXY_STALE_TCP_TIMEOUT", staleTcpTimeoutStr);
    if (staleTcpTimeoutStr.isNull())
    {
        staleTcpTimeout = 3600;
    }
    else
    {
        // get the parameter value as an integer
        configDb.get("SIP_AUTHPROXY_STALE_TCP_TIMEOUT", staleTcpTimeout);
    }

    if(staleTcpTimeout <= 0) staleTcpTimeout = -1;
    else if(staleTcpTimeout < 180) staleTcpTimeout = 180;
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_STALE_TCP_TIMEOUT : %d",
                  staleTcpTimeout);
    osPrintf("SIP_AUTHPROXY_STALE_TCP_TIMEOUT : %d\n", staleTcpTimeout);

    int maxNumSrvRecords = -1;
    configDb.get("SIP_AUTHPROXY_DNSSRV_MAX_DESTS", maxNumSrvRecords);
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_DNSSRV_MAX_DESTS : %d",
              maxNumSrvRecords);
    // If explicitly set to a valid number
    if(maxNumSrvRecords > 0)
    {
        osPrintf("SIP_AUTHPROXY_DNSSRV_MAX_DESTS : %d\n", maxNumSrvRecords);
    }
    else
    {
        maxNumSrvRecords = 4;
    }

    int dnsSrvTimeout = -1; //seconds
    configDb.get("SIP_AUTHPROXY_DNSSRV_TIMEOUT", dnsSrvTimeout);
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_AUTHPROXY_DNSSRV_TIMEOUT : %d",
              dnsSrvTimeout);
    // If explicitly set to a valid number
    if(dnsSrvTimeout > 0)
    {
        osPrintf("SIP_AUTHPROXY_DNSSRV_TIMEOUT : %d\n", dnsSrvTimeout);
    }
    else
    {
        dnsSrvTimeout = 4;
    }

    //osPrintf("SIP_AUTHPROXY_DNSSRV_TIMEOUT : %d\n", dnsSrvTimeout);

    //configDb.get("SIP_AUTHPROXY_RECORD_ROUTE", proxyRecordRoute);
    //UtlBoolean recordRouteEnabled = FALSE;
    //proxyRecordRoute.toLower();
    //if(proxyRecordRoute.compareTo("enable") == 0)
    //{
    //    recordRouteEnabled = TRUE;
    //    osPrintf("SIP_AUTHPROXY_RECORD_ROUTE : ENABLE\n");
    //}

    //configDb.get("SIP_AUTHPROXY_MAX_FORWARDS", maxForwards);
    //if(maxForwards <= 0) maxForwards = SIP_DEFAULT_MAX_FORWARDS;
    //osPrintf("SIP_AUTHPROXY_MAX_FORWARDS : %d\n", maxForwards);

    // Start the sip stack
    SipUserAgent sipUserAgent(proxyTcpPort,
        proxyUdpPort,
        NULL, // public IP address (nopt used in proxy)
        NULL, // default user (not used in proxy)
        NULL, // default SIP address (not used in proxy)
        NULL, // outbound proxy
        NULL, // directory server
        NULL, // registry server
        NULL, // auth scheme
        NULL, //auth realm
        NULL, // auth DB
        NULL, // auth user IDs
        NULL, // auth passwords
        NULL, // nat ping URL
        0, // nat ping frequency
        "PING", // nat ping method
        NULL, // line mgr
        SIP_DEFAULT_RTT, // first resend timeout
        TRUE, // default to UA transaction
        SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
        SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE // OsServerTask message queue size
        );
    sipUserAgent.setIsUserAgent(FALSE);
    //sipUserAgent.setMaxForwards(maxForwards);
    sipUserAgent.setDnsSrvTimeout(dnsSrvTimeout);
    sipUserAgent.setMaxSrvRecords(maxNumSrvRecords);

    // No need for this log as it all goes in the systlog anyway
    //sipUserAgent.startMessageLog(100000);

    sipUserAgent.setForking(FALSE);  // Diable forking
    sipUserAgent.setHostAliases(hostAliases);
    sipUserAgent.start();

    UtlString buffer;

    // Create a router to route stuff either
    // to a local server or on out to the real world
    pServerTask = new SipAaa(sipUserAgent, realm, routeName);

    // Start the router running
    pServerTask->start();

    AuthProxyCseObserver* cseObserver = NULL;
    if (enableCallStateObserver)
    {
       // Set up the call state event log file
       OsPath callStateLogPath(callStateLogFileName);
       callStateLog = new OsFile(callStateLogPath);

       OsStatus callStateLogStatus = callStateLog->open(OsFile::CREATE|OsFile::APPEND);
       if (OS_SUCCESS == callStateLogStatus)
       {
          // get the identifier for this observer
          int protocol = OsSocket::UDP;
          UtlString domainName;
          int port;
          sipUserAgent.getViaInfo(protocol, domainName, port);

          char portString[12];
          sprintf(portString,":%d", port);
          domainName.append(portString);
          
          // and start the observer
          cseObserver = new AuthProxyCseObserver(sipUserAgent, domainName, callStateLog);
          cseObserver->start();
       }
       else
       {
          OsSysLog::add(FAC_SIP, PRI_ERR,
                        "SipAuthProxyMain:: failed (%d) to open Call State Event Log '%s'",
                        callStateLogStatus, callStateLogFileName.data()
                        );
          enableCallStateObserver = false;
       }
       
    }

    // Do not exit, let the proxy do its stuff
    while( !gShutdownFlag )
    {
        if( interactiveSet)
        {

            int charCode = getchar();

            if(charCode != '\n' && charCode != '\r')
            {
                if( charCode == 'e')
                {
                    OsSysLog::enableConsoleOutput(TRUE);
                } else if( charCode == 'd')
                {
                    OsSysLog::enableConsoleOutput(FALSE);
                } else
                {
                    sipUserAgent.printStatus();
                    sipUserAgent.getMessageLog(buffer);
                    printf("=================>\n%s\n", buffer.data());
                }
            }
        } else
        {
            OsTask::delay(2000);
        }
    }

    // Remove the current process's row from the IMDB
    // Persisting the database if necessary
    cout << "Cleaning Up..Start." << endl;

    // This is a server task so gracefully shutdown the
    // server task using the waitForShutdown method, this
    // will implicitly request a shutdown for us if one is
    // not already in progress
    if ( pServerTask != NULL )
    {
        // Deleting a server task is the only way of
        // waiting for shutdown to complete cleanly
        pServerTask->requestShutdown();
        delete pServerTask;
        pServerTask = NULL;
    }

    // now deregister this process's database references from the IMDB
    closeIMDBConnections();

    // flush and close the call state event log
    if (enableCallStateObserver)
    {
       delete cseObserver;
       callStateLog->close();
    }
    
    // Flush the log file
    OsSysLog::flush();

    cout << "Cleanup...Finished" << endl;

    return 0;
}
