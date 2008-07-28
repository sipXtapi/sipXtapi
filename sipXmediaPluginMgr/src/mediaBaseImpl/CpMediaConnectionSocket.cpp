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
// Copyright (C) 2005-2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>
#if defined(__pingtel_on_posix__)
#   include <arpa/inet.h>
#endif

// APPLICATION INCLUDES
#include "mediaBaseImpl/CpMediaConnectionSocket.h"
#include "mediaBaseImpl/CpMediaServer.h"
#include "mediaBaseImpl/CpMediaNetTask.h"
#include "mediaBaseImpl/CpMediaServer.h"
#include "os/TurnMessage.h"
#include "os/StunMessage.h"
#include "os/OsProtectEvent.h"
#include "os/OsProtectEventMgr.h"
#include "utl/UtlInt.h"
#include "utl/UtlHashMapIterator.h"

#include "os/OsLock.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
// FORWARD DECLARATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaConnectionSocket::CpMediaConnectionSocket(    int type,
                                                     int remoteHostPortNum,
                                                     const char* remoteHost, 
                                                     int localHostPortNum, 
                                                     const char* localHost,
                                                     const char* szTurnServer,
                                                     const RtpTcpRoles role)
        : 
          OsNatConnectionSocket(remoteHostPortNum, remoteHost, true, localHost, false, role),
          mEnabledMutex(OsMutex::Q_FIFO),
          mMediaServerMutex(OsMutex::Q_FIFO),
          mbEnabled(false),
          mFragmentSize(0)
{    
    localHostPort = localHostPortNum;
    miType = type ;
    mPreferredReceivePort = 0 ;

    // create the server socket, and wait for a connection
    if ((role & RTP_TCP_ROLE_ACTIVE) != RTP_TCP_ROLE_ACTIVE)
    {
        mMediaServerMutex.acquireWrite();
        mpMediaServer = new CpMediaServer(this, localHost, localHostPortNum);
        mpMediaServer->start();
        mMediaServerMutex.releaseWrite();
    }
    else
    {
        mpMediaServer = NULL;
    }
    memset(mRtpContexts, 0, sizeof(mRtpContexts)) ;
    OsSocket* pDatagramSocket = NULL;
}


// Destructor
CpMediaConnectionSocket::~CpMediaConnectionSocket()
{    
    OsTime maxEventTime(1, 0);

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    CpMediaNetTask* pNetTask = CpMediaNetTask::getCpMediaNetTask();

    // remove as input source
    OsNatConnectionSocket* pConnectionSocket = NULL;
    UtlString* pKey;
    UtlInt* pSocketContainer = NULL;
    UtlHashMapIterator iterator(mClientConnectionSockets);
    OsProtectedEvent* rtpSocketRemoveEvent = eventMgr->alloc();
    while (pKey = (UtlString*)iterator())
    {
        pSocketContainer = (UtlInt*)mClientConnectionSockets.findValue(pKey);
        if (pSocketContainer)
        {
            pConnectionSocket = (OsNatConnectionSocket*)pSocketContainer->getValue();
            if (pConnectionSocket)
            {
                pConnectionSocket->close();
                pNetTask->removeInputSource((IMediaSocket*) pConnectionSocket, rtpSocketRemoveEvent) ;
                if (rtpSocketRemoveEvent->wait(0, maxEventTime) != OS_SUCCESS)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR, 
                            " *** VoiceEngineMediaInterface: failed to wait for audio rtp socket release") ;
                }
                eventMgr->release(rtpSocketRemoveEvent);
                delete pConnectionSocket;
            }
        }
    }

    close();
    destroy();
    mMediaServerMutex.acquireWrite();
    delete mpMediaServer;
    mMediaServerMutex.releaseWrite();
}

