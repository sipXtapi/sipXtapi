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
#include <net/SipTlsServer.h>
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#ifdef SIP_TLS
#    include <os/OsSSLServerSocket.h>
#    include <os/OsSSLConnectionSocket.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_PRINT
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipTlsServer::SipTlsServer(int port, SipUserAgent* userAgent) :
 SipTcpServer(-1, userAgent, SIP_TRANSPORT_TLS, "SipTlsServer-%d")
{
   OsSysLog::add(FAC_SIP,PRI_DEBUG,"SipTlsServer::~ port %d", port);
   
   if(port >= 0)
   {
#ifdef SIP_TLS
        mServerSocket = new OsSSLServerSocket(64, port);
#else
        mServerSocket = new OsServerSocket(64, port);
#endif
        mServerPort = mServerSocket->getLocalHostPort() ;
    }
    else
    {
        mServerSocket = NULL;
        mServerPort = -1 ;
    }

   mDefaultPort = SIP_TLS_PORT;

}


// Destructor
SipTlsServer::~SipTlsServer()
{
    if(mServerSocket) delete mServerSocket;
    mServerSocket = NULL;
}

/* ============================ MANIPULATORS ============================== */

OsSocket* SipTlsServer::buildClientSocket(int hostPort, const char* hostAddress)
{
    OsSocket* socket;
#ifdef SIP_TLS
    socket = new OsSSLConnectionSocket(hostPort, hostAddress);
#else
    // Create the socket in non-blocking mode so it does not block
    // while conecting
    socket = new OsConnectionSocket(hostPort, hostAddress, FALSE);
#endif

   socket->makeBlocking();
   return(socket);
}

/* ============================ ACCESSORS ================================= */

// The the local server port for this server
int SipTlsServer::getServerPort() const 
{
    return mServerPort ;
}

/* ============================ INQUIRY =================================== */
UtlBoolean SipTlsServer::isOk()
{
        return mServerSocket->isOk();
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

