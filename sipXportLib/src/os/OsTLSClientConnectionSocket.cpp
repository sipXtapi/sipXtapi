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
#include "os/OsTLSClientConnectionSocket.h"
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
OsTLSClientConnectionSocket::OsTLSClientConnectionSocket(int serverPort,
                                             const char* serverName,
                                             const UtlString certNickname,
                                             const UtlString password,
                                             const UtlString dbLocation,
                                             long timeoutInSecs,
                                             const char* localIp,
                                             ITlsSink* pSink) :
    OsTLSConnectionSocket(serverPort,serverName,certNickname,password,dbLocation,timeoutInSecs, localIp),
    mpTlsSink(pSink)
{
    mCertPassword = password;
    NSSInitSocket(socketDescriptor, timeoutInSecs, password.data());
    OsSysLog::add(FAC_KERNEL, PRI_DEBUG, 
                  "OsTLSClientConnectionSocket::_(port %d, name '%s', timeout %ld)",
                  serverPort, serverName, timeoutInSecs
                  );
}

// Destructor
OsTLSClientConnectionSocket::~OsTLSClientConnectionSocket()
{
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, 
                 "OsTLSClientConnectionSocket::~"
                 );
   remoteHostName = OsUtil::NULL_OS_STRING;
   close();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsTLSClientConnectionSocket&
OsTLSClientConnectionSocket::operator=(const OsTLSClientConnectionSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


void OsTLSClientConnectionSocket::NSSInitSocket(int socket, long timeoutInSecs, const char* szPassword)
{
    PRFileDesc         *tcpSocket = NULL;
    PRSocketOptionData      socketOption;
    PRStatus            prStatus;
    SECStatus           secStatus; 
    PRIntn      hostenum;
    PRNetAddr       addr;
    unsigned char* szPwd = NULL;

        if (socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR)
        {
            mpCert = PK11_FindCertFromNickname((char*)mCertNickname.data(), (char*)mCertPassword.data());

            if (mpCert)
            {
                szPwd = (unsigned char*) PR_Malloc(mCertPassword.length()+ 1);
                strncpy((char*)szPwd, mCertPassword.data(), mCertPassword.length()+1);
                mpPrivKey = PK11_FindKeyByAnyCert(mpCert, (unsigned char*) szPwd);
                if (mpPrivKey == NULL)
                {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_BAD_PASSWORD);
                    goto TlsFailure;
                }
            }
            tcpSocket = PR_ImportTCPSocket(socketDescriptor);
 
            if (tcpSocket)
            {
                /* Make the socket blocking. */
                socketOption.option                 = PR_SockOpt_Nonblocking;
                socketOption.value.non_blocking = PR_FALSE;

                prStatus = PR_SetSocketOption(tcpSocket, &socketOption);
                if (prStatus != PR_SUCCESS)
                {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_LIBRARY_FAILURE);
                    goto TlsFailure;
                } 

                /* Import the socket into the SSL layer. */
                mpPRfd = SSL_ImportFD(NULL, tcpSocket);
                if (!mpPRfd)
                {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_LIBRARY_FAILURE);
                    goto TlsFailure;
                }

                OsTLS::setTlsSink(mpPRfd, mpTlsSink);

                /* Set configuration options. */
                secStatus = SSL_OptionSet(mpPRfd, SSL_SECURITY, PR_TRUE);
                if (secStatus != SECSuccess)
                {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_LIBRARY_FAILURE);
                    goto TlsFailure;
                }

	            secStatus = SSL_OptionSet(mpPRfd, SSL_HANDSHAKE_AS_CLIENT, PR_TRUE);
	            if (secStatus != SECSuccess)
	            {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_LIBRARY_FAILURE);
                    goto TlsFailure;
	            }


                /* Set SSL callback routines. */
                secStatus = SSL_GetClientAuthDataHook(mpPRfd,
                                                      (SSLGetClientAuthData)OsTLS::GetClientAuthData,
                                                      (void *)mCertNickname.data());
                if (secStatus != SECSuccess)
                {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_LIBRARY_FAILURE);
                    goto TlsFailure;
                }

                secStatus = SSL_AuthCertificateHook(mpPRfd,
                                                    (SSLAuthCertificate)OsTLS::AuthCertificate,
                                                    (void *)CERT_GetDefaultCertDB());
                if (secStatus != SECSuccess)
                {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_LIBRARY_FAILURE);
                    goto TlsFailure;
                }

                secStatus = SSL_BadCertHook(mpPRfd, 
                                            (SSLBadCertHandler)OsTLS::BadCertHandler, NULL);
                if (secStatus != SECSuccess)
                {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_LIBRARY_FAILURE);
                    goto TlsFailure;
                }

                secStatus = SSL_HandshakeCallback(mpPRfd, 
                                                  (SSLHandshakeCallback)OsTLS::HandshakeCallback,
                                                  (void*)this);
                if (secStatus != SECSuccess)
                {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_LIBRARY_FAILURE);
                    goto TlsFailure;
                }
                
                secStatus = SSL_SetURL(mpPRfd, remoteHostName.data() );
                if (secStatus != SECSuccess)
                {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_REMOTE_HOST_UNREACHABLE);
                    goto TlsFailure;
                }

                PRHostEnt hostEntry;
                /* Prepare and setup network connection. */
                char buffer[PR_NETDB_BUF_SIZE];
                PR_GetHostByName(remoteHostName.data(), buffer, sizeof(buffer), &hostEntry);
         
                hostenum = PR_EnumerateHostEnt(0, &hostEntry, remoteHostPort, &addr);
         
                prStatus = PR_Connect(mpPRfd, &addr, PR_INTERVAL_NO_TIMEOUT);
                if (prStatus != PR_SUCCESS)
                {
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_CONNECTION_FAILURE);
                    goto TlsFailure;
                }
         
                /* Established SSL connection, ready to send data. */
                secStatus = SSL_ResetHandshake(mpPRfd, /* asServer */ PR_FALSE);
                if (secStatus != SECSuccess)
                {
                        prStatus = PR_Shutdown(mpPRfd, PR_SHUTDOWN_BOTH);
                        prStatus = PR_Close(mpPRfd);
                        if (prStatus != PR_SUCCESS)
                        {
                        }
                    mpTlsSink->onTlsEvent(E_SECURITY_CAUSE_TLS_HANDSHAKE_FAILURE);
                    goto TlsFailure;
                }
                CERTCertificate *pServerCert = SSL_PeerCertificate(mpPRfd);
                PR_Free(szPwd);
            }
            else
            {
                mIsConnected = FALSE;
                OsConnectionSocket::close();
                socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;          
            }
        }
TlsFailure:
        return;
}


/* ============================ FUNCTIONS ================================= */
#endif