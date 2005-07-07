// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include "os/OsStunDatagramSocket.h"
#include "os/OsLock.h"
#include "os/OsStunQueryAgent.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

// FORWARD DECLARATIONS
void StunMsgQFlushHookPtr(const OsMsg& rMsg) ;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsStunDatagramSocket::OsStunDatagramSocket(int remoteHostPortNum,
                                           const char* remoteHost, 
                                           int localHostPortNum, 
                                           const char* localHost,
                                           bool bEnable, 
                                           const char* szStunServer,
                                           int iRefreshPeriodInSec) 
        : OsDatagramSocket(remoteHostPortNum, remoteHost, localHostPortNum, localHost)
        , mShutdownMutex(OsMutex::Q_FIFO)
        , mbEnabled(bEnable)
        , mStunServer(szStunServer)
        , mStunRefreshErrors(0)
{    
    mStunPort = 0 ;
    mStunAddress = "" ;

    // flush hook used to free memory in stun packets
    mStunMsgQ.setFlushHook(StunMsgQFlushHookPtr) ;

    mpRefreshEvent = new StunRefreshEvent(this) ;
    mpTimer = new OsTimer(*mpRefreshEvent) ;

    // If enabled, kick off first stun request
    if (mbEnabled)
    {
        refreshStunBinding(TRUE) ;
    }

    // If valid refresh period, set timer
    if (iRefreshPeriodInSec > 0)
    {
        setKeepAlivePeriod(iRefreshPeriodInSec) ;
    }
}


// Destructor
OsStunDatagramSocket::~OsStunDatagramSocket()
{
    enableStun(FALSE) ;    

    // Use mShutdownMutex to guard against deleting the timer/refresh event 
    // while they are running (very possible).
    mShutdownMutex.acquire() ;
    delete mpTimer ;
    delete mpRefreshEvent ;
    mShutdownMutex.release() ;
}

/* ============================ MANIPULATORS ============================== */


int OsStunDatagramSocket::read(char* buffer, int bufferLength)
{
    bool bStunPacket = FALSE ;
    int iRC ;

    do
    {
        iRC = OsDatagramSocket::read(buffer, bufferLength) ;

        // Look for stun packet
        if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {            
            // Make copy and queue it. 
            char* szCopy = (char*) malloc(iRC) ;
            if (szCopy)
            {
                memcpy(szCopy, buffer, iRC) ;
                StunMsg msg(szCopy, iRC);
                mStunMsgQ.send(msg) ;
            }
        }

    } while ((iRC >= 0) && bStunPacket) ;

    return iRC ;
}

int OsStunDatagramSocket::read(char* buffer, int bufferLength,
       UtlString* ipAddress, int* port)
{
    bool bStunPacket = FALSE ;
    int iRC ;

    do
    {
        iRC = OsSocket::read(buffer, bufferLength, ipAddress, port) ;

        // Look for stun packet
        if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {            
            // Make copy and queue it. 
            char* szCopy = (char*) malloc(iRC) ;
            if (szCopy)
            {
                memcpy(szCopy, buffer, iRC) ;
                StunMsg msg(szCopy, iRC);
                mStunMsgQ.send(msg) ;
            }
        }

    } while ((iRC >= 0) && bStunPacket) ;

    return iRC ;
}

int OsStunDatagramSocket::read(char* buffer, int bufferLength,
       struct in_addr* ipAddress, int* port)
{
    bool bStunPacket = FALSE ;
    int iRC ;

    do
    {
        iRC = OsSocket::read(buffer, bufferLength, ipAddress, port) ;

        // Look for stun packet
        if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {            
            // Make copy and queue it. 
            char* szCopy = (char*) malloc(iRC) ;
            if (szCopy)
            {
                memcpy(szCopy, buffer, iRC) ;
                StunMsg msg(szCopy, iRC);
                mStunMsgQ.send(msg) ;
            }
        }

    } while ((iRC >= 0) && bStunPacket) ;

    return iRC ;
}

int OsStunDatagramSocket::read(char* buffer, int bufferLength, long waitMilliseconds)
{
    bool bStunPacket = FALSE ;
    int iRC ;

    do
    {
        iRC = OsSocket::read(buffer, bufferLength, waitMilliseconds) ;

        // Look for stun packet
        if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {            
            // Make copy and queue it. 
            char* szCopy = (char*) malloc(iRC) ;
            if (szCopy)
            {
                memcpy(szCopy, buffer, iRC) ;
                StunMsg msg(szCopy, iRC);
                mStunMsgQ.send(msg) ;
            }
        }

    } while ((iRC >= 0) && bStunPacket) ;

    return iRC ;
}



void OsStunDatagramSocket::setKeepAlivePeriod(int secs) 
{
    mKeepAlivePeriod = secs ;
    mpTimer->stop() ;

    if ((mbEnabled) && secs > 0)
    {
        mpTimer->periodicEvery(OsTime(secs, 0), OsTime(secs, 0)) ;
    }
}


void OsStunDatagramSocket::setStunServer(const char* szHostname) 
{
    mStunServer = szHostname ;
}


void OsStunDatagramSocket::enableStun(bool bEnable) 
{
    mbEnabled = bEnable ;
    if (mbEnabled)
    {
        refreshStunBinding(FALSE) ;
    }
    else
    {
        mStunMsgQ.flush() ;
    }
    setKeepAlivePeriod(mKeepAlivePeriod) ;    
}


void OsStunDatagramSocket::refreshStunBinding(UtlBoolean bFromReadSocket)
{
    mShutdownMutex.acquire() ;

    mStunMsgQ.flush() ;

    bool bSuccess = FALSE ;
    OsStunQueryAgent agent;
    if (agent.setServer(mStunServer))
    {
        OsTime timeout(0, STUN_TIMEOUT_RESPONSE_MS * OsTime::USECS_PER_MSEC) ;
        

        if (bFromReadSocket)
        {
            if (mStunMsgQ.numMsgs() > 0)
            {
                // We can rely on someone else to pump messages by calling read.
                bSuccess = agent.getMappedAddressSharedSocket(this, mStunAddress, mStunPort, timeout) ;
            }
            else
            {
                // We must touch the socket and look for the next stun packet.
                bSuccess = agent.getMappedAddress(this, mStunAddress, mStunPort, timeout) ;
            }
        }
        else
        {
            // We can rely on someone else to pump messages by calling read.
            bSuccess = agent.getMappedAddressSharedSocket(this, mStunAddress, mStunPort, timeout) ;
        }
    }
    else
    {
        bool bSuccess = FALSE ;
    }

    // Report/Mark errors
    if (!bSuccess)
    {
        mStunRefreshErrors++ ;

        if ((mStunRefreshErrors == STUN_INITIAL_REFRESH_REPORT_THRESHOLD) || 
                (mStunRefreshErrors % STUN_REFRESH_REPORT_THRESHOLD) == 0)
        {
            OsSysLog::add(FAC_NET, PRI_WARNING, 
                    "STUN failed to obtain binding from %s (attempt=%d)\n",
                    mStunServer.data(), mStunRefreshErrors) ;
        }
        else if (mStunRefreshErrors >= STUN_ABORT_THRESHOLD)
        {
            // Shutdown if we never received a valid address
            if (mStunServer.length() > 0)
            {
                OsSysLog::add(FAC_NET, PRI_ERR, 
                    "STUN Aborted; Failed to obtain stun binding from %s (attempt=%d)\n",
                    mStunServer.data(), mStunRefreshErrors) ;
                enableStun(FALSE) ;
            }
        }
    }
    else
    {
        mStunRefreshErrors = 0 ;
    }

    mShutdownMutex.release() ;
}


/* ============================ ACCESSORS ================================= */

// Return the external IP address for this socket.
UtlBoolean OsStunDatagramSocket::getExternalIp(UtlString* ip, int* port) 
{
    UtlBoolean bSuccess = false ;

    if (mStunAddress.length() && mbEnabled) 
    {
        if (ip)
        {
            *ip = mStunAddress ;
        }

        if (port)
        {
            *port = mStunPort ;
        }

        bSuccess = true ;
    }

    return bSuccess ;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

int OsStunDatagramSocket::readStunPacket(char* buffer, int bufferLength, const OsTime& rTimeout) 
{        
    int iRead = 0 ;
    OsMsg* msg;
    if (mStunMsgQ.receive(msg, rTimeout) == OS_SUCCESS)
    {
        StunMsg* pStunMsg = (StunMsg*) msg ;
        char* cStunBuffer = pStunMsg->getBuffer() ;
        int iStunLength = pStunMsg->getLength() ;

        if (iStunLength > bufferLength)
        {
            iStunLength = bufferLength ;
        }

        memcpy(buffer, cStunBuffer, iStunLength) ;
        StunMsgQFlushHookPtr(*pStunMsg) ;
        pStunMsg->releaseMsg() ;

        iRead = iStunLength ;
    }

    return iRead ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

void StunMsgQFlushHookPtr(const OsMsg& rMsg)
{
    char* szBuffer = ((StunMsg&) rMsg).getBuffer() ;
    free(szBuffer) ;
}


/* ///////////////////////// HELPER CLASSES /////////////////////////////// */


StunRefreshEvent::StunRefreshEvent(OsStunDatagramSocket* pSocket) 
{
    mpSocket = pSocket ;
}


StunRefreshEvent::~StunRefreshEvent()
{

}


OsStatus StunRefreshEvent::signal(const int eventData) 
{
    // This should probably NOT block the timer thread context.
    //   http://track.sipfoundry.org/browse/XPL-97

    mpSocket->refreshStunBinding(FALSE) ;
 
    return OS_SUCCESS ;
}


StunMsg::StunMsg(char* szBuffer, int nLength)
    : OsMsg(OsMsg::USER_START, 0)
{
    mBuffer = szBuffer ;    // Shallow copy
    mLength = nLength ;
}


StunMsg::StunMsg(const StunMsg& rStunMsg)
    : OsMsg(OsMsg::USER_START, 0)
{
    mBuffer = rStunMsg.mBuffer ;
    mLength  = rStunMsg.mLength ;
}


OsMsg* StunMsg::createCopy(void) const
{
    return new StunMsg(*this);
}


StunMsg::~StunMsg()
{

}


StunMsg& StunMsg::operator=(const StunMsg& rhs)
{
    if (this != &rhs)            // handle the assignment to self case
    {
        mBuffer = rhs.mBuffer ;
        mLength = rhs.mLength ;
    }

    return *this ;
}


char* StunMsg::getBuffer()
{
    return mBuffer ;
}

   
int StunMsg::getLength() 
{
    return mLength ;
}