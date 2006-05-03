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
#include "os/OsTLSConnectionSocket.h"
#include "os/OsUtil.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"
#include "os/OsLock.h"
#include "os/OsDateTime.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTLSConnectionSocket::OsTLSConnectionSocket(int socketDescriptor,
                                             const UtlString certNickname,
                                             const UtlString password,
                                             const UtlString dbLocation,
                                             long timeoutInSecs
                                             ) :
    OsConnectionSocket(socketDescriptor),
    mSocketGuard(OsMutex::Q_FIFO),
    mpPRfd(NULL),
    mCertNickname(certNickname),
    mCertPassword(password),
    mDbLocation(dbLocation),
    mpPrivKey(NULL),
    mpCert(NULL),
    mbHandshakeComplete(false)
{
    OsTLS::Initialize(dbLocation.data());
}


// Constructor
OsTLSConnectionSocket::OsTLSConnectionSocket(int serverPort,
                                             const char* serverName,
                                             const UtlString certNickname,
                                             const UtlString password,
                                             const UtlString dbLocation,
                                             long timeoutInSecs,
                                             const char* localIp) :
    OsConnectionSocket(serverPort,serverName, TRUE, localIp, false),
    mSocketGuard(OsMutex::Q_FIFO),
    mpPRfd(NULL),
    mCertNickname(certNickname),
    mCertPassword(password),
    mDbLocation(dbLocation),
    mbHandshakeComplete(false)
{
    OsTLS::Initialize(dbLocation.data());
}

// Destructor
OsTLSConnectionSocket::~OsTLSConnectionSocket()
{
//   OsLock lock(mSocketGuard) ;
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, 
                 "OsTLSConnectionSocket::~"
                 );
   remoteHostName = OsUtil::NULL_OS_STRING;
   close();
}


/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsTLSConnectionSocket&
OsTLSConnectionSocket::operator=(const OsTLSConnectionSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void OsTLSConnectionSocket::close()
{
//    OsLock lock(mSocketGuard) ;
    if (socketDescriptor > OS_INVALID_SOCKET_DESCRIPTOR && mpPRfd)
    {
        PR_Shutdown(mpPRfd, PR_SHUTDOWN_BOTH);
        PR_Close(mpPRfd);
        mpPRfd = NULL;
        socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
    }
}

UtlBoolean OsTLSConnectionSocket::reconnect()
{
    return FALSE; //TODO
}

int OsTLSConnectionSocket::write(const char* buffer, int bufferLength)
{
    int bytesWritten = -1;

    if (mpPRfd)
    {
        bytesWritten = PR_Write(mpPRfd, buffer, bufferLength);
        int nssError = PR_GetError();
        // if connection reset by peer, try again
        if (-5961 == nssError)
        {
            OsTask::delay(100);
            bytesWritten = PR_Write(mpPRfd, buffer, bufferLength);
        }

        if (bytesWritten > 0)
        {
            UtlString bytes(buffer, bytesWritten);    
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG, 
                        "OsTLSConnectionSocket::write %s", bytes.data()
                        );
        }
    }
    return(bytesWritten);
}

int OsTLSConnectionSocket::write(const char* buffer, int bufferLength, long waitMilliseconds)
{
    int bytesWritten = -1;
    if(isReadyToWrite(waitMilliseconds))
    {
        bytesWritten = PR_Write(mpPRfd, buffer, bufferLength);
        int nssError = PR_GetError();
        // if connection reset by peer, try again
        if (-5961 == nssError)
        {
            OsTask::delay(100);
            bytesWritten = PR_Write(mpPRfd, buffer, bufferLength);
        }

        if (bytesWritten)
        {
            UtlString bytes(buffer, bytesWritten);    
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG, 
                        "OsTLSConnectionSocket::write %s", bytes.data()
                        );
        }
    }

    return(bytesWritten);
}

int OsTLSConnectionSocket::read(char* buffer, int bufferLength)
{
    // Use base class implementation
    int bytesRead = -1;

    if (mpPRfd)//  && SSL_DataPending(mpPRfd))
    {
        bytesRead = PR_Read (mpPRfd, buffer, bufferLength);
    }

    if (bytesRead > 0)
    {
        UtlString bytes(buffer, bytesRead);    
        OsSysLog::add(FAC_KERNEL, PRI_DEBUG, 
                    "OsTLSConnectionSocket::read %s", bytes.data()
                    );
    }
    return(bytesRead);
}

