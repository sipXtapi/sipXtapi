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
#include "mediaBaseImpl/CpMediaConnection.h"
#include "mediaInterface/IMediaSocket.h"
#include "os/OsNatConnectionSocket.h"
#include "ARS/ArsConnectionSocket.h"

//////////////////////////////////////////////////////////////////////////////

CpMediaConnection::CpMediaConnection(int connectionId) 
    : UtlInt(connectionId),
          mbHeld(false),
          mRole(RTP_TCP_ROLE_ACTPASS),
          mpAudioCodec(NULL),
          mIsMulticast(false),
          mbEnableARS(false)
{
    mpRtpAudioSocketArray[0] = NULL;
    mpRtpAudioSocketArray[1] = NULL;
    mpRtpAudioReadNotifyAdapter[0] = NULL ;
    mpRtpAudioReadNotifyAdapter[1] = NULL ;

    mpRtcpAudioSocketArray[0] = NULL;
    mpRtcpAudioSocketArray[1] = NULL;
    mRtpAudioSendHostPort = 0;
    mRtcpAudioSendHostPort = 0;
    mRtpAudioReceivePort = 0;
    mRtcpAudioReceivePort = 0;
    mpAudioSocketAdapterArray[0] = NULL;
    mpAudioSocketAdapterArray[1] = NULL;

    mVideoConnectionId = -1;
    mpRtpVideoSocketArray[0] = NULL;
    mpRtpVideoSocketArray[1] = NULL;
    mpRtpVideoReadNotifyAdapter[0] = NULL ;
    mpRtpVideoReadNotifyAdapter[1] = NULL ;
    mpRtcpVideoSocketArray[0] = NULL;
    mpRtcpVideoSocketArray[1] = NULL;
    mRtpVideoSendHostPort = 0;
    mRtcpVideoSendHostPort = 0;
    mRtpVideoReceivePort = 0;
    mRtcpVideoReceivePort = 0;
    mpVideoSocketAdapterArray[0] = NULL;
    mpVideoSocketAdapterArray[1] = NULL;
    memset((void*)&mPrimaryVideoCodec, 0, sizeof(SdpCodec));
    mbIsPrimaryVideoCodecSet = false;
    mRtpVideoPayloadType = 0;
    mRtpPayloadType = 0;
    mDestinationSet = FALSE;       
    mRtpSendingAudio = FALSE;
    mRtpSendingVideo = FALSE;
    mRtpReceivingAudio = FALSE;
    mRtpReceivingVideo = FALSE;
    mpCodecFactory = NULL;
    mPrimaryCodecType = SdpCodec::SDP_CODEC_UNKNOWN;
    mContactType = CONTACT_AUTO ;
    mContactId = 0 ;
    mConnectionBitrate = 0;
    mbVideoStarted = false;
    mbAudioInitialized = false;
    mbVideoInitialized = false;

    mbAlternateDestinations = FALSE ;
    mpInStreamLocal = NULL ;
    mpInStreamRemote = NULL ;
    mpMediaEventListener = NULL ;

    mRebuiltAudioBandwidth = 0;        

    mbEnableICE = true ;
    mbEnableTURN = true ;

    mpArsAudioSocket = NULL ;
    mpArsAudioRAdapter = NULL ;
    miArsAudioPriority = 0 ;
    mpArsAudioReadNotifyAdapter = NULL ;
    mpArsVideoSocket = NULL ;
    mpArsVideoRAdapter = NULL ;
    miArsVideoPriority = 0 ;
    mbInitiating = false ;    
    mpArsVideoReadNotifyAdapter = NULL ;
}

//////////////////////////////////////////////////////////////////////////////

