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

#ifndef _VoiceEngineDatagramSocket_h_
#define _VoiceEngineDatagramSocket_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsNatDatagramSocket.h"
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"
#include "include/VoiceEngineDefs.h"
#include "os/IOsNatSocket.h"
#include "mediaInterface/IMediaSocket.h"

// FORWARD DECLARATIONS
class VoiceEngine;

//: VoiceEngineDatagramSocket creates a OsDatagramSocket and automatically initiates
//: stun lookups and keep-alives.  
class VoiceEngineDatagramSocket : public IMediaSocket, public OsNatDatagramSocket
{
private:
/* ============================ CREATORS ================================== */

    VoiceEngineDatagramSocket(VoiceEngine* pVoiceEngine,
                              GipsVideoEnginePlatform* pVideoEngine,
                              int type,
                              int remoteHostPort, 
                              const char* remoteHostName, 
                              int localHostPort = 0, 
                              const char* localHostName = NULL) ;

      //: Constructor

    virtual ~VoiceEngineDatagramSocket();
       //:Destructor

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    friend class VoiceEngineSocketFactory;
   
/* ============================ MANIPULATORS ============================== */

    virtual bool pushPacket();

    void setEnabled(bool bEnabled);
       
/* ============================ ACCESSORS ================================= */

    virtual OsSocket* getSocket()
        { return this ;} ;
           
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:    
    bool validRtpPacket(int type,
                        const char* pBuf, 
                        int         nBuf,
                        const char* szFromAddress, 
                        int         iFromPort) ;

    void syncToSource(int         type,
                      RTP_HEADER* pHeader,
                      const char* szFromAddress, 
                      int         iFromPort) ;

#ifdef TRANSPORT_DUMP
    void transportDump(const char* szLabel,     
                       const char* pBuf,
                       int         nBuf) ;
#endif

    int doReadPacket(char* pData, const int maxSize, UtlString& address, int& port);
    void doPushPacket(char* rtpPacket, size_t packetSize, int port) ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */

    VoiceEngine* mpVoiceEngine ;
    GipsVideoEnginePlatform *mpVideoEngine ;
    int miType ;
    OsMutex mEnabledMutex;
    RTP_MONITOR_CONEXT  mRtpContexts[4] ;
    bool mbEnabled ;
};

/* ============================ INLINE METHODS ============================ */


#endif  // _VoiceEngineDatagramSocket_h_

