//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>

#if defined(_WIN32)
#   include <winsock.h>
#undef OsSS_CONST
#define OsSS_CONST const
#elif defined(_VXWORKS)
#   include <inetLib.h>
#   include <sockLib.h>
#   include <unistd.h>
#elif defined(__pingtel_on_posix__)
#undef OsSS_CONST
#define OsSS_CONST const
#   include <netinet/in.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   include <resolv.h>
#else
#error Unsupported target platform.
#endif

// APPLICATION INCLUDES
#include <os/OsServerSocket.h>
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

#define SOCKET_LEN_TYPE
#ifdef __pingtel_on_posix__
#undef SOCKET_LEN_TYPE
#define SOCKET_LEN_TYPE (socklen_t *)
#endif

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsServerSocket::OsServerSocket(int connectionQueueSize,
    int serverPort,
    const char* szBindAddr,
    const bool bPerformBind)
{
   const int one = 1;
   int error = 0;
   socketDescriptor = 0;
   struct sockaddr_in localAddr;
   int addrSize;

   // Windows specific startup
   if(!OsSocket::socketInit())
   {
      goto EXIT;
   }

   localHostPort = serverPort;

   // Create the socket
   socketDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(socketDescriptor == OS_INVALID_SOCKET_DESCRIPTOR)
   {
      error = OsSocketGetERRNO();
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsServerSocket: socket call failed with error: %d=0x%x\n",
         error, error);
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      goto EXIT;
   }

#ifndef WIN32
   if(setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)))
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsServerSocket: setsockopt(SO_REUSEADDR) failed!\n");
#endif
    setsockopt(socketDescriptor, SOL_SOCKET, SO_DONTROUTE, (char *)&one, sizeof(one)) ;

#  if defined(__MACH__)
   // Under OS X, we use SO_NOSIGPIPE here because MSG_NOSIGNAL
   // is not supported for the write() call.
   if(setsockopt(socketDescriptor, SOL_SOCKET, SO_NOSIGPIPE, (char *)&one, sizeof(one)))
   {
      error = OsSocketGetERRNO();
      close();
      perror("call to setsockopt failed in OsServerSocket::OsServerSocket\n");
      OsSysLog::add(FAC_SIP, PRI_ERR, "setsockopt call failed with error: 0x%x in OsServerSocket::OsServerSocket\n", error);
      goto EXIT;
   }
#       endif

   localAddr.sin_family = AF_INET;

   // Bind to a specific server port if given, or let the system pick
   // any available port number if PORT_DEFAULT.
   localAddr.sin_port = htons((PORT_DEFAULT == serverPort) ? 0 : serverPort);

#ifndef _DISABLE_MULTIPLE_INTERFACE_SUPPORT
   // Allow IP in on any of this host's addresses or NICs.
   if (szBindAddr)
   {
      localAddr.sin_addr.s_addr = inet_addr (szBindAddr);
      mLocalIp = szBindAddr;
   }
   else
   {
      localAddr.sin_addr.s_addr=OsSocket::getDefaultBindAddress();
      mLocalIp = inet_ntoa(localAddr.sin_addr);
   }
#else
   localAddr.sin_addr.s_addr=htonl(INADDR_ANY); // Allow IP in on
   mLocalIp = inet_ntoa(localAddr.sin_addr);
#endif

   if (bPerformBind)
   {
        error = bind(socketDescriptor,
                        (OsSS_CONST struct sockaddr*) &localAddr,
                        sizeof(localAddr));
        if (error == OS_INVALID_SOCKET_DESCRIPTOR)
        {
            error = OsSocketGetERRNO();
            OsSysLog::add(FAC_KERNEL, PRI_ERR,
                            "OsServerSocket:  bind to port %d failed with error: %d = 0x%x\n",
                            ((PORT_DEFAULT == serverPort) ? 0 : serverPort), error, error);
            socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
            goto EXIT;
        }
   }
   addrSize = sizeof(struct sockaddr_in);
   error = getsockname(socketDescriptor,
                           (struct sockaddr*) &localAddr, SOCKET_LEN_TYPE &addrSize);
   if (error) {
      error = OsSocketGetERRNO();
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsServerSocket: getsockname call failed with error: %d=0x%x\n",
         error, error);
   } else {
      localHostPort = htons(localAddr.sin_port);
   }

    // Setup the queue for connection requests
    if (bPerformBind)
    {
        error = listen(socketDescriptor,  connectionQueueSize);
        if (error)
        {
            error = OsSocketGetERRNO();
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsServerSocket: listen call failed with error: %d=0x%x\n",
                error, error);
            socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
        }
    }
EXIT:
   ;

}

// Destructor
OsServerSocket::~OsServerSocket()
{
   close();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsServerSocket&
OsServerSocket::operator=(const OsServerSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsConnectionSocket* OsServerSocket::accept()
{
   OsConnectionSocket* connectSock = NULL;

   /* Block while waiting for a client to connect. */
   struct sockaddr_in clientSocketAddr;
   int clientAddrLength = sizeof clientSocketAddr;
   int clientSocket = ::accept(socketDescriptor,
                     (struct sockaddr*) &clientSocketAddr,
                     SOCKET_LEN_TYPE &clientAddrLength);
   if (clientSocket < 0)
   {
      int error = OsSocketGetERRNO();
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsServerSocket: accept call failed with error: %d=0x%x\n",
         error, error);
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      return NULL;
   }
   
   const int one = 1 ;
   setsockopt(clientSocket, SOL_SOCKET, SO_DONTROUTE, (char *)&one, sizeof(one)) ;

   connectSock = new OsConnectionSocket(mLocalIp,clientSocket);

   return(connectSock);
}

void OsServerSocket::close()
{
   if(socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
   {
#if defined(_WIN32)
      closesocket(socketDescriptor);
#elif defined(_VXWORKS) || defined(__pingtel_on_posix__)
      // Call shutdown first to unblock blocking calls on Linux
      ::shutdown(socketDescriptor,2);
      ::close(socketDescriptor);
#else
#error Unsupported target platform.
#endif
       socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
   }
}

/* ============================ ACCESSORS ================================= */

int OsServerSocket::getLocalHostPort() const
{
   return(localHostPort);
}

/* ============================ INQUIRY =================================== */

UtlBoolean OsServerSocket::isOk() const
{
    return(socketDescriptor != OS_INVALID_SOCKET_DESCRIPTOR);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
