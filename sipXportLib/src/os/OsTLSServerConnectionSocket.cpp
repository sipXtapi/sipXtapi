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

#include <seccomon.h>
#include <private/pprio.h>
#include <ssl.h>
#include <sslerr.h>
#include "pk11func.h"

// APPLICATION INCLUDES


#include "os/OsTLS.h"
#include "os/OsTLSServerConnectionSocket.h"
#include "os/OsUtil.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTLSServerConnectionSocket::OsTLSServerConnectionSocket(int socketDescriptor,
                                             const UtlString certNickname,
                                             const UtlString password,
                                             const UtlString dbLocation,
                                             long timeoutInSecs
                                             ) :
    OsTLSConnectionSocket(socketDescriptor,
                          certNickname,
                          password,
                          dbLocation,
                          timeoutInSecs),
    mbInitializeFailed(false)
{
    NSSInitSocket(socketDescriptor, timeoutInSecs, password.data());
    OsSysLog::add(FAC_KERNEL, PRI_DEBUG, 
                "OsTLSServerConnectionSocket::_(socket %d , timeout %ld)",
                socketDescriptor, timeoutInSecs
                );
}

// Destructor
OsTLSServerConnectionSocket::~OsTLSServerConnectionSocket()
{
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, 
                 "OsTLSServerConnectionSocket::~"
                 );
   remoteHostName = OsUtil::NULL_OS_STRING;
   close();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsTLSServerConnectionSocket&
OsTLSServerConnectionSocket::operator=(const OsTLSServerConnectionSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}



/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


void OsTLSServerConnectionSocket::NSSInitSocket(int socketDescriptor,
                                                long timeoutInSecs,
                                                const char* szPassword)
{
    PRFileDesc         *tcpSocket = NULL;
    tcpSocket = PR_ImportTCPSocket(socketDescriptor);
    NSSInitSocket(tcpSocket, timeoutInSecs, szPassword);
}


void OsTLSServerConnectionSocket::NSSInitSocket(PRFileDesc* pDescriptor, long timeoutInSecs, const char* szPassword)
{
    PRFileDesc         *tcpSocket = NULL;
    PRSocketOptionData      socketOption;
    PRStatus            prStatus;
    SECStatus           secStatus; 
//    PRIntn      hostenum;
//    PRNetAddr       addr;
	SSLKEAType  certKEA;

     tcpSocket = pDescriptor;
        if (socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
        {
            mpCert = PK11_FindCertFromNickname((char*)mCertNickname.data(), (char*)mCertPassword.data());
            if (mpCert == NULL)
            {
                mbInitializeFailed = true;
                goto TlsError;
            }

            unsigned char* szPwd = (unsigned char*) PR_Malloc(mCertPassword.length()+ 1);
            strncpy((char*)szPwd, mCertPassword.data(), mCertPassword.length()+1);
            mpPrivKey = PK11_FindKeyByAnyCert(mpCert, (char*)szPwd);
            if (mpPrivKey == NULL)
            {
                mbInitializeFailed = true;
                goto TlsError;
            }

            if (tcpSocket)
            {
                /* Make the socket blocking. */
                socketOption.option                 = PR_SockOpt_Nonblocking;
                socketOption.value.non_blocking = PR_FALSE;

                prStatus = PR_SetSocketOption(tcpSocket, &socketOption);
                if (prStatus != PR_SUCCESS)
                {
                    mbInitializeFailed = true;
                    goto TlsError;
                } 

                /* Import the socket into the SSL layer. */
                mpPRfd = SSL_ImportFD(NULL, tcpSocket);
                if (!mpPRfd)
                {
                    mbInitializeFailed = true;
                    goto TlsError;
                }

                /* Set configuration options. */
                secStatus = SSL_OptionSet(mpPRfd, SSL_SECURITY, PR_TRUE);
                if (secStatus != SECSuccess)
                {
                    mbInitializeFailed = true;
                    goto TlsError;
                }

	            secStatus = SSL_OptionSet(mpPRfd, SSL_HANDSHAKE_AS_SERVER, PR_TRUE);
	            if (secStatus != SECSuccess)
	            {
                    mbInitializeFailed = true;
                    goto TlsError;
	            }

                secStatus = SSL_AuthCertificateHook(mpPRfd,
                                                    (SSLAuthCertificate)OsTLS::AuthCertificate,
                                                    (void *)CERT_GetDefaultCertDB());
                if (secStatus != SECSuccess)
                {
                    mbInitializeFailed = true;
                    goto TlsError;
                }

                secStatus = SSL_BadCertHook(mpPRfd, 
                                            (SSLBadCertHandler)OsTLS::BadCertHandler, NULL);
                if (secStatus != SECSuccess)
                {
                    mbInitializeFailed = true;
                    goto TlsError;
                }

                secStatus = SSL_HandshakeCallback(mpPRfd, 
                                                  (SSLHandshakeCallback)OsTLS::HandshakeCallback,
                                                  (void*)this);
                if (secStatus != SECSuccess)
                {
                    mbInitializeFailed = true;
                    goto TlsError;
                }

                secStatus = SSL_SetPKCS11PinArg(mpPRfd, (void*)szPassword);
                if (secStatus != SECSuccess)
                {
                    mbInitializeFailed = true;
                    goto TlsError;
                }


	            certKEA = NSS_FindCertKEAType(mpCert);

	            secStatus = SSL_ConfigSecureServer(mpPRfd, mpCert, mpPrivKey, certKEA);
	            if (secStatus != SECSuccess)
	            {
                    mbInitializeFailed = true;
                    goto TlsError;
	            }
	            
                secStatus = SSL_ResetHandshake(mpPRfd, /* asServer */ PR_TRUE);
                if (secStatus != SECSuccess)
                {
                    mbInitializeFailed = true;
                    goto TlsError;
                }
	            
	            PR_Free(szPwd);
            }
            else
            {
                mIsConnected = FALSE;
                OsConnectionSocket::close();
                socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;          
                mbInitializeFailed = true;
            }
        }
TlsError:
        return;
}

#endif
/* ============================ FUNCTIONS ================================= */
