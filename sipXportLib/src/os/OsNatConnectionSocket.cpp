//  
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2006-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsIntTypes.h"
#include <assert.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>     
#endif

// APPLICATION INCLUDES
#include "os/OsSocket.h"
#include "os/OsNatConnectionSocket.h"
#include "os/OsNatAgentTask.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "os/OsEvent.h"
#include "tapi/sipXtapi.h"
#include "os/OsProtectEvent.h"
#include "os/OsProtectEventMgr.h"
#include "utl/UtlVoidPtr.h"
#include "utl/UtlHashMapIterator.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DEFAULT_MEDIA_STUN_KEEPALIVE        28

// STATIC VARIABLE INITIALIZATIONS

// FORWARD DECLARATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsNatConnectionSocket::OsNatConnectionSocket(int connectedSocketDescriptor,
                                             const RtpTcpRoles role)
    : OsConnectionSocket(connectedSocketDescriptor),
      mRole(role),
      mRoleMutex(OsMutex::Q_FIFO),
      mStreamHandlerMutex(OsMutex::Q_FIFO),
      mpDatagramSocket(NULL),
      mpNatAgent(NULL),
      mFragmentSize(0)
{    
    if (0 == mRole)
    {
        mRole = RTP_TCP_ROLE_ACTPASS;
    }
    socketDescriptor = connectedSocketDescriptor;
    mpNatAgent = OsNatAgentTask::getInstance() ;
    mbTransparentReads = TRUE ;    
    mDestAddress = mRemoteIpAddress ;
    miDestPriority = -1 ;
}

OsNatConnectionSocket::OsNatConnectionSocket(const char* szLocalIp,
                                             int connectedSocketDescriptor,
                                             const RtpTcpRoles role)
    : OsConnectionSocket(szLocalIp, connectedSocketDescriptor),
      mRole(role),
      mRoleMutex(OsMutex::Q_FIFO),
      mStreamHandlerMutex(OsMutex::Q_FIFO),
      mpDatagramSocket(NULL),
      mpNatAgent(NULL),
      mFragmentSize(0)
      
{
    if (0 == mRole)
    {
        mRole = RTP_TCP_ROLE_ACTPASS;
    }
    socketDescriptor = connectedSocketDescriptor;
    mLocalIp = szLocalIp;
    mpNatAgent = OsNatAgentTask::getInstance() ;
    mbTransparentReads = TRUE ;    
    mDestAddress = mRemoteIpAddress ;
    miDestPriority = -1 ;
}

// Constructor
OsNatConnectionSocket::OsNatConnectionSocket(int serverPort,
                                       const char* serverName,
                                       UtlBoolean blockingConnect,
                                       const char* localIp,
                                       const bool bConnect,
                                       const RtpTcpRoles role)
        : OsConnectionSocket(serverPort, serverName, blockingConnect, localIp, bConnect),
          mRole(role),
          mRoleMutex(OsMutex::Q_FIFO),
          mStreamHandlerMutex(OsMutex::Q_FIFO),
          mpDatagramSocket(NULL),
          mpNatAgent(NULL),
          mFragmentSize(0)
{    
    if (0 == mRole)
    {
        mRole = RTP_TCP_ROLE_ACTPASS;
    }

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
}


// Destructor
OsNatConnectionSocket::~OsNatConnectionSocket()
{
    destroy();
}
 
 
void OsNatConnectionSocket::setRole(const RtpTcpRoles role)
{
    mRoleMutex.acquireWrite();
    mRole = role;
    mRoleMutex.releaseWrite();
}
const RtpTcpRoles OsNatConnectionSocket::getRole() const
{
    RtpTcpRoles role;
    mRoleMutex.acquireRead();
    role = mRole;
    mRoleMutex.releaseRead();
    return role;
}
 
void OsNatConnectionSocket::destroy()
{
    if (mpNatAgent)
    {
        mpNatAgent->removeKeepAlives(this) ;
        mpNatAgent->removeStunProbes(this) ;
    }
    disableStun() ;
    disableTurn() ;

    mpNatAgent->synchronize() ;
}

/* ============================ MANIPULATORS ============================== */

OsSocket* OsNatConnectionSocket::getSocket()
{
   return this;
}

int OsNatConnectionSocket::read(char* buffer, int bufferLength)
{
    int iRC ;
    UtlString receivedIp ;
    int iReceivedPort ;

    iRC = OsSocket::read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;
    handleFramedStream(buffer, bufferLength,  receivedIp.data(), iReceivedPort);

    return iRC ;
}

