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
#include <assert.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "os/OsServerSocket.h"
#ifdef HAVE_SSL
#include "os/OsSSLServerSocket.h"
#endif /* HAVE_SSL */
#include "net/Url.h"
#include "net/HttpServer.h"
#include "net/SipUserAgent.h"
#include "net/SipMessage.h"
#include "net/SipMessageEvent.h"
#include "net/NameValueTokenizer.h"
#include "statusserver/Notifier.h"
#include "statusserver/WebServer.h"
#include "statusserver/StatusServer.h"
#include "statusserver/SubscribePersistThread.h"
#include "statusserver/SubscribeServerThread.h"
#ifndef SIPX_VERSION
#  include "sipxpublisher-buildstamp.h"
#  define SIPX_VERSION SipXpublisherVersion
#  define SIPX_BUILD SipXpublisherBuildStamp
#else
#  define SIPX_BUILD ""
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
StatusServer* StatusServer::spInstance = NULL;
OsBSem StatusServer::sLock( OsBSem::Q_PRIORITY, OsBSem::FULL );
OsConfigDb& StatusServer::sConfigDb = *(new OsConfigDb());


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StatusServer::StatusServer(
    Notifier* notifier,
    int maxExpiresTime,
    const UtlString& defaultDomain,
    const UtlString& defaultMinExpiresTime,
    const UtlBoolean& useCredentialDB,
    const UtlString& defaultAuthAlgorithm,
    const UtlString& defaultAuthQop,
    const UtlString& defaultRealm,
    const UtlString& configDir,
    HttpServer* httpServer) :
    OsServerTask("StatusServer", NULL, 2000),
    mNotifier(NULL),
    mSubscribeServerThread(NULL),
    mSubscribeServerThreadQ(NULL),
    mSubscribeThreadInitialized(FALSE),
    mSubscribePersistThread(NULL),
    mHttpServer(httpServer)
{
    mConfigDirectory.remove(0);
    mConfigDirectory.append(configDir);

    mDefaultRegistryPeriod = maxExpiresTime;
    mNotifier = notifier;

    if ( mNotifier )
    {
        mpSipUserAgent = mNotifier->getUserAgent();
        mpSipUserAgent->addMessageConsumer(this);
        mpSipUserAgent->allowMethod(SIP_NOTIFY_METHOD);
        mpSipUserAgent->allowMethod(SIP_SUBSCRIBE_METHOD);
        mpSipUserAgent->allowExtension("sip-cc-01");
        mpSipUserAgent->allowExtension("timer");
    }

    if(!defaultAuthAlgorithm.isNull())
    {
        mAuthAlgorithm = defaultAuthAlgorithm;
    }

    if(!defaultAuthQop.isNull())
    {
        mAuthQop = defaultAuthQop;
    }

    if ( !defaultDomain.isNull() )
    {
        mDefaultDomain.remove(0);
        mDefaultDomain.append(defaultDomain);
    }

    if ( !defaultMinExpiresTime.isNull() )
    {
        mMinExpiresTime.remove(0);
        mMinExpiresTime.append(defaultMinExpiresTime);
    }

    if ( !defaultDomain.isNull() )
    {
        mDefaultDomain.remove(0);
        mDefaultDomain.append(defaultDomain);
    }

    if ( !defaultRealm.isNull() )
    {
        mRealm.remove(0);
        mRealm.append(defaultRealm);
    }
    
    Url domain(mDefaultDomain);
    domain.getHostAddress(mlocalDomainHost);

    mIsCredentialDB = useCredentialDB;
    
    UtlString fileName = mConfigDirectory + 
        OsPathBase::separator + "status-plugin.xml";

    mPluginTable.loadPlugins(fileName, mNotifier);
    
    // Start Webserver and initialize the CGIs
    WebServer::getWebServerTask(&mPluginTable)->initWebServer(mHttpServer) ;

    startSubscribePersistThread();
    startSubscribeServerThread();
}

// Destructor
StatusServer::~StatusServer()
{
   // Wait for the owned servers to shutdown first
   if ( mSubscribeServerThread )
   {
      // Deleting a server task is the only way of 
      // waiting for shutdown to complete cleanly
      mSubscribeServerThread->requestShutdown();
      delete mSubscribeServerThread;
      mSubscribeServerThread = NULL;
      mSubscribeServerThreadQ = NULL;
   }
   // HTTP server shutdown
   if (mHttpServer)
   {
      mHttpServer->requestShutdown();
      delete mHttpServer;
      mHttpServer = NULL;
   }
   if( mNotifier)
   {
      delete mNotifier;
      mNotifier = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
/*StatusServer&
StatusServer::operator=(const StatusServer& rhs)
{
   if (this == &rhs)              // handle the assignment to self case
      return *this;

   return *this;
}*/

/* ============================ MANIPULATORS ============================== */
UtlBoolean 
StatusServer::handleMessage( OsMsg& eventMessage )
{
    syslog(FAC_SIP, PRI_DEBUG, "StatusServer::handleMessage() :: Start processing SIP message") ;

    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    if ( msgType == OsMsg::PHONE_APP && msgSubType == SipMessage::NET_SIP_MESSAGE )
    {
        const SipMessage* message = ((SipMessageEvent&)eventMessage).getMessage();
        UtlString callId;

        if ( message )
        {
            message->getCallIdField(&callId);
            UtlString method;
            message->getRequestMethod(&method);

            if ( !message->isResponse() )
            {
               if ( method.compareTo(SIP_SUBSCRIBE_METHOD) == 0 )
               {
                   //send to SubscribeThread
                   sendToSubscribeServerThread(eventMessage);
               } 
               else
               {
                   //send to redirect thread
                   //sendToRedirectServer(eventMessage);
               }
            } else // response processing
            {
                // get the method as we are only interested in 481 NOTIFY responses
                int cSequenceNum;
                if ( message->getCSeqField(&cSequenceNum, &method) && 
                     method.compareTo(SIP_NOTIFY_METHOD) == 0 ) 
                {   //send to SubscribeThread
                    sendToSubscribeServerThread(eventMessage);
                }
            }
        }
    }
    return(TRUE);
}

StatusServer* 
StatusServer::startStatusServer ( 
    const UtlString workingDir, 
    const char* configFileName )
{
    int httpPort  = PORT_NONE;
    int httpsPort = PORT_NONE;
    int tcpPort   = PORT_NONE;
    int udpPort   = PORT_NONE;
    int tlsPort   = PORT_NONE;
    UtlString defaultMaxExpiresTime;
    UtlString defaultMinExpiresTime;

    UtlString authAlgorithm;
    UtlString authQop;
    UtlString authRealm;
    UtlString authScheme;
    UtlString domainName;

    UtlBoolean isCredentialDB = TRUE;

    // if the configuration file exists, load the name value pairs
    if ( sConfigDb.loadFromFile(configFileName) == OS_SUCCESS )
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "Found config file: %s", configFileName);
    } else
    {
        sConfigDb.set("SIP_STATUS_AUTHENTICATE_ALGORITHM", "");
        sConfigDb.set("SIP_STATUS_AUTHENTICATE_QOP", "");
        sConfigDb.set("SIP_STATUS_AUTHENTICATE_REALM", "");
        // sConfigDb.set("SIP_STATUS_AUTHENTICATE_SCHEME", "");
        sConfigDb.set("SIP_STATUS_DOMAIN_NAME", "");
        // sConfigDb.set("SIP_STATUS_HTTP_AUTH_DB.", "");
        sConfigDb.set("SIP_STATUS_HTTP_PORT", "");
        sConfigDb.set("SIP_STATUS_HTTP_VALID_IPS", "");
        sConfigDb.set("SIP_STATUS_HTTPS_PORT", "");
        sConfigDb.set("SIP_STATUS_MAX_EXPIRES", "");
        sConfigDb.set("SIP_STATUS_MIN_EXPIRES", "");
        sConfigDb.set("SIP_STATUS_TCP_PORT", "5110");
        sConfigDb.set("SIP_STATUS_UDP_PORT", "5110");
        sConfigDb.set("SIP_STATUS_TLS_PORT", "5111");
        sConfigDb.set(CONFIG_SETTING_LOG_CONSOLE, "");
        sConfigDb.set(CONFIG_SETTING_LOG_DIR, "");
        sConfigDb.set(CONFIG_SETTING_LOG_LEVEL, "");
    
        if ( sConfigDb.storeToFile(configFileName) != OS_SUCCESS )
        {
            OsSysLog::add(FAC_SIP, PRI_INFO,
                          "Could not write config file: %s", configFileName);
        }
    }

    OsSysLog::add(LOG_FACILITY, PRI_INFO, "Starting - version: %s %s\n", SIPX_VERSION, SIPX_BUILD);

    sConfigDb.get("SIP_STATUS_AUTHENTICATE_ALGORITHM", authAlgorithm);
    sConfigDb.get("SIP_STATUS_AUTHENTICATE_QOP", authQop);
    sConfigDb.get("SIP_STATUS_AUTHENTICATE_REALM", authRealm);
    sConfigDb.get("SIP_STATUS_AUTHENTICATE_SCHEME", authScheme);
    sConfigDb.get("SIP_STATUS_DOMAIN_NAME", domainName);
    sConfigDb.get("SIP_STATUS_HTTP_PORT", httpPort);
    sConfigDb.get("SIP_STATUS_HTTPS_PORT", httpsPort);
    sConfigDb.get("SIP_STATUS_MAX_EXPIRES", defaultMaxExpiresTime);
    sConfigDb.get("SIP_STATUS_MIN_EXPIRES", defaultMinExpiresTime);

    // SIP_STATUS_UDP_PORT
    udpPort = sConfigDb.getPort("SIP_STATUS_UDP_PORT") ;
    if ( udpPort == PORT_NONE )
    {
       udpPort = 5110;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_STATUS_UDP_PORT : %d", udpPort);

    // SIP_STATUS_TCP_PORT
    tcpPort = sConfigDb.getPort("SIP_STATUS_TCP_PORT") ;
    if ( tcpPort == PORT_NONE )
    {
       tcpPort = 5110;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_STATUS_TCP_PORT : %d", tcpPort);

    // SIP_STATUS_TLS_PORT
    tlsPort = sConfigDb.getPort("SIP_STATUS_TLS_PORT") ;
    if ( tlsPort == PORT_NONE )
    {
       tlsPort = 5111;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_STATUS_TLS_PORT : %d", tlsPort);
    
    // Get the HTTP server authentication database
    UtlString separatedList;
    OsConfigDb* pUserPasswordDigestDb = new OsConfigDb();
    sConfigDb.get("SIP_STATUS_HTTP_AUTH_DB", separatedList);
    parseList ("SIP_STATUS_HTTP_AUTH_DB", separatedList, *pUserPasswordDigestDb );
    if( pUserPasswordDigestDb->isEmpty() )
    {
        delete pUserPasswordDigestDb;
        pUserPasswordDigestDb = NULL;
    }

    // Get the HTTP server Valid IP address database
    OsConfigDb* pValidIpAddressDB = new OsConfigDb();
    sConfigDb.get("SIP_STATUS_HTTP_VALID_IPS", separatedList);
    parseList ("SIP_STATUS_HTTP_VALID_IPS", separatedList, *pValidIpAddressDB);

    if( pValidIpAddressDB->isEmpty() )
    {
        delete pValidIpAddressDB;
        pValidIpAddressDB = NULL;
    }

    // Check for default values and, if not specified, set to defaults

    // SIP_STATUS_AUTHENTICATE_ALGORITHM
    if ( authAlgorithm.isNull() ) /* MD5/MD5SESS */
    {
        authAlgorithm.append("MD5"); 
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_AUTHENTICATE_ALGORITHM : %s", 
                  authAlgorithm.data());

    // SIP_STATUS_AUTHENTICATE_QOP
    if ( authQop.isNull() ) /* AUTH/AUTH-INT/NONE */
    {
        authQop.append("NONE"); 
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_AUTHENTICATE_QOP : %s", 
                  authQop.data());
    
    // SIP_STATUS_DOMAIN_NAME - need this before the SIP_STATUS_AUTHENTICATE_REALM
    // below since we get the domain name from the socket
    if ( domainName.isNull() )
    {
        OsSocket::getHostIp(&domainName);
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_DOMAIN_NAME : %s", 
                  domainName.data());
    
    // SIP_STATUS_AUTHENTICATE_REALM
    if(authRealm.isNull())
    {
        authRealm.append(domainName);
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_AUTHENTICATE_REALM : %s", 
                  authRealm.data());

    // SIP_STATUS_AUTHENTICATE_SCHEME (Hidden) NONE/DIGEST
    if ( authScheme.compareTo("NONE" , UtlString::ignoreCase) == 0 ) 
    {
        isCredentialDB = FALSE;
    }

    // SIP_STATUS_HTTPS_PORT - See if the SECURE one is set first
    OsStatus result;
    UtlString portStr;
    result = sConfigDb.get("SIP_STATUS_HTTPS_PORT", portStr);
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "startStatusServer : HTTPS port %s result %d", 
                  portStr.data(), result);
    // If the key is missing or not set, set it to the default HTTPS port
    if ( result == OS_NOT_FOUND || portStr.isNull() )
    {   
        httpsPort = HTTPS_SERVER_PORT;
    }

    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_STATUS_HTTPS_PORT : %d", httpsPort );

    // Only search for the non secure SIP_STATUS_HTTP_PORT 
    // if the secure one is disabled
    if ( httpsPort == PORT_NONE )
    {
        // SIP_STATUS_HTTP_PORT
        result = sConfigDb.get("SIP_STATUS_HTTP_PORT", portStr);
        OsSysLog::add(FAC_SIP, PRI_INFO,
                      "startStatusServer : HTTP port %s result %d", 
                      portStr.data(), result);
        // If the key is missing or not set, set it to the default HTTP port
        if ( result == OS_NOT_FOUND || portStr.isNull() )
        {
            httpPort = HTTP_SERVER_PORT;
        } 
        OsSysLog::add( FAC_SIP, PRI_INFO, "SIP_STATUS_HTTP_PORT : %d", httpPort );
    }

    int maxNumSrvRecords = -1;
    sConfigDb.get("SIP_STATUS_DNSSRV_MAX_DESTS", maxNumSrvRecords);
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_STATUS_DNSSRV_MAX_DESTS : %d",
              maxNumSrvRecords);
    // If explicitly set to a valid number
    if(maxNumSrvRecords > 0)
    {
        osPrintf("SIP_STATUS_DNSSRV_MAX_DESTS : %d\n", maxNumSrvRecords);
    }
    else
    {
        maxNumSrvRecords = 4;
    }

    int dnsSrvTimeout = -1; //seconds
    sConfigDb.get("SIP_STATUS_DNSSRV_TIMEOUT", dnsSrvTimeout);
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_STATUS_DNSSRV_TIMEOUT : %d",
              dnsSrvTimeout);
    // If explicitly set to a valid number
    if(dnsSrvTimeout > 0)
    {
        osPrintf("SIP_STATUS_DNSSRV_TIMEOUT : %d\n", dnsSrvTimeout);
    }
    else
    {
        dnsSrvTimeout = 4;
    }

    // SIP_STATUS_MAX_EXPIRES
    if ( defaultMaxExpiresTime.isNull() )
    {
        defaultMaxExpiresTime.append("604800"); // default to 1 week
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_MAX_EXPIRES : %s", 
                  defaultMaxExpiresTime.data());

    int maxExpiresTime = atoi(defaultMaxExpiresTime.data());

    // SIP_STATUS_MIN_EXPIRES
    if ( defaultMinExpiresTime.isNull() )
    {
        defaultMinExpiresTime.append("300");  // default to 300 seconds
    }
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SIP_STATUS_MIN_EXPIRES : %s", 
                  defaultMinExpiresTime.data());


    int webServerPort = portIsValid(httpsPort) ? httpsPort : httpPort;
    UtlBoolean isSecureServer = portIsValid(httpsPort);
    // Start the HTTP server
    HttpServer* httpServer = 
        initHttpServer(
            webServerPort,
            isSecureServer,
            authRealm,
            pUserPasswordDigestDb,
            pValidIpAddressDB );

    // Start the SIP stack
    SipUserAgent* sipUserAgent = 
        new SipUserAgent( 
            tcpPort, 
            udpPort,
            tlsPort,
            NULL,   // public IP address (nopt used in proxy)
            NULL,   // default user (not used in proxy)
            NULL,   // default SIP address (not used in proxy)
            NULL,   // outbound proxy
            NULL,   // directory server
            NULL,   // registry server
            NULL,   // auth scheme
            NULL,   // auth realm
            NULL,   // auth DB
            NULL,   // auth user IDs
            NULL,   // auth passwords
            NULL,   // nat ping URL
            0,      // nat ping frequency
            "PING", // nat ping method
            NULL,   // line mgr
            SIP_DEFAULT_RTT, // first resend timeout
            TRUE,   // default to UA transaction
            SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
            SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE // OsServerTask message queue size
         );

    // No need to log the message as it goes in the syslog
    //sipUserAgent->startMessageLog(100000);
    sipUserAgent->setDnsSrvTimeout(dnsSrvTimeout);
    sipUserAgent->setMaxSrvRecords(maxNumSrvRecords);
    sipUserAgent->setUserAgentHeaderProperty("sipX/publisher");
    
    sipUserAgent->start();

    Notifier* notifier = new Notifier(sipUserAgent);

    // Start the status server
    StatusServer* status = new StatusServer(
                notifier,
                maxExpiresTime,
                domainName,
                defaultMinExpiresTime,
                isCredentialDB,
                authAlgorithm,
                authQop,
                authRealm,
                workingDir,
                httpServer);
    status->start();
    return(status);
}


