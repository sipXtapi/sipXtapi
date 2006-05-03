//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
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
#include "os/OsNatDatagramSocket.h"
#include "os/OsNatAgentTask.h"
#include "os/StunMessage.h"
#include "os/StunUtils.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "os/OsEvent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

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
}


// Destructor
OsNatDatagramSocket::~OsNatDatagramSocket()
{
    mpNatAgent->removeKeepAlives(this) ;
    mpNatAgent->removeStunProbes(this) ;
    disableStun() ;
    disableTurn() ;

    mpNatAgent->synchronize() ;
 }

/* ============================ MANIPULATORS ============================== */


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
        if ((iRC > 0) && TurnMessage::isTurnMessage(buffer, iRC))
        {
            handleTurnMessage(buffer, iRC, receivedIp, iReceivedPort) ;
            if (!mbTransparentReads)
            {
                iRC = 0 ;
            }
            else
            {
                bNatPacket = TRUE ;
            }
        }
        else if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {
            handleStunMessage(buffer, iRC, receivedIp, iReceivedPort) ;
            if (!mbTransparentReads)
            {
                iRC = 0 ;
            }
            else
            {
                bNatPacket = TRUE ;
            }
        }
    } while ((iRC >= 0) && bNatPacket) ;   

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
        if ((iRC > 0) && TurnMessage::isTurnMessage(buffer, iRC))
        {
            handleTurnMessage(buffer, iRC, receivedIp, iReceivedPort) ;
            if (!mbTransparentReads)
            {
                iRC = 0 ;
            }
            else
            {
                bNatPacket = TRUE ;
            }
        }
        else if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {
            handleStunMessage(buffer, iRC, receivedIp, iReceivedPort) ;
            if (!mbTransparentReads)
            {
                iRC = 0 ;
            }
            else
            {
                bNatPacket = TRUE ;
            }
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

    return iRC ;
}

int OsNatDatagramSocket::read(char* buffer, int bufferLength,
       struct in_addr* ipAddress, int* port)
{
    bool bNatPacket ;
    int iRC ;    
    struct in_addr fromSockAddress;
    int iReceivedPort ;
   
    do
    {
        bNatPacket = FALSE ;
        iRC = OsSocket::read(buffer, bufferLength, &fromSockAddress, &iReceivedPort) ;      
        if ((iRC > 0) && TurnMessage::isTurnMessage(buffer, iRC))
        {
            UtlString receivedIp ;
            inet_ntoa_pt(fromSockAddress, receivedIp);
            handleTurnMessage(buffer, iRC, receivedIp, iReceivedPort) ;
            if (!mbTransparentReads)
            {
                iRC = 0 ;
            }
            else
            {
                bNatPacket = TRUE ;
            }
        }
        else if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {
            UtlString receivedIp ;
            inet_ntoa_pt(fromSockAddress, receivedIp);
            handleStunMessage(buffer, iRC, receivedIp, iReceivedPort) ;
            if (!mbTransparentReads)
            {
                iRC = 0 ;
            }
            else
            {
                bNatPacket = TRUE ;
            }
        }
    } while ((iRC >= 0) && bNatPacket) ;

    if (ipAddress != NULL)
    {
        memcpy(ipAddress, &fromSockAddress, sizeof(struct in_addr)) ;
    }

    if (port != NULL)
    {
        *port = iReceivedPort ;
    }

    return iRC ;
}


int OsNatDatagramSocket::read(char* buffer, int bufferLength, long waitMilliseconds)
{        
    bool bNatPacket = FALSE ;
    int iRC = 0 ;
    UtlString receivedIp ;
    int iReceivedPort ;

    do
    {
        if (isReadyToRead(waitMilliseconds))
        {
            bNatPacket = FALSE ;
            iRC = OsSocket::read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;            
            if ((iRC > 0) && TurnMessage::isTurnMessage(buffer, iRC))
            {
                handleTurnMessage(buffer, iRC, receivedIp, iReceivedPort) ;
                if (!mbTransparentReads)
                {
                    iRC = 0 ;
                }
                else
                {
                    bNatPacket = TRUE ;
                }
            }
            else if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
            {
                handleStunMessage(buffer, iRC, receivedIp, iReceivedPort) ;
                if (!mbTransparentReads)
                {
                    iRC = 0 ;
                }
                else
                {
                    bNatPacket = TRUE ;
                }
            }
        }
        else
        {
            break ;
        }
    } while ((iRC >= 0) && bNatPacket) ;

    return iRC ;    
}


void OsNatDatagramSocket::enableStun(const char* szStunServer, int stunPort, int iKeepAlive,  int iStunOptions, bool bReadFromSocket) 
{    
    if (!mStunState.bEnabled)
    {
        mStunState.bEnabled = true ;

        bool bRC = mpNatAgent->enableStun(this, szStunServer, stunPort, iStunOptions, iKeepAlive) ;
        if (bRC)
        {
            if (bReadFromSocket)
            {
                bool bTransparent = mbTransparentReads ;
                mbTransparentReads = false ;

                char cBuf[2048] ;            

                while (mStunState.status == NAT_STATUS_UNKNOWN)
                {
                    read(cBuf, sizeof(cBuf), 500) ;                
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
    
        bool bRC = mpNatAgent->enableTurn(this, szTurnSever, turnPort, iKeepAlive, username, password) ;
        if (bRC)
        { 
            if (bReadFromSocket)
            {
                bool bTransparent = mbTransparentReads ;
                mbTransparentReads = false ;

                char cBuf[2048] ;

                while (mTurnState.status == NAT_STATUS_UNKNOWN)
                {
                    read(cBuf, sizeof(cBuf), 500) ;
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


void OsNatDatagramSocket::enableTransparentReads(bool bEnable)
{
    mbTransparentReads = bEnable ;
}

UtlBoolean OsNatDatagramSocket::addCrLfKeepAlive(const char* szRemoteIp,
                                                 const int   remotePort, 
                                                 const int   keepAliveSecs) 
{
    return mpNatAgent->addCrLfKeepAlive(this, szRemoteIp, remotePort, 
            keepAliveSecs) ;
}


UtlBoolean OsNatDatagramSocket::removeCrLfKeepAlive(const char* szRemoteIp, 
                                                    const int   remotePort) 
{
    return mpNatAgent->removeCrLfKeepAlive(this, szRemoteIp, remotePort) ;
}

UtlBoolean OsNatDatagramSocket::addStunKeepAlive(const char* szRemoteIp, 
                                                 const int   remotePort, 
                                                 const int   keepAliveSecs) 
{
    return mpNatAgent->addStunKeepAlive(this, szRemoteIp, remotePort, 
            keepAliveSecs) ;
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


void OsNatDatagramSocket::markStunSuccess()
{
    mStunState.status = NAT_STATUS_SUCCESS ;

    // Signal external identities interested in the STUN outcome.
    if (mpNotification)
    {   
        char szAdapterName[256];
        memset((void*)szAdapterName, 0, sizeof(szAdapterName));
        
        #ifdef _WIN32
            getContactAdapterName(szAdapterName, mLocalIp.data(), false);
        #else
            // TODO - call the appropriate Linux function to the adapter name
            assert(false) ;
        #endif

        CONTACT_ADDRESS* pContact = new CONTACT_ADDRESS();
        
        strcpy(pContact->cIpAddress, mStunState.mappedAddress);
        pContact->iPort = mStunState.mappedPort;
        strcpy(pContact->cInterface, szAdapterName);
        pContact->eContactType = NAT_MAPPED;
        pContact->transportType = OsSocket::UDP ;
                
        mpNotification->signal((int) pContact) ;        
        mpNotification = NULL ;
    }
}


void OsNatDatagramSocket::markStunFailure() 
{
    mStunState.status = NAT_STATUS_FAILURE ;

    // Signal external identities interested in the STUN outcome.
    if (mpNotification)
    {
        mpNotification->signal(0) ;
        mpNotification = NULL ;
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
    UtlBoolean bRC = mpNatAgent->setTurnDestination(this, szAddress, iPort) ;

    return bRC ;
}


void OsNatDatagramSocket::handleStunMessage(char* pBuf, 
                                            int length, 
                                            UtlString& fromAddress, 
                                            int fromPort) 
{
    // Make copy and queue it. 
    char* szCopy = (char*) malloc(length) ;
    if (szCopy)
    {
        memcpy(szCopy, pBuf, length) ;
        NatMsg msg(NatMsg::STUN_MESSAGE, szCopy, length, this, fromAddress, fromPort);
        mpNatAgent->postMessage(msg) ;
    }
}


void OsNatDatagramSocket::handleTurnMessage(char* pBuf, 
                                            int length, 
                                            UtlString& fromAddress, 
                                            int fromPort) 
{
    // Make copy and queue it. 
    char* szCopy = (char*) malloc(length) ;
    if (szCopy)
    {
        memcpy(szCopy, pBuf, length) ;
        NatMsg msg(NatMsg::TURN_MESSAGE, szCopy, length, this, fromAddress, fromPort);
        mpNatAgent->postMessage(msg) ;
    }    
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

/* ///////////////////////// HELPER CLASSES /////////////////////////////// */

NatMsg::NatMsg(int                  type,
               char*                szBuffer, 
               int                  nLength, 
               OsNatDatagramSocket* pSocket,
               UtlString            receivedIp,
               int                  iReceivedPort)
    : OsMsg(NAT_MSG_TYPE, 0)
{
    miType = type ;
    mBuffer = szBuffer ;    // Shallow copy
    mLength = nLength ;
    mpSocket = pSocket ;
    mReceivedIp = receivedIp ;
    miReceivedPort = iReceivedPort ;
    mpContext = NULL ;
}

NatMsg::NatMsg(int   type,
               void* pContext)
    : OsMsg(NAT_MSG_TYPE, 0)
{
    miType = type ;
    mBuffer = NULL ;
    mLength = 0;
    mpSocket = NULL ;
    mReceivedIp.remove(0) ;
    miReceivedPort = 0 ;
    mpContext = pContext ;
}


NatMsg::NatMsg(const NatMsg& rNatMsg)
    : OsMsg(NAT_MSG_TYPE, 0)
{
    miType = rNatMsg.miType ;
    mBuffer = rNatMsg.mBuffer ;
    mLength  = rNatMsg.mLength ;
    mpSocket = rNatMsg.mpSocket ;
    mReceivedIp = rNatMsg.mReceivedIp ;
    miReceivedPort = rNatMsg.miReceivedPort ;
    mpContext = rNatMsg.mpContext ;
}


OsMsg* NatMsg::createCopy(void) const
{
    return new NatMsg(*this);
}


NatMsg::~NatMsg()
{

}


NatMsg& NatMsg::operator=(const NatMsg& rhs)
{
    if (this != &rhs)            // handle the assignment to self case
    {
        miType = rhs.miType ;
        mBuffer = rhs.mBuffer ;
        mLength = rhs.mLength ;
        mpSocket = rhs.mpSocket ;
        mReceivedIp = rhs.mReceivedIp ;
        miReceivedPort = rhs.miReceivedPort ;
        mpContext = rhs.mpContext ;
    }

    return *this ;
}


char* NatMsg::getBuffer() const
{
    return mBuffer ;
}

   
int NatMsg::getLength() const
{
    return mLength ;
}


OsNatDatagramSocket* NatMsg::getSocket() const 
{
    return mpSocket ;
}


UtlString NatMsg::getReceivedIp() const 
{
    return mReceivedIp ;
}


int NatMsg::getReceivedPort() const 
{
    return miReceivedPort ;
}

int NatMsg::getType() const 
{
    return miType ;
}

void* NatMsg::getContext() const
{
    return mpContext ;
}
