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

#if defined(_WIN32)
#   include <winsock.h>
#elif defined(_VXWORKS)
#   include <inetLib.h>
#   include <netdb.h>
#   include <resolvLib.h>
#   include <sockLib.h>                                     
#elif defined(__pingtel_on_posix__)
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#else
#error Unsupported target platform.
#endif

// APPLICATION INCLUDES

//SSL related includes
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "os/OsSSLConnectionSocket.h"
#include "os/OsUtil.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
#if defined(_VXWORKS) || defined(__pingtel_on_posix__)
static const int INVALID_SOCKET = -1;
#endif

// STATIC VARIABLE INITIALIZATIONS
UtlBoolean OsSSLConnectionSocket::sbSSLInit = FALSE;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsSSLConnectionSocket::OsSSLConnectionSocket(int connectedSocketDescriptor, long timeoutInSecs) :
    OsConnectionSocket(connectedSocketDescriptor),
    mCTX(NULL),
    mSSL(NULL),
    mMeth(NULL)
{
    if (!mIsConnected)
	OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsSSLConnectionSocket::_ underline OsConnectionSocket not connected!!");

    mbExternalSSLSocket = FALSE;
//I added the NO_SSL_LIB to stop linux from inlcluding SSL.
//It was causing duplicate definitions with an apache module
//At some point we will use the shared library and then we can take out the define
#ifndef NO_SSL_LIB
        SSLInitSocket(connectedSocketDescriptor, timeoutInSecs);
#endif
}

OsSSLConnectionSocket::OsSSLConnectionSocket(SSL *s, int connectedSocketDescriptor) :
    OsConnectionSocket(connectedSocketDescriptor),
    mCTX(NULL)
{
    mbExternalSSLSocket = TRUE;
    mSSL = s;
    socketDescriptor = connectedSocketDescriptor;
}

// Constructor
OsSSLConnectionSocket::OsSSLConnectionSocket(int serverPort, const char* serverName,
                                             long timeoutInSecs) :
    OsConnectionSocket(serverPort,serverName),
    mCTX(NULL),
    mSSL(NULL),
    mMeth(NULL)
{
    
#ifndef NO_SSL_LIB
     mbExternalSSLSocket = FALSE;
    SSLInitSocket(socketDescriptor, timeoutInSecs);
#endif
}


// Destructor
OsSSLConnectionSocket::~OsSSLConnectionSocket()
{
	remoteHostName = OsUtil::NULL_OS_STRING;
	close();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsSSLConnectionSocket& 
OsSSLConnectionSocket::operator=(const OsSSLConnectionSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean OsSSLConnectionSocket::reconnect()
{
    return FALSE; //TODO
}

void OsSSLConnectionSocket::close()
{

#ifndef NO_SSL_LIB    
    if (mSSL)
    {
       if (mIsConnected)
       	  SSL_shutdown(mSSL);
       else
          SSL_clear(mSSL);
    }
#endif

   /* Clean up. */
    OsConnectionSocket::close();

#ifndef NO_SSL_LIB
   if (mSSL)
   {
      SSL_free(mSSL);
      ERR_remove_state(0);
      mSSL = NULL;
   }
   if (mCTX)
   {
      SSL_CTX_free (mCTX);
      mCTX = NULL;
   }
#endif

}

int OsSSLConnectionSocket::write(const char* buffer, int bufferLength)
{
    int bytesWritten = -1;

#ifndef NO_SSL_LIB
    bytesWritten = SSL_write (mSSL, buffer, bufferLength);
#endif

    return(bytesWritten);
}

int OsSSLConnectionSocket::write(const char* buffer, int bufferLength, long waitMilliseconds)
{
    int bytesWritten = -1;
    if(isReadyToWrite(waitMilliseconds))
    {
#ifndef NO_SSL_LIB
        bytesWritten = SSL_write (mSSL, buffer, bufferLength);
#endif
    }

    return(bytesWritten);
}

int OsSSLConnectionSocket::read(char* buffer, int bufferLength)
{
    // Use base class implementation
    int bytesRead = -1;
#ifndef NO_SSL_LIB
    bytesRead = SSL_read (mSSL, buffer, bufferLength);
#endif
#ifdef VALGRIND_MAKE_READABLE
    // If we are using Valgrind, we have to compensate for the fact that
    // Valgrind thinks all the output of SSL_read is undefined.
    // (This probably comes from SSL's use of uninitialized memory as part
    // of its key-generation algorithm.)
    VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(&bytesRead, sizeof (bytesRead)));
    if (bytesRead > 0)
    {
       VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(buffer, bytesRead));
    }
#endif /* VALGRIND_MAKE_READABLE */

    return(bytesRead);
}

int OsSSLConnectionSocket::read(char* buffer, 
                             int bufferLength,
                             UtlString* ipAddress, 
                             int* port)
{
    // Overide base class version as recvfrom does not
    // seem to return host info correctly for TCP
    // Use base class version without the remote host info
    int bytesRead = -1;
#ifndef NO_SSL_LIB
    bytesRead = SSL_read (mSSL, buffer, bufferLength);
#endif
#ifdef VALGRIND_MAKE_READABLE
    // If we are using Valgrind, we have to compensate for the fact that
    // Valgrind thinks all the output of SSL_read is undefined.
    // (This probably comes from SSL's use of uninitialized memory as part
    // of its key-generation algorithm.)
    VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(&bytesRead, sizeof (bytesRead)));
    if (bytesRead > 0)
    {
       VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(buffer, bytesRead));
    }
