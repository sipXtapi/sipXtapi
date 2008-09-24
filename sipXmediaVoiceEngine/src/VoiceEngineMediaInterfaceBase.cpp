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


//
// This file contains "stuff" that should be moved into a common ancestor 
// (between OS and VoiceEngine) or someplace else.

// SYSTEM INCLUDES
#include <assert.h>

// for Windows, we need the COM version defs
#ifdef _WIN32
    #define _OLE32_
    #define _WIN32_DCOM
    #define _WIN32_WINNT 0x0400 
#endif

#define UDP_TRANSPORT_INDEX   0
#define TCP_TRANSPORT_INDEX   1

#define FORCE_AUDIO_TURN      0
#define FORCE_VIDEO_TURN      0

// APPLICATION INCLUDES
#include <utl/UtlSListIterator.h>
#include <os/OsProtectEventMgr.h>
#include "os/OsPerfLog.h"
#include <sdp/SdpCodec.h>
#include <net/SdpBody.h>
#include <net/NetBase64Codec.h>
#include "net/SipContactDb.h"
#include "mediaBaseImpl/CpMediaNetTask.h"
#include "os/IOsNatSocket.h"
#include "os/OsSocket.h"
#include "os/OsMediaContact.h"
#include "mediaInterface/IMediaSocket.h"
#include "include/VoiceEngineDatagramSocket.h"
#include "include/VoiceEngineMediaInterface.h"
#include "mediaBaseImpl/CpMediaSocketMonitorTask.h"
#include "include/VoiceEngineBufferInStream.h"
#include "include/VoiceEngineDefs.h"
#include "mediaBaseImpl/CpMediaConnection.h"
#include "include/VoiceEngineConnectionSocket.h"
#include "include/VideoEngine.h"
#ifdef WIN32
#ifndef WINCE
#undef WIN32_LEAN_AND_MEAN
#include <unknwn.h>
#endif 
#endif
//////////////////////////////////////////////////////////////////////////////

