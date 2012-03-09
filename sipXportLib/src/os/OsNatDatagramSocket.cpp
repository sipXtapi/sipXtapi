//
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsIntTypes.h"
#include <assert.h>
#include <stdio.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

// APPLICATION INCLUDES
#include "os/OsNatDatagramSocket.h"
#include "os/OsNatAgentTask.h"
#include "os/StunMessage.h"
#include "os/StunUtils.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "os/OsEvent.h"
#include "os/OsNotification.h"
#include "tapi/sipXtapi.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DEFAULT_MEDIA_STUN_KEEPALIVE        28

// STATIC VARIABLE INITIALIZATIONS

// FORWARD DECLARATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsNatDatagramSocket::OsNatDatagramSocket(int remoteHostPortNum,
                                         const char* remoteHost, 
                                         int localHostPortNum, 
                                         const char* localHost,
                                         OsNotification *pNotification) 
        : OsDatagramSocket(remoteHostPortNum, remoteHost, localHostPortNum, localHost)
{    
    miRecordTimes = ONDS_MARK_NONE ;
    mpReadNotification = NULL ;

    // Init Stun state
    mStunState.bEnabled = false ;
    mStunState.status = NAT_STATUS_UNKNOWN ;
    mStunState.mappedAddress = NULL ;
    mStunState.mappedPort = PORT_NONE ;
           
    // Init Turn state
    mTurnState.bEnabled = false ;
    mTurnState.status = NAT_STATUS_UNKNOWN ;
    mTurnState.relayAddress = NULL ;
    mTurnState.relayPort = PORT_NONE ;

    // Init other attributes
    mpNatAgent = OsNatAgentTask::getInstance() ;
    mbTransparentReads = TRUE ;
    mDestAddress = mRemoteIpAddress ;
    miDestPort = remoteHostPort ;
    miDestPriority = -1 ;

    mpNotification = pNotification ;      
    mbNotified = false ;
}


// Destructor
OsNatDatagramSocket::~OsNatDatagramSocket()
{
    destroy() ;
}

void OsNatDatagramSocket::destroy()
{
    mpNatAgent->removeKeepAlives(this) ;
    mpNatAgent->removeStunProbes(this) ;
    disableStun() ;
    disableTurn() ;

    mpNatAgent->synchronize() ;
 }

/* ============================ MANIPULATORS ============================== */

OsSocket* OsNatDatagramSocket::getSocket()
{
    return this;
}

int OsNatDatagramSocket::read(char* buffer, int bufferLength)
{
    bool bNatPacket ;
    int iRC ;
    UtlString receivedIp ;
    int iReceivedPort ;

    do
    {
        bNatPacket = FALSE ;
        iRC = OsSocket::read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;
        if (handleSturnData(buffer, iRC, receivedIp, iReceivedPort))
        {
            if (!mbTransparentReads)
                iRC = 0 ;
            else
                bNatPacket = TRUE ;
        }
    } while ((iRC >= 0) && bNatPacket) ;

    // Make read time for non-NAT packets
    if (iRC > 0 && !bNatPacket)
        markReadTime() ;

    return iRC ;
}

int OsNatDatagramSocket::read(char* buffer, int bufferLength,
       UtlString* ipAddress, int* port)
{
    bool bNatPacket ;
    int iRC ;
    UtlString receivedIp ;
    int iReceivedPort ;

    do
    {
        bNatPacket = FALSE ;
        iRC = OsSocket::read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;       
        if (handleSturnData(buffer, iRC, receivedIp, iReceivedPort))
        {
            if (!mbTransparentReads)
                iRC = 0 ;
            else
                bNatPacket = TRUE ;
        }    
    } while ((iRC >= 0) && bNatPacket) ;

    if (ipAddress)
    {
        *ipAddress = receivedIp ;
    }

    if (port)
    {
        *port = iReceivedPort ;
    }

    // Make read time for non-NAT packets
    if (iRC > 0 && !bNatPacket)
    {
        markReadTime() ;
    }

    return iRC ;
}

int OsNatDatagramSocket::read(char* buffer, int bufferLength,
       struct in_addr* ipAddress, int* port)
{
    int iRC ;
    int iReceivedPort ;
    UtlString receivedIp ;

    iRC = read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;
    if (ipAddress)
        ipAddress->s_addr = inet_addr(receivedIp) ;

    if (port)
        *port = iReceivedPort ;

    return iRC ;
}


int OsNatDatagramSocket::read(char* buffer, int bufferLength, long waitMilliseconds)
{        
    bool bNatPacket ;
    int iRC = 0 ;
    UtlString receivedIp ;
    int iReceivedPort ;

    do
    {
        bNatPacket = FALSE ;
        if (isReadyToRead(waitMilliseconds))
        {
            iRC = OsSocket::read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;            
            if (handleSturnData(buffer, iRC, receivedIp, iReceivedPort))
            {
                if (!mbTransparentReads)
                    iRC = 0 ;
                else
                    bNatPacket = TRUE ;
            }
        }
        else
        {
            break ;
        }
    } while ((iRC >= 0) && bNatPacket) ;

    // Make read time for non-NAT packets
    if (iRC > 0 && !bNatPacket)
    {
        markReadTime() ;
    }

    return iRC ;    
}

int OsNatDatagramSocket::socketWrite(const char* buffer, int bufferLength,
                               const char* ipAddress, int port, PacketType packetType)
{
    if (packetType == MEDIA_PACKET)
    {
        markWriteTime() ;
    }

    return OsDatagramSocket::write(buffer, bufferLength, ipAddress, port) ;
}

int OsNatDatagramSocket::write(const char* buffer, int bufferLength)
{
    int rc ;

    markWriteTime() ;

    // Datagram sockets can be simulate a connection-oriented socket (in API) 
    // by allowing a connect().  This filters inbound packets from others (on 
    // Win32 -- not sure if this is the case for all platforms) and breaks 
    // ICE.
    if (mIsConnected)
    {
        rc = OsDatagramSocket::write(buffer, bufferLength) ;
    }
    else
    {
        if (mDestAddress.length() > 0 && miDestPort > 0)
        {
            rc = OsDatagramSocket::write(buffer, bufferLength, mDestAddress.data(), miDestPort) ;
        }
        else
        {
            // This is suggest something is writing without applying a 
            // destination address. 
            assert(false) ;
            rc = 0 ;
        }
    }

    return rc ;
}

int OsNatDatagramSocket::write(const char* buffer, 
                               int bufferLength,
                               const char* ipAddress, 
                               int port)
{
    markWriteTime() ;
    return OsDatagramSocket::write(buffer, bufferLength, ipAddress, port) ;
}

int OsNatDatagramSocket::write(const char* buffer, int bufferLength, 
                               long waitMilliseconds)
{
    markWriteTime() ;
    return OsSocket::write(buffer, bufferLength, waitMilliseconds) ;
}

void OsNatDatagramSocket::enableStun(const char* szStunServer, int stunPort, int iKeepAlive,  int iStunOptions, bool bReadFromSocket) 
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
        "OsNatDatagramSocket::enableStun(szStunServer=%s, stunPort=%d, iKeepAlive=%d, iStunOptions=%d, bReadFromSocket=%s) port: %d descriptor: %d this: %p",
        szStunServer, stunPort, iKeepAlive, iStunOptions, bReadFromSocket, getLocalHostPort(), getSocketDescriptor(), this);
#endif

    if (!mStunState.bEnabled)
    {
        mStunState.bEnabled = true ;

        UtlBoolean bRC = mpNatAgent->enableStun(this, szStunServer, stunPort, iStunOptions, iKeepAlive) ;
        if (bRC)
        {
            if (bReadFromSocket)
            {
                bool bTransparent = mbTransparentReads ;
                mbTransparentReads = false ;

                char cBuf[2048] ;            

                while (mStunState.status == NAT_STATUS_UNKNOWN)
                {
                    read(cBuf, sizeof(cBuf), 100) ;                
                    if (mStunState.status == NAT_STATUS_UNKNOWN)
                    {
                        OsTask::yield() ;
                    }
                }

                mbTransparentReads = bTransparent ;
            }
        }
        else
        {
            mStunState.status = NAT_STATUS_FAILURE ;
        }
    }
}


void OsNatDatagramSocket::disableStun()
{
    if (mStunState.bEnabled)
    {
        mStunState.bEnabled = false ;
        mpNatAgent->disableStun(this) ;
    }
}


void OsNatDatagramSocket::enableTurn(const char* szTurnSever,
                                     int turnPort,
                                     int iKeepAlive,
                                     const char* username,
                                     const char* password,
                                     bool bReadFromSocket)
{
    if (!mTurnState.bEnabled)
    {
        mTurnState.bEnabled = true ;
    
        UtlBoolean bRC = mpNatAgent->enableTurn(this, szTurnSever, turnPort, iKeepAlive, username, password) ;
        if (bRC)
        { 
            if (bReadFromSocket)
            {
                bool bTransparent = mbTransparentReads ;
                mbTransparentReads = false ;

                char cBuf[2048] ;

                while (mTurnState.status == NAT_STATUS_UNKNOWN)
                {
                    read(cBuf, sizeof(cBuf), 100) ;
                    if (mTurnState.status == NAT_STATUS_UNKNOWN)
                    {
                        OsTask::yield() ;
                    }
                }

                mbTransparentReads = bTransparent ;
            }
        }
        else
        {
            mTurnState.status = NAT_STATUS_FAILURE ;    
        }
    }
}

void OsNatDatagramSocket::disableTurn() 
{
    if (mTurnState.bEnabled)
    {
        mTurnState.bEnabled = false ;   
        mpNatAgent->disableTurn(this) ;
    }
}


bool OsNatDatagramSocket::waitForBinding(NAT_BINDING binding, bool bWaitUntilReady)
{
   bool result = false;

   if (binding != NO_BINDING)
   {
      char cBuf[2048];

      do
      {
         if (!(((binding == STUN_BINDING || binding == STUN_TURN_BINDING) &&
            mStunState.status == NAT_STATUS_UNKNOWN)
            ||
            ((binding == TURN_BINDING || binding == STUN_TURN_BINDING) &&
            mTurnState.status == NAT_STATUS_UNKNOWN)))
         {
            // leave loop, since stun/turn result is known
            break;
         }

         read(cBuf, sizeof(cBuf), 50);
      } while (bWaitUntilReady);

      if (!(((binding == STUN_BINDING || binding == STUN_TURN_BINDING) &&
         mStunState.status == NAT_STATUS_UNKNOWN)
         ||
         ((binding == TURN_BINDING || binding == STUN_TURN_BINDING) &&
         mTurnState.status == NAT_STATUS_UNKNOWN)))
      {
         // if stun or turn result is known and is requested
         result = false; // no need to call this function again
      }
      else
      {
         // either stun or turn result is not yet available and is requested
         result = true; // we need to call this function again
      }
   }
   
   return result;
}


void OsNatDatagramSocket::enableTransparentReads(bool bEnable)
{
    mbTransparentReads = bEnable ;
}

UtlBoolean OsNatDatagramSocket::addCrLfKeepAlive(const char* szRemoteIp,
                                                 const int   remotePort, 
                                                 const int   keepAliveSecs,
                                                 OsNatKeepaliveListener* pListener) 
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
        "OsNatDatagramSocket::addCrLfKeepAlive(szRemoteIp=%s, remotePort=%d, keepAliveSecs=%d, pListener=%p) for port: %d descriptor: %d this: %p",
        szRemoteIp, remotePort, keepAliveSecs, pListener, getLocalHostPort(), getSocketDescriptor(), this);
#endif

    return mpNatAgent->addCrLfKeepAlive(this, szRemoteIp, remotePort, 
            keepAliveSecs, pListener) ;
}


UtlBoolean OsNatDatagramSocket::removeCrLfKeepAlive(const char* szRemoteIp, 
                                                    const int   remotePort) 
{
    return mpNatAgent->removeCrLfKeepAlive(this, szRemoteIp, remotePort) ;
}

UtlBoolean OsNatDatagramSocket::addStunKeepAlive(const char* szRemoteIp, 
                                                 const int   remotePort, 
                                                 const int   keepAliveSecs,
                                                 OsNatKeepaliveListener* pListener) 
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
        "OsNatDatagramSocket::addStunKeepAlive(szRemoteIp=%s, remotePort=%d, keepAliveSecs=%d, pListener=%p) for port: %d descriptor: %d this: %p",
        szRemoteIp, remotePort, keepAliveSecs, pListener, getLocalHostPort(), getSocketDescriptor(), this);
#endif

    return mpNatAgent->addStunKeepAlive(this, szRemoteIp, remotePort, 
            keepAliveSecs, pListener) ;
}


UtlBoolean OsNatDatagramSocket::removeStunKeepAlive(const char* szRemoteIp, 
                                                    const int   remotePort) 
{
    return mpNatAgent->removeStunKeepAlive(this, szRemoteIp, remotePort) ;
}


/* ============================ ACCESSORS ================================= */

// Return the external mapped IP address for this socket.
UtlBoolean OsNatDatagramSocket::getMappedIp(UtlString* ip, int* port) 
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

    if (mStunState.mappedAddress.length() && mStunState.bEnabled) 
    {
        if (ip)
        {
            *ip = mStunState.mappedAddress ;
        }

        if (port)
        {
            *port = mStunState.mappedPort ;
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
UtlBoolean OsNatDatagramSocket::getRelayIp(UtlString* ip, int* port) 
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

    if (mTurnState.relayAddress.length() && mTurnState.bEnabled) 
    {
        if (ip)
        {
            *ip = mTurnState.relayAddress ;
        }

        if (port)
        {
            *port = mTurnState.relayPort ;
        }

        // Success if we were able to set either the ip or port
        if (ip || port)
        {
            bSuccess = true ;
        }
    }

    return bSuccess ;
}



void OsNatDatagramSocket::addAlternateDestination(const char* szAddress, int iPort, int priority) 
{
    mpNatAgent->sendStunProbe(this, szAddress, iPort, priority) ;
}


void OsNatDatagramSocket::setReadNotification(OsNotification* pNotification)
{
    OsLock lock(mReadNotificationLock) ;

    mpReadNotification = pNotification ;
}


void OsNatDatagramSocket::readyDestination(const char* szAddress, int iPort) 
{
    if (mTurnState.bEnabled && (mTurnState.status == NAT_STATUS_SUCCESS))
    {
        mpNatAgent->primeTurnReception(this, szAddress, iPort) ;
    }
}


void OsNatDatagramSocket::setNotifier(OsNotification* pNotification) 
{
    mpNotification = pNotification ;
    mbNotified = false ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void OsNatDatagramSocket::setStunAddress(const UtlString& address, 
                                         const int iPort) 
{
    mStunState.mappedAddress = address ;
    mStunState.mappedPort = iPort ;
}

void OsNatDatagramSocket::setTurnAddress(const UtlString& address, 
                                         const int iPort) 
{
    mTurnState.relayAddress = address ;
    mTurnState.relayPort = iPort ;
}


void OsNatDatagramSocket::markStunSuccess(bool bAddressChanged)
{
    mStunState.status = NAT_STATUS_SUCCESS ;

    // Signal external identities interested in the STUN outcome.
    if (mpNotification && (!mbNotified || bAddressChanged))
    {   
        UtlString adapterName;
        
        getContactAdapterName(adapterName, mLocalIp, false);

        SIPX_CONTACT_ADDRESS* pContact = new SIPX_CONTACT_ADDRESS();
        
        strcpy(pContact->cIpAddress, mStunState.mappedAddress);
        pContact->iPort = mStunState.mappedPort;
        strcpy(pContact->cInterface, adapterName.data());
        pContact->eContactType = CONTACT_NAT_MAPPED;
        pContact->eTransportType = TRANSPORT_UDP ;
                
        mpNotification->signal((intptr_t) pContact) ;
        mbNotified = true ;
    }
}


void OsNatDatagramSocket::markStunFailure() 
{
    mStunState.status = NAT_STATUS_FAILURE ;

    // Signal external identities interested in the STUN outcome.
    if (mpNotification && !mbNotified)
    {
        mpNotification->signal(0) ;
        mbNotified = true ;
    }
}


void OsNatDatagramSocket::markTurnSuccess() 
{
    mTurnState.status = NAT_STATUS_SUCCESS ;
}


void OsNatDatagramSocket::markTurnFailure() 
{
    mTurnState.status = NAT_STATUS_FAILURE ;
}


void OsNatDatagramSocket::evaluateDestinationAddress(const UtlString& address, 
                                                    int              iPort, 
                                                    int              priority) 
{
    if ((address.compareTo(mDestAddress, UtlString::ignoreCase) != 0) && 
            (iPort != priority))
    {
        if (priority > miDestPriority)
        {
            miDestPriority = priority ;
            mDestAddress = address ;
            miDestPort = iPort ;
        }
    } 
    else if (priority > miDestPriority) 
    {
        // No change in host/port, just store updated priority.
        miDestPriority = priority ;   
    }
}


UtlBoolean OsNatDatagramSocket::getBestDestinationAddress(UtlString& address,
                                                      int&       iPort)
{
    UtlBoolean bRC = false ;

    // Wait for stun request to complete for anything of a higher priority
    while (mpNatAgent->areProbesOutstanding(this, miDestPriority))
    {
        OsTask::delay(20) ;
    }

    // Return success value
    if (mDestAddress.length())
    {
        address = mDestAddress ;
        iPort = miDestPort ;

        bRC = portIsValid(iPort) ;
    }

    return bRC ;
}


UtlBoolean OsNatDatagramSocket::applyDestinationAddress(const char* szAddress, int iPort) 
{
    UtlBoolean bRC = false ;

    mDestAddress = szAddress ;
    miDestPort = iPort ;

    // ::TODO:: The keepalive period should be configurable (taken from 
    // default stun keepalive setting)
    if (!addStunKeepAlive(szAddress, iPort, DEFAULT_MEDIA_STUN_KEEPALIVE, NULL))
    {
        // Bob: [2006-06-13] The only way this fails right now is if the 
        //      binding is already added.
    }
   
    if (mpNatAgent->setTurnDestination(this, szAddress, iPort))
    {
        bRC = true ;
    }

    return bRC ;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