#endif /* VALGRIND_MAKE_READABLE */
    // Explicitly get the remote host info.
    getRemoteHostIp(ipAddress);
    *port = getRemoteHostPort();
    return(bytesRead);
}

// Because we have overided one read method, we
// must implement them all in OsSSLConnectionSocket or
// we end up hiding some of the methods.
int OsSSLConnectionSocket::read(char* buffer, 
                            int bufferLength, 
                            long waitMilliseconds)
{

    int bytesRead = -1;
    if(isReadyToRead(waitMilliseconds))
    {
#ifndef NO_SSL_LIB
        bytesRead = SSL_read (mSSL, buffer, bufferLength);
#endif
    }
#ifdef VALGRIND_MAKE_READABLE
    // If we are using Valgrind, we have to compensate for the fact that
    // Valgrind thinks all the output of SSL_read is undefined.
    // (This probably comes from SSL's use of uninitialized memory as part
    // of its key-generation algorithm.)
    VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(&bytesRead, sizeof (bytesRead)));
    if (bytesRead > 0)
    {
       VALGRIND_DISCARD(VALGRIND_MAKE_READABLE(buffer, bytesRead));
    }
#endif /* VALGRIND_MAKE_READABLE */
    return(bytesRead);
}

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
int OsSSLConnectionSocket::getIpProtocol() const
{
    return(OsSocket::SSL_SOCKET);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


void OsSSLConnectionSocket::SSLInitSocket(int socket, long timeoutInSecs)
{

#ifndef NO_SSL_LIB

    if (mIsConnected)
    {
        int err = -1;
        char*    str1;
        char*    str2;
        char buf[128];
        X509*    server_cert;

        if (!sbSSLInit)
        {
            //you MUST init random number gen before using SSL
            /* make a random number and set the top and bottom bits */

            for (unsigned int i = 0; i < sizeof(buf)/sizeof(int);i+=sizeof(int))
            {
                int val = rand() + i;
                memcpy(buf+i,&val,sizeof(int));
            }

            RAND_seed(buf,sizeof(buf));
            SSLeay_add_ssl_algorithms();
            
            //it is suggested by the OpenSSL group that embedded systems 
            //only enable loading of error string when debugging.
//            SSL_load_error_strings();

            sbSSLInit = TRUE;
        }
 
        mMeth = SSLv3_client_method();
        mCTX = SSL_CTX_new (mMeth);

        if (mCTX)
        {
            mSSL = SSL_new (mCTX);
            if (mSSL && (socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR))
            {
                SSL_set_fd (mSSL, socketDescriptor);
                
#ifdef TEST_PRINT
		OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSSLConnectionSocket trying SSL_connect...");
#endif
                err = SSL_connect (mSSL);
#ifdef TEST_PRINT
		OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSSLConnectionSocket SSL_connect returned %d", err);
#endif
		if (err <= 0)
		{
		    int rc = SSL_get_error(mSSL, err);
		    OsSysLog::add(FAC_KERNEL, PRI_ERR,
				  "OsSSLConnectionSocket SSL_connect failed %d, "
				  "errno=%d, closing socket...", rc, errno);
                    mIsConnected = FALSE;
                    OsConnectionSocket::close();
                    socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
		}
		else
		{
                    /* Following two steps are optional and not required for
                       data exchange to be successful. */
  
                    /* Get the cipher - opt */

                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SSL connection using %s\n", SSL_get_cipher (mSSL));
  
                    /* Get server's certificate (note: beware of dynamic allocation) - opt */

                    server_cert = SSL_get_peer_certificate (mSSL); 
                    if (server_cert)
                    {
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "Server certificate:\n");
  
                        str1 = X509_NAME_oneline (X509_get_subject_name (server_cert),0,0);
                        if (str1)
                            OsSysLog::add(FAC_SIP, PRI_DEBUG, "\t subject: %s\n", str1);

                        str2 = X509_NAME_oneline (X509_get_issuer_name  (server_cert),0,0);
                        if (str2)
                            OsSysLog::add(FAC_SIP, PRI_DEBUG, "\t issuer: %s\n", str2);

                        /* We could do all sorts of certificate verification stuff here before
                           deallocating the certificate. */
    
                    
                        OPENSSL_free(str1);
                        OPENSSL_free(str2);

                        X509_free (server_cert);
                    }
		}
            }
            else
            {
		OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSSLConnectionSocket bad parameters mSSL=%p,%d, closing socket...", mSSL, socketDescriptor);
                mIsConnected = FALSE;
                OsConnectionSocket::close();
                socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
            }

        }
        else
        {
	    OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSSLConnectionSocket bad mCTX=%p,%d closing socket...", mCTX, socketDescriptor);
            mIsConnected = FALSE;
            OsConnectionSocket::close();
            socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
        }

    }
#endif

}
/* ============================ FUNCTIONS ================================= */