void CpMediaConnectionSocket::setRole(const RtpTcpRoles role)
{
    RtpTcpRoles origRole = getRole();
    if (origRole == RTP_TCP_ROLE_ACTPASS && role == RTP_TCP_ROLE_PASSIVE)
    {
        // looks like we just learned that we are RTP_TCP_ROLE_PASSIVE, so remove the client connection
        close();
    
        // add the input source
        CpMediaNetTask* pTask = CpMediaNetTask::getCpMediaNetTask();
        OsNatConnectionSocket::setRole(role);
    }
    if (origRole == RTP_TCP_ROLE_ACTPASS && role == RTP_TCP_ROLE_ACTIVE)
    {
        // looks like we just learned that we are RTP_TCP_ROLE_ACTIVE, 
        // so, remove the server
        mMediaServerMutex.acquireWrite();
        if (mpMediaServer)
        {
            // looks like we just learned that we are RTP_TCP_ROLE_ACTIVE, so 
            // shut down
            // the server
            OsConnectionSocket* connSock = mpMediaServer->acquireConnectionSocket();
            if (connSock)
            {
                connSock->close();
            }
            mpMediaServer->releaseConnectionSocket();
            delete mpMediaServer;
            mpMediaServer = NULL;
        }
        mMediaServerMutex.releaseWrite();
        OsNatConnectionSocket::setRole(role);
    }
}

int CpMediaConnectionSocket::getSocketDescriptor() const
{
    int descriptor = 0;
    RtpTcpRoles role = getRole();
    if (RTP_TCP_ROLE_PASSIVE == role && mpMediaServer)
    {
        mMediaServerMutex.acquireRead();
        OsConnectionSocket* pReadSocket = mpMediaServer->acquireConnectionSocket();
        if (pReadSocket)
        {
            descriptor = pReadSocket->getSocketDescriptor();
        }
        mpMediaServer->releaseConnectionSocket();
        mMediaServerMutex.releaseRead();
    }
    else
    {
        descriptor = OsSocket::getSocketDescriptor();
    }
    return descriptor;
}

int CpMediaConnectionSocket::write(const char* buffer,
                  int bufferLength,
                  const char* ipAddress,
                  int port)
{
    return socketWrite(buffer, bufferLength, ipAddress, port, UNKNOWN_PACKET);
}                  

/* ============================ MANIPULATORS ============================== */

