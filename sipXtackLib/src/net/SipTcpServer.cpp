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

// APPLICATION INCLUDES
#include <net/SipTcpServer.h>
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_PRINT
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipTcpServer::SipTcpServer(int port, SipUserAgent* userAgent,
                           const char* protocolString, 
                           const char* taskName) :
 SipProtocolServerBase(userAgent, protocolString, taskName)
{
   mServerPort = port;
   if(port > 0)
   {
       mServerSocket = new OsServerSocket(64, port);
   }
   else
   {
       mServerSocket = NULL;
   }

   mDefaultPort = SIP_PORT;

}

// Copy constructor
SipTcpServer::SipTcpServer(const SipTcpServer& rSipTcpServer) :
    SipProtocolServerBase(NULL, SIP_TRANSPORT_TCP, "SipTcpServer-%d")
{
}

// Destructor
SipTcpServer::~SipTcpServer()
{
    if(mServerSocket) delete mServerSocket;
    mServerSocket = NULL;
}

/* ============================ MANIPULATORS ============================== */

int SipTcpServer::run(void* runArgument)
{
    OsConnectionSocket* clientSocket = NULL;
    UtlBoolean clientStarted;
    SipClient* client = NULL;

    if(mServerSocket == NULL)
    {
        osPrintf("Sip%sServer NULL server socket\n", mProtocolString.data() );
        assert( mServerSocket );
    }
    else if(!mServerSocket->isOk())
    {
        int sipTcpPortIsBusyPhoneMayAlreadyBeRunning = TRUE;
        osPrintf("Failed to bind to port: %d\n  Another process using this port?\n",
            mServerPort);
        assert(!sipTcpPortIsBusyPhoneMayAlreadyBeRunning);
    }
    else
    {
        osPrintf("SIP Server binding to %s port %d\n", 
            mProtocolString.data(), mServerPort);
    }

    while(!isShuttingDown() &&
        mServerSocket->isOk())
    {
        clientSocket = mServerSocket->accept();
#ifdef TEST_PRINT
        osPrintf("SIP Server accepting %d connection on server port %d\n",
            mProtocolString.data(), mServerPort);
#endif

        if(clientSocket)
        {
            client = new SipClient(clientSocket);
            if(mSipUserAgent)
            {
                client->setUserAgent(mSipUserAgent);
            }

            UtlString hostAddress;
            int hostPort;
            clientSocket->getRemoteHostIp(&hostAddress, &hostPort);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::run client: %p %s:%d\n",
                mProtocolString.data(), client, hostAddress.data(), hostPort);

            clientStarted = client->start();
            if(!clientStarted)
            {
                osPrintf("SIP %s Client failed to start", mProtocolString.data());
            }
            addClient(client);
        }
    }

    return(0);
}

UtlBoolean SipTcpServer::startListener()
{
   start();

    // For each client start listening

   startClients();
    return(TRUE);
}

void SipTcpServer::shutdownListener()
{
    osPrintf("Sip%sServer::shutdownListener() - before requestShutDown\n",
        mProtocolString.data());
    requestShutdown();

    shutdownClients();
}


OsSocket* SipTcpServer::buildClientSocket(int hostPort, const char* hostAddress)
{
    // Create a socket in non-blocking mode while connecting
    OsConnectionSocket* socket = new OsConnectionSocket(hostPort, hostAddress, FALSE);
    socket->makeBlocking();
    return(socket);
}

// Assignment operator
SipTcpServer&
SipTcpServer::operator=(const SipTcpServer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */



/* ============================ INQUIRY =================================== */
UtlBoolean SipTcpServer::isOk()
{
    return mServerSocket->isOk();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

