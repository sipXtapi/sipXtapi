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
#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

// APPLICATION INCLUDES
#include "os/OsNatConnectionSocket.h"
#include "os/OsNatAgentTask.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "os/OsEvent.h"
#include "tapi/sipXtapi.h"
#include "os/OsProtectEvent.h"
#include "os/OsProtectEventMgr.h"
#include "utl/UtlInt.h"
#include "utl/UtlHashMapIterator.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// FORWARD DECLARATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsNatConnectionSocket::OsNatConnectionSocket(int connectedSocketDescriptor,
                                             const RtpTcpRoles role)
    : OsConnectionSocket(connectedSocketDescriptor),
      mRole(role),
      mRoleMutex(OsMutex::Q_FIFO),
      mStreamHandlerMutex(OsMutex::Q_FIFO),
      mFragmentSize(0)
{    
    if (0 == mRole)
    {
        mRole = RTP_TCP_ROLE_ACTPASS;
    }
    socketDescriptor = connectedSocketDescriptor;    
    mDestAddress = mRemoteIpAddress ;
    miDestPort = remoteHostPort ;
}

OsNatConnectionSocket::OsNatConnectionSocket(const char* szLocalIp,
                                             int connectedSocketDescriptor,
                                             const RtpTcpRoles role)
    : OsConnectionSocket(szLocalIp, connectedSocketDescriptor),
      mRole(role),
      mRoleMutex(OsMutex::Q_FIFO),
      mStreamHandlerMutex(OsMutex::Q_FIFO),
      mFragmentSize(0)
      
{
    if (0 == mRole)
    {
        mRole = RTP_TCP_ROLE_ACTPASS;
    }
    socketDescriptor = connectedSocketDescriptor;
    mLocalIp = szLocalIp;
    mDestAddress = mRemoteIpAddress ;
    miDestPort = remoteHostPort ;
}

// Constructor
OsNatConnectionSocket::OsNatConnectionSocket(int serverPort,
                                       const char* serverName,
                                       UtlBoolean blockingConnect,
                                       const char* localIp,
                                       const bool bConnect,
                                       const RtpTcpRoles role)
        : OsConnectionSocket(serverPort, serverName, blockingConnect, localIp, bConnect),
          mRole(role),
          mRoleMutex(OsMutex::Q_FIFO),
          mStreamHandlerMutex(OsMutex::Q_FIFO),
          mFragmentSize(0)
{    
    if (0 == mRole)
    {
        mRole = RTP_TCP_ROLE_ACTPASS;
    }
           
    // Init other attributes
    mDestAddress = mRemoteIpAddress ;
    miDestPort = remoteHostPort ;
}


// Destructor
OsNatConnectionSocket::~OsNatConnectionSocket()
{
    destroy();
}
 
 
void OsNatConnectionSocket::setRole(const RtpTcpRoles role)
{
    mRoleMutex.acquireWrite();
    mRole = role;
    mRoleMutex.releaseWrite();
}
const RtpTcpRoles OsNatConnectionSocket::getRole() const
{
    RtpTcpRoles role;
    mRoleMutex.acquireRead();
    role = mRole;
    mRoleMutex.releaseRead();
    return role;
}
 
void OsNatConnectionSocket::destroy()
{  
}

/* ============================ MANIPULATORS ============================== */


int OsNatConnectionSocket::read(char* buffer, int bufferLength)
{
    int iRC ;
    UtlString receivedIp ;
    int iReceivedPort ;

    iRC = OsSocket::read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;
    handleFramedStream(buffer, bufferLength,  receivedIp.data(), iReceivedPort);

    return iRC ;
}

int OsNatConnectionSocket::read(char* buffer, int bufferLength,
       UtlString* ipAddress, int* port)
{
    int iRC ;

    iRC = OsSocket::read(buffer, bufferLength, ipAddress, port) ;       
    handleFramedStream(buffer, iRC, ipAddress->data(), *port);


    return iRC ;
}

int OsNatConnectionSocket::read(char* buffer, int bufferLength,
       struct in_addr* ipAddress, int* port)
{
    int iRC ;
    int iReceivedPort ;
    UtlString receivedIp ;

    iRC = read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;
   
    if (ipAddress)
        ipAddress->s_addr = inet_addr(receivedIp) ;

    if (port)
        *port = iReceivedPort ;

    handleFramedStream(buffer, bufferLength, receivedIp.data(), iReceivedPort);

    return iRC ;
}


