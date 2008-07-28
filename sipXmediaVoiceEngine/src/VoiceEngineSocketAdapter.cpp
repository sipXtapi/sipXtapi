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

// APPLICATION INCLUDES
#include "include/VoiceEngineSocketAdapter.h"
#include <assert.h>


VoiceEngineSocketAdapter::VoiceEngineSocketAdapter(SIPX_MEDIA_TYPE mediaType,
                                                   IOsNatSocket* pRtpSocket, 
                                                   IOsNatSocket* pRtcpSocket,
                                                   SIPX_MEDIA_PACKET_CALLBACK pMediaCallback) :
    CpMediaSocketAdapter(mediaType,
                             pRtpSocket, 
                             pRtcpSocket,
                             pMediaCallback)
{
}

int VoiceEngineSocketAdapter::SendPacket(int channel, const void *data, int len) 
{
    return doSendPacket(channel, data, len);
}


int VoiceEngineSocketAdapter::SendRTCPPacket(int channel, const void *data, int len) 
{
    return doSendRTCPPacket(channel, data, len);
}

