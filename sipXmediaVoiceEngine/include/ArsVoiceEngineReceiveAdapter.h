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

#ifndef _ARSVOICEENGINERECEIVEADAPTER_H
#define _ARSVOICEENGINERECEIVEADAPTER_H

#include "ARS/ArsConnectionSocket.h"
#include "mediaInterface/IMediaPacketPusher.h"
#include "include/VoiceEngineDefs.h"
#include "ARS/ArsReceiveAdapter.h"
#include "os/OsMutex.h"


class VoiceEngine;

//////////////////////////////////////////////////////////////////////////////

class ArsVoiceEngineReceiveAdapter 
    : public ArsReceiveAdapter
{
protected:
    VoiceEngine*      mpVoiceEngine ;
    GipsVideoEnginePlatform* mpVideoEngine ;
       
public:
    ArsVoiceEngineReceiveAdapter(SIPX_MEDIA_TYPE mediaType,
                                 VoiceEngine*      pVoiceEngine,
                                 GipsVideoEnginePlatform* pVideoEngine,
                                 OsMutex*                 pGuard) ;
    virtual ~ArsVoiceEngineReceiveAdapter() ;

    // IArsPacketHandler impl
    virtual bool handleData(char* pData, int len, int channel, bool& bMarkReadData) ;


protected:
} ;

//////////////////////////////////////////////////////////////////////////////

#endif