int OsTLSConnectionSocket::read(char* buffer,
                             int bufferLength,
                             UtlString* ipAddress,
                             int* port)
{
    // Overide base class version as recvfrom does not
    // seem to return host info correctly for TCP
    // Use base class version without the remote host info
    int bytesRead = -1;

    if (mpPRfd)//  && SSL_DataPending(mpPRfd))
    {
        bytesRead = PR_Read (mpPRfd, buffer, bufferLength);
    }
    if (bytesRead > 0)
    {
        UtlString bytes(buffer, bytesRead);    
        OsSysLog::add(FAC_KERNEL, PRI_DEBUG, 
                    "OsTLSConnectionSocket::read %s", bytes.data()
                    );
    }
    // Explicitly get the remote host info.
    getRemoteHostIp(ipAddress);
    *port = getRemoteHostPort();
    return(bytesRead);
}

// Because we have overided one read method, we
// must implement them all in OsTLSServerConnectionSocket or
// we end up hiding some of the methods.
int OsTLSConnectionSocket::read(char* buffer,
                            int bufferLength,
                            long waitMilliseconds)
{
    int bytesRead = -1;
    if(isReadyToRead(waitMilliseconds))
    {
        bytesRead = PR_Read (mpPRfd, buffer, bufferLength);
        if (bytesRead > 0)
        {
            UtlString bytes(buffer, bytesRead);    
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG, 
                        "OsTLSConnectionSocket::read %s", bytes.data()
                        );
        }
    }
    return(bytesRead);
}

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
int OsTLSConnectionSocket::getIpProtocol() const
{
    return(OsSocket::SSL_SOCKET);
}

UtlBoolean OsTLSConnectionSocket::isReadyToReadEx(long waitMilliseconds, UtlBoolean &rSocketError) const
{
    return OsConnectionSocket::isReadyToReadEx(waitMilliseconds, rSocketError);
}

UtlBoolean  OsTLSConnectionSocket::isReadyToRead(long waitMilliseconds) const
{
    UtlBoolean bReady = TRUE;
    UtlBoolean bSocketError = FALSE;
    bReady = isReadyToReadEx(waitMilliseconds, bSocketError);
    return bReady;
}

UtlBoolean OsTLSConnectionSocket::isReadyToWrite(long timeoutMilliSec) const
{
    UtlBoolean readyToWrite = FALSE;
    return OsSocket::isReadyToWrite(timeoutMilliSec);
    if (true)//waitForHandshake(timeoutMilliSec))
    {
        PRIntervalTime prTimeout = PR_INTERVAL_NO_WAIT;

        if(timeoutMilliSec < 0)
        {
            prTimeout = PR_INTERVAL_NO_TIMEOUT;
        }
        else if(timeoutMilliSec > 0)
        {
            prTimeout = PR_SecondsToInterval(timeoutMilliSec / 1000);
        }
        PRPollDesc pollDescriptor;


        pollDescriptor.fd = mpPRfd;
        pollDescriptor.in_flags = PR_POLL_WRITE | PR_POLL_EXCEPT;
        pollDescriptor.out_flags = 0;

        PRInt32 numDescriptorsReady = 
            PR_Poll(&pollDescriptor, 
                    1, // only one descriptor
                    prTimeout);

        if(numDescriptorsReady > 0)
        {
            if(pollDescriptor.out_flags | PR_POLL_WRITE)
            {
                readyToWrite = TRUE;
            }
            else if(pollDescriptor.out_flags | PR_POLL_ERR)
            {
                printf("error polling if NSS SSL socket is ready to write");
                OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                    "error polling if NSS SSL socket is ready to write");
            }
            else if(pollDescriptor.out_flags | PR_POLL_NVAL)
            {
                printf("invalid NSS SSL socket while polling if ready to write");
                OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                    "invalid NSS SSL socket while polling if ready to write");
            }
            else if(pollDescriptor.out_flags | PR_POLL_EXCEPT)
            {
                printf("NSS SSL socket exception while polling if ready to write");
                OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                    "NSS SSL socket exception while polling if ready to write");
            }
            else
            {
                printf("unknown NSS SSL socket state while polling if ready to write");
                OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                    "unknown NSS SSL socket state while polling if ready to write");
            }
        }
    }
    return(readyToWrite);
}

bool OsTLSConnectionSocket::waitForHandshake(long milliseconds) const
{
    OsTime start;
    OsTime now;
    OsTime end;

    OsDateTime::getCurTime(start);
    if (milliseconds < 0)
    {
        milliseconds = 30000;
    }
    end = OsTime(OsTime(milliseconds) + start);

    while (!mbHandshakeComplete)
    {
        OsDateTime::getCurTime(now);
        if (now > end)
        {
            break;
        }
        OsTask::delay(40);
    }
    return mbHandshakeComplete;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
#endif 