CpMediaConnection::~CpMediaConnection()
{
    stopReadNotifyAdapters() ;

    if(mpCodecFactory)
    {
        delete mpCodecFactory;
        mpCodecFactory = NULL;
    }

    if (mpInStreamLocal)
    {
        delete mpInStreamLocal ;
        mpInStreamLocal = NULL ;
    }

    if (mpInStreamRemote)
    {
        delete mpInStreamRemote ;
        mpInStreamRemote = NULL ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void CpMediaConnection::setHold(const bool bHold)
{
    mbHeld = bHold;
}

//////////////////////////////////////////////////////////////////////////////

const bool CpMediaConnection::getHold() const
{
    return mbHeld;
}

//////////////////////////////////////////////////////////////////////////////

void CpMediaConnection::setTcpRole(const RtpTcpRoles role)
{
    mRole = role;
    for (int i = 0; i < 2; i++) // UPD and TCP
    {
        OsNatConnectionSocket* pAudioSocket = dynamic_cast<OsNatConnectionSocket*>(mpRtpAudioSocketArray[i]);
        OsNatConnectionSocket* pAudioRtcpSocket = dynamic_cast<OsNatConnectionSocket*>(mpRtcpAudioSocketArray[i]);
        OsNatConnectionSocket* pVideoSocket = dynamic_cast<OsNatConnectionSocket*>(mpRtpVideoSocketArray[i]);
        OsNatConnectionSocket* pVideoRtcpSocket = dynamic_cast<OsNatConnectionSocket*>(mpRtcpVideoSocketArray[i]);
        if (pAudioSocket)
        {
            pAudioSocket->setRole(role);
        }
        if (pAudioRtcpSocket)
        {
            pAudioRtcpSocket->setRole(role);
        }
        if (pVideoSocket)
        {
            pVideoSocket->setRole(role);
        }
        if (pVideoRtcpSocket)
        {
            pVideoRtcpSocket->setRole(role);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

const RtpTcpRoles CpMediaConnection::getTcpRole() const
{
    return mRole;
}

//////////////////////////////////////////////////////////////////////////////

IOsNatSocket* CpMediaConnection::getRtpAudioSocket()
{
    IOsNatSocket* pSocket = NULL ;

    for (int i = 0; i < 2; i++) // UPD and TCP
    {
        if (mpRtpAudioSocketArray[i])
        {
            pSocket = mpRtpAudioSocketArray[i] ;
            break ;
        }
    }

    return pSocket ;
} 

//////////////////////////////////////////////////////////////////////////////

IOsNatSocket* CpMediaConnection::getRtcpAudioSocket()
{
    IOsNatSocket* pSocket = NULL ;

    for (int i = 0; i < 2; i++) // UPD and TCP
    {
        if (mpRtcpAudioSocketArray[i])
        {
            pSocket = mpRtcpAudioSocketArray[i] ;
            break ;
        }
    }

    return pSocket ;
} 

//////////////////////////////////////////////////////////////////////////////

IOsNatSocket* CpMediaConnection::getRtpVideoSocket()
{
    IOsNatSocket* pSocket = NULL ;

    for (int i = 0; i < 2; i++) // UPD and TCP
    {
        if (mpRtpVideoSocketArray[i])
        {
            pSocket = mpRtpVideoSocketArray[i] ;
            break ;
        }
    }

    return pSocket ;
} 

//////////////////////////////////////////////////////////////////////////////

IOsNatSocket* CpMediaConnection::getRtcpVideoSocket()
{
    IOsNatSocket* pSocket = NULL ;

    for (int i = 0; i < 2; i++) // UPD and TCP
    {
        if (mpRtcpVideoSocketArray[i])
        {
            pSocket = mpRtcpVideoSocketArray[i] ;
            break ;
        }
    }

    return pSocket ;
} 

//////////////////////////////////////////////////////////////////////////////

void CpMediaConnection::startReadNotifyAdapters(ISocketEvent* pSocketEvent) 
{
    if (pSocketEvent)
    {
        for (int i = 0; i < 2; i++) // UPD and TCP
        {
            if (mpRtpAudioReadNotifyAdapter[i] == NULL &&
                mpRtpAudioSocketArray[i] != NULL)                      
            {
                mpRtpAudioReadNotifyAdapter[i] = new SocketReadNotifyAdapter(
                        mpRtpAudioSocketArray[i]->getSocket(),
                        RTP_AUDIO,
                        pSocketEvent) ;
            }
            if (mpRtpVideoReadNotifyAdapter[i] == NULL && 
                mpRtpVideoSocketArray[i] != NULL)
            {
                mpRtpVideoReadNotifyAdapter[i] = new SocketReadNotifyAdapter(
                        mpRtpVideoSocketArray[i]->getSocket(),
                        RTP_VIDEO,
                        pSocketEvent) ;
            }
        }

        if (mpArsAudioReadNotifyAdapter == NULL && mpArsAudioSocket)
        {
            mpArsAudioReadNotifyAdapter = new SocketReadNotifyAdapter(
                    mpArsAudioSocket,
                    RTP_AUDIO,
                    pSocketEvent) ;
        }
        if (mpArsVideoReadNotifyAdapter == NULL && mpArsVideoSocket)
        {
            mpArsVideoReadNotifyAdapter = new SocketReadNotifyAdapter(
                    mpArsVideoSocket,
                    RTP_AUDIO,
                    pSocketEvent) ;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void CpMediaConnection::stopReadNotifyAdapters() 
{
    for (int i = 0; i < 2; i++) // UPD and TCP
    {
        if (mpRtpAudioReadNotifyAdapter[i])
        {
            delete mpRtpAudioReadNotifyAdapter[i] ;
            mpRtpAudioReadNotifyAdapter[i] = NULL ;
        }
        if (mpRtpVideoReadNotifyAdapter[i])
        {
            delete mpRtpVideoReadNotifyAdapter[i] ;
            mpRtpVideoReadNotifyAdapter[i] = NULL ;
        }
    }

        if (mpArsAudioReadNotifyAdapter)
        {
            delete mpArsAudioReadNotifyAdapter ;
            mpArsAudioReadNotifyAdapter = NULL ;
        }
        if (mpArsVideoReadNotifyAdapter)
        {
            delete mpArsVideoReadNotifyAdapter ;
            mpArsVideoReadNotifyAdapter = NULL ;
        }
}

//////////////////////////////////////////////////////////////////////////////

void CpMediaConnection::setSocketEnabled(OsSocket* pSocket, bool bEnabled)
{
    IMediaSocket* pMediaSocket = dynamic_cast<IMediaSocket*>(pSocket) ;
    assert(pMediaSocket != NULL) ;
    if (pMediaSocket)
    {
        pMediaSocket->setEnabled(bEnabled);
    }
}

//////////////////////////////////////////////////////////////////////////////

void CpMediaConnection::setSocketsEnabled(bool bEnable) 
{
    for (int i = 0; i < 2; i++) // UDP and TCP
    {
        if (mpRtpAudioSocketArray[i])
            setSocketEnabled(mpRtpAudioSocketArray[i]->getSocket(), bEnable);

        if (mpRtcpAudioSocketArray[i])
            setSocketEnabled(mpRtcpAudioSocketArray[i]->getSocket(), bEnable);

        if (mpArsAudioSocket)
            setSocketEnabled(mpArsAudioSocket->getSocket(), bEnable);        

        if (mpRtpVideoSocketArray[i])
            setSocketEnabled(mpRtpVideoSocketArray[i]->getSocket(), bEnable);

        if (mpRtcpVideoSocketArray[i])
            setSocketEnabled(mpRtcpVideoSocketArray[i]->getSocket(), bEnable);

        if (mpArsVideoSocket)
            setSocketEnabled(mpArsVideoSocket->getSocket(), bEnable);
    }            
}

//////////////////////////////////////////////////////////////////////////////
