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
      mReadNotificationLock(OsMutex::Q_FIFO)
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
        mpReadNotification->signal((int) this) ;
        mpReadNotification = NULL ;
    }
}

OsSocket* OsNatSocketBaseImpl::getSocket()
{
    OsSocket* pSocket = dynamic_cast<OsSocket*>(this);
    assert(pSocket);
    return pSocket;
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
        char*          pData ;
        unsigned short nData ;

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
