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
#include "os/OsBSem.h"
#include "os/OsFS.h"
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "utl/PluginHooks.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "net/SipUserAgent.h"
#include "sipdb/RegistrationDB.h"
#include "SipRegistrar.h"
#include "registry/RegisterPlugin.h"
#include "SipRedirectServer.h"
#include "SipRegistrarServer.h"
#include "RegistrarPeer.h"
#include "RegistrarTest.h"
#include "RegistrarSync.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

#define CONFIG_SETTING_LOG_LEVEL      "SIP_REGISTRAR_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_REGISTRAR_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_REGISTRAR_LOG_DIR"

#define REGISTRAR_DEFAULT_RPC_PORT  5077
#define REGISTRAR_DEFAULT_SIP_PORT  5070
#define REGISTRAR_DEFAULT_SIPS_PORT 5071

const char* RegisterPlugin::Prefix  = "SIP_REGISTRAR";
const char* RegisterPlugin::Factory = "getRegisterPlugin";

// STATIC VARIABLE INITIALIZATIONS
SipRegistrar* SipRegistrar::spInstance = NULL;
OsBSem SipRegistrar::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipRegistrar::SipRegistrar(OsConfigDb* configDb, const char* configFile) :
   OsServerTask("SipRegistrarMain", NULL, SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE),
   mConfigDb(configDb),
   mConfigFileName(configFile),
   mHttpServer(NULL),
   mXmlRpcDispatcher(NULL),
   mReplicationConfigured(false),
   mInitialSyncThread(NULL),
   mSipUserAgent(NULL),
   mRedirectServer(NULL),
   mRedirectMsgQ(NULL),
   mRegistrarServer(NULL),
   mRegistrarMsgQ(NULL)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRegistrar:: constructed."
                 ) ;

   configurePeers();
}

int SipRegistrar::run(void* pArg)
{
   startRpcServer();

   reloadPersistentRegistrations();

   /*
    * If replication is configured,
    *   the following blocks until the state of each peer is known
    */
   startupPhase(); 
 
   operationalPhase();

   int taskResult = OsServerTask::run(pArg);

   if (mRegistrationDb)
   {
      mRegistrationDb->releaseInstance();
      mRegistrationDb = NULL;
   }

   return taskResult;
}


/// Load persistent soft state
void SipRegistrar::reloadPersistentRegistrations()
{
   mRegistrationDb = RegistrationDB::getInstance();
}


/// Launch all Startup Phase threads.
void SipRegistrar::startupPhase()
{
   OsSysLog::add(FAC_SIP, PRI_INFO, "SipRegistrar entering startup phase");

   if (mReplicationConfigured)
   {
      // :TODO: Begin the RegistrarInitialSync thread and then wait for it.
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "SipRegistrar::startupPhase no replication configured"
                    );
   }
}

/// Launch all Operational Phase threads.
void SipRegistrar::operationalPhase()
{
   OsSysLog::add(FAC_SIP, PRI_INFO, "SipRegistrar entering operational phase");

   // Start the sip stack
   int tcpPort = PORT_DEFAULT;
   int udpPort = PORT_DEFAULT;
   int tlsPort = PORT_DEFAULT;

   udpPort = mConfigDb->getPort("SIP_REGISTRAR_UDP_PORT");
   if (udpPort == PORT_DEFAULT)
   {
      udpPort = REGISTRAR_DEFAULT_SIP_PORT;
   }
  
   tcpPort = mConfigDb->getPort("SIP_REGISTRAR_TCP_PORT");
   if (tcpPort == PORT_DEFAULT)
   {
      tcpPort = REGISTRAR_DEFAULT_SIP_PORT;
   }

   tlsPort = mConfigDb->getPort("SIP_REGISTRAR_TLS_PORT");
   if (tlsPort == PORT_DEFAULT)
   {
      tlsPort = REGISTRAR_DEFAULT_SIPS_PORT;
   }

   mSipUserAgent = new SipUserAgent(tcpPort,
                                    udpPort,
                                    tlsPort,
                                    NULL,   // public IP address (not used)
                                    NULL,   // default user (not used)
                                    NULL,   // default SIP address (not used)
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
                                    SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE, // OsServerTask message queue size
                                    FALSE,  // use next available port                                                  
                                    FALSE   // do not do UA message checks for METHOD, requires, etc...
                                    );

   if ( mSipUserAgent )
   {
      mSipUserAgent->addMessageObserver( *this->getMessageQueue(), NULL /* all methods */ );

      // the above causes us to actually receive all methods
      // the following sets what we send in Allow headers
      mSipUserAgent->allowMethod(SIP_REGISTER_METHOD);
      mSipUserAgent->allowMethod(SIP_SUBSCRIBE_METHOD);
      mSipUserAgent->allowMethod(SIP_OPTIONS_METHOD);
      mSipUserAgent->allowMethod(SIP_CANCEL_METHOD);

      mSipUserAgent->allowExtension("gruu"); // should be moved to gruu processor?
   }

   mSipUserAgent->start();

   startRegistrarServer();

   startRedirectServer();
}

/// Get the RegistrarTest thread object
RegistrarTest* SipRegistrar::getRegistrarTest()
{
   return mRegistrarTest;
}

/// Get the RegistrarSync thread object
RegistrarSync* SipRegistrar::getRegistrarSync()
{
   return mRegistrarSync;
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

UtlBoolean SipRegistrar::handleMessage( OsMsg& eventMessage )
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRegistrar::handleMessage()"
                  " Start processing SIP message") ;

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

            if ( !message->isResponse() ) // is a request ?
            {
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
            else
            {
               // responses are ignored.
            }
        }
        else
        {
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipRegistrar::handleMessage no message."
                         ) ;
        }
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipRegistrar::handleMessage unexpected message type %d/%d",
                     msgType, msgSubType
                     ) ;
    }
    
    return(TRUE);
}

SipRegistrar*
SipRegistrar::getInstance(OsConfigDb* configDb, const char* configFile)
{
    OsLock singletonLock(sLock);

    if ( spInstance == NULL )
    {
       OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRegistrar::getInstance(%p, %s)",
                     configDb, configFile);
       
       spInstance = new SipRegistrar(configDb, configFile);
    }

    UtlBoolean isStarted = spInstance->isStarted();
    if ( !isStarted )
    {
        isStarted = spInstance->start();
        assert( isStarted );
    }

    return spInstance;
}

/// Read peer configuration and initialize peer state
void SipRegistrar::configurePeers()
{
   UtlString myName;
   mConfigDb->get("SIP_REGISTRAR_NAME", mPrimaryName);

   UtlString peerNames;
   mConfigDb->get("SIP_REGISTRAR_SYNC_WITH", peerNames);

   if (!mPrimaryName.isNull() && !peerNames.isNull())
   {
      // :TODO: initialize mPeers by instantiating each RegisterPeer not myself.
      // if all is well:
      //   mReplicationConfigured = true;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRegistrar::configurePeers - no peers configured"
                    );
   }
}

    
/// If replication configured, name of this registrar as primary, else NULL
const char* SipRegistrar::primaryName()
{
   return (  mReplicationConfigured
           ? mPrimaryName.data()
           : NULL
           );
}


/// Server for XML-RPC requests
void SipRegistrar::startRpcServer()
{
   // Begins operation of the HTTP/RPC service
   // sets mHttpServer and mXmlRpcDispatcher

   int httpPort = mConfigDb->getPort("SIP_REGISTRAR_XMLRPC_PORT");
   if (PORT_NONE == httpPort)
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SipRegistrar::startRpcServer"
                    " SIP_REGISTRAR_XMLRPC_PORT == PORT_NONE :"
                    " peer synchronization disabled"
                    );
      mReplicationConfigured = false; // force all replication interfaces off
   }
   else
   {
      if (PORT_DEFAULT == httpPort)
      {
         httpPort = REGISTRAR_DEFAULT_RPC_PORT;
      }

      // :TODO: initialize mHttpServer and mXmlRpcDispatcher
   }
}

/// Get an iterator over all peers.
UtlSListIterator* SipRegistrar::getPeers()
{
   return (  ( ! mReplicationConfigured || mPeers.isEmpty() )
           ? NULL : new UtlSListIterator(mPeers));
}

/// Get peer state object by name.
RegistrarPeer* SipRegistrar::getPeer(const UtlString& peerName)
{
   return (  mReplicationConfigured
           ? dynamic_cast<RegistrarPeer*>(mPeers.find(&peerName))
           : NULL
           );
}


/* ============================ ACCESSORS ================================= */
/////////////////////////////////////////////////////////////////////////////
void
SipRegistrar::startRedirectServer()
{
   mRedirectServer = new SipRedirectServer(mConfigDb, mSipUserAgent);
   mRedirectMsgQ = mRedirectServer->getMessageQueue();
   mRedirectServer->start();
}

void
SipRegistrar::startRegistrarServer()
{
    mRegistrarServer = new SipRegistrarServer(mConfigDb, mSipUserAgent);
    mRegistrarMsgQ = mRegistrarServer->getMessageQueue();
    mRegistrarServer->start();
}

void
SipRegistrar::sendToRedirectServer(OsMsg& eventMessage)
{
    if ( mRedirectMsgQ )
    {
        mRedirectMsgQ->send(eventMessage);
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_CRIT, "sendToRedirectServer - queue not initialized.");
    }
}

void
SipRegistrar::sendToRegistrarServer(OsMsg& eventMessage)
{
    if ( mRegistrarMsgQ )
    {
        mRegistrarMsgQ->send(eventMessage);
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_CRIT, "sendToRegistrarServer - queue not initialized.");
    }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