int CpMediaConnectionSocket::readPacket(char* pData, const int maxSize, UtlString& address, int& port)
{
    int bytes = -1;
    {    
        RtpTcpRoles role = getRole();
        if (RTP_TCP_ROLE_PASSIVE == role)
        {
            mMediaServerMutex.acquireRead();
            if (mpMediaServer)
            {
                OsConnectionSocket* pReadSocket = mpMediaServer->acquireConnectionSocket();
                if (pReadSocket)
                {
                    bytes = pReadSocket->read(pData, maxSize, &address, &port);
                }
                mpMediaServer->releaseConnectionSocket();
            }
            mMediaServerMutex.releaseRead();
        }
        else if (RTP_TCP_ROLE_ACTIVE == role || RTP_TCP_ROLE_CONNECTION == role)
        {
            bytes = read(pData, maxSize, &address, &port) ;
        }
    }
        
    if (bytes > 0)
    {
        handleFramedStream(pData, bytes, address.data(), port);
            
        // Do transport debugging
#ifdef TRANSPORT_DUMP
        {
            char cLabel[64] ;
            sprintf(cLabel, "pushPacket type=%d (IN)", miType) ;
            transportDump(cLabel, cBuf, bytes) ;
        }
#endif
    }
    return bytes;
}
/*
void VoiceEngineConnectionSocket::processStreamBuffer(char* pData,
                                                      const int size, 
                                                      const int port)
{
    unsigned int streamBufferSize = size;

    // first memcpy in any leftover fragments
    if (mFragmentSize)
    {
        memcpy(mszStreamBuffer, mszFragment, mFragmentSize);
        streamBufferSize += mFragmentSize;
    }
    memcpy(mszStreamBuffer + mFragmentSize, pData, size);
    mFragmentSize = 0;
    
    u_short packLen;
    memcpy(&packLen, mszStreamBuffer, sizeof(u_short));
    packLen = ntohs(packLen);
    assert(packLen <= CHUNK_SIZE);
    
    char* pStreamBuffer = mszStreamBuffer;
    while (streamBufferSize >= (packLen + sizeof(u_short) ) )
    {
        pushPacket(pStreamBuffer + sizeof(u_short), packLen, port);
        streamBufferSize -= packLen + sizeof(u_short);
        pStreamBuffer += packLen + sizeof(u_short);        

        if (streamBufferSize >= sizeof(short))
        {
            memcpy(&packLen, pStreamBuffer, sizeof(u_short));
            packLen = ntohs(packLen);
            assert(packLen <= CHUNK_SIZE);
        }
        else
        {
            break;
        }

    }
    mFragmentSize = streamBufferSize;
    if (mFragmentSize)
    {
        memcpy(mszFragment, pStreamBuffer, mFragmentSize);
    }
}
*/
bool CpMediaConnectionSocket::pushPacket()
{
    UtlString address ;
    int port ;
    char cBuf[MAX_RTP_BYTES + 4] ;
    bool bSuccess = true ;


    if (readPacket(cBuf, sizeof(cBuf), address, port) < 0)
    {
        bSuccess = false ;
    }
    return bSuccess;
}
bool CpMediaConnectionSocket::handleUnframedBuffer(const TURN_FRAMING_TYPE type,
                                                       const char* buff,
                                                       const int buffSize,
                                                       const char* receivedIp,
                                                       const int port)
{
    bool bHandled = OsNatConnectionSocket::handleUnframedBuffer(type, buff, buffSize, receivedIp, port);
    if (!bHandled)
    {
        pushPacket((char*)buff, buffSize, port);
    }
    return true;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
bool CpMediaConnectionSocket::validRtpPacket(int         type,
                                               const char* pBuf, 
                                               int         nBuf,
                                               const char* szFromAddress,
                                               int         iFromPort)
{
    bool bValid = false ;

    if (nBuf >= sizeof(RTP_HEADER))
    {
        RTP_HEADER* pHeader = (RTP_HEADER*) pBuf ;
        if ((pHeader->vpxcc & 0xC0) == 0x80)
        {
            if (!mRtpContexts[type].bInitialized)
            {
                // Lock onto first RTP packet
                syncToSource(type, pHeader, szFromAddress, iFromPort) ;
                bValid = true ;
            }
            else
            {
                if (mRtpContexts[type].seq_number == ntohs(pHeader->seq_number) &&
                        (mRtpContexts[type].fromAddress.compareTo(szFromAddress) == 0) &&
                        (iFromPort == mRtpContexts[type].fromPort))
                {
                    // Expected value -- same seq number from same source
                    bValid = true ;
                    mRtpContexts[type].mismatches = 0; 
                }
                else
                {
                    mRtpContexts[type].mismatches++ ;

                    if ((mPreferredReceiveAddress.compareTo(szFromAddress) == 0) &&                     
                            mPreferredReceivePort == iFromPort)
                    {
                        // Always switch to preferred source
                        syncToSource(type, pHeader, szFromAddress, iFromPort) ;
                        bValid = true ;
                    } 
                    else if ((mPreferredReceiveAddress.compareTo(szFromAddress) == 0) &&
                            mRtpContexts[type].fromAddress.compareTo(szFromAddress) != 0)
                    {
                        // If we just receive something from the preferred IP (but not
                        // preferred port, switch if the old-sync was a different IP.

                        syncToSource(type, pHeader, szFromAddress, iFromPort) ;
                        bValid = true ;
                    }
                    else if (mRtpContexts[type].mismatches >= 13)
                    {
                        // Lastly, only switch to a new source if the previous source
                        // stopped sending for more then 260 ms.

                        syncToSource(type, pHeader, szFromAddress, iFromPort) ;
                        bValid = true ;
                    }
                }
            }           
        }
    }

    return bValid ;
}

void CpMediaConnectionSocket::syncToSource(int         type,
                                             RTP_HEADER* pHeader,
                                             const char* szFromAddress, 
                                             int         iFromPort)
{
    mRtpContexts[type].payloadType = pHeader->mp & 0x7F ;
    mRtpContexts[type].seq_number = ntohs(pHeader->seq_number) ;
    mRtpContexts[type].SSRC = ntohl(pHeader->SSRC) ;
    mRtpContexts[type].fromAddress = szFromAddress ;
    mRtpContexts[type].fromPort = iFromPort ;
    mRtpContexts[type].mismatches = 0; 
}

void CpMediaConnectionSocket::setEnabled(bool bEnabled)
{
    OsLock lock(mEnabledMutex);
    mbEnabled = bEnabled ;
    
    OsNatConnectionSocket* pConnectionSocket = NULL;
    UtlInt* pSocketContainer = NULL;
    UtlString* pKey;
    UtlHashMapIterator iterator(mClientConnectionSockets);
    while (pKey = (UtlString*)iterator())
    {
        pSocketContainer = (UtlInt*) mClientConnectionSockets.findValue(pKey);
        if (pSocketContainer)
        {
            pConnectionSocket = (OsNatConnectionSocket*)pSocketContainer->getValue();
            if (pConnectionSocket)
            {
                CpMediaConnectionSocket* pVoiceEngineConnectionSocket = dynamic_cast<CpMediaConnectionSocket*>(pConnectionSocket);
                if (pVoiceEngineConnectionSocket)
                {
                    pVoiceEngineConnectionSocket->setEnabled(true);
                }
            }
        }
    }    
}

                                    
int CpMediaConnectionSocket::write(const char* buffer, int bufferLength)
{
    int ret = 0;
    RtpTcpRoles role = getRole();
    if (RTP_TCP_ROLE_ACTIVE == role || 
        RTP_TCP_ROLE_ACTPASS == role || 
        RTP_TCP_ROLE_CONNECTION == role )
    {
        ret = OsNatConnectionSocket::write(buffer, bufferLength);
    }    
    else if (RTP_TCP_ROLE_PASSIVE == role)
    {
        mMediaServerMutex.acquireRead();
        if (mpMediaServer)
        {
            OsConnectionSocket* pWriteSocket = mpMediaServer->acquireConnectionSocket();
            if (pWriteSocket)
            {
                ret = pWriteSocket->write(buffer, bufferLength);
            }
            mpMediaServer->releaseConnectionSocket();

        }
        mMediaServerMutex.releaseRead();
    }
    return ret;
}                  

int CpMediaConnectionSocket::connect()
{
    return OsNatConnectionSocket::connect();
}


#ifdef TRANSPORT_DUMP
void CpMediaConnectionSocket::transportDump(const char* szLabel,     
                                              const char* pBuf,
                                              int         nBuf) 
{
    UtlString output ;

    output.append(szLabel) ;
    output.append(":\r\n") ;

    for (int i=0; i<nBuf; i++)
    {
        char cTemp[20] ;
        memset(cTemp, 0, sizeof(cTemp)) ;
        sprintf(cTemp, "%02X", (unsigned char) pBuf[i]) ;
        output.append(cTemp) ;
        if (((i+1) % 16) == 0)
        {
            output.append("\r\n") ;
        }    
    }

    output.append("\r\n") ;

    FILE *fp = fopen(TRANSPORT_DUMP_FILE, "at") ;
    if (fp)
    {
        fwrite(output.data(), 1, output.length(), fp) ;
        fclose(fp) ;
    }
}

#endif
/* //////////////////////////// PRIVATE /////////////////////////////////// */

                                                   


/* ============================ FUNCTIONS ================================= */
