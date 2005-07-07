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

#define OPTIONAL_CONST const
#define OPTIONAL_VXW_CHARSTAR_CAST
#define resolvGetHostByName(a, b, c) gethostbyname(a)
#if defined(_WIN32)
#   include <winsock.h>
#   include <time.h>
#elif defined(_VXWORKS)
#   undef resolvGetHostByName
#   include <inetLib.h>
#   include <netdb.h>
#   include <resolvLib.h>
#   include <sockLib.h>
#   undef OPTIONAL_CONST
#   define OPTIONAL_CONST
#   undef OPTIONAL_VXW_CHARSTAR_CAST
#   define OPTIONAL_VXW_CHARSTAR_CAST (char*)
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
#include "os/OsDatagramSocket.h"
#include "os/OsSysLog.h"

// DEFINES
#define SOCKET_LEN_TYPE
#ifdef __pingtel_on_posix__
#undef SOCKET_LEN_TYPE
#define SOCKET_LEN_TYPE (socklen_t *)
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

#define MIN_REPORT_SECONDS 10

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsDatagramSocket::OsDatagramSocket(int remoteHostPortNum,
       const char* remoteHost, int localHostPortNum, const char* localHost) :
   mNumTotalWriteErrors(0),
   mNumRecentWriteErrors(0)
{
    int                error = 0;
    UtlBoolean          isIp = FALSE;
    struct sockaddr_in localAddr;
    struct hostent*    server = NULL;
#   if defined(_VXWORKS)
    char hostentBuf[512];
#   endif

    {
        time_t rightNow;
        (void) time(&rightNow);
        mLastWriteErrorTime = rightNow;
    }
    mToSockaddrValid = FALSE;
    mpToSockaddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
    assert(NULL != mpToSockaddr);
    memset(mpToSockaddr, 0, sizeof(struct sockaddr_in));

    socketDescriptor = OS_INVALID_SOCKET_DESCRIPTOR;
    remoteHostPort = remoteHostPortNum;
    localHostPort = localHostPortNum;

    if(localHost)
    {
        localHostName.append(localHost);
    }

    if(!socketInit())
    {
        goto EXIT;
    }

    if(remoteHost)
    {
        isIp = isIp4Address(remoteHost);

        remoteHostName.append(remoteHost);
        if (!isIp)
        {
#       if defined(_WIN32) || defined(__pingtel_on_posix__)
            server = gethostbyname(remoteHost);
#       elif defined(_VXWORKS)
            server = resolvGetHostByName((char*) remoteHost,
                                         hostentBuf, sizeof(hostentBuf));
#       else
#       error Unsupported target platform.
#       endif //_VXWORKS
        }

        if(server)
        {
            inet_ntoa_pt(*((in_addr*) (server->h_addr)), mRemoteIpAddress);
#ifdef TEST
            osPrintf("OsDatagramSocket::OsDatagramSocket remoteIpAddress: \"%s\"\n",
                     mRemoteIpAddress.data());
#endif
        }
        else if (isIp)
        {
            mRemoteIpAddress = remoteHostName;
        }
    }
    localHostPort = localHostPortNum;

    // Create the socket
    socketDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socketDescriptor == OS_INVALID_SOCKET_DESCRIPTOR)
    {
#       if defined(_VXWORKS)
        static int failures = 0;
#       endif

        error = OsSocketGetERRNO();

        close();
        perror("call to socket failed\n");
#       if defined(_VXWORKS)
        failures++;
        if (failures > 10) {
            taskDelay(60);
        }
        assert(failures < 200);
#       endif
        osPrintf("socket(%s:%d, %s:%d) call failed with error: %d\n",
            remoteHost, remoteHostPortNum, localHost, localHostPortNum, error);
        goto EXIT;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(localHostPort);

    // Should use host address (if specified in localHost) but for now use any
    localAddr.sin_addr.s_addr=OsSocket::getDefaultBindAddress(); // $$$

#   if defined(_WIN32)
    error = bind( socketDescriptor, (const struct sockaddr*) &localAddr,
            sizeof(localAddr));
#   elif defined(_VXWORKS) || defined(__pingtel_on_posix__)

    error = bind( socketDescriptor, (struct sockaddr*) &localAddr,
            sizeof(localAddr));
#   else
#   error Unsupported target platform.
#   endif

    if(error == OS_INVALID_SOCKET_DESCRIPTOR)
    {
        // error = OsSocketGetERRNO();

        close();

        // perror("bind to socket failed\n");
        goto EXIT;
    }
    else
    {
        sockaddr_in addr ;
        int addrSize = sizeof(struct sockaddr_in);
        error = getsockname(socketDescriptor, (struct sockaddr*) &addr, SOCKET_LEN_TYPE& addrSize);
        localHostPort = htons(addr.sin_port);
    }

    mSimulatedConnect = FALSE;

    // This should be replaced by a call to doConnect
    // Connect to a remote host if given
    if(remoteHostPort > 0 && remoteHost)
    {
        if(!isIp && !server)
        {
            close();
            osPrintf("DNS failed to lookup host: %s\n", remoteHost);
            goto EXIT;
        }
#       if 0 // turn on to debug socket creation problems
        OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                      "OsDatagramSocket::_: connecting to %s:%d\n",
                      mRemoteIpAddress.data(), remoteHostPort);
#       endif
        struct in_addr* serverAddr;
        struct sockaddr_in serverSockAddr;

        if (!isIp)
        {
            serverAddr = (in_addr*) (server->h_addr);
            serverSockAddr.sin_family = server->h_addrtype;
            serverSockAddr.sin_port = htons(remoteHostPort);
            serverSockAddr.sin_addr.s_addr = (serverAddr->s_addr);
        }
        else
        {
            serverSockAddr.sin_family = AF_INET;
            serverSockAddr.sin_port = htons(remoteHostPort);
            serverSockAddr.sin_addr.s_addr = inet_addr(remoteHost);
        }

        // Set the default destination address for the socket
#       if defined(_WIN32) || defined(__pingtel_on_posix__)
        if(connect(socketDescriptor, (const struct sockaddr*) &serverSockAddr,
            sizeof(serverSockAddr)))
#       elif defined(_VXWORKS)
        if(connect(socketDescriptor, (struct sockaddr*) &serverSockAddr,
            sizeof(serverSockAddr)))
#       else
#       error Unsupported target platform.
#       endif
        {
            error = OsSocketGetERRNO();
            close();

            OsSysLog::add(FAC_KERNEL, PRI_ERR,
                          "OsDatagramSocket::_ connect failed: %d", error);
            goto EXIT;
        }

        mIsConnected = TRUE;

    }

