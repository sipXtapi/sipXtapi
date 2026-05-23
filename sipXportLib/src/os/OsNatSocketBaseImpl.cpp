// 
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
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
#include "os/OsIntTypes.h"
#include <assert.h>
#include <stdio.h>
#ifndef _WIN32
#include <netinet/in.h>
#endif

// APPLICATION INCLUDES
#include "os/OsNatSocketBaseImpl.h"
#include "os/OsNatDatagramSocket.h"
#include "os/OsNatAgentTask.h"
#include "os/StunMessage.h"
#include "os/StunUtils.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "os/OsEvent.h"
#include "tapi/sipXtapi.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

// FORWARD DECLARATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsNatSocketBaseImpl::OsNatSocketBaseImpl() :
      mReadNotificationLock(OsMutex::Q_FIFO),
      mIceCredsLock(OsMutex::Q_FIFO),
      mbIceCredsSet(false),
      mpIceNominationCallback(NULL),
      mpIceNominationUserData(NULL),
      mbIceNominationFired(false)
{
    miRecordTimes = ONDS_MARK_NONE ;
    mpReadNotification = NULL ;
    
}

// Constructor
OsNatSocketBaseImpl::~OsNatSocketBaseImpl()
{

}

bool OsNatSocketBaseImpl::getFirstReadTime(OsDateTime& time) 
{
    bool bRC = (miRecordTimes & ONDS_MARK_FIRST_READ) == 
            ONDS_MARK_FIRST_READ ;

    if (bRC)
    {
        time = mFirstRead ;
    }

    return bRC ;
}


bool OsNatSocketBaseImpl::getLastReadTime(OsDateTime& time)
{
    bool bRC = (miRecordTimes & ONDS_MARK_LAST_READ) == 
            ONDS_MARK_LAST_READ ;

    if (bRC)
    {
        time = mLastRead ;
    }

    return bRC ;
}


bool OsNatSocketBaseImpl::getFirstWriteTime(OsDateTime& time) 
{
    bool bRC = (miRecordTimes & ONDS_MARK_FIRST_WRITE) == 
            ONDS_MARK_FIRST_WRITE ;

    if (bRC)
    {
        time = mFirstWrite ;
    }

    return bRC ;
}


bool OsNatSocketBaseImpl::getLastWriteTime(OsDateTime& time) 
{
    bool bRC = (miRecordTimes & ONDS_MARK_LAST_WRITE) == 
            ONDS_MARK_LAST_WRITE ;

    if (bRC)
    {
        time = mLastWrite ;
    }

    return bRC ;
}

void OsNatSocketBaseImpl::setReadNotification(OsNotification* pNotification) 
{
    OsLock lock(mReadNotificationLock) ;

    mpReadNotification = pNotification ;
}


void OsNatSocketBaseImpl::setIceCredentials(const UtlString& localUfrag,
                                            const UtlString& localPwd,
                                            const UtlString& remoteUfrag,
                                            const UtlString& remotePwd)
{
    OsLock lock(mIceCredsLock) ;

    mIceLocalUfrag  = localUfrag ;
    mIceLocalPwd    = localPwd ;
    mIceRemoteUfrag = remoteUfrag ;
    mIceRemotePwd   = remotePwd ;

    // Treat as "set" only when all required fields are non-empty.
    // Empty localUfrag, remoteUfrag or localPwd would render integrity
    // validation and username validation meaningless; we'd rather fall
    // back to legacy mode in that case.
    mbIceCredsSet = !localUfrag.isNull() && !localPwd.isNull() && !remoteUfrag.isNull() ;
}


bool OsNatSocketBaseImpl::getIceCredentials(UtlString& localUfrag,
                                            UtlString& localPwd,
                                            UtlString& remoteUfrag,
                                            UtlString& remotePwd)
{
    OsLock lock(mIceCredsLock) ;

    if (!mbIceCredsSet)
    {
        return false ;
    }

    localUfrag  = mIceLocalUfrag ;
    localPwd    = mIceLocalPwd ;
    remoteUfrag = mIceRemoteUfrag ;
    remotePwd   = mIceRemotePwd ;
    return true ;
}