int OsNatConnectionSocket::read(char* buffer, int bufferLength, long waitMilliseconds)
{        
    bool bNatPacket = FALSE ;
    int iRC = 0 ;
    UtlString receivedIp ;
    int iReceivedPort ;

    do
    {
        if (isReadyToRead(waitMilliseconds))
        {
            bNatPacket = FALSE ;
            iRC = OsSocket::read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;            
            if (handleSturnData(buffer, iRC, receivedIp, iReceivedPort, NULL, NULL))
            {
                if (!getTransparentStunRead())
                    iRC = 0 ;
                else
                    bNatPacket = TRUE ;
            }
        }
        else
        {
            break ;
        }
    } while ((iRC >= 0) && bNatPacket) ;

    // Make read time for non-NAT packets
    if (iRC > 0 && !bNatPacket)
    {
        markReadTime() ;
        checkDelayedShutdown() ;
    }

    return iRC ;    
}


int OsNatConnectionSocket::write(const char* buffer, int bufferLength)
{
    int framedLength = 0;
    const char* framedBuffer = frameBuffer(STUN /* use the STUN octet*/, buffer, bufferLength, framedLength);
    int written = 0;
    markWriteTime() ;
    written = OsConnectionSocket::write(framedBuffer, framedLength);
    free((void*)framedBuffer);
    return written;
}


int OsNatConnectionSocket::socketWrite(const char* buffer, int bufferLength,
                               const char* ipAddress, int port, OS_NAT_PACKET_TYPE packetType)
{
    TURN_FRAMING_TYPE type = STUN;
    
    // ironically, STUN probes must be sent with the DATA type, not the STUN type,
    // all other requests are written with the STUN (TURN) type.
    if (STUN_PROBE_PACKET == packetType)
    {
        type = DATA;
    }
    int framedLength = 0;
    const char* framedBuffer = frameBuffer(type, buffer, bufferLength, framedLength);
    
    int written = 0;
    if (MEDIA_PACKET == packetType)
    {
        markWriteTime() ;
    }
    written = OsConnectionSocket::write(framedBuffer, framedLength, ipAddress, port) ;
    free((void*)framedBuffer);
    return written;
}

int OsNatConnectionSocket::write(const char* buffer, int bufferLength,
                               const char* ipAddress, int port)
{
    int framedLength = 0;
    const char* framedBuffer = frameBuffer(STUN, buffer, bufferLength, framedLength);
    
    int written = 0;
    markWriteTime() ;
    written = OsConnectionSocket::write(framedBuffer, framedLength, ipAddress, port) ;
    free((void*)framedBuffer);
    return written;
}


int OsNatConnectionSocket::write(const char* buffer, int bufferLength, 
                               long waitMilliseconds)
{
    int framedLength = 0;
    const char* framedBuffer = frameBuffer(STUN, buffer, bufferLength, framedLength);
    
    int written = 0;
    markWriteTime() ;
    written = OsConnectionSocket::write(framedBuffer, framedLength, waitMilliseconds) ;
    free((void*)framedBuffer);
    return written;
}


void OsNatConnectionSocket::enableTurn(const char* szTurnServer,
                                     int turnPort,
                                     int iKeepAlive,
                                     const char* username,
                                     const char* password,
                                     bool bReadFromSocket)
{
    if (!isClientConnected(szTurnServer, turnPort))
    {
        clientConnect(szTurnServer, turnPort);
    }

    OsNatSocketBaseImpl::enableTurn(szTurnServer, turnPort, iKeepAlive, username, password, bReadFromSocket) ;
}

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


const char* OsNatConnectionSocket::frameBuffer(TURN_FRAMING_TYPE type,
                                                    const char* buffer,
                                                    const int bufferLength,
                                                    int& framedBufferLen)
{
   /*
    *   0                   1                   2                   3
    *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    *  |     Type      |  Reserved = 0 |            Length             |
    *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    *
    */
    char* szFramedBuffer = (char*) malloc(MAX_RTP_BYTES + 4);
    
    
    u_short typeByte = 0;
    typeByte = type;
    typeByte = typeByte << 8;
    typeByte = htons((u_short) typeByte);
    memcpy(szFramedBuffer, &typeByte, sizeof(u_short));

    u_short packetLength = htons((u_short) bufferLength);
    memcpy(szFramedBuffer + sizeof(u_short), &packetLength, sizeof(u_short));
    
    memcpy(szFramedBuffer + (2 * sizeof(u_short)), buffer, bufferLength);
    framedBufferLen = bufferLength + (2 * sizeof(u_short));
    return szFramedBuffer;
}

