// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
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
#   include <winsock2.h>
#   include <Ws2tcpip.h>
#elif defined(_VXWORKS)
#   include <inetLib.h>
#   include <netdb.h>
#   include <resolvLib.h>
#   include <sockLib.h>
#elif defined(__pingtel_on_posix__)
#   include <netinet/in.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <resolv.h>
#else
#error Unsupported target platform.
#endif


// APPLICATION INCLUDES
#include <os/OsMulticastSocket.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsMulticastSocket::OsMulticastSocket(int multicastPortNum, const char* multicastHost,
                                     int localHostPortNum, const char* localHost)
: OsNatDatagramSocket(-1, NULL, localHostPortNum, localHost)
{
    // Avoid EADDRINUSE error on bind()
    int iTmp = TRUE;
    if (setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, (char *)&iTmp,
                   sizeof(iTmp)))
    {
        osPrintf("setsockopt SO_REUSEADDR call failed with error: %d\n", OsSocketGetERRNO());
    }
#ifdef SO_REUSEPORT
    if (setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEPORT, (char *)&iTmp,
                   sizeof(iTmp)))
    {
        osPrintf("setsockopt SO_REUSEPORT call failed with error: %d\n", OsSocketGetERRNO());
    }
#endif

    // Bind to the socket
    bind(localHostPortNum, localHost);
    if (!isOk())
    {
        goto EXIT;
    }

    // Simulate connect: Set multicast address as the default destination
    doConnect(multicastPortNum, multicastHost, TRUE);

    // Join the multicast group
    joinGroup(multicastHost);

EXIT:
    return;
}

// Destructor
OsMulticastSocket::~OsMulticastSocket()
{
    close();
}

/* ============================ MANIPULATORS ============================== */

int OsMulticastSocket::joinGroup(const char* multicastHostName)
{
    int error = 0;
    struct hostent* server = NULL;

#if defined(_WIN32) || defined(__pingtel_on_posix__)
    server = gethostbyname(multicastHostName);

#elif defined(_VXWORKS)
    char hostentBuf[512];
    server = resolvGetHostByName((char*) multicastHostName,
                                 hostentBuf, sizeof(hostentBuf));
#else
#   error Unsupported target platform.
#endif //_VXWORKS

    if (server == NULL)
    {
        error = OsSocketGetERRNO();
        close();
        osPrintf("gethostbyname(%s) call failed with error: %d\n",multicastHostName,
                 error);
        return error;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr = *((in_addr*) (server->h_addr));

    mreq.imr_interface.s_addr = OsSocket::getDefaultBindAddress();
    //mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    // NOTE: If this setsockopt() call return error 1042 under Windows, it mean
    //       you linked your program with wsock32.lib instead of ws2_32.lib.
    //       If you really want to link with wsock32.lib, change include
    //       to point to <winsock.h> instead of <winsock2.h>. This is known
    //       Winsock bug:
    //       http://blogs.msdn.com/wndp/archive/2006/12/18/winsock-header-weirdness.aspx
    if (setsockopt(socketDescriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)))
    {
        error = OsSocketGetERRNO();
        //close();
        osPrintf("setsockopt IP_ADD_MEMBERSHIP call failed with error: %d\n", error);
    }

    return error;
}

int OsMulticastSocket::setHopCount(unsigned char hopCount)
{
    int error = 0;

    if (setsockopt(socketDescriptor, IPPROTO_IP, IP_MULTICAST_TTL,
                   (const char *)&hopCount, sizeof(hopCount)))
    {
        error = OsSocketGetERRNO();
        //close();
        osPrintf("setsockopt IP_MULTICAST_TTL call failed with error: %d\n", error);
    }

    return error;
}

int OsMulticastSocket::setLoopback(bool enabled)
{
    int error = 0;
    unsigned char loopValue = enabled ? 1 : 0;

    if (setsockopt(socketDescriptor, IPPROTO_IP, IP_MULTICAST_LOOP,
                   (const char *)&loopValue, sizeof(loopValue)))
    {
        error = OsSocketGetERRNO();
        //close();
        osPrintf("setsockopt IP_MULTICAST_LOOP call failed with error: %d\n", error);
    }

    return error;
}

/* ============================ ACCESSORS ================================= */

OsSocket::IpProtocolSocketType OsMulticastSocket::getIpProtocol() const
{
    return(MULTICAST);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
