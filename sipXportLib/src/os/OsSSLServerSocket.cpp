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
#include "os/OsSSLServerSocket.h"
#include "os/OsDefs.h"
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

#define PUBCERTFILE SIPX_CONFDIR "/ssl/ssl.crt"
#define PRIVKEYFILE SIPX_CONFDIR "/ssl/ssl.key"

// STATIC VARIABLE INITIALIZATIONS
UtlString  OsSSLServerSocket::sPublicCertificateLocation(PUBCERTFILE);
UtlString  OsSSLServerSocket::sPrivateKeyLocation(PRIVKEYFILE);
UtlString  OsSSLServerSocket::sPrivateKeyPassword("");
UtlBoolean OsSSLServerSocket::sbSSLInit = FALSE;

/* //////////////////////////// PUBLIC //////////////////////////////////// */


/* Make these what you want for cert & key files */

/* ============================ CREATORS ================================== */

// Constructor
OsSSLServerSocket::OsSSLServerSocket(int connectionQueueSize, int serverPort) :
OsServerSocket(connectionQueueSize,serverPort),
mCTX(NULL),
mMeth(NULL)
{
    initializeCTX();

}

// Destructor
OsSSLServerSocket::~OsSSLServerSocket()
{
  close();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsSSLServerSocket&
OsSSLServerSocket::operator=(const OsSSLServerSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsConnectionSocket* OsSSLServerSocket::accept()
{
   OsConnectionSocket* newSocket = NULL;
   
   if (socketDescriptor == OS_INVALID_SOCKET_DESCRIPTOR)
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR
                    , "OsSSLServerSocket: accept exiting because socketDescriptor is %d\n"
                    ,socketDescriptor);
   }
   else
   {
      int err;

      /* Block while waiting for a client to connect. */
      struct sockaddr_in clientSocketAddr;
      int clientAddrLength = sizeof clientSocketAddr;
      int clientSocket = ::accept(socketDescriptor,
                                  (struct sockaddr*) &clientSocketAddr,
                                  SOCKET_LEN_TYPE &clientAddrLength);
      if (clientSocket < 0)
      {
         int error = OsSocketGetERRNO();
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsSSLServerSocket: accept call failed with error: %d=0x%x\n",
                       error, error);
         socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
      }

      if (socketDescriptor != OS_INVALID_SOCKET_DESCRIPTOR)
      {
         SSL*     pSSL = SSL_new (mCTX);

         if (pSSL)
         {
            SSL_set_fd (pSSL, clientSocket);

            newSocket = new OsSSLConnectionSocket(pSSL,clientSocket);

            err = SSL_accept (pSSL);

            if (newSocket)
            {
               OsSysLog::add(FAC_KERNEL, PRI_INFO
                             ,"OsSSLServerSocket::accept - SUCCESS creating SSL socket for accept.\n");
            }
         }
         else
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR
                          , "Error creating new SSL object in SSL server accept method.\n");
            socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
         }
      }
   }
   
   return(newSocket);
}

void OsSSLServerSocket::close()
{

   if(socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
   {
      OsServerSocket::close();

#ifndef NO_SSL_LIB
      //Since error queue data structures are allocated automatically for new threads, they must be freed when threads are terminated in order to avoid memory leaks.
      ERR_remove_state(0);
      if (mCTX)
        SSL_CTX_free (mCTX);

      mCTX = NULL;
#endif

   }
}

void OsSSLServerSocket::initializeCTX()
{

    if (!sbSSLInit)
    {

        //it is suggested by the OpenSSL group that embedded systems
        //only enable loading of error string when debugging.
        SSL_load_error_strings();

        SSLeay_add_ssl_algorithms();

        sbSSLInit = TRUE;
    }

    mMeth = SSLv23_server_method();
    mCTX = SSL_CTX_new (mMeth);

    if (mCTX)
    {
        // disable server connection caching
        SSL_CTX_set_session_cache_mode(mCTX, SSL_SESS_CACHE_OFF);

        if (SSL_CTX_use_certificate_file(mCTX, sPublicCertificateLocation.data(), SSL_FILETYPE_PEM) > 0)
        {
            if (SSL_CTX_use_PrivateKey_file(mCTX, sPrivateKeyLocation.data(), SSL_FILETYPE_PEM) > 0)
            {
                if (! SSL_CTX_check_private_key(mCTX))
                {
                    OsSysLog::add(FAC_KERNEL, PRI_ERR
                                  ,"Private key '%s' does not match the certificate public key '%s'\n"
                                  ,sPrivateKeyLocation.data()
                                  ,sPublicCertificateLocation.data()
                                  );
                }
            }
            else
            {
                OsSysLog::add(FAC_KERNEL, PRI_ERR
                              ,"Private key '%s' could not be initialized. Is Password set?\n"
                              ,sPrivateKeyLocation.data()
                              );
                socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
            }
         }
         else
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "Public key '%s' could not be initialized.\n"
                          ,sPublicCertificateLocation.data()
                          );
            socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
         }
     }
     else
     {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not create mCTX (SSL Context).\n");
         socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
     }
}

OsStatus OsSSLServerSocket::setSSLPublicCertificateLocation(const UtlString &rPublicCertificatePath)
{
    OsStatus retval = OS_FAILED;

    sPublicCertificateLocation = rPublicCertificatePath;
    if (OsFileSystem::exists(sPublicCertificateLocation))
        retval = OS_SUCCESS;

    return retval;
}

OsStatus OsSSLServerSocket::setSSLPrivateKeyLocation(const UtlString &rPrivateKeyPath)
{
    OsStatus retval = OS_FAILED;

    sPrivateKeyLocation = rPrivateKeyPath;
    if (OsFileSystem::exists(sPrivateKeyLocation))
        retval = OS_SUCCESS;

    return retval;
}

OsStatus OsSSLServerSocket::setSSLPrivateKeyPassword(const UtlString &rPrivateKeyPassword)
{
    OsStatus retval = OS_FAILED;

    sPrivateKeyPassword = rPrivateKeyPassword;
    if (OsFileSystem::exists(sPrivateKeyPassword))
        retval = OS_SUCCESS;

    return retval;
}

/* ============================ ACCESSORS ================================= */

int OsSSLServerSocket::getLocalHostPort() const
{
   return(localHostPort);
}

/* ============================ INQUIRY =================================== */

int OsSSLServerSocket::getIpProtocol() const
{
    return(OsSocket::SSL_SOCKET);
}

UtlBoolean OsSSLServerSocket::isOk() const
{
    return(socketDescriptor != OS_INVALID_SOCKET_DESCRIPTOR);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