EXIT:
    return;
}


// Destructor
OsDatagramSocket::~OsDatagramSocket()
{
    close();
    free(mpToSockaddr);
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsDatagramSocket&
OsDatagramSocket::operator=(const OsDatagramSocket& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean OsDatagramSocket::reconnect()
{
    osPrintf("WARNING: reconnect NOT implemented!\n");
    return(FALSE);
}

void OsDatagramSocket::doConnect(int remoteHostPortNum, const char* remoteHost,
                                 UtlBoolean simulateConnect)
{
    struct hostent* server;
    UtlString temp_output_address;
    int error = 0;

#   if 0 // turn on to debug connection problems
    OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                  "OsDatagramSocket::doConnect( %s:%d %s )",
                  remoteHost, remoteHostPortNum, simulateConnect ? "SIMULATE" : "REAL" );
#   endif

#   ifdef _VXWORKS
    char hostentBuf[512];
#   endif

    mToSockaddrValid = FALSE;
    memset(mpToSockaddr, 0, sizeof(struct sockaddr_in));
    remoteHostPort = remoteHostPortNum;
    if(remoteHost)
    {
        if(strlen(remoteHost) == 0)
        {
            osPrintf("OsDatagramSocket::doConnect with zero length remoteHost\n");
        }
        remoteHostName.remove(0);
        remoteHostName.append(remoteHost);
        getHostIpByName(remoteHostName.data(), &mRemoteIpAddress);
#ifdef TEST
        osPrintf("OsDatagramSocket::doConnect remoteIpAddress: \"%s\"\n",
            mRemoteIpAddress.data());
//        assert(!mRemoteIpAddress.isNull());
#endif
    }

    // Connect to a remote host if given
    if(remoteHostPort && remoteHost && !simulateConnect)
    {
#       if defined(_WIN32) || defined(__pingtel_on_posix__)
        server = gethostbyname(remoteHost);
#       elif defined(_VXWORKS)

        server = resolvGetHostByName((char*) remoteHost,
                                    hostentBuf, sizeof(hostentBuf));
#       else
#       error Unsupported target platform.
#       endif //_VXWORKS

        if(! server)
        {
            close();
            osPrintf("DNS failed to lookup host: %s\n", remoteHost);
            goto EXIT;
        }

        inet_ntoa_pt(*((in_addr*) (server->h_addr)), temp_output_address);
        osPrintf("OsDatagramSocket::doConnect: connecting to host at: %s port: %d\n", temp_output_address.data(), remoteHostPortNum);
        struct in_addr* serverAddr = (in_addr*) (server->h_addr);
        struct sockaddr_in serverSockAddr;
        serverSockAddr.sin_family = server->h_addrtype;
        serverSockAddr.sin_port = htons(remoteHostPort);
        serverSockAddr.sin_addr.s_addr = (serverAddr->s_addr);

        // Set the default destination address for the socket
#       if defined(_WIN32) || defined(__pingtel_on_posix__)
        if(connect(socketDescriptor, (const struct sockaddr*) &serverSockAddr,
            sizeof(serverSockAddr)))
#       elif defined(_VXWORKS)
        if(connect(socketDescriptor, (struct sockaddr*) &serverSockAddr,
            sizeof(serverSockAddr)))
#       else
#       error Unsupported target platform.
#       endif
        {
            error = OsSocketGetERRNO();
            close();
            perror("call to connect failed\n");
            osPrintf("connect call failed with error: %d\n", error);
            goto EXIT;
        }

        mIsConnected = TRUE;
    }
    else if(remoteHostPort && remoteHost && simulateConnect)
    {
        mIsConnected = TRUE;
        mSimulatedConnect = TRUE;
    }
EXIT:
    temp_output_address.remove(0);
}

int OsDatagramSocket::write(const char* buffer, int bufferLength)
{
    int returnCode;

    if(mSimulatedConnect)
    {
        returnCode = writeTo(buffer, bufferLength);
    }
    else
    {
        returnCode = OsSocket::write(buffer, bufferLength);
    }

    return(returnCode);
}

int OsDatagramSocket::write(const char* buffer, int bufferLength,
      const char* ipAddress, int port)
{
    int bytesSent = 0;

    struct sockaddr_in toSockAddress;
    toSockAddress.sin_family = AF_INET;
    toSockAddress.sin_port = htons(port);

    if(ipAddress == NULL || !strcmp(ipAddress, "0.0.0.0") ||
        strlen(ipAddress) == 0 ||
        (toSockAddress.sin_addr.s_addr = inet_addr(ipAddress)) ==
            OS_INVALID_INET_ADDRESS)
    {
        osPrintf("OsDatagramSocket::write invalid IP address: \"%s\"\n",
            ipAddress);
    }
    else
    {
        int flags = 0;

#if defined(__pingtel_on_posix__)
    // We do not want send to throw signals if there is a
    // problem with the socket as this results in the process
    // getting aborted.  We just want it to return an error
        flags = MSG_NOSIGNAL;
#endif


        // Why isn't this abstracted into OsSocket, as is done in ::write(2)?
        bytesSent = sendto(socketDescriptor,
#ifdef _VXWORKS
            (char*)
#endif
            buffer, bufferLength,
            flags,
            (struct sockaddr*) &toSockAddress, sizeof(struct sockaddr_in));

        if(bytesSent != bufferLength)
        {
           OsSysLog::add(FAC_SIP, PRI_ERR,
                         "OsDatagramSocket::write(4) bytesSent = %d, "
                         "bufferLength = %d, errno = %d",
                         bytesSent, bufferLength, errno);
            time_t rightNow;

            (void) time(&rightNow);

            mNumRecentWriteErrors++;

            if (MIN_REPORT_SECONDS <= (rightNow - mLastWriteErrorTime)) {

                mNumTotalWriteErrors += mNumRecentWriteErrors;
                if (0 == mNumTotalWriteErrors) {
                    mLastWriteErrorTime = rightNow;
                }
                osPrintf("OsDataGramSocket::write:\n"
                    "     In last %ld seconds: %d errors; total %d errors;"
                    " last errno=%d\n",
                    (rightNow - mLastWriteErrorTime), mNumRecentWriteErrors,
                    mNumTotalWriteErrors, OsSocketGetERRNO());

                mLastWriteErrorTime = rightNow;
                mNumRecentWriteErrors = 0;
            }
        }
    }
    return(bytesSent);
}

UtlBoolean OsDatagramSocket::getToSockaddr()
{
    const char* ipAddress = mRemoteIpAddress.data();

    if (!mToSockaddrValid) {
        mpToSockaddr->sin_family = AF_INET;
        mpToSockaddr->sin_port = htons(remoteHostPort);

        if (ipAddress == NULL || !strcmp(ipAddress, "0.0.0.0") ||
            strlen(ipAddress) == 0 ||
            (mpToSockaddr->sin_addr.s_addr = inet_addr(ipAddress)) ==
                OS_INVALID_INET_ADDRESS)
        {
/*
            osPrintf(
               "OsDatagramSocket::getToSockaddr: invalid IP address: \"%s\"\n",
                ipAddress);
*/
        } else {
            mToSockaddrValid = TRUE;
        }
    }
    return mToSockaddrValid;
}

int OsDatagramSocket::writeTo(const char* buffer, int bufferLength)
{
    int bytesSent = 0;

    if (getToSockaddr()) {
        int flags = 0;

#if defined(__pingtel_on_posix__)
    // We do not want send to throw signals if there is a
    // problem with the socket as this results in the process
    // getting aborted.  We just want it to return an error
        flags = MSG_NOSIGNAL;
#endif

        bytesSent = sendto(socketDescriptor,
#ifdef _VXWORKS
            (char*)
#endif
            buffer, bufferLength,
            flags,
            (struct sockaddr*) mpToSockaddr, sizeof(struct sockaddr_in));

        if(bytesSent != bufferLength)
        {
            time_t rightNow;

            (void) time(&rightNow);

            mNumRecentWriteErrors++;

            if (MIN_REPORT_SECONDS <= (rightNow - mLastWriteErrorTime)) {

                mNumTotalWriteErrors += mNumRecentWriteErrors;
                if (0 == mNumTotalWriteErrors) {
                    mLastWriteErrorTime = rightNow;
                }
                osPrintf("OsDataGramSocket::write:\n"
                    "     In last %ld seconds: %d errors; total %d errors;"
                    " last errno=%d\n",
                    (rightNow - mLastWriteErrorTime), mNumRecentWriteErrors,
                    mNumTotalWriteErrors, OsSocketGetERRNO());

                mLastWriteErrorTime = rightNow;
                mNumRecentWriteErrors = 0;
            }
        }
    }
    return(bytesSent);
}

#ifdef WANT_GET_FROM_INFO /* [ */
#ifdef _VXWORKS /* [ */
static int getFromInfo = 0;
extern "C" { extern int setFromInfo(int x);};
int setFromInfo(int getIt) {
   int save = getFromInfo;
   getFromInfo = getIt ? 1 : 0;
   return save;
}
#define GETFROMINFO getFromInfo
#endif /* _VXWORKS ] */
#endif /* WANT_GET_FROM_INFO ] */

int OsDatagramSocket::read(char* buffer, int bufferLength)
{
    int bytesRead;

    // If the remote end is not "connected" we cannot use recv
    if(mSimulatedConnect || remoteHostPort <= 0 || remoteHostName.isNull())
    {
#ifdef GETFROMINFO /* [ */
        if (GETFROMINFO) {
          int fromPort;
          UtlString fromAddress;
          bytesRead = OsSocket::read(buffer, bufferLength,
             &fromAddress, &fromPort);
          fromAddress.remove(0);
        } else
#endif /* GETFROMINFO ] */
        {
           bytesRead = OsSocket::read(buffer, bufferLength,
              (struct in_addr*) NULL, NULL);
        }
    }
    else
    {
        bytesRead = OsSocket::read(buffer, bufferLength);
    }
    return(bytesRead);
}

/* ============================ ACCESSORS ================================= */
int OsDatagramSocket::getIpProtocol() const
{
    return(UDP);
}

void OsDatagramSocket::getRemoteHostIp(struct in_addr* remoteHostAddress,
                               int* remotePort)
{
#ifdef __pingtel_on_posix__
    socklen_t len;
#else
    int len;
#endif
    struct sockaddr_in remoteAddr;
    const struct sockaddr_in* pAddr;

    if (mSimulatedConnect) {
        getToSockaddr();
        pAddr = mpToSockaddr;
    } else {
        pAddr = &remoteAddr;

        len = sizeof(struct sockaddr_in);

        if (getpeername(socketDescriptor, (struct sockaddr *)pAddr, &len) != 0)
        {
            memset(&remoteAddr, 0, sizeof(struct sockaddr_in));
        }
    }
    memcpy(remoteHostAddress, &(pAddr->sin_addr), sizeof(struct in_addr));

#ifdef TEST_PRINT
    {
        int p = ntohs(pAddr->sin_port);
        UtlString o;
        inet_ntoa_pt(*remoteHostAddress, o);
        osPrintf("getRemoteHostIP: Remote name: %s:%d\n"
           "   (pAddr->sin_addr) = 0x%X, sizeof(struct in_addr) = %d\n",
           o.data(), p, (pAddr->sin_addr), sizeof(struct in_addr));
    }
#endif

    if (NULL != remotePort)
    {
        *remotePort = ntohs(pAddr->sin_port);
    }
}

// Return the external IP address for this socket.
UtlBoolean OsDatagramSocket::getExternalIp(UtlString* ip, int* port) 
{
    return FALSE ;
}
    

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
