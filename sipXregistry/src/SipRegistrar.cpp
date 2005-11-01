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
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "utl/PluginHooks.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "net/SipUserAgent.h"
#include "SipRegistrar.h"
#include "registry/RegisterPlugin.h"
#include "SipRedirectServer.h"
#include "SipRegistrarServer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CONFIG_ETC_DIR SIPX_CONFDIR

#define CONFIG_SETTING_LOG_LEVEL      "SIP_REGISTRAR_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_REGISTRAR_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_REGISTRAR_LOG_DIR"

const char* RegisterPlugin::Prefix  = "SIP_REGISTRAR";
const char* RegisterPlugin::Factory = "getRegisterPlugin";

// STATIC VARIABLE INITIALIZATIONS
SipRegistrar* SipRegistrar::spInstance = NULL;
OsBSem SipRegistrar::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipRegistrar::SipRegistrar(SipUserAgent* sipUserAgent,
                           PluginHooks* sipRegisterPlugins,
                           int maxExpiresTime,
                           const UtlString& defaultDomain,
                           const UtlString& domainAliases,
                           const UtlString& defaultMinExpiresTime,
                           const UtlBoolean& useCredentialDB,
                           const UtlString& defaultAuthAlgorithm,
                           const UtlString& defaultAuthQop,
                           const UtlString& defaultRealm,
                           const UtlString& configDir,
                           const UtlString& mediaServer,
                           const UtlString& voicemailServer,
                           const char* configFileName):
OsServerTask("SipRegistrarMain"),
mRedirectServer(NULL),
mRedirectMsgQ(NULL),
mRedirectThreadInitialized(FALSE),
mRegistrarThreadInitialized(FALSE),
mSipRegisterPlugins(sipRegisterPlugins)
{
   mConfigDirectory.remove(0);
   mConfigDirectory.append(configDir);

   mDefaultRegistryPeriod = maxExpiresTime;
   mSipUserAgent = sipUserAgent;
   if ( sipUserAgent )
   {
      sipUserAgent->addMessageConsumer(this);
      sipUserAgent->allowMethod(SIP_REGISTER_METHOD);
      sipUserAgent->allowMethod(SIP_NOTIFY_METHOD);
      sipUserAgent->allowMethod(SIP_SUBSCRIBE_METHOD);
      sipUserAgent->allowMethod(SIP_INVITE_METHOD);
      sipUserAgent->allowMethod(SIP_INFO_METHOD);
      sipUserAgent->allowMethod(SIP_OPTIONS_METHOD);
      sipUserAgent->allowMethod(SIP_BYE_METHOD);
      sipUserAgent->allowMethod(SIP_ACK_METHOD);
      sipUserAgent->allowMethod(SIP_REFER_METHOD);
      sipUserAgent->allowMethod(SIP_CANCEL_METHOD);

      sipUserAgent->allowExtension("sip-cc-01");
      sipUserAgent->allowExtension("timer");
      sipUserAgent->allowExtension("gruu");
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

   if ( !domainAliases.isNull() )
   {
      mDomainAliases.remove(0);
      mDomainAliases.append(domainAliases);
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

   if ( !mediaServer.isNull() )
   {
      mMediaServer.remove(0);
      mMediaServer.append(mediaServer);
   }

   if ( !voicemailServer.isNull() )
   {
      mVoicemailServer.remove(0);
      mVoicemailServer.append(voicemailServer);
   }

   Url domain(mDefaultDomain);
   domain.getHostAddress(mlocalDomainHost);

   mIsCredentialDB = useCredentialDB;

   startRedirectServer(configFileName);
   startRegistrarServer();
}

// Destructor
SipRegistrar::~SipRegistrar()
{
    // Wait for the owned servers to shutdown first
    if ( mRedirectServer )
    {
        // Deleting a server task is the only way of
        // waiting for shutdown to complete cleanly
        mRedirectServer->requestShutdown();
        delete mRedirectServer;
        mRedirectServer = NULL;
        mRedirectMsgQ = NULL;
    }

    if ( mRegistrarServer )
    {
        // Deleting a server task is the only way of
        // waiting for shutdown to complete cleanly
        mRegistrarServer->requestShutdown();
        delete mRegistrarServer;
        mRegistrarServer = NULL;
        mRegistrarMsgQ = NULL;
    }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
/*SipRegistrar&
SipRegistrar::operator=(const SipRegistrar& rhs)
{
   if (this == &rhs)              // handle the assignment to self case
      return *this;

   return *this;
}*/

/* ============================ MANIPULATORS ============================== */
UtlBoolean SipRegistrar::handleMessage( OsMsg& eventMessage )
{
    syslog(FAC_SIP, PRI_DEBUG, "SipRegistrar::handleMessage() :: Start processing SIP message\n") ;

    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    if ( (msgType == OsMsg::PHONE_APP) &&
         (msgSubType == SipMessage::NET_SIP_MESSAGE) )
    {
        const SipMessage* message =
           ((SipMessageEvent&)eventMessage).getMessage();
        UtlString callId;
        if ( message )
        {
            message->getCallIdField(&callId);
            UtlString method;
            message->getRequestMethod(&method);

            if ( !message->isResponse() )
            {
                // osPrintf("SipRegistrar::handleMessage() Message is Authorized\n");
                if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
                {
                    //send to Register Thread
                    sendToRegistrarServer(eventMessage);
                }
                else
                {
                    //send to redirect thread
                    sendToRedirectServer(eventMessage);
                }
            }
        }
    }
    return(TRUE);
}

SipRegistrar*
SipRegistrar::startRegistrar(
    const UtlString workingDir,
    const char* configFileName )
{
    int tcpPort = PORT_DEFAULT;
    int udpPort = PORT_DEFAULT;
    int tlsPort = PORT_DEFAULT;
    UtlString defaultMaxExpiresTime;
    UtlString defaultMinExpiresTime;

    UtlString domainName;
    UtlString domainAliases;
    UtlString authenticateScheme;
    UtlString authAlgorithm;
    UtlString authQop;
    UtlString realm;
    UtlString mediaServer;
    UtlString voicemailServer;

    UtlBoolean isCredentialDB = TRUE;

    OsConfigDb configDb;

    if ( configDb.loadFromFile(configFileName) == OS_SUCCESS )
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "Found config file: %s", configFileName);
        //osPrintf("Found config file: %s\n", configFileName);
    }
    else
    {
        configDb.set("SIP_REGISTRAR_UDP_PORT", "5070");
        configDb.set("SIP_REGISTRAR_TCP_PORT", "5070");
        configDb.set("SIP_REGISTRAR_TLS_PORT", "5071");
        configDb.set("SIP_REGISTRAR_MAX_EXPIRES", "");
        configDb.set("SIP_REGISTRAR_MIN_EXPIRES", "");
        configDb.set("SIP_REGISTRAR_DOMAIN_NAME", "");
        //configDb.set("SIP_REGISTRAR_AUTHENTICATE_SCHEME", "");
        configDb.set("SIP_REGISTRAR_AUTHENTICATE_ALGORITHM", "");
        configDb.set("SIP_REGISTRAR_AUTHENTICATE_QOP", "");
        configDb.set("SIP_REGISTRAR_AUTHENTICATE_REALM", "");

        configDb.set("SIP_REGISTRAR_MEDIA_SERVER", "");
        configDb.set("SIP_REGISTRAR_VOICEMAIL_SERVER", "");
        configDb.set(CONFIG_SETTING_LOG_DIR, "");
        configDb.set(CONFIG_SETTING_LOG_LEVEL, "");
        configDb.set(CONFIG_SETTING_LOG_CONSOLE, "");

        if ( configDb.storeToFile(configFileName) == OS_SUCCESS )
        {
            OsSysLog::add( FAC_SIP, PRI_INFO,
                           "Could not write config file: %s",
                           configFileName);
        }
    }

    udpPort = configDb.getPort("SIP_REGISTRAR_UDP_PORT");
    if (udpPort == PORT_DEFAULT)
    {
       udpPort = 5070;
    }
  
    tcpPort = configDb.getPort("SIP_REGISTRAR_TCP_PORT");
    if (tcpPort == PORT_DEFAULT)
    {
       tcpPort = 5070;
    }

    tlsPort = configDb.getPort("SIP_REGISTRAR_TLS_PORT");
    if (tlsPort == PORT_DEFAULT)
    {
       tlsPort = 5071;
    }

    configDb.get("SIP_REGISTRAR_MAX_EXPIRES", defaultMaxExpiresTime);
    configDb.get("SIP_REGISTRAR_MIN_EXPIRES", defaultMinExpiresTime);

    configDb.get("SIP_REGISTRAR_DOMAIN_NAME", domainName);
    configDb.get("SIP_REGISTRAR_DOMAIN_ALIASES", domainAliases);

    configDb.get("SIP_REGISTRAR_AUTHENTICATE_SCHEME", authenticateScheme);
    configDb.get("SIP_REGISTRAR_AUTHENTICATE_ALGORITHM", authAlgorithm);
    configDb.get("SIP_REGISTRAR_AUTHENTICATE_QOP", authQop);
    configDb.get("SIP_REGISTRAR_AUTHENTICATE_REALM", realm);

    configDb.get("SIP_REGISTRAR_MEDIA_SERVER", mediaServer);
    configDb.get("SIP_REGISTRAR_VOICEMAIL_SERVER", voicemailServer);
    //configDb.get("SIP_REGISTRAR_LOCAL_DOMAIN", localDomain);

    if ( defaultMinExpiresTime.isNull() )
    {
        defaultMinExpiresTime.append("300"); //300 seconds
    }
    osPrintf("SIP_REGISTRAR_MIN_EXPIRES : %s\n", defaultMinExpiresTime.data());

    if ( defaultMaxExpiresTime.isNull() )
    {
        defaultMaxExpiresTime.append("7200"); //default to 2 hrs
    }
    osPrintf("SIP_REGISTRAR_MAX_EXPIRES : %s\n", defaultMaxExpiresTime.data());

    if(domainName.isNull())
    {
       OsSocket::getHostIp(&domainName);
    }
    osPrintf("SIP_REGISTRAR_DOMAIN_NAME : %s\n", domainName.data());


    if ( authenticateScheme.compareTo("NONE" , UtlString::ignoreCase) == 0 ) /* NONE/DIGEST */
    {
        isCredentialDB = FALSE;
        osPrintf("SIP_AUTHPROXY_AUTHENTICATE_SCHEME : NONE\n");
    }

    if ( authAlgorithm.isNull() ) /* MD5/MD5SESS */
    {
        authAlgorithm.append("MD5");
    }
    osPrintf("SIP_REGISTRAR_AUTHENTICATE_ALGORITHM : %s\n", authAlgorithm.data());

    if ( authQop.isNull() ) /* AUTH/AUTH-INT/NONE */
    {
        authQop.append("NONE");
    }
    osPrintf("SIP_REGISTRAR_AUTHENTICATE_QOP : %s\n", authQop.data());

    if(realm.isNull())
    {
        realm.append(domainName);
    }
    osPrintf("SIP_REGISTRAR_AUTHENTICATE_REALM : %s\n", realm.data());

    if(mediaServer.isNull())
    {
        OsSocket::getHostIp(&mediaServer);
        mediaServer.append(":");
        mediaServer.append("5100");
    }
    osPrintf("SIP_REGISTRAR_MEDIA_SERVER : %s\n", mediaServer.data());

    if(voicemailServer.isNull())
    {
        OsSocket::getHostIp(&voicemailServer);
        voicemailServer.append(":");
        voicemailServer.append("8090");
    }
    osPrintf("SIP_REGISTRAR_VOICEMAIL_SERVER: %s\n", voicemailServer.data());

    int maxExpiresTime = atoi(defaultMaxExpiresTime.data());

    // Start the sip stack
    SipUserAgent* sipUserAgent = new SipUserAgent(tcpPort,
        udpPort,
        tlsPort,
        NULL,   // public IP address (not used in proxy)
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

    PluginHooks* sipRegisterPlugins = new PluginHooks( RegisterPlugin::Factory
                                                      ,RegisterPlugin::Prefix
                                                      );
    sipRegisterPlugins->readConfig( configDb );

    sipUserAgent->start();

    if(domainAliases.isNull())
    {
       UtlString ipAddress;
       OsSocket::getHostIp(&ipAddress);
       domainAliases = ipAddress;
       char portBuf[20];
       sprintf(portBuf, ":%d", sipUserAgent->getUdpPort());
       domainAliases.append(portBuf);
    }
    osPrintf("SIP_REGISTRAR_DOMAIN_ALIASES : %s\n", domainAliases.data());
    // Start the registrar.
    SipRegistrar* registrar =
        new SipRegistrar(
            sipUserAgent,
            sipRegisterPlugins,
            maxExpiresTime ,
            domainName,
            domainAliases,
            defaultMinExpiresTime,
            isCredentialDB,
            authAlgorithm,
            authQop,
            realm,
            workingDir,
            mediaServer,
            voicemailServer,
            configFileName);

    registrar->start();

    return(registrar);
}

void
SipRegistrar::printMessageLog()
{
   UtlString buffer;
   mSipUserAgent->printStatus();
   mSipUserAgent->getMessageLog(buffer);
   printf("=================>\n%s\n", buffer.data());
}

SipRegistrar*
SipRegistrar::getInstance()
{
    // If the task does not yet exist or hasn't been started, then acquire
    // the lock to ensure that only one instance of the task is started
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
           "registrar-config";

        spInstance = startRegistrar(workingDirectory, fileName);
    }

    UtlBoolean isStarted = spInstance->isStarted();
    if ( !isStarted )
    {
        isStarted = spInstance->start();
        assert( isStarted );
    }
    sLock.release();

    return spInstance;
}

/* ============================ ACCESSORS ================================= */
/////////////////////////////////////////////////////////////////////////////
void
SipRegistrar::startRedirectServer(const char* configFileName)
{
   UtlString localdomain;
   OsSocket::getHostIp(&localdomain);

   mRedirectServer = new SipRedirectServer();
   if ( mRedirectServer->initialize(mSipUserAgent,
                                    mConfigDirectory,
                                    mMediaServer,
                                    mVoicemailServer,
                                    mlocalDomainHost,
                                    configFileName) )
   {
      mRedirectMsgQ = mRedirectServer->getMessageQueue();
      mRedirectThreadInitialized = TRUE;
   }
}

void
SipRegistrar::startRegistrarServer()
{
    mRegistrarServer = new SipRegistrarServer();
    if ( mRegistrarServer->initialize(mSipUserAgent,
                                      mSipRegisterPlugins,
                                      mDefaultRegistryPeriod,
                                      mMinExpiresTime,
                                      mDefaultDomain,
                                      mDomainAliases,
                                      mIsCredentialDB,
                                      mRealm) )
    {
        mRegistrarMsgQ = mRegistrarServer->getMessageQueue();
        mRegistrarThreadInitialized = TRUE;
    }
}

void
SipRegistrar::sendToRedirectServer(OsMsg& eventMessage)
{
    if ( mRedirectThreadInitialized )
    {
        mRedirectMsgQ->send(eventMessage);
    }
}

void
SipRegistrar::sendToRegistrarServer(OsMsg& eventMessage)
{
    if ( mRegistrarThreadInitialized )
    {
        mRegistrarMsgQ->send(eventMessage);
    }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