int OsNatConnectionSocket::read(char* buffer, int bufferLength,
       UtlString* ipAddress, int* port)
{
    int iRC ;

    iRC = OsSocket::read(buffer, bufferLength, ipAddress, port) ;       
    handleFramedStream(buffer, iRC, ipAddress->data(), *port);


    return iRC ;
}

int OsNatConnectionSocket::read(char* buffer, int bufferLength,
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

    handleFramedStream(buffer, bufferLength, receivedIp.data(), iReceivedPort);

    return iRC ;
}


int OsNatConnectionSocket::read(char* buffer, int bufferLength, long waitMilliseconds)
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


int OsNatConnectionSocket::write(const char* buffer, int bufferLength)
{
    int framedLength = 0;
    const char* framedBuffer = frameBuffer(STUN /* use the STUN octet*/, buffer, bufferLength, framedLength);
    int written = 0;
    markWriteTime() ;
    written = OsConnectionSocket::write(framedBuffer, framedLength);
    free((void*)framedBuffer);
    return written;
}


int OsNatConnectionSocket::socketWrite(const char* buffer, int bufferLength,
                               const char* ipAddress, int port, PacketType packetType)
{
    TURN_FRAMING_TYPE type = STUN;
    
    // ironically, STUN probes must be sent with the DATA type, not the STUN type,
    // all other requests are written with the STUN (TURN) type.
    if (STUN_PROBE_PACKET == packetType)
    {
        type = DATA;
    }
    int framedLength = 0;
    const char* framedBuffer = frameBuffer(type, buffer, bufferLength, framedLength);
    
    int written = 0;
    if (MEDIA_PACKET == packetType)
    {
        markWriteTime() ;
    }
    written = OsConnectionSocket::write(framedBuffer, framedLength, ipAddress, port) ;
    free((void*)framedBuffer);
    return written;
}

int OsNatConnectionSocket::write(const char* buffer, int bufferLength,
                               const char* ipAddress, int port)
{
    int framedLength = 0;
    const char* framedBuffer = frameBuffer(STUN, buffer, bufferLength, framedLength);
    
    int written = 0;
    markWriteTime() ;
    written = OsConnectionSocket::write(framedBuffer, framedLength, ipAddress, port) ;
    free((void*)framedBuffer);
    return written;
}


int OsNatConnectionSocket::write(const char* buffer, int bufferLength, 
                               long waitMilliseconds)
{
    int framedLength = 0;
    const char* framedBuffer = frameBuffer(STUN, buffer, bufferLength, framedLength);
    
    int written = 0;
    markWriteTime() ;
    written = OsConnectionSocket::write(framedBuffer, framedLength, waitMilliseconds) ;
    free((void*)framedBuffer);
    return written;
}


void OsNatConnectionSocket::enableStun(const char* szStunServer, int stunPort, int iKeepAlive,  int iStunOptions, bool bReadFromSocket) 
{    
    assert(false);
}


void OsNatConnectionSocket::disableStun()
{
    if (mStunState.bEnabled)
    {
        mStunState.bEnabled = false ;
        mpNatAgent->disableStun(this) ;
    }
}


void OsNatConnectionSocket::enableTurn(const char* szTurnServer,
                                     int turnPort,
                                     int iKeepAlive,
                                     const char* username,
                                     const char* password,
                                     bool bReadFromSocket)
{
    if (!mTurnState.bEnabled)
    {
        mTurnState.bEnabled = true ;
        
        if (!isClientConnected(szTurnServer, turnPort))
        {
            //initialize(szTurnServer, turnPort, true) ;
            clientConnect(szTurnServer, turnPort);
        }
    
        UtlBoolean bRC = mpNatAgent->enableTurn(this, szTurnServer, turnPort, iKeepAlive, username, password) ;
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

void OsNatConnectionSocket::disableTurn() 
{
    if (mTurnState.bEnabled)
    {
        mTurnState.bEnabled = false ;   
        mpNatAgent->disableTurn(this) ;
    }
}


void OsNatConnectionSocket::enableTransparentReads(bool bEnable)
{
    mbTransparentReads = bEnable ;
}

UtlBoolean OsNatConnectionSocket::addCrLfKeepAlive(const char* szRemoteIp,
                                                 const int   remotePort, 
                                                 const int   keepAliveSecs,
                                                 OsNatKeepaliveListener* pListener) 
{
    return mpNatAgent->addCrLfKeepAlive(this, szRemoteIp, remotePort, 
            keepAliveSecs, pListener) ;
}


UtlBoolean OsNatConnectionSocket::removeCrLfKeepAlive(const char* szRemoteIp, 
                                                    const int   remotePort) 
{
    return mpNatAgent->removeCrLfKeepAlive(this, szRemoteIp, remotePort) ;
}

UtlBoolean OsNatConnectionSocket::addStunKeepAlive(const char* szRemoteIp, 
                                                 const int   remotePort, 
                                                 const int   keepAliveSecs,
                                                 OsNatKeepaliveListener* pListener) 
{
    return mpNatAgent->addStunKeepAlive(this, szRemoteIp, remotePort, 
            keepAliveSecs, pListener) ;
}


UtlBoolean OsNatConnectionSocket::removeStunKeepAlive(const char* szRemoteIp, 
                                                    const int   remotePort) 
{
    return mpNatAgent->removeStunKeepAlive(this, szRemoteIp, remotePort) ;
}


/* ============================ ACCESSORS ================================= */

// Return the external mapped IP address for this socket.
UtlBoolean OsNatConnectionSocket::getMappedIp(UtlString* ip, int* port) 
{
    UtlBoolean result(false);
    if (mpDatagramSocket)
    {
        mpDatagramSocket->getMappedIp(ip, port);
    }
    return result;
}

// Return the external relay/return IP address for this socket.
UtlBoolean OsNatConnectionSocket::getRelayIp(UtlString* ip, int* port) 
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



void OsNatConnectionSocket::addAlternateDestination(const char* szAddress, int iPort, int priority) 
{
    mpNatAgent->sendStunProbe(this, szAddress, iPort, priority) ;
}


void OsNatConnectionSocket::readyDestination(const char* szAddress, int iPort) 
{
    if (mTurnState.bEnabled && (mTurnState.status == NAT_STATUS_SUCCESS))
    {
        mpNatAgent->primeTurnReception(this, szAddress, iPort) ;
    }
}


void OsNatConnectionSocket::setNotifier(OsNotification* pNotification) 
{
    mpNotification = pNotification ;
    mbNotified = false ;
}



/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void OsNatConnectionSocket::setStunAddress(const UtlString& address, 
                                         const int iPort) 
{
    mStunState.mappedAddress = address ;
    mStunState.mappedPort = iPort ;
}

void OsNatConnectionSocket::setTurnAddress(const UtlString& address, 
                                         const int iPort) 
{
    mTurnState.relayAddress = address ;
    mTurnState.relayPort = iPort ;
}


void OsNatConnectionSocket::markStunSuccess(bool bAddressChanged)
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


void OsNatConnectionSocket::markStunFailure() 
{
    mStunState.status = NAT_STATUS_FAILURE ;

    // Signal external identities interested in the STUN outcome.
    if (mpNotification && !mbNotified)
    {
        mpNotification->signal(0) ;
        mbNotified = true ;
    }
}


void OsNatConnectionSocket::markTurnSuccess() 
{
    mTurnState.status = NAT_STATUS_SUCCESS ;
}


void OsNatConnectionSocket::markTurnFailure() 
{
    mTurnState.status = NAT_STATUS_FAILURE ;
}


void OsNatConnectionSocket::evaluateDestinationAddress(const UtlString& address, 
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


UtlBoolean OsNatConnectionSocket::getBestDestinationAddress(UtlString& address,
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


UtlBoolean OsNatConnectionSocket::applyDestinationAddress(const char* szAddress, int iPort) 
{
    UtlBoolean bRC = false ;

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

const char* OsNatConnectionSocket::frameBuffer(TURN_FRAMING_TYPE type,
                                                    const char* buffer,
                                                    const int bufferLength,
                                                    int& framedBufferLen)
{
   /*
    *   0                   1                   2                   3
    *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    *  |     Type      |  Reserved = 0 |            Length             |
    *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    *
    */
    char* szFramedBuffer = (char*) malloc(MAX_RTP_BYTES + 4);
    
    
    u_short typeByte = 0;
    typeByte = type;
    typeByte = typeByte << 8;
    typeByte = htons((u_short) typeByte);
    memcpy(szFramedBuffer, &typeByte, sizeof(u_short));

    u_short packetLength = htons((u_short) bufferLength);
    memcpy(szFramedBuffer + sizeof(u_short), &packetLength, sizeof(u_short));
    
    memcpy(szFramedBuffer + (2 * sizeof(u_short)), buffer, bufferLength);
    framedBufferLen = bufferLength + (2 * sizeof(u_short));
    return szFramedBuffer;
}

void OsNatConnectionSocket::handleFramedStream(       char* pData,
                                                      const int size,
                                                      const char* receivedIp,
                                                      const int port)
{
    mStreamHandlerMutex.acquire();
    char szStreamChunk [(MAX_RTP_BYTES + 4) * 2];
    
    unsigned int streamBufferSize = size;
    if (size < 1 || size > (MAX_RTP_BYTES+4))
    {
        mStreamHandlerMutex.release();
        return;  // can't handle this
    }

    // first memcpy in any leftover fragments
    if (mFragmentSize < 0 || mFragmentSize > ((MAX_RTP_BYTES + 4) * 2))
    {
        assert(false);
        mFragmentSize = 0;
    }
    if (mFragmentSize)
    {
        memcpy(szStreamChunk, mszFragment, mFragmentSize);
        memcpy(szStreamChunk + mFragmentSize, pData, size);
        streamBufferSize += mFragmentSize;
        mFragmentSize = 0;        
    }
    else
    {
        memcpy(szStreamChunk, pData, size);
    }

    
    u_short packLen;
    memcpy(&packLen, szStreamChunk + sizeof(u_short), sizeof(u_short));
    packLen = ntohs(packLen);
    assert(packLen > 0);
    assert(packLen <= MAX_RTP_BYTES);
    
    if (packLen > MAX_RTP_BYTES)
    {
        OsSysLog::add(FAC_STREAMING, PRI_DEBUG, "OsNatConnectionSocket::handleFramedStream - Received invalid framing header.");
        mStreamHandlerMutex.release();
        return;
    }
    
    TURN_FRAMING_TYPE type;
    char* pStreamBuffer = szStreamChunk;
    if (streamBufferSize >= (packLen + (2*sizeof(u_short))) )
    {
        u_short typeByte = *pStreamBuffer;
        typeByte = ntohs((u_short)typeByte);
        type = (TURN_FRAMING_TYPE)typeByte;
        
        handleUnframedBuffer(type, pStreamBuffer + (2*sizeof(u_short)), packLen, receivedIp, port);

        streamBufferSize = streamBufferSize - packLen - (2*sizeof(u_short));
        pStreamBuffer = pStreamBuffer + (2*sizeof(u_short)) + packLen;        

        if (streamBufferSize)
        {
            handleFramedStream(pStreamBuffer, streamBufferSize, receivedIp, port);
        }
    }
    else
    {
        mFragmentSize = streamBufferSize;
        if (mFragmentSize)
        {
            memcpy(mszFragment, pStreamBuffer, mFragmentSize);
        }
      
    }
    mStreamHandlerMutex.release();
    return;    
}                      

 bool OsNatConnectionSocket::handleUnframedBuffer(const TURN_FRAMING_TYPE type,
                                        const char* buff,
                                        const int buffSize,
                                        const char* receivedIp,
                                        const int port)
{
    bool bNatPacket = false;
    
    int iRC = buffSize;
    int receivedPort ;
    UtlString sReceivedIp(receivedIp);
    bool bHandled = handleSturnData((char*)buff, iRC, sReceivedIp, receivedPort);
    if (iRC)
    {
        if (!mbTransparentReads)
            iRC = 0 ;
        else
            bNatPacket = TRUE ;
    }
    
    // Make read time for non-NAT packets
    if (iRC > 0 && !bNatPacket)
    {
        markReadTime() ;
    }
    
    return bHandled;
}                                    

void OsNatConnectionSocket::addClientConnection(const char* ipAddress, const int port, OsNatConnectionSocket* pClient)
{
    char szPort[16];
    sprintf(szPort, "%d", port);
    UtlString* key = new UtlString();;
    *key = UtlString(ipAddress) + UtlString(":") + UtlString(szPort);
    UtlVoidPtr* container = new UtlVoidPtr(pClient);
    mClientConnectionSockets.insertKeyAndValue(key, container);

}

int OsNatConnectionSocket::clientConnect(const char* szServer, const int port)
{
    int iRet = -1;
    OsNatConnectionSocket* pClient = getClientConnection(szServer, port);
    if (pClient && !pClient->isConnected())
    {
        iRet = pClient->connect();
    }

    return iRet;
}

bool OsNatConnectionSocket::isClientConnected(const char* szServer, const int port)
{
    UtlBoolean bRet = false;
    OsNatConnectionSocket* pClient = getClientConnection(szServer, port);
    if (pClient)
    {
        bRet = pClient->isConnected();
    }
    return bRet==TRUE;
}

OsNatConnectionSocket* OsNatConnectionSocket::getClientConnection(const char* szServer, const int port)
{
    OsNatConnectionSocket* pClient = NULL;
    
    UtlVoidPtr* pSocketContainer = NULL;
    UtlString key(szServer);
    key += ":";
    char szPort[16];
    sprintf(szPort, "%d", port);
    key += szPort;
    pSocketContainer = (UtlVoidPtr*)mClientConnectionSockets.findValue(&key);
    if (pSocketContainer)
    {
        pClient = (OsNatConnectionSocket*)pSocketContainer->getValue();
    }
    
    return pClient;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

/* ///////////////////////// HELPER CLASSES /////////////////////////////// */
