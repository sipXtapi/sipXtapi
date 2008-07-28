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
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _VoiceEngineConnectionSocket_h_
#define _VoiceEngineConnectionSocket_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsNatConnectionSocket.h"
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"
#include "os/IOsNatSocket.h"
#include "os/OsServerSocket.h"
#include "mediaInterface/IMediaSocket.h"
#include "mediaBaseImpl/CpMediaConnectionSocket.h"
#include "mediaBaseImpl/CpMediaServer.h"
#include "include/VoiceEngineDatagramSocket.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

//: VoiceEngineConnectionSocket creates a OsConnectionSocket and automatically initiates
//: stun lookups and keep-alives.  
class VoiceEngineConnectionSocket : public CpMediaConnectionSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */

public:
/* ============================ CREATORS ================================== */
    VoiceEngineConnectionSocket(GipsVoiceEngineLib* pVoiceEngine,
                              GipsVideoEnginePlatform* pVideoEngine,
                              int type,
                              int remoteHostPort, 
                              const char* remoteHostName, 
                              int localHostPort = 0, 
                              const char* localHostName = NULL,
                              const char* szTurnServer = NULL,
                              const RtpTcpRoles role = RTP_TCP_ROLE_ACTPASS) ;

      //: Constructor accepting remote host port, name and optional local
      //: host name and port, and stun attributes.

    virtual ~VoiceEngineConnectionSocket();
       //:Destructor
         
/* ============================ MANIPULATORS ============================== */
    
/* ============================ ACCESSORS ================================= */

    virtual void pushPacket(char* rtpPacket, size_t packetSize, int port) ;

       
   int socketWrite(const char* buffer,
                   int bufferLength,
                   const char* ipAddress,
                   int port,
                   OS_NAT_PACKET_TYPE packetType=UNKNOWN_PACKET);
   
    
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

#ifdef TRANSPORT_DUMP
    void transportDump(const char* szLabel,     
                       const char* pBuf,
                       int         nBuf) ;
#endif


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    GipsVoiceEngineLib* mpVoiceEngine ;
    GipsVideoEnginePlatform *mpVideoEngine ;
};

/* ============================ INLINE METHODS ============================ */


#endif  // _VoiceEngineConnectionSocket_h_

