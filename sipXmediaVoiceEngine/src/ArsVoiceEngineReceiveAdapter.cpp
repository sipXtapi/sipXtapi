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

#include <assert.h>

#include "include/ArsVoiceEngineReceiveAdapter.h"
#include "os/OsLock.h"
#include "include/VoiceEnginePacketPusher.h"

//////////////////////////////////////////////////////////////////////////////
       
ArsVoiceEngineReceiveAdapter::ArsVoiceEngineReceiveAdapter(SIPX_MEDIA_TYPE mediaType,
                                                           GipsVoiceEngineLib*      pVoiceEngine,
                                                           GipsVideoEnginePlatform* pVideoEngine,
                                                           OsMutex*                 pGuard) :
    mpVoiceEngine(pVoiceEngine),
    mpVideoEngine(pVideoEngine),
    ArsReceiveAdapter(mediaType, new VoiceEnginePacketPusher(pVoiceEngine, pVideoEngine), pGuard)
{
    assert( (mpVoiceEngine == NULL && mpVideoEngine != NULL) ||
            (mpVoiceEngine != NULL && mpVideoEngine == NULL) ) ;

    assert(mpGuard != NULL) ;
}

//////////////////////////////////////////////////////////////////////////////

ArsVoiceEngineReceiveAdapter::~ArsVoiceEngineReceiveAdapter() 
{

}

//////////////////////////////////////////////////////////////////////////////

bool ArsVoiceEngineReceiveAdapter::handleData(char* pData, int len, int channel, bool& bMarkReadData) 
{
    bool bRC = false ;

    assert(channel >= 0) ;
    bMarkReadData = false ;

    if (len > ARS_FRAME_TYPE_SIZE)
    {
        unsigned short type ;
        memcpy((char*) &type, pData, sizeof(unsigned short)) ;
        type = ntohs(type) ;        

        if (mpVoiceEngine)
        {        
            OsLock lock(*mpGuard) ;
            switch (type)
            {
                case ARS_PACKET_RTP:
                    mpVoiceEngine->GIPSVE_ReceivedRTPPacket(channel, 
                            &pData[ARS_FRAME_TYPE_SIZE],
                            len-ARS_FRAME_TYPE_SIZE) ;
                    bMarkReadData = true ;
                    break ;
                case ARS_PACKET_RTCP:
                    mpVoiceEngine->GIPSVE_ReceivedRTCPPacket(channel, 
                        &pData[ARS_FRAME_TYPE_SIZE],
                        len-ARS_FRAME_TYPE_SIZE) ;
                    break ;
                default:
                    assert(false) ;
            }
        }
        else if (mpVideoEngine)
        {
            OsLock lock(*mpGuard) ;
            switch (type)
            {
                case ARS_PACKET_RTP:
                    mpVideoEngine->GIPSVideo_ReceivedRTPPacket(channel, 
                            &pData[ARS_FRAME_TYPE_SIZE],
                            len-ARS_FRAME_TYPE_SIZE) ;
                    break ;
                case ARS_PACKET_RTCP:
                    mpVideoEngine->GIPSVideo_ReceivedRTCPPacket(channel, 
                            &pData[ARS_FRAME_TYPE_SIZE],
                            len-ARS_FRAME_TYPE_SIZE) ;
                    break ;
                default:
                    assert(false) ;
            }
        }
        else
            assert(false) ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////
