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

// APPLICATION INCLUDES
#include "mediaBaseImpl/CpMediaSocketAdapter.h"
#include "os/OsSocket.h"
#include "ARS/ArsConnectionSocket.h"
#include <assert.h>

#ifndef __MIN
#define __MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

CpMediaSocketAdapter::CpMediaSocketAdapter(SIPX_MEDIA_TYPE mediaType,
                                           IOsNatSocket* pRtpSocket, 
                                                   IOsNatSocket* pRtcpSocket,
                                                   SIPX_MEDIA_PACKET_CALLBACK pMediaCallback) :
    mpMediaCallback(pMediaCallback)
{
    mMediaType = mediaType;
    mpRtpSocket = pRtpSocket ;
    mpRtcpSocket = pRtcpSocket ;
    miRtpPort = -1 ;
    miRtcpPort = -1 ;
    miRtpErrorThrottle = 0 ;
    miRtcpErrorThrottle = 0 ;
    mpArsSocket = NULL ;
}

CpMediaSocketAdapter::~CpMediaSocketAdapter()
{
}

void CpMediaSocketAdapter::setRtpDestination(int iDestinationPort, const UtlString& destinationAddress)
{
    miRtpPort = iDestinationPort ;
    mRtpAddress = destinationAddress ;        
    miRtpErrorThrottle = 0 ;
}


void CpMediaSocketAdapter::setRtcpDestination(int iDestinationPort, const UtlString& destinationAddress)
{
    miRtcpPort = iDestinationPort ;
    mRtcpAddress = destinationAddress ;        
    miRtcpErrorThrottle = 0 ;
}


int CpMediaSocketAdapter::doSendPacket(int channel, const void *data, int len) 
{
    if (mpArsSocket)
        return doSendArs(channel, true, data, len) ;
    else
        return doSend(channel, true, data, len) ;
}

int CpMediaSocketAdapter::doSendRTCPPacket(int channel, const void *data, int len) 
{
    if (mpArsSocket)
        return doSendArs(channel, false, data, len) ;
    else
        return doSend(channel, false, data, len) ;
}


void CpMediaSocketAdapter::setArsSocket(ArsConnectionSocket* pArsSocket)
{
    assert(pArsSocket != NULL) ;
    mpArsSocket = pArsSocket ;
}
   

int CpMediaSocketAdapter::doSend(int channel, bool bRtp, const void* data, int len) 
{
    IOsNatSocket* pSocket = bRtp ? mpRtpSocket : mpRtcpSocket ;
    int*          pThottleCount = bRtp ? &miRtpErrorThrottle : &miRtcpErrorThrottle ;
    UtlString*    destAddr = bRtp ? &mRtpAddress : &mRtcpAddress ;
    int           destPort = bRtp ? miRtpPort : miRtcpPort ;

    if (pSocket && pSocket->getSocket())
    {
#ifdef TRANSPORT_DUMP
        pSocket->transportDump(bRtp ? "SendRtpPacket OUT: " : "SendRtcpPacket OUT: ",            
            (const char*) data, len) ;
#endif
        if (*pThottleCount < 0)
        {
            // Drop packets
            (*pThottleCount)++ ;
            len = 0 ;
        }
        else if (!destAddr->isNull() && (destPort != -1) && pSocket->getSocket()->isOk())
        {
            if (pSocket->socketWrite((char*) data,
                                       len,
                                       destAddr->data(),
                                       destPort,
                                       MEDIA_PACKET) > 0)
            {
                *pThottleCount = 0 ;
            }
            else
            {
                // If we hit 200ms worth of errors, stop sending for 200 ms
                (*pThottleCount)++ ;
                if (*pThottleCount >= 10)
                {
                    *pThottleCount = -(*pThottleCount) ;
                }
                len = 0 ;
            }
        }
    }
    return len;
}


int CpMediaSocketAdapter::doSendArs(int channel, bool bRtp, const void* data, int len) 
{
    assert(len <= ARS_MAX_PACKET_SIZE) ;

    if (mpArsSocket && mpArsSocket->isConnected() && mpArsSocket->isOk())
    {
        int packetLen = __MIN(len, ARS_MAX_PACKET_SIZE) ;
        unsigned short frameLen = htons(packetLen + sizeof(unsigned short)) ;
        unsigned short frameType = htons(bRtp ? 0 : 1) ; // 0=RTP, 1=RTCP

        memcpy(cArsBuf, &frameLen, sizeof(frameLen)) ;
        memcpy(&cArsBuf[2], &frameType, sizeof(frameType)) ;
        memcpy(&cArsBuf[4], data, packetLen) ;

        // Write w/o blocking.  If we were going to block -- just drop the packet
        mpArsSocket->write(cArsBuf, packetLen + sizeof(frameLen) + sizeof(frameType), 0) ;
        mpArsSocket->markWriteTime() ;

        // Do we need error throttles?  Errors will show up if either the socket is 
        // full or if the socket died.  In both cases, error handling will happen
        // out of bounds.
    }
    else
        len = 0 ;

    return len ;
}