bool OsNatSocketBaseImpl::hasIceCredentials()
{
    OsLock lock(mIceCredsLock) ;
    return mbIceCredsSet ;
}


void OsNatSocketBaseImpl::setIceNominationCallback(
        IStunSocket::IceNominationCallback callback,
        void*                              userData)
{
    OsLock lock(mIceCredsLock) ;

    mpIceNominationCallback = callback ;
    mpIceNominationUserData = userData ;
    mbIceNominationFired    = false ;   // reset on reconfigure
}


void OsNatSocketBaseImpl::fireIceNomination(const UtlString& remoteIp, int remotePort)
{
    IceNominationCallback cb ;
    void*                 ud ;

    {
        OsLock lock(mIceCredsLock) ;

        // One-shot: ignore subsequent USE-CANDIDATE packets for this session.
        if (mbIceNominationFired || mpIceNominationCallback == NULL)
        {
            return ;
        }
        mbIceNominationFired = true ;
        cb = mpIceNominationCallback ;
        ud = mpIceNominationUserData ;
    }

    // Invoke outside the lock — the callback may call back into sipX
    // layers that acquire their own locks.
    OsSysLog::add(FAC_NET, PRI_INFO,
        "OsNatSocketBaseImpl::fireIceNomination: ICE candidate nominated: %s:%d",
        remoteIp.data(), remotePort) ;

    cb(remoteIp.data(), remotePort, ud) ;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

void OsNatSocketBaseImpl::markReadTime()
{
    // Always mark last read
    miRecordTimes |= ONDS_MARK_LAST_READ ;
    OsDateTime::getCurTime(mLastRead) ;

    // Mark first read if not already set
    if ((miRecordTimes & ONDS_MARK_FIRST_READ) == 0)
    {
        miRecordTimes |= ONDS_MARK_FIRST_READ ;
        mFirstRead = mLastRead ;
    }

    OsLock lock(mReadNotificationLock) ;
    if (mpReadNotification)
    {
        mpReadNotification->signal((intptr_t) this) ;
        mpReadNotification = NULL ;
    }
}

void OsNatSocketBaseImpl::markWriteTime()
{
    // Always mark last write
    miRecordTimes |= ONDS_MARK_LAST_WRITE ;
    OsDateTime::getCurTime(mLastWrite) ;

    // Mark first write if not already set
    if ((miRecordTimes & ONDS_MARK_FIRST_WRITE) == 0)
    {
            miRecordTimes |= ONDS_MARK_FIRST_WRITE ;
            mFirstWrite = mLastWrite ;
    }
}

void OsNatSocketBaseImpl::handleStunMessage(char* pBuf, 
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
        OsNatAgentTask::getInstance()->postMessage(msg) ;
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
        memcpy(szCopy, pBuf, length) ;
        NatMsg msg(NatMsg::TURN_MESSAGE, szCopy, length, this, fromAddress, fromPort);
        OsNatAgentTask::getInstance()->postMessage(msg) ;
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
        char*    pData ;
        uint16_t nData ;

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
                    char     remoteAddr[32] ;
                    uint16_t remotePort ;
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
                                          int&       receivedPort)
{
    bool bHandled = false ;
    bool bDataIndication = false ;

    if ((bufferLength > 0) && TurnMessage::isTurnMessage(buffer, bufferLength, &bDataIndication))
    {
        if (bDataIndication)
        {
            bufferLength = handleTurnDataIndication(buffer, bufferLength, 
                    &receivedIp, &receivedPort) ;

            // We need to recurse for DIs -- it may be an encapsulated STUN 
            // message (e.g. ICE)
            return handleSturnData(buffer, bufferLength, receivedIp, receivedPort) ;
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
        handleStunMessage(buffer, bufferLength, receivedIp, receivedPort) ;
        bHandled = true ;
    }

    return bHandled ;
}



/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
