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
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
// 

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "include/VoiceEnginePacketPusher.h"
#include "include/VoiceEngine.h"


VoiceEnginePacketPusher::VoiceEnginePacketPusher(VoiceEngine* pVoiceEngine,
                                                 GipsVideoEnginePlatform* pVideoEngine) :
    mpVoiceEngine(pVoiceEngine),
    mpVideoEngine(pVideoEngine)
{
}

void VoiceEnginePacketPusher::pushRtpPacket(const int channel, SIPX_MEDIA_TYPE mediaType, const char* buffer, size_t len)
{
    if (MEDIA_TYPE_AUDIO == mediaType)
    {
        if (mpVoiceEngine)
        {
			mpVoiceEngine->getNetwork()->GIPSVE_ReceivedRTPPacket(channel, buffer, len);
        }
    }
    else if (MEDIA_TYPE_VIDEO == mediaType)
    {
        if (mpVideoEngine)
        {
            mpVideoEngine->GIPSVideo_ReceivedRTPPacket(channel, buffer, len);
        }
    }
}

void VoiceEnginePacketPusher::pushRtcpPacket(const int channel, SIPX_MEDIA_TYPE mediaType, const char* buffer, size_t len)
{
    if (MEDIA_TYPE_AUDIO == mediaType)
    {
        if (mpVoiceEngine)
        {
            mpVoiceEngine->getNetwork()->GIPSVE_ReceivedRTCPPacket(channel, buffer, len);
        }
    }
    else if (MEDIA_TYPE_VIDEO == mediaType)
    {
        if (mpVideoEngine)
        {
            mpVideoEngine->GIPSVideo_ReceivedRTCPPacket(channel, buffer, len);
        }
    }
}



