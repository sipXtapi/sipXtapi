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
// Licensed to SIPfoundry under a Contributor Agreement.// 
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _CpMediaConnection_h_
#define _CpMediaConnection_h_

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/IOsNatSocket.h>
#include <utl/UtlInt.h>
#include "mediaInterface/IMediaTransportAdapter.h"
#include "sdp/SdpCodec.h"
#include "tapi/sipXtapi.h"
#include "mediaBaseImpl/CpMediaInStream.h"
#include "ARS/ArsConnectionSocket.h"

/**
 * Simple adapter class that can be passed to the socket as an 
 * OsNotification and then fires an onReadData events to the upper 
 * layers.
 */
class SocketReadNotifyAdapter : OsNotification
{
public:
    SocketReadNotifyAdapter(OsSocket*     pSocket,
                            SocketPurpose purpose,
                            ISocketEvent* pSocketNotifySink)
    {
        mpSocket = pSocket ;
        mPurpose = purpose ;
        mpNotifySink = pSocketNotifySink ;

        mpSocket->setReadNotification(this) ;
    }
    
    virtual OsStatus signal(const intptr_t eventData) 
    {
        if (mpNotifySink)
        {
            mpNotifySink->onReadData(mpSocket, mPurpose) ;
        }

        return OS_SUCCESS ;
    }

    virtual ~SocketReadNotifyAdapter()
    {
        mpNotifySink = NULL ;
        mpSocket->setReadNotification(NULL) ;
    }

protected:
    OsSocket*      mpSocket ;
    SocketPurpose  mPurpose ; 
    ISocketEvent*  mpNotifySink ;
} ;


class CpMediaConnection : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    CpMediaConnection(int connectionId = -1); 

    virtual ~CpMediaConnection();
    void setHold(const bool bHold);
    const bool getHold() const;
    void setTcpRole(const RtpTcpRoles role);
    const RtpTcpRoles getTcpRole() const;   
    IOsNatSocket* getRtpAudioSocket() ;
    IOsNatSocket* getRtcpAudioSocket() ;
    IOsNatSocket* getRtpVideoSocket() ;
    IOsNatSocket* getRtcpVideoSocket() ;
    void startReadNotifyAdapters(ISocketEvent* pEvent) ;
    void stopReadNotifyAdapters() ;

    void setSocketEnabled(OsSocket* pSocket, bool bEnabled) ;
    void setSocketsEnabled(bool bEnable) ;

    ArsConnectionSocket*          mpArsAudioSocket ;
    IArsPacketHandler*            mpArsAudioRAdapter ;
    int                           miArsAudioPriority ;
    SocketReadNotifyAdapter*      mpArsAudioReadNotifyAdapter ;

    ArsConnectionSocket*          mpArsVideoSocket ;
    IArsPacketHandler*            mpArsVideoRAdapter ;
    int                           miArsVideoPriority ;
    SocketReadNotifyAdapter*      mpArsVideoReadNotifyAdapter ;
    bool mbInitiating ;
    
    IOsNatSocket* mpRtpAudioSocketArray[2]; // UDP and TCP
    SocketReadNotifyAdapter* mpRtpAudioReadNotifyAdapter[2] ;  // UDP and TCP
    IOsNatSocket* mpRtcpAudioSocketArray[2]; // UDP and TCP
    IMediaTransportAdapter* mpAudioSocketAdapterArray[2] ;  // UDP and TCP
    int mRtpAudioSendHostPort;
    int mRtcpAudioSendHostPort;
    int mRtpAudioReceivePort;
    int mRtcpAudioReceivePort;
    int mRtpPayloadType;

    int mConnectionBitrate ;
    int mConnectionFramerate ;
    int mConnectionCPU ;
    int mConnectionQuality ;

    IOsNatSocket* mpRtpVideoSocketArray[2];   // UDP and TCP
    SocketReadNotifyAdapter* mpRtpVideoReadNotifyAdapter[2] ;  // UDP and TCP
    IOsNatSocket* mpRtcpVideoSocketArray[2];  // UDP and TCP
    IMediaTransportAdapter* mpVideoSocketAdapterArray[2] ;  // UDP and TCP
    int mVideoConnectionId ;
    int mRtpVideoSendHostPort ;
    int mRtcpVideoSendHostPort ;
    int mRtpVideoReceivePort ;
    int mRtcpVideoReceivePort ;
    SdpCodec mPrimaryVideoCodec;
    int mRtpVideoPayloadType;

    bool mDestinationSet;
    bool mRtpSendingAudio;
    bool mRtpReceivingAudio;
    bool mRtpSendingVideo;
    bool mRtpReceivingVideo;

    bool mbVideoStarted;

    SdpCodecList* mpCodecFactory;
    SdpCodec::SdpCodecTypes mPrimaryCodecType ;
    SIPX_CONTACT_TYPE mContactType ;  
    SIPX_CONTACT_ID   mContactId ;
    bool mbAudioInitialized;
    bool mbVideoInitialized;
    bool mbIsPrimaryVideoCodecSet;
    int mRebuiltAudioBandwidth;
    int mRtpTransport;

    UtlString mRtpSendHostAddress;
    bool mbAlternateDestinations ;
    UtlString mLocalAddress ;
    CpMediaInStream* mpInStreamLocal ;
    CpMediaInStream* mpInStreamRemote ;
    IMediaEventListener*  mpMediaEventListener;
    SdpCodec* mpAudioCodec;

    bool mbEnableLocal ;
    bool mbEnableSTUN ;
    bool mbEnableTURN ;
    bool mbEnableARS ;
    bool mbEnableICE ;

    bool mIsMulticast;
    
    UtlString mUserAgent ;
    UtlHashMap mConnectionProperties;

    
/* //////////////////////////// PROTECTED ////////////////////////////////// */

/* //////////////////////////// PRIVATE //////////////////////////////////// */
private:
    RtpTcpRoles mRole;
    bool mbHeld;    
};
#endif

