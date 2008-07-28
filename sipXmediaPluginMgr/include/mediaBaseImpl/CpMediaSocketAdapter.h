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

#ifndef _CpMediaSocketAdapter_h_
#define _CpMediaSocketAdapter_h_

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/IOsNatSocket.h>
#include "ARS/ArsConnectionSocket.h"
#include "tapi/sipXtapi.h"
#include "mediaInterface/IMediaTransportAdapter.h"

class CpMediaSocketAdapter : public IMediaTransportAdapter 
{
public:
    CpMediaSocketAdapter(SIPX_MEDIA_TYPE mediaType,
                             IOsNatSocket* pRtpSocket, 
                             IOsNatSocket* pRtcpSocket,
                             SIPX_MEDIA_PACKET_CALLBACK pMediaCallback);

    ~CpMediaSocketAdapter();

    virtual void setRtpDestination(int iDestinationPort, const UtlString& destinationAddress);

    virtual void setRtcpDestination(int iDestinationPort, const UtlString& destinationAddress);

    virtual int doSendPacket(int channel, const void *data, int len);
    virtual int doSendRTCPPacket(int channel, const void *data, int len);

    // Sets Ars socket and assumes ars mode
    virtual void setArsSocket(ArsConnectionSocket* pArsSocket) ;

    void setRtpSocket(IOsNatSocket* pSocket) { mpRtpSocket = pSocket; }
    void setRtcpSocket(IOsNatSocket* pSocket) { mpRtcpSocket = pSocket; }
    IOsNatSocket* getRtpSocket() { return mpRtpSocket; }
    IOsNatSocket* getRtcpSocket() { return mpRtcpSocket; }
protected:
    int doSend(int channel, bool bRtp, const void* data, int len) ;
    int doSendArs(int channel, bool bRtp, const void* data, int len) ;
    
private:
    SIPX_MEDIA_PACKET_CALLBACK  mpMediaCallback;
    SIPX_MEDIA_TYPE        mMediaType;
    IOsNatSocket*        mpRtpSocket ;
    IOsNatSocket*        mpRtcpSocket ;
    int                  miRtpErrorThrottle ;
    int                  miRtcpErrorThrottle ;
    UtlString            mRtpAddress ;
    int                  miRtpPort ;
    UtlString            mRtcpAddress ;
    int                  miRtcpPort ;
    ArsConnectionSocket* mpArsSocket ;
    char                 cArsBuf[ARS_MAX_PACKET_SIZE+ARS_FRAMING_SIZE] ;
} ;

#endif

