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

#ifndef _VoiceEnginePacketPusher_h_
#define _VoiceEnginePacketPusher_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mediaInterface/IMediaPacketPusher.h"
#include "include/VoiceEngine.h"
#include "VoiceEngineDefs.h"

// FORWARD DECLARATIONS

class VoiceEnginePacketPusher : public IMediaPacketPusher
{
    /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    /* ============================ CREATORS ================================== */
    VoiceEnginePacketPusher(VoiceEngine* pVoiceEngine,
                            GipsVideoEnginePlatform* pVideoEngine);
    virtual void pushRtpPacket(const int channel, SIPX_MEDIA_TYPE mediaType, const char* buffer, size_t len);
    virtual void pushRtcpPacket(const int channel, SIPX_MEDIA_TYPE mediaType, const char* buffer, size_t len);
    /* ============================ MANIPULATORS ============================== */

    /* ============================ ACCESSORS ================================= */
    /* ============================ INQUIRY =================================== */

    //////////////////////////////////////////////////////////////////////////////

    /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    VoiceEngine* mpVoiceEngine;
    GipsVideoEnginePlatform* mpVideoEngine ;
};
#endif 