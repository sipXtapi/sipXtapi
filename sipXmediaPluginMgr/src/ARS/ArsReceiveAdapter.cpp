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

#include "ARS/ArsReceiveAdapter.h"
#include "os/OsLock.h"

//////////////////////////////////////////////////////////////////////////////
       
ArsReceiveAdapter::ArsReceiveAdapter(SIPX_MEDIA_TYPE mediaType,
                                     IMediaPacketPusher* pPacketPusher,
                                     OsMutex* pGuard)
{
    mMediaType = mediaType;
    mpPacketPusher = pPacketPusher;
    mpGuard = pGuard ;
    
}

//////////////////////////////////////////////////////////////////////////////

ArsReceiveAdapter::~ArsReceiveAdapter() 
{
    delete mpPacketPusher;
}

//////////////////////////////////////////////////////////////////////////////

bool ArsReceiveAdapter::handleData(char* pData, int len, int channel, bool& bMarkReadData) 
{
    bool bRC = false ;

    assert(channel >= 0) ;
    bMarkReadData = false ;

    if (len > ARS_FRAME_TYPE_SIZE)
    {
        unsigned short type ;
        memcpy((char*) &type, pData, sizeof(unsigned short)) ;
        type = ntohs(type) ;        

        OsLock lock(*mpGuard) ;
        switch (type)
        {
            case ARS_PACKET_RTP:
                mpPacketPusher->pushRtpPacket(channel, 
                                                mMediaType,
                                                &pData[ARS_FRAME_TYPE_SIZE],
                                                len-ARS_FRAME_TYPE_SIZE) ;
                bMarkReadData = true ;
                break ;
            case ARS_PACKET_RTCP:
                mpPacketPusher->pushRtpPacket(channel, 
                                                mMediaType,
                                                &pData[ARS_FRAME_TYPE_SIZE],
                                                len-ARS_FRAME_TYPE_SIZE) ;
                break ;
            default:
                assert(false) ;
        }
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////
