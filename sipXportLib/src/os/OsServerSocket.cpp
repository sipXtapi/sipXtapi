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
#include <stdio.h>

#define OsSS_CONST
#if defined(_WIN32)
#   include <winsock.h>
#undef OsSS_CONST
#define OsSS_CONST const
#elif defined(_VXWORKS)
#   include <inetLib.h>
#   include <sockLib.h>
#   include <unistd.h>
#elif defined(__pingtel_on_posix__)
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
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
#if defined(_VXWORKS) || defined(__pingtel_on_posix__)
static const int INVALID_SOCKET = -1;
#endif

#define SOCKET_LEN_TYPE
#ifdef __pingtel_on_posix__
#undef SOCKET_LEN_TYPE
#define SOCKET_LEN_TYPE (socklen_t *)
#endif

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsServerSocket::OsServerSocket(int connectionQueueSize, int serverPort)
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
   if(socketDescriptor == INVALID_SOCKET)
   {
      error = OsSocketGetERRNO();
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsServerSocket: socket call failed with error: %d=0x%x\n",
         error, error);
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      goto EXIT;
   }
   if(setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)))
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsServerSocket: setsockopt(SO_REUSEADDR) failed!\n");

   localAddr.sin_family = AF_INET;

   // Bind to a specific server port if given, or let the system pick
   // any available port number if defaulted to -1.
   localAddr.sin_port = htons((-1 == serverPort) ? 0 : serverPort);

   // Allow IP in on any of this host's addresses or NICs.
   localAddr.sin_addr.s_addr=OsSocket::getDefaultBindAddress();
//   localAddr.sin_addr.s_addr=htonl(INADDR_ANY); // Allow IP in on

   error = bind(socketDescriptor,
                (OsSS_CONST struct sockaddr*) &localAddr,
                sizeof(localAddr));
   if (error == INVALID_SOCKET)
   {
      error = OsSocketGetERRNO();
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "OsServerSocket:  bind to port %d failed with error: %d = 0x%x\n",
                    ((-1 == serverPort) ? 0 : serverPort), error, error);
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      goto EXIT;
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
   error = listen(socketDescriptor,  connectionQueueSize);
   if (error)
   {
      error = OsSocketGetERRNO();
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsServerSocket: listen call failed with error: %d=0x%x\n",
         error, error);
      socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
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

   connectSock = new OsConnectionSocket(clientSocket);

   return(connectSock);
}

void OsServerSocket::close()
{
   if(socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
   {
#if defined(_WIN32)
      closesocket(socketDescriptor);
#elif defined(_VXWORKS) || defined(__pingtel_on_posix__)
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
