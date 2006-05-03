//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifdef SIP_TLS_NSS

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
#   include <netinet/in.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <unistd.h>
#   include <resolv.h>
#else
#error Unsupported target platform.
#endif

#include <nspr.h>
#include <nss.h>
#include <seccomon.h>
#include <private/pprio.h>
#include <ssl.h>
#include <sslerr.h>
#include <sslproto.h>
#include "pk11func.h"

// APPLICATION INCLUDES
#include "os/OsTLS.h"
#include "os/OsTLSServerSocket.h"
#include "os/OsTLSServerConnectionSocket.h"
#include "os/OsDefs.h"
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

/* Make these what you want for cert & key files */

/* ============================ CREATORS ================================== */

// Constructor
OsTLSServerSocket::OsTLSServerSocket(int connectionQueueSize,
                                     int serverPort,
                                     UtlString certNickname,
                                     UtlString certPassword,
                                     UtlString dbLocation,
                                     const UtlString bindAddress)
   : OsServerSocket(connectionQueueSize,serverPort, bindAddress.data(), false),
     mCertNickname(certNickname),
     mCertPassword(certPassword),
     mDbLocation(dbLocation),
     mpMozillaSSLSocket(NULL),
     mpCert(NULL),
     mpPrivKey(NULL),
     mTlsInitCode(TLS_INIT_SUCCESS)
{
    PRSocketOptionData      socketOption;
    PRStatus                prStatus;
    SECStatus secStatus;
    
    // import the newly created socket into NSS, and set the PRFileDesc.
    if (socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
    {
        /* Call the NSPR initialization routines. */
        PR_Init( PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

        /* Set the cert database password callback. */
        PK11_SetPasswordFunc(OsTLS::PasswordCallback);

        secStatus = NSS_Init(dbLocation.data());
        if (secStatus != SECSuccess)
        {
            mTlsInitCode = TLS_INIT_DATABASE_FAILURE;
            return ;
        }

        /* Set the policy for this server (REQUIRED - no default). */
        secStatus = NSS_SetExportPolicy();
        if (secStatus != SECSuccess)
        {
            mTlsInitCode = TLS_INIT_DATABASE_FAILURE;
            return ;
        }

        /* Get own certificate and private key. */
        mpCert = PK11_FindCertFromNickname((char*) certNickname.data(), (void*)certPassword.data());
        if (mpCert == NULL)
        {
            return ;
        }
            
        unsigned char* szPwd = (unsigned char*) PR_Malloc(certPassword.length()+ 1);
        strncpy((char*)szPwd, certPassword.data(), certPassword.length()+1);
        mpPrivKey = PK11_FindKeyByAnyCert(mpCert, (void*)szPwd);
        if (mpPrivKey == NULL)
        {
            mTlsInitCode = TLS_INIT_BAD_PASSWORD;
            // probably a wrong password
            return ;
        }

        /* Configure the server's cache for a multi-process application
        * using default timeout values (24 hrs) and directory location (/tmp). 
        */
        SSL_ConfigMPServerSIDCache(256, 0, 0, NULL);

        mpMozillaSSLSocket = PR_ImportTCPSocket(socketDescriptor);

        if (!mpMozillaSSLSocket)
        {
            mTlsInitCode = TLS_INIT_TCP_IMPORT_FAILURE;
        }
        else
        {
            /* Make the socket blocking. */
            socketOption.option                 = PR_SockOpt_Nonblocking;
            socketOption.value.non_blocking = PR_FALSE;

            prStatus = PR_SetSocketOption(mpMozillaSSLSocket, &socketOption);
            if (prStatus != PR_SUCCESS)
            {
                mTlsInitCode = TLS_INIT_NSS_FAILURE;
                return;
            }
            
            secStatus = SSL_CipherPrefSetDefault(SSL_RSA_WITH_NULL_MD5, PR_TRUE);
            if (secStatus != SECSuccess)
            {
                mTlsInitCode = TLS_INIT_NSS_FAILURE;
                return;
            }            
            
            PRNetAddr addr;
            /* Configure the network connection. */
            addr.inet.family = PR_AF_INET;
            addr.inet.ip     = inet_addr(bindAddress.data());
            addr.inet.port   = PR_htons(serverPort);

            /* Bind the address to the listener socket. */
            prStatus = PR_Bind(mpMozillaSSLSocket, &addr);
            if (prStatus != PR_SUCCESS)
            {
                mTlsInitCode = TLS_INIT_NSS_FAILURE;
                return;
            }
 
            /* Listen for connection on the socket.  The second argument is
            * the maximum size of the queue for pending connections.
            */
            prStatus = PR_Listen(mpMozillaSSLSocket, connectionQueueSize);
            if (prStatus != PR_SUCCESS)
            {
                mTlsInitCode = TLS_INIT_NSS_FAILURE;
                return;
            }
        }            
    }        
}

// Destructor
OsTLSServerSocket::~OsTLSServerSocket()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsTLSServerSocket&
OsTLSServerSocket::operator=(const OsTLSServerSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsConnectionSocket* OsTLSServerSocket::accept()
{
   OsConnectionSocket* newSocket = NULL;
   
   if (socketDescriptor == OS_INVALID_SOCKET_DESCRIPTOR)
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR
                    , "OsTLSServerSocket: accept exiting because socketDescriptor is %d"
                    ,socketDescriptor);
   }
   else
   {
        /* Block while waiting for a client to connect. */
        struct sockaddr_in clientSocketAddr;
        int clientAddrLength = sizeof clientSocketAddr;
      
        PRNetAddr   addr;
        PRFileDesc *tcpSocket;

        PRFileDesc* listenSocket = PR_ImportTCPSocket(socketDescriptor);

        /* Accept a connection to the socket. */
        tcpSocket = PR_Accept(listenSocket, &addr, PR_INTERVAL_NO_TIMEOUT);
      

      if (!tcpSocket)
      {
         int error = OsSocketGetERRNO();
         OsSysLog::add(FAC_KERNEL, PRI_ERR, 
                       "OsTLSServerSocket: accept call failed with error: %d=%x",
                       error, error);
         socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      }
      else
      {

         // need to create a new OsTLSConnectionSocket
        int socketDescriptor = PR_FileDesc2NativeHandle(tcpSocket);
        newSocket = new OsTLSServerConnectionSocket(socketDescriptor, mCertNickname, mCertPassword, mDbLocation );
        if (newSocket)
        {
        }
      }
   }
   
   return(newSocket);
}

void OsTLSServerSocket::close()
{
    /*
    if(socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
    {
        OsServerSocket::close();
    }
    */
    if (mpMozillaSSLSocket)
    {
        PR_Shutdown(mpMozillaSSLSocket, PR_SHUTDOWN_BOTH);
        PR_Close(mpMozillaSSLSocket);
        mpMozillaSSLSocket = NULL;
        socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
    }
    
}

/* ============================ ACCESSORS ================================= */

int OsTLSServerSocket::getLocalHostPort() const
{
   return(localHostPort);
}

/* ============================ INQUIRY =================================== */

int OsTLSServerSocket::getIpProtocol() const
{
    return(OsSocket::SSL_SOCKET);
}

UtlBoolean OsTLSServerSocket::isOk() const
{
    return(socketDescriptor != OS_INVALID_SOCKET_DESCRIPTOR);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif