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
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>
#ifndef _WIN32
#include <netinet/in.h>
#endif

// APPLICATION INCLUDES
#include "os/OsNatSocketBaseImpl.h"
#include "os/OsNatKeepaliveListener.h"
#include "os/OsNatAgentTask.h"
#include "os/StunMessage.h"
#include "os/StunUtils.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "os/OsEvent.h"
#include "os/OsSocket.h"
#include "os/OsContact.h"
#include "tapi/sipXtapi.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DELAY_TURN_THRESHOLD_SECS       45

// STATIC VARIABLE INITIALIZATIONS

// FORWARD DECLARATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsNatSocketBaseImpl::OsNatSocketBaseImpl() 
{   
    // Init Stun state
    mStunState.bEnabled = false ;
    mStunState.status = NAT_STATUS_UNKNOWN ;
    mStunState.address = NULL ;
    mStunState.port = PORT_NONE ;
           
    // Init Turn state
    mTurnState.bEnabled = false ;
    mTurnState.status = NAT_STATUS_UNKNOWN ;
    mTurnState.address = NULL ;
    mTurnState.port = PORT_NONE ;

    // Init other attributes
    mpNatAgent = OsNatAgentTask::getInstance() ;
    miDestPort = PORT_NONE ;
    miDestPriority = -1 ;
    mbDestViaOurRelay = false ;
    mbTransparentStunRead = true ;    
    mpStunNotification = NULL ;
    mbStunNotified = false ;
    mbReceivedDI = false ;
    mbDelayedTurnShutdown = false ;
    miFallbackPriority = -1 ;

    miUpnpMappedPort = PORT_NONE ;

    mIceState = IS_NOT_STARTED ;
    mIceStartMS = 0; 
    mIceEndMS = 0 ;
}


// Destructor
OsNatSocketBaseImpl::~OsNatSocketBaseImpl()
{
    destroy() ;
}


void OsNatSocketBaseImpl::destroy() 
{
    if (mpNatAgent)
    {
        mpNatAgent->removeKeepAlives(this) ;
        mpNatAgent->removeStunProbes(this) ;
    
        disableStun() ;
        disableTurn() ;

        mpNatAgent->synchronize() ;
    }
    mpNatAgent = NULL ;
}

/* ============================ MANIPULATORS ============================== */


void OsNatSocketBaseImpl::setTransparentStunRead(bool bEnable) 
{
    mbTransparentStunRead = bEnable ;
}


UtlBoolean OsNatSocketBaseImpl::addCrLfKeepAlive(const char* szRemoteIp,
                                                 const int   remotePort, 
                                                 const int   keepAliveSecs,
                                                 OsNatKeepaliveListener* pListener) 
{
    UtlBoolean bRC = false ;
    if (mpNatAgent && getSocket())
    {
        bRC = mpNatAgent->addCrLfKeepAlive(this, szRemoteIp, remotePort, 
                keepAliveSecs, pListener) ;
    }

    return bRC ;
}


UtlBoolean OsNatSocketBaseImpl::removeCrLfKeepAlive(const char* szRemoteIp, 
                                                    const int   remotePort) 
{
    UtlBoolean bRC = false ;
    if (mpNatAgent && getSocket())
    {
        bRC = mpNatAgent->removeCrLfKeepAlive(this, szRemoteIp, remotePort) ;
    }
    return bRC ;
}

UtlBoolean OsNatSocketBaseImpl::addStunKeepAlive(const char* szRemoteIp, 
                                                 const int   remotePort, 
                                                 const int   keepAliveSecs,
                                                 OsNatKeepaliveListener* pListener) 
{
    UtlBoolean bRC = false ;
    if (mpNatAgent && getSocket())
    {
        bRC = mpNatAgent->addStunKeepAlive(this, szRemoteIp, remotePort, 
                keepAliveSecs, pListener) ;
    }
    return bRC ;
}


UtlBoolean OsNatSocketBaseImpl::removeStunKeepAlive(const char* szRemoteIp, 
                                                    const int   remotePort) 
{
    UtlBoolean bRC = false ;
    if (mpNatAgent && getSocket())
    {
        bRC = mpNatAgent->removeStunKeepAlive(this, szRemoteIp, remotePort) ;
    }
    return bRC ;
}

void OsNatSocketBaseImpl::enableStun(const char* szStunServer, 
                                     int stunPort, 
                                     int iKeepAlive, 
                                     int iStunOptions, 
                                     bool bReadFromSocket,
                                     int iNCPeriod) 
{    
    if (!mStunState.bEnabled && mpNatAgent)
    {
        mStunState.bEnabled = true ;

        bool bRC = mpNatAgent->enableStun(this, szStunServer, stunPort, iStunOptions, iKeepAlive) ;
        if (bRC)
        {
            if (bReadFromSocket)
            {
                bool bTransparent = getTransparentStunRead() ;
                setTransparentStunRead(false) ;

                char cBuf[2048] ;            

                while (mStunState.status == NAT_STATUS_UNKNOWN)
                {
                    getSocket()->read(cBuf, sizeof(cBuf), 10) ;
                    if (mStunState.status == NAT_STATUS_UNKNOWN)
                    {
                        OsTask::yield() ;
                    }
                }
                setTransparentStunRead(bTransparent) ; 

            }

            if (iNCPeriod >= 0)
            {
                if (!bReadFromSocket)
                {
                    mpNatAgent->performNatClassification(this, szStunServer, stunPort, iNCPeriod, this) ;
                }
                else
                {
                    // Cannot do nat classification in read from socket mode
                    assert(false) ;
                }
            }
        }
        else
        {
            mStunState.status = NAT_STATUS_FAILURE ;
        }
    }
}


void OsNatSocketBaseImpl::disableStun()
{
    if (mStunState.bEnabled && mpNatAgent)
    {
        mStunState.bEnabled = false ;
        mpNatAgent->disableStun(this) ;
    }
}


void OsNatSocketBaseImpl::enableTurn(const char* szTurnSever,
                                     int turnPort,
                                     int iKeepAlive,
                                     const char* username,
                                     const char* password,
                                     bool bReadFromSocket)
{
    if (!mTurnState.bEnabled && mpNatAgent)
    {
        mTurnState.bEnabled = true ;
    
        bool bRC = mpNatAgent->enableTurn(this, szTurnSever, turnPort, iKeepAlive, username, password) ;
        if (bRC)
        { 
            if (bReadFromSocket)
            {
                bool bTransparent = getTransparentStunRead() ;
                setTransparentStunRead(false) ;

                char cBuf[2048] ;

                while (mTurnState.status == NAT_STATUS_UNKNOWN)
                {
                    getSocket()->read(cBuf, sizeof(cBuf), 500) ;
                    if (mTurnState.status == NAT_STATUS_UNKNOWN)
                    {
                        OsTask::yield() ;
                    }
                }
                setTransparentStunRead(bTransparent) ;
            }
        }
        else
        {
            mTurnState.status = NAT_STATUS_FAILURE ;    
        }
    }
}

void OsNatSocketBaseImpl::primeTurnDestination(const char* szAddress, int iPort) 
{
    if (mTurnState.bEnabled && (mTurnState.status == NAT_STATUS_SUCCESS) && mpNatAgent)
    {
        mpNatAgent->primeTurnReception(this, szAddress, iPort);
    }
}

UtlBoolean OsNatSocketBaseImpl::setTurnDestination(const char* szAddress, int iPort) 
{
    UtlBoolean bRC = false ;

    mDestAddress = szAddress ;
    miDestPort = iPort ;

    OsSysLog::add(FAC_NET, PRI_INFO, "Setting Turn Destination to %s:%d",
            szAddress, iPort) ;
   
    if (mpNatAgent && mpNatAgent->setTurnDestination(this, szAddress, iPort))
    {
        bRC = true ;
    }

    // ::TODO:: The keepalive period should be configurable (taken from 
    // default stun keepalive setting)
    if (!addStunKeepAlive(szAddress, iPort, DEFAULT_MEDIA_STUN_KEEPALIVE, NULL))
    {
        // Bob: [2006-06-13] The only way this fails right now is if the 
        //      binding is already added.
    }


    return bRC ;
}

void OsNatSocketBaseImpl::disableTurn(bool bForce) 
{
    // Requesting a soft-disable.  Keep the turn connection open in case 
    // the remote party is using it (partial ICE failure).
    if (bForce == false)
    {
        // Only delaying closing if we have received some DIs.
        OsDateTime lastDI ;
        if (!getLastDI(lastDI))
        {
            bForce = true ;            
        }
        else
            mbDelayedTurnShutdown = true ;
    }

    if (bForce)
    {
        mbDelayedTurnShutdown = false ;
        if (mTurnState.bEnabled && mpNatAgent)
        {
            mTurnState.bEnabled = false ;   
            mpNatAgent->disableTurn(this) ;
        }

        mDestAddress.remove(0) ;
        miDestPort = PORT_NONE ;
        miDestPriority = -1 ;
        mbDestViaOurRelay = false ;
        miFallbackPriority = -1 ;

        mTurnState.status = NAT_STATUS_UNKNOWN ;
        mTurnState.address = NULL ;
        mTurnState.port = PORT_NONE ;
    }
}

// Return the external mapped IP address for this socket.
UtlBoolean OsNatSocketBaseImpl::getMappedIp(UtlString* ip, int* port) 
{
    UtlBoolean bSuccess = false ;

    // Wait for mapped IP to become available
    while (mStunState.status == NAT_STATUS_UNKNOWN && mStunState.bEnabled)
    {
        if (mStunState.status == NAT_STATUS_UNKNOWN && mStunState.bEnabled)
        {
            OsTask::yield() ;
        }
    }

    if (mStunState.address.length() && mStunState.bEnabled) 
    {
        if (ip)
        {
            *ip = mStunState.address ;
        }

        if (port)
        {
            *port = mStunState.port ;
        }

        // Success if we were able to set either the ip or port
        if (ip || port)
        {
            bSuccess = true ;
        }
    }

    return bSuccess ;
}

// Return the external relay/return IP address for this socket.
UtlBoolean OsNatSocketBaseImpl::getTurnIp(UtlString* ip, int* port) 
{
    UtlBoolean bSuccess = false ;

    // Wait for relay IP to become available
    while (mTurnState.status == NAT_STATUS_UNKNOWN && mTurnState.bEnabled)
    {
        if (mTurnState.status == NAT_STATUS_UNKNOWN && mTurnState.bEnabled)
        {
            OsTask::yield() ;
        }
    }

    if (mTurnState.address.length() && mTurnState.bEnabled) 
    {
        if (ip)
        {
            *ip = mTurnState.address ;
        }

        if (port)
        {
            *port = mTurnState.port ;
        }

        // Success if we were able to set either the ip or port
        if (ip || port)
        {
            bSuccess = true ;
        }
    }

    return bSuccess ;
}

void OsNatSocketBaseImpl::addAlternateDestination(const char* szAddress, int iPort, int priority)
{

    if (mIceState == IS_NOT_STARTED)
    {
        mIceState = IS_STARTED ;
        mIceStartMS = OsDateTime::getCurTimeInMS() ;
    }

    if (priority > miFallbackPriority)
    {
        mFallbackContact.setAddress(szAddress) ;
        mFallbackContact.setPort(iPort) ;
        mFallbackContact.setProtocol(OsSocket::UDP) ;
        mFallbackContact.setAddressType(IP4) ;

        miFallbackPriority = priority ;
    }

    if (mpNatAgent)
        mpNatAgent->sendStunProbe(this, szAddress, iPort, NULL, 0, priority) ;    
}

void OsNatSocketBaseImpl::setStunNotifier(OsNotification* pStunNotification) 
{
    mpStunNotification = pStunNotification ;
    mbStunNotified = false ;
}


void OsNatSocketBaseImpl::OnClassificationComplete(NAT_CLASSIFICATION_TYPE type) 
{
    if (mpStunNotification)
        mpStunNotification->signal((int) new OsNatOutcomeEvent(type)) ;

}

/* ============================ ACCESSORS ================================= */                           

OsSocket* OsNatSocketBaseImpl::getSocket()
{
    OsSocket* pSocket = dynamic_cast<OsSocket*>(this);
    assert(pSocket);

    return pSocket;
}

bool OsNatSocketBaseImpl::getLastDI(OsDateTime& time) 
{
    if (mbReceivedDI)
    {
        time = mLastDI ;
    }

    return mbReceivedDI ;
}

bool OsNatSocketBaseImpl::getTransparentStunRead() const 
{
    return mbTransparentStunRead ;
}


bool OsNatSocketBaseImpl::waitForBestDestination(bool       bLongWait,
                                                 UtlString& address,
                                                 int&       iPort,
                                                 bool&      bViaOurRelay,
                                                 int&       priority)
{
    // Wait for stun request to complete for anything of a higher priority
    while (mpNatAgent && mpNatAgent->areProbesOutstanding(this, miDestPriority, bLongWait))
    {
        OsTask::delay(50) ;
    }

    address = mDestAddress ;
    iPort = miDestPort ;
    bViaOurRelay = mbDestViaOurRelay ;
    priority = miDestPriority ;

    return (address.length() > 0) ;
}


UtlBoolean OsNatSocketBaseImpl::getBestDestinationAddress(UtlString& address,
                                                          int&       iPort,
                                                          bool&      bViaOurRelay,
                                                          int&       priority,
                                                          int&       totalTimeMS)
{
    UtlBoolean bRC = false ;

    if (mIceEndMS == 0)
    {
        mIceEndMS = OsDateTime::getCurTimeInMS() ;
    }
    mIceState = IS_COMPLETED ;

    totalTimeMS = (int) ((mIceEndMS - mIceStartMS) & 0x7FFFFFFF);
   
    if (mpNatAgent)
        mpNatAgent->logProbeResults(this) ;

    // Return success value
    if (mDestAddress.length())
    {
        address = mDestAddress ;
        iPort = miDestPort ;
        bViaOurRelay = mbDestViaOurRelay ;
        priority = miDestPriority ;

        bRC = portIsValid(iPort) ;
    }
    else if (miFallbackPriority > -1) 
    {
        address = mFallbackContact.getAddress() ;
        iPort = mFallbackContact.getPort() ;
        bViaOurRelay = false ;

        OsSysLog::add(FAC_MP, PRI_INFO, "ICE failed; returning fallback %s:%d\n",
                address.data(),
                iPort) ;

        bRC = false ;
    }

    if (mpNatAgent)
        mpNatAgent->removeStunProbes(this) ;

    return bRC ;
}


void OsNatSocketBaseImpl::setUpnpMappedPort(int port)
{
    miUpnpMappedPort = port ;
}