void OsNatConnectionSocket::handleFramedStream(       char* pData,
                                                      const int size,
                                                      const char* receivedIp,
                                                      const int port)
{
    mStreamHandlerMutex.acquire();
    char szStreamChunk [(MAX_RTP_BYTES + 4) * 2];
    
    unsigned int streamBufferSize = size;
    if (size < 1 || size > (MAX_RTP_BYTES+4))
    {
        mStreamHandlerMutex.release();
        return;  // can't handle this
    }

    // first memcpy in any leftover fragments
    if (mFragmentSize < 0 || mFragmentSize > ((MAX_RTP_BYTES + 4) * 2))
    {
        assert(false);
        mFragmentSize = 0;
    }
    if (mFragmentSize)
    {
        memcpy(szStreamChunk, mszFragment, mFragmentSize);
        memcpy(szStreamChunk + mFragmentSize, pData, size);
        streamBufferSize += mFragmentSize;
        mFragmentSize = 0;        
    }
    else
    {
        memcpy(szStreamChunk, pData, size);
    }

    
    u_short packLen;
    memcpy(&packLen, szStreamChunk + sizeof(u_short), sizeof(u_short));
    packLen = ntohs(packLen);
    assert(packLen > 0);
    assert(packLen <= MAX_RTP_BYTES);
    
    if (packLen > MAX_RTP_BYTES)
    {
        OsSysLog::add(FAC_STREAMING, PRI_DEBUG, "OsNatConnectionSocket::handleFramedStream - Received invalid framing header.");
        mStreamHandlerMutex.release();
        return;
    }
    
    TURN_FRAMING_TYPE type;
    char* pStreamBuffer = szStreamChunk;
    if (streamBufferSize >= (packLen + (2*sizeof(u_short))) )
    {
        u_short typeByte = *pStreamBuffer;
        typeByte = ntohs((u_short)typeByte);
        type = (TURN_FRAMING_TYPE)typeByte;
        
        handleUnframedBuffer(type, pStreamBuffer + (2*sizeof(u_short)), packLen, receivedIp, port);

        streamBufferSize = streamBufferSize - packLen - (2*sizeof(u_short));
        pStreamBuffer = pStreamBuffer + (2*sizeof(u_short)) + packLen;        

        if (streamBufferSize)
        {
            handleFramedStream(pStreamBuffer, streamBufferSize, receivedIp, port);
        }
    }
    else
    {
        mFragmentSize = streamBufferSize;
        if (mFragmentSize)
        {
            memcpy(mszFragment, pStreamBuffer, mFragmentSize);
        }
      
    }
    mStreamHandlerMutex.release();
    return;    
}                      

 bool OsNatConnectionSocket::handleUnframedBuffer(const TURN_FRAMING_TYPE type,
                                        const char* buff,
                                        const int buffSize,
                                        const char* receivedIp,
                                        const int port)
{
    bool bNatPacket = false;
    
    int iRC = buffSize;
    UtlString sReceivedIp(receivedIp);
    int actPort = port ;
    bool bHandled = handleSturnData((char*)buff, iRC, sReceivedIp, actPort, NULL, NULL);
    if (iRC)
    {
        if (!getTransparentStunRead())
            iRC = 0 ;
        else
            bNatPacket = TRUE ;
    }
    
    // Make read time for non-NAT packets
    if (iRC > 0 && !bNatPacket)
    {
        markReadTime() ;
        checkDelayedShutdown() ;
    }
    
    return bHandled;
}                                    

void OsNatConnectionSocket::addClientConnection(const char* ipAddress, const int port, OsNatConnectionSocket* pClient)
{
    char szPort[16];
    sprintf(szPort, "%d", port);
    UtlString* key = new UtlString();;
    *key = UtlString(ipAddress) + UtlString(":") + UtlString(szPort);
    UtlInt* container = new UtlInt((int) pClient);
    mClientConnectionSockets.insertKeyAndValue(key, container);

}

int OsNatConnectionSocket::clientConnect(const char* szServer, const int port)
{
    int iRet = -1;
    OsNatConnectionSocket* pClient = getClientConnection(szServer, port);
    if (pClient && !pClient->isConnected())
    {
        iRet = pClient->connect();
    }

    return iRet;
}

bool OsNatConnectionSocket::isClientConnected(const char* szServer, const int port)
{
    bool bRet = false;
    OsNatConnectionSocket* pClient = getClientConnection(szServer, port);
    if (pClient)
    {
        bRet = pClient->isConnected();
    }
    return bRet;
}

OsNatConnectionSocket* OsNatConnectionSocket::getClientConnection(const char* szServer, const int port)
{
     OsNatConnectionSocket* pClient = NULL;
    
    UtlInt* pSocketContainer = NULL;
    UtlString key(szServer);
    key += ":";
    char szPort[16];
    sprintf(szPort, "%d", port);
    key += szPort;
    pSocketContainer = (UtlInt*)mClientConnectionSockets.findValue(&key);
    if (pSocketContainer)
    {
        pClient = (OsNatConnectionSocket*)pSocketContainer->getValue();
    }
    
    return pClient;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

/* ///////////////////////// HELPER CLASSES /////////////////////////////// */

