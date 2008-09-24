// Copyright 2007,2008 AOL LLC.
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
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
// 

#ifndef _VoiceEngineSocketAdapter_h_
#define _VoiceEngineSocketAdapter_h_

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/IOsNatSocket.h>
#include "ARS/ArsConnectionSocket.h"
#include "tapi/sipXtapi.h"
#include "mediaBaseImpl/CpMediaSocketAdapter.h"
#include "include/VoiceEngineDefs.h"


class VoiceEngineSocketAdapter : public GIPS_transport, public CpMediaSocketAdapter
{
public:
    VoiceEngineSocketAdapter(SIPX_MEDIA_TYPE mediaType,
                             IOsNatSocket* pRtpSocket, 
                             IOsNatSocket* pRtcpSocket,
                             SIPX_MEDIA_PACKET_CALLBACK pMediaCallback);
    virtual int SendPacket(int channel, const void *data, int len);
    virtual int SendRTCPPacket(int channel, const void *data, int len);
} ;

#endif
