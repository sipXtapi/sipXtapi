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
#include "os/OsLock.h"
#include "os/OsProtectEventMgr.h"
#include "os/OsProtectEvent.h"

#include "include/VoiceEngineSocketFactory.h"
#include "mediaBaseImpl/CpMediaNetTask.h"
#include "mediaInterface/IMediaDeviceMgr.h"
#include "utl/UtlInt.h"
#include "utl/UtlHashMapIterator.h"
#include "upnp/UPnpAgent.h"

#define VSF_TRACE   0

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
VoiceEngineSocketFactory::VoiceEngineSocketFactory(IMediaDeviceMgr* pFactoryImpl)
{
    mpFactoryImpl = pFactoryImpl;
    assert(mpFactoryImpl != NULL) ;
    mpNetTask = CpMediaNetTask::getCpMediaNetTask() ;
    assert(mpNetTask != NULL) ;
}

VoiceEngineSocketFactory::~VoiceEngineSocketFactory()
{

}

/* ============================ MANIPULATORS ============================== */

IOsNatSocket*
VoiceEngineSocketFactory::getUdpSocket(GipsVoiceEngineLib* pVoice,
                                       GipsVideoEnginePlatform* pVideo,
                                       int type,
                                       const char* localHost,
                                       const ProxyDescriptor& stunServer,
                                       const ProxyDescriptor& turnProxy)
{
    assert(type >=0 && type <= 4) ; // Defined in IMediaSocket

    // Allocate port number
    int port = 0;
    mpFactoryImpl->getNextRtpPort(port) ;
           
    // Create Socket
    IOsNatSocket* pSocket = new VoiceEngineDatagramSocket(
        pVoice, pVideo,
        type,
        0, NULL,                // remote host
        port, localHost) ;    
    assert (pSocket != NULL) ;


    // Add to NetInTask
    if (pSocket)
    {
        UPnpAgent::getInstance()->bindToAvailablePort(localHost, port, UPnpAgent::getInstance()->getTimeoutSeconds());
        pSocket->setTransparentStunRead(false);
        if (mpNetTask)
            mpNetTask->addInputSource(getMediaSocketPtr(pSocket)) ;

        if (turnProxy.isValid()) 
        {
            // TODO:: Should only enable TURN and should pull the mapped 
            //        IP out of this.  For this to work, we will need to 
            //        handle failure cases for TURN allocation requests 
            //        (switch to STUN post exit of MAPPED_ADDRESS isn't
            //        present -- not sure if that should ever happen) and 
            //        more importantly graceful TURN->STUN transitions
            //        when turn is not needed -- don't want to keep the
            //        server resources allocated

            pSocket->enableStun(turnProxy.getAddress(), turnProxy.getPort(),
                    turnProxy.getKeepalive(), 0, false) ;

            pSocket->enableTurn(turnProxy.getAddress(), turnProxy.getPort(),
                    turnProxy.getKeepalive(),
                    turnProxy.getUsername(), turnProxy.getPassword(), 
                    false) ;
        }
        else
        {
            if (stunServer.isValid())
            {
                pSocket->enableStun(stunServer.getAddress(), stunServer.getPort(),
                        stunServer.getKeepalive(), 0, false) ;
            }
        }
    }


#if VSF_TRACE
    printf("VoiceEngineSocketFactory::getUdpSocket 0x%08X, port=%d\n", pSocket, port) ;
#endif
    
    return pSocket;
}


