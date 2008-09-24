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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a 
// Contributor Agreement.  Contributors retain copyright to elements 
// licensed under a Contributor Agreement.  Licensed to the User under the 
// LGPL license.
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
#include "include/VoiceEngineDatagramSocket.h"
#include "mediaBaseImpl/CpMediaNetTask.h"

#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "os/OsProtectEvent.h"
#include "os/OsProtectEventMgr.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
// FORWARD DECLARATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
VoiceEngineDatagramSocket::VoiceEngineDatagramSocket(GipsVoiceEngineLib* pVoiceEngine,
                                                     GipsVideoEnginePlatform* pVideoEngine,
                                                     int type,
                                                     int remoteHostPortNum,
                                                     const char* remoteHost, 
                                                     int localHostPortNum, 
                                                     const char* localHost)
        : OsNatDatagramSocket(remoteHostPortNum, remoteHost, localHostPortNum, localHost),
          mEnabledMutex(OsMutex::Q_FIFO),
          mbEnabled(false)
{    
    mpVoiceEngine = pVoiceEngine ;
    mpVideoEngine = pVideoEngine ;
    miType = type ;
    mPreferredReceivePort = 0 ;
    memset(mRtpContexts, 0, sizeof(mRtpContexts)) ;
}


// Destructor
VoiceEngineDatagramSocket::~VoiceEngineDatagramSocket()
{
    OsTime maxEventTime(20, 0);

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    CpMediaNetTask* pNetTask = CpMediaNetTask::getCpMediaNetTask();
    // remove as input source

    OsProtectedEvent* rtpSocketRemoveEvent = eventMgr->alloc();
    pNetTask->removeInputSource(this, rtpSocketRemoveEvent) ;
    if (rtpSocketRemoveEvent->wait(0, maxEventTime) != OS_SUCCESS)
    {
        OsSysLog::add(FAC_MP, PRI_ERR, 
                " *** VoiceEngineMediaInterface: failed to wait for audio rtp socket release") ;
    }
    eventMgr->release(rtpSocketRemoveEvent);
    destroy();
    mpVoiceEngine = NULL ;
    mpVideoEngine = NULL ;
}

/* ============================ MANIPULATORS ============================== */

bool VoiceEngineDatagramSocket::pushPacket()
{
    bool bSuccess = true ;
    UtlString address ;
    int port ;
    char cBuf[MAX_RTP_BYTES] ;

    int bytes = doReadPacket(cBuf, sizeof(cBuf), address, port) ;
    if (bytes > 0)
    {
        doPushPacket(cBuf, bytes, port);
    }
    else if (bytes < 0)
    {
        bSuccess = false;
    }
    return bSuccess;
}

void VoiceEngineDatagramSocket::setEnabled(bool bEnabled)
{
    OsLock lock(mEnabledMutex);
    mbEnabled = bEnabled ;
}

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
bool VoiceEngineDatagramSocket::validRtpPacket(int         type,
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

void VoiceEngineDatagramSocket::syncToSource(int         type,
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


#ifdef TRANSPORT_DUMP
void VoiceEngineDatagramSocket::transportDump(const char* szLabel,     
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


int VoiceEngineDatagramSocket::doReadPacket(char* pData, const int maxSize, UtlString& address, int& port)
{
    int bytes = read(pData, maxSize, &address, &port) ;
    if (bytes > 0)
    {
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

void VoiceEngineDatagramSocket::doPushPacket(char* rtpPacket, size_t packetSize, int port) 
{
    UtlString address ;
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
                        mpVoiceEngine->GIPSVE_ReceivedRTPPacket(getAudioChannel(), cBuf, bytes) ;
                    }
                    break ;
                case TYPE_AUDIO_RTCP:
                    if (mpVoiceEngine)
                    {
                        mpVoiceEngine->GIPSVE_ReceivedRTCPPacket(getAudioChannel(), cBuf, bytes) ;
                    }
                    break ;
                case TYPE_VIDEO_RTP:
                    if (mpVideoEngine && validRtpPacket(miType, cBuf, bytes, address, port))
                    {
                        if (getVideoChannel() > -1)
                        {
                            int i = mpVideoEngine->GIPSVideo_ReceivedRTPPacket(getVideoChannel(), cBuf, bytes) ;
                        }
                    }
                    break ;
                case TYPE_VIDEO_RTCP:
                    if (mpVideoEngine)
                    {
                        if (getVideoChannel() > -1)
                        {
                            mpVideoEngine->GIPSVideo_ReceivedRTCPPacket(getVideoChannel(), cBuf, bytes) ;
                        }
                    }
                    break ;
                default:
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
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
