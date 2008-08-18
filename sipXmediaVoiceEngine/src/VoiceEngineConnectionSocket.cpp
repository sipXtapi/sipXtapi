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
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
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
#include "include/VoiceEngineConnectionSocket.h"
#include "mediaBaseImpl/CpMediaServer.h"
#include "mediaBaseImpl/CpMediaNetTask.h"
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
VoiceEngineConnectionSocket::VoiceEngineConnectionSocket(GipsVoiceEngineLib* pVoiceEngine,
                                                     GipsVideoEnginePlatform* pVideoEngine,
                                                     int type,
                                                     int remoteHostPortNum,
                                                     const char* remoteHost, 
                                                     int localHostPortNum, 
                                                     const char* localHost,
                                                     const char* szTurnServer,
                                                     const RtpTcpRoles role)
        : 
          CpMediaConnectionSocket(type, remoteHostPortNum, remoteHost, true, localHost, false, role)
{    
    mpVoiceEngine = pVoiceEngine ;
    mpVideoEngine = pVideoEngine ;
}


// Destructor
VoiceEngineConnectionSocket::~VoiceEngineConnectionSocket()
{    
    mpVoiceEngine = NULL ;
    mpVideoEngine = NULL ;
}



int VoiceEngineConnectionSocket::socketWrite(const char* buffer, int bufferLength,
                               const char* ipAddress, int port, OS_NAT_PACKET_TYPE packetType)
{
    RtpTcpRoles role = getRole();
    int bytesWritten = 0;
    VoiceEngineConnectionSocket* pClientWriteSocket = NULL;    
    if (role == RTP_TCP_ROLE_ACTIVE || role == RTP_TCP_ROLE_ACTPASS)
    {
        role = RTP_TCP_ROLE_ACTIVE;
        /*
        if (packetType == MEDIA_PACKET && isClientConnected(ipAddress, port))
        {
            pClientW
            pClientWriteSocket = this;
        }
        else if (packetType == MEDIA_PACKET && !isClientConnected(ipAddress, port))
        {
            // if it is the first time for a media packet, 
            // connect to the media destination
            initialize(ipAddress, port, true);
            mRemoteIpAddress = ipAddress;
            remoteHostPort = port;
            // try to connect
            int retVal = connect();
            int connectRetries = 2;
            int connectRetryInterval = 100;
            for (int i = 0; i < connectRetries && retVal != 0; i++)
            {
                OsTask::delay(connectRetryInterval * (i + 1));
                retVal = clientConnect(ipAddress, port);
            }
            // add the this pointer to our collection of clientConnections
            addClientConnection(ipAddress, port, this);
            CpMediaNetTask::getCpMediaNetTask()->addInputSource(this);
            pClientWriteSocket = this;
            role = RTP_TCP_ROLE_ACTIVE;
        }
        else
        */
        {
            UtlString localHost;
            getLocalHostIp(&localHost);
            pClientWriteSocket = (VoiceEngineConnectionSocket*) getClientConnection(ipAddress, port);
            if (!pClientWriteSocket)
            {
                UtlString localHostIp;
                getLocalHostIp(&localHostIp);
                pClientWriteSocket = new VoiceEngineConnectionSocket(mpVoiceEngine,
                                                     mpVideoEngine,
                                                     miType,
                                                     port,
                                                     ipAddress,
                                                     getLocalHostPort(),
                                                     localHost,
                                                     NULL,
                                                     RTP_TCP_ROLE_CONNECTION);

                pClientWriteSocket->setAudioChannel(getAudioChannel()) ;
                pClientWriteSocket->setVideoChannel(getVideoChannel()) ;
                                                                
                int connectRet = 0;
                if (!pClientWriteSocket->isConnected())
                {
                    connectRet = pClientWriteSocket->connect();
                }
                if (connectRet)  // no connection 
                {
                    return -1;
                }             
                CpMediaNetTask::getCpMediaNetTask()->addInputSource(pClientWriteSocket);
                addClientConnection(ipAddress, port, pClientWriteSocket);
                role = RTP_TCP_ROLE_ACTIVE;
            }
        }
        bytesWritten = pClientWriteSocket->write(buffer, bufferLength);
    }
    else
    {
        OsNatConnectionSocket* pWriteSocket = mpMediaServer->acquireConnectionSocket();
        if (pWriteSocket)
        {
            bytesWritten = pWriteSocket->write(buffer, bufferLength);
        }
    }
    
    return bytesWritten;
}                               

bool VoiceEngineConnectionSocket::getSendRecvStats(int&     rPacketsSent,
                                                   int64_t& rBytesSent,
                                                   int&     rPacketsRecv,
                                                   int64_t  rBytesRecv) 
{
    rPacketsSent = 0 ;
    rBytesSent = 0 ;
    rPacketsRecv = 0 ;
    rBytesRecv = 0 ;

    return false ; // not implemented
}


/* ============================ MANIPULATORS ============================== */

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

void VoiceEngineConnectionSocket::pushPacket(char* rtpPacket, size_t packetSize, int port) 
{
    UtlString address ;
    int rc;
#ifdef TRANSPORT_DEBUG
    int static count = 0 ;
#endif
    char* cBuf = rtpPacket;

    int bytes = packetSize;
    if (bytes > 0)
    {
        // Hand packet to GIPS VoiceEngine/VideoEngine
        bool bEnabled;

        mEnabledMutex.acquire();
        bEnabled = mbEnabled;
        if (bEnabled)
        {
            switch (miType)
            {
                case TYPE_AUDIO_RTP:
                    if (mpVoiceEngine && validRtpPacket(miType, cBuf, bytes, address, port))
                    {
                        rc = mpVoiceEngine->GIPSVE_ReceivedRTPPacket(getAudioChannel(), cBuf, bytes) ;
                    }
                    break ;
                case TYPE_AUDIO_RTCP:
                    if (mpVoiceEngine)
                    {
                        rc = mpVoiceEngine->GIPSVE_ReceivedRTCPPacket(getAudioChannel(), cBuf, bytes) ;
                    }
                    break ;
                case TYPE_VIDEO_RTP:
                    if (mpVideoEngine && validRtpPacket(miType, cBuf, bytes, address, port))
                    {
                        if (getVideoChannel() > -1)
                        {
                            rc = mpVideoEngine->GIPSVideo_ReceivedRTPPacket(getVideoChannel(), cBuf, bytes) ;
                        }
                    }
                    break ;
                case TYPE_VIDEO_RTCP:
                    if (mpVideoEngine)
                    {
                        if (getVideoChannel() > -1)
                        {
                            rc = mpVideoEngine->GIPSVideo_ReceivedRTCPPacket(getVideoChannel(), cBuf, bytes) ;
                        }
                    }
                    break ;
                default:
                    assert(false) ;
                    break ;
            }
        }
        mEnabledMutex.release();
#ifdef TRANSPORT_DEBUG        
        if (count % 100 == 0)
        {
            printf("TRANSPORT_DEBUG: pushPacket type=%d\n", miType) ;
        }
        count++ ;
#endif
    }
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


#ifdef TRANSPORT_DUMP
void VoiceEngineConnectionSocket::transportDump(const char* szLabel,     
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