int OsNatSocketBaseImpl::getUpnpMappedPort() const 
{
    return miUpnpMappedPort ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */


void OsNatSocketBaseImpl::handleStunMessage(char*      pBuf,
                                            int        length,
                                            UtlString& fromAddress, 
                                            int        fromPort,
                                            UtlString* pRelayIp,
                                            int*       pRelayPort)
{
    // Make copy and queue it. 
    char* szCopy = (char*) malloc(length) ;
    if (szCopy)
    {
        OsTime timeout(10) ;
        memcpy(szCopy, pBuf, length) ;
        NatMsg msg(NatMsg::STUN_MESSAGE, szCopy, length, this, fromAddress, 
                fromPort, pRelayIp, pRelayPort);

        if (OsNatAgentTask::getInstance()->postMessage(msg, timeout) != OS_SUCCESS)
        {
            free(szCopy) ;
        }
    }
}


void OsNatSocketBaseImpl::handleTurnMessage(char* pBuf, 
                                            int length, 
                                            UtlString& fromAddress, 
                                            int fromPort) 
{
    // Make copy and queue it. 
    char* szCopy = (char*) malloc(length) ;
    if (szCopy)
    { 
        OsTime timeout(10) ;
        memcpy(szCopy, pBuf, length) ;
        NatMsg msg(NatMsg::TURN_MESSAGE, szCopy, length, this, fromAddress, fromPort);
        if (OsNatAgentTask::getInstance()->postMessage(msg, timeout) != OS_SUCCESS)
        {
            free(szCopy) ;
        }
    }    
}


int OsNatSocketBaseImpl::handleTurnDataIndication(char*      buffer, 
                                                  int        bufferLength,
                                                  UtlString* pRecvFromIp,
                                                  int*       pRecvFromPort)
{
    int rc = 0 ;
    TurnMessage msg ;

    if (msg.parse(buffer, bufferLength))
    {
        char*          pData ;
        unsigned short nData ;

        mbReceivedDI = true ;
        OsDateTime::getCurTime(mLastDI) ;

        if (msg.getData(pData, nData))
        {
            assert(pData) ;
            assert(nData < bufferLength) ;
            if (pData && nData < bufferLength)
            {
                memcpy(buffer, pData, nData) ;
                rc = nData ;

                if (pRecvFromIp || pRecvFromPort)
                {
                    char           remoteAddr[32] ;
                    unsigned short remotePort ;
                    if (msg.getTurnRemoteAddress(remoteAddr, remotePort))
                    {
                        if (pRecvFromIp)
                            *pRecvFromIp = remoteAddr ;
                        if (pRecvFromPort)
                            *pRecvFromPort = remotePort ;
                    }
                }
            }
        }
    }

    return rc ;
}

bool OsNatSocketBaseImpl::handleSturnData(char*      buffer, 
                                          int&       bufferLength,
                                          UtlString& receivedIp,
                                          int&       receivedPort,
                                          UtlString* pRelayIp,
                                          int*       pRelayPort)
{
    bool bHandled = false ;
    bool bDataIndication = false ;

    if ((bufferLength > 0) && TurnMessage::isTurnMessage(buffer, bufferLength, &bDataIndication))
    {
        if (bDataIndication)
        {
            UtlString originalIp ;
            int       originalPort ;
            bufferLength = handleTurnDataIndication(buffer, bufferLength, 
                    &originalIp, &originalPort) ;

            // We need to recurse for DIs -- it may be an encapsulated STUN 
            // message (e.g. ICE)
            return handleSturnData(buffer, bufferLength, originalIp, originalPort, &receivedIp, &receivedPort) ;
        }
        else
        {
            handleTurnMessage(buffer, bufferLength, receivedIp, receivedPort) ;
            bHandled = true ;           
        }
    }
    else if ((bufferLength > 0) && StunMessage::isStunMessage(buffer, 
            bufferLength))
    {
        handleStunMessage(buffer, bufferLength, receivedIp, receivedPort, pRelayIp, pRelayPort) ;
        bHandled = true ;
    }

    return bHandled ;
}

void OsNatSocketBaseImpl::setStunAddress(const UtlString& address, 
                                         const int iPort) 
{
    mStunState.address = address ;
    mStunState.port = iPort ;
}

void OsNatSocketBaseImpl::setTurnAddress(const UtlString& address, 
                                         const int iPort) 
{
    mTurnState.address = address ;
    mTurnState.port = iPort ;
}


void OsNatSocketBaseImpl::markStunSuccess(bool bAddressChanged)
{
    mStunState.status = NAT_STATUS_SUCCESS ;

    // Signal external identities interested in the STUN outcome.
    if (mpStunNotification && (!mbStunNotified || bAddressChanged))
    {   
        UtlString adapterName;
        
        UtlString localIp ;
        getSocket()->getLocalHostIp(&localIp) ;
        getContactAdapterName(adapterName, localIp.data(), false);

        SIPX_CONTACT_ADDRESS contact ;
        
        strcpy(contact.cIpAddress, mStunState.address);
        contact.iPort = mStunState.port;
        strcpy(contact.cInterface, adapterName.data());
        contact.eContactType = CONTACT_NAT_MAPPED;
        contact.eTransportType = TRANSPORT_UDP ;
                
        mpStunNotification->signal((int) new OsNatOutcomeEvent(contact)) ;
        mbStunNotified = true ;
    }
}


void OsNatSocketBaseImpl::markStunFailure() 
{
    mStunState.status = NAT_STATUS_FAILURE ;

    // Signal external identities interested in the STUN outcome.
    if (mpStunNotification && !mbStunNotified)
    {
        mpStunNotification->signal((int) new OsNatOutcomeEvent()) ;
        mbStunNotified = true ;
    }
}


void OsNatSocketBaseImpl::markTurnSuccess() 
{
    mTurnState.status = NAT_STATUS_SUCCESS ;
}


void OsNatSocketBaseImpl::markTurnFailure() 
{
    mTurnState.status = NAT_STATUS_FAILURE ;
}


void OsNatSocketBaseImpl::evaluateDestinationAddress(const UtlString& address, 
                                                     int              iPort, 
                                                     int              priority,
                                                     bool             bViaOurRelay) 
{
    // Record last receive stun probe message
    if (mIceState == IS_STARTED)
    {
        mIceEndMS = OsDateTime::getCurTimeInMS() ;
    }

    if ((address.compareTo(mDestAddress, UtlString::ignoreCase) != 0) && 
            (iPort != miDestPort))
    {
        if (priority > miDestPriority)
        {
            miDestPriority = priority ;
            mDestAddress = address ;
            miDestPort = iPort ;
            mbDestViaOurRelay = bViaOurRelay ;
        }
    } 
    else if (priority > miDestPriority) 
    {
        // No change in host/port, just store updated priority.
        miDestPriority = priority ; 
        mbDestViaOurRelay = bViaOurRelay ;  // should not change
    }
}


void OsNatSocketBaseImpl::checkDelayedShutdown() 
{
    if (mbDelayedTurnShutdown)
    {
        OsDateTime now ;
        OsDateTime lastDI ;
        now.getCurTime(now) ;
        getLastDI(lastDI) ;

        if ((now.getSecsSinceEpoch() + DELAY_TURN_THRESHOLD_SECS) >= lastDI.getSecsSinceEpoch())
        {
            OsSysLog::add(FAC_NET, PRI_INFO, "Delayed shutdown of TURN connection to %s:%d",
                    mTurnState.address.data(),
                    mTurnState.port) ;

            disableTurn() ;
        }
    }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
