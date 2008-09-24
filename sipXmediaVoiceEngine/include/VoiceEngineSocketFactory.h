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

#ifndef _VoiceEngineSocketFactory_h_
#define _VoiceEngineSocketFactory_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "include/VoiceEngineDatagramSocket.h"
#include "include/VoiceEngineConnectionSocket.h"
#include "utl/UtlHashMap.h"
#include "os/OsSocket.h"
#include "os/OsContact.h"
#include "os/OsMutex.h"
#include "ARS/ArsConnectionSocket.h"
#include "net/ProxyDescriptor.h"
#include "mediaInterface/IMediaDeviceMgr.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// ENUMS


// FORWARD DECLARATIONS
class CpMediaInterfaceFactoryImpl ;
class CpMediaNetTask ;

/**
 * This class/singleton object will generate VoiceEngine sockets, or return existing ones, 
 * based on the OsContact record supplied to it.  
 */
class VoiceEngineSocketFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Default constructor
     */
    VoiceEngineSocketFactory(IMediaDeviceMgr* pFactoryImpl);


    /**
     * Destructor
     */
    virtual ~VoiceEngineSocketFactory();

/* ============================ MANIPULATORS ============================== */
                                         
    IOsNatSocket* getUdpSocket(GipsVoiceEngineLib* pVoice,
                               GipsVideoEnginePlatform* pVideo,
                               int type,
                               const char* localHost,
                               const ProxyDescriptor& stunServer,
                               const ProxyDescriptor& turnProxy);

    void releaseUdpSocket(IOsNatSocket* pSocket) ;

    IOsNatSocket* getTcpSocket(GipsVoiceEngineLib* pVoice,
                               GipsVideoEnginePlatform* pVideo,
                               int type,
                               int remoteHostPort,
                               const char* remoteHost,
                               const char* localHost,
                               const RtpTcpRoles tcpRole=0);
    void releaseTcpSocket(IOsNatSocket* pSocket) ;

    ArsConnectionSocket* getArsSocket(const ProxyDescriptor& arsProxy,
                                      const ProxyDescriptor& arsHttpsProxy,
                                      ARS_MIMETYPE mimeType,
                                      const char* szLocalAddress) ;

    void releaseArsSocket(ArsConnectionSocket* pSocket) ;

                                             
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    IMediaDeviceMgr* mpFactoryImpl ;
    CpMediaNetTask* mpNetTask ;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    
    /** Disabled copy constructor */
    VoiceEngineSocketFactory(const VoiceEngineSocketFactory& rVoiceEngineSocketFactory);

    /** Disabled equals operator*/
    VoiceEngineSocketFactory& operator=(const VoiceEngineSocketFactory& rhs);    
};

/* ============================ INLINE METHODS ============================ */

#endif  // _VoiceEngineSocketFactory_h_