StatusServer* 
StatusServer::getInstance()
{
    // crit sec, ensure only one instance starts the status server
    sLock.acquire();

    if ( spInstance == NULL )
    {
        OsPath workingDirectory;
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

        UtlString fileName =  workingDirectory + 
            OsPathBase::separator +
            "status-config";

        spInstance = startStatusServer( 
            workingDirectory, 
            fileName );
    }

    // If the task object already exists, and the corresponding low-level task
    // has been started, then use it
    if ( !spInstance->isStarted() )
    {
        UtlBoolean isStarted = spInstance->start();
        assert(isStarted);
    }
    // release the critsec
    sLock.release();

    return spInstance;
}


OsConfigDb& StatusServer::getConfigDb()
{
   return sConfigDb;
}


SubscribePersistThread* StatusServer::getSubscribePersistThread()
{
   return mSubscribePersistThread;
}


SubscribeServerThread* StatusServer::getSubscribeServerThread()
{
   return mSubscribeServerThread;
}


/* ============================ ACCESSORS ================================= */
/////////////////////////////////////////////////////////////////////////////


/// Start the thread that periodically persists the subscription DB
void
StatusServer::startSubscribePersistThread()
{
   mSubscribePersistThread = new SubscribePersistThread(*this);
   mSubscribePersistThread->start();
}