void VoiceEngineSocketFactory::releaseUdpSocket(IOsNatSocket* pSocket) 
{
#if VSF_TRACE
printf("VoiceEngineSocketFactory::releaseUdpSocket 0x%08X\n", pSocket) ;
#endif

    if (pSocket != NULL)
    {
        // Remove from NetInTask
        OsTime maxEventTime(20, 0);
        OsProtectEventMgr* pEventMgr = OsProtectEventMgr::getEventMgr();
        OsProtectedEvent* pSocketRemoveEvent = pEventMgr->alloc();
        mpNetTask->removeInputSource(getMediaSocketPtr(pSocket), pSocketRemoveEvent) ;
        if (pSocketRemoveEvent->wait(0, maxEventTime) != OS_SUCCESS)
            OsSysLog::add(FAC_NET, PRI_ERR, "Timeout failure while removing media socket from NetInTask") ;
        pEventMgr->release(pSocketRemoveEvent);

        // Release Port number
        int port = pSocket->getSocket()->getLocalHostPort() ;
        mpFactoryImpl->releaseRtpPort(port) ;

        delete pSocket ;
    }
}


IOsNatSocket* 
VoiceEngineSocketFactory::getTcpSocket(GipsVoiceEngineLib* pVoice,
                                       GipsVideoEnginePlatform* pVideo,
                                       int type,
                                       int remoteHostPort,
                                       const char* remoteHost,
                                       const char* localHost,
                                       const RtpTcpRoles tcpRole)
{
    // Allocate port number
    int port = 0;
    mpFactoryImpl->getNextRtpPort(port) ;
           
    // Create Socket
    IOsNatSocket* pSocket = new VoiceEngineConnectionSocket(
            pVoice, pVideo,
            type,
            remoteHostPort, remoteHost, 
            port, localHost,
            NULL, 
            tcpRole) ;
    assert (pSocket != NULL) ;

#if VSF_TRACE
    printf("VoiceEngineSocketFactory::getTcpSocket 0x%08X, port=%d\n", pSocket, port) ;
#endif


    return pSocket;
}

void VoiceEngineSocketFactory::releaseTcpSocket(IOsNatSocket* pSocket) 
{
#if VSF_TRACE
printf("VoiceEngineSocketFactory::releaseTcpSocket 0x%08X\n", pSocket) ;
#endif

    if (pSocket != NULL)
    {
        // Release Port number
        int port = pSocket->getSocket()->getLocalHostPort() ;
        mpFactoryImpl->releaseRtpPort(port) ;

        delete pSocket ;
    }
}


ArsConnectionSocket* 
VoiceEngineSocketFactory::getArsSocket(const ProxyDescriptor& arsProxy,
                                       const ProxyDescriptor& arsHttpsProxy,
                                       ARS_MIMETYPE mimeType,
                                       const char* szLocalAddress) 
{
    ArsConnectionSocket* pSocket = new ArsConnectionSocket(
            arsProxy.getAddress(), arsProxy.getPort(),
            arsProxy.getUsername(),
            mimeType,
            szLocalAddress) ;

    if (arsHttpsProxy.isValid())
    {
        pSocket->setHttpsProxy(
                arsHttpsProxy.getAddress(),
                arsHttpsProxy.getPort(),
                arsHttpsProxy.getUsername(),
                arsHttpsProxy.getPassword()) ;
    }

#if VSF_TRACE
    printf("VoiceEngineSocketFactory::getArsSocket 0x%08X\n", pSocket) ;
#endif

    return pSocket ;
}


void VoiceEngineSocketFactory::releaseArsSocket(ArsConnectionSocket* pSocket) 
{
#if VSF_TRACE
printf("VoiceEngineSocketFactory::releaseArsSocket 0x%08X\n", pSocket) ;
#endif

    if (pSocket != NULL)
    {
        // Remove from NetInTask
        OsTime maxEventTime(20, 0);
        OsProtectEventMgr* pEventMgr = OsProtectEventMgr::getEventMgr();
        OsProtectedEvent* pSocketRemoveEvent = pEventMgr->alloc();
        mpNetTask->removeInputSource(pSocket, pSocketRemoveEvent) ;
        if (pSocketRemoveEvent->wait(0, maxEventTime) != OS_SUCCESS)
            OsSysLog::add(FAC_NET, PRI_ERR, "Timeout failure while removing media socket from NetInTask") ;
        pEventMgr->release(pSocketRemoveEvent);

        pSocket->close() ;
        delete pSocket ;
    }
}