void 
StatusServer::startSubscribeServerThread()
{
    UtlString localdomain;
    OsSocket::getHostIp(&localdomain);

    mSubscribeServerThread = new SubscribeServerThread(*this);
    if ( mSubscribeServerThread->initialize(
            mpSipUserAgent,
            mDefaultRegistryPeriod,
            mMinExpiresTime,
            mDefaultDomain,
            mIsCredentialDB,
            mRealm,
            &mPluginTable))
    {
        mSubscribeServerThreadQ = mSubscribeServerThread->getMessageQueue();
        mSubscribeThreadInitialized = TRUE;
    }
}

void 
StatusServer::sendToSubscribeServerThread(OsMsg& eventMessage)
{
    if ( mSubscribeThreadInitialized )
    {
        mSubscribeServerThreadQ->send(eventMessage);
    }
}

HttpServer* 
StatusServer::initHttpServer (
    int httpServerPort,
    const UtlBoolean& isSecureServer,
    const UtlString& authRealm,
    OsConfigDb* pUserPasswordDigestDb,
    OsConfigDb* pValidIpAddressDB )
{
    UtlString osBaseUriDirectory ;
    HttpServer* httpServer = NULL;

    OsPath workingDirectory;
    if ( OsFileSystem::exists( HTTP_SERVER_ROOT_DIR ) )
    {
        workingDirectory = HTTP_SERVER_ROOT_DIR;
        OsPath path(workingDirectory);
        path.getNativePath(workingDirectory);
    } else
    {
        OsPath path;
        OsFileSystem::getWorkingDirectory(path);
        path.getNativePath(workingDirectory);
    }

    osBaseUriDirectory =  workingDirectory + OsPathBase::separator;

    // Determine whether we should start the web server or not.  Use the port
    // value as the decision point.  A valid port means enable. 
    // If isSecureServer then start the port as a secure web server.
    // 
    if( portIsValid(httpServerPort) )
    {
        OsSysLog::add(FAC_SIP, PRI_INFO,
                      "Starting Embedded Web Server on port %d...",
                      httpServerPort);

        OsServerSocket *pServerSocket = NULL;  

        if ( isSecureServer )
        {
#ifdef HAVE_SSL
            pServerSocket = new OsSSLServerSocket(50, httpServerPort);
            httpServer = new HttpServer(
                pServerSocket, 
                pUserPasswordDigestDb,
                authRealm, 
                pValidIpAddressDB,
                false /* no persistent tcp connection */
                                        );
#else /* ! HAVE_SSL */
            // SSL is not configured in, so we cannot open the requested
            // socket.
            OsSysLog::add(FAC_SIP, PRI_CRIT,
                          "StatusServer::initHttpServer SSL not configured; "
                          "cannot open secure socket %d", httpServerPort);
            httpServer = NULL;
#endif /* HAVE_SSL */
        }
	else
        {
            pServerSocket = new OsServerSocket(50, httpServerPort);
            httpServer = new HttpServer(
                pServerSocket, 
                pUserPasswordDigestDb,
                authRealm, 
                pValidIpAddressDB );
        }
        if (httpServer)
        {
           // Set the web server root to the current directory
           httpServer->addUriMap( "/", osBaseUriDirectory.data() );
        }
    }

    if( httpServer )
    {   // Method to post files to WebServer for processing MWI events
        httpServer->start();
    }
    return httpServer;
}

void 
StatusServer::parseList (
    const UtlString& keyPrefix, 
    const UtlString& separatedList, 
    OsConfigDb& list )
{
    if (!separatedList.isNull())
    {
        int index = 1;
        UtlString value;
        while( NameValueTokenizer::getSubField( separatedList.data(), index, ", \t", &value) )
        {
            char temp[10];
            sprintf(temp, "%d", index);
            UtlString key = temp;
            list.set(key, value);
            index ++;
        }
    }
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */