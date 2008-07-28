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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
#include "os/OsDateTime.h"
#include "os/OsTime.h"
#include "tapi/sipXtapi.h"
#include "utl/UtlHashMapIterator.h"


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
                                         OsNotification *pStunNotification) 
        : OsDatagramSocket(remoteHostPortNum, remoteHost, localHostPortNum, localHost)
{
    mDestAddress = mRemoteIpAddress ;
    miDestPort = remoteHostPort ;
    setStunNotifier(pStunNotification) ;

    mRateLimit = 0 ;
    mCurrentRate = 0.0 ;
    mIgnorePeriodSecs = 0 ;
}


// Destructor
OsNatDatagramSocket::~OsNatDatagramSocket()
{
    destroy() ;
    mRateLimitedMap.destroyAll() ;
}

void OsNatDatagramSocket::destroy()
{
    OsNatSocketBaseImpl::destroy() ;
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
        if (shouldRateLimit(&receivedIp, &iReceivedPort))
            iRC = 0 ;
        else if (handleSturnData(buffer, iRC, receivedIp, iReceivedPort))
        {
            if (!getTransparentStunRead())
                iRC = 0 ;
            else
                bNatPacket = TRUE ;
        }
    } while ((iRC >= 0) && bNatPacket) ;

    // Make read time for non-NAT packets
    if (iRC > 0 && !bNatPacket)
    {
        markReadTime() ;
        checkDelayedShutdown() ;
    }

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
        if (shouldRateLimit(&receivedIp, &iReceivedPort))
            iRC = 0 ;
        else if (handleSturnData(buffer, iRC, receivedIp, iReceivedPort))
        {
            if (!getTransparentStunRead())
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
        checkDelayedShutdown() ;
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
            if (shouldRateLimit(&receivedIp, &iReceivedPort))
                iRC = 0 ;
            else if (handleSturnData(buffer, iRC, receivedIp, iReceivedPort))
            {
                if (!getTransparentStunRead())
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
        checkDelayedShutdown() ;
    }

    return iRC ;    
}

int OsNatDatagramSocket::socketWrite(const char* buffer, int bufferLength,
                               const char* ipAddress, int port, OS_NAT_PACKET_TYPE packetType)
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


void OsNatDatagramSocket::setReadRateLimiting(int readsPerSec, int ignorePeriodSecs) 
{
    assert(readsPerSec >= 0) ;
    assert(ignorePeriodSecs >= 0) ;

    mRateLimit = readsPerSec ;
    mIgnorePeriodSecs = ignorePeriodSecs ;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

double OsNatDatagramSocket::calculateRate(unsigned long delta, double oldRate) 
{
    double rate = oldRate ;

    if (delta >= 1000)
        rate = 1.0 ;        
    else if (delta == 0)
        rate += 1.0 ;
    else
        rate = (((1000.0-delta)/1000.0) * mCurrentRate) + 1.0 ;

    return rate ;
}


bool OsNatDatagramSocket::checkTotalRateLimit() 
{
    static unsigned long lastCheck = 0 ;

    OsTime now ;
    OsDateTime::getCurTime(now) ;
    unsigned long msNow = now.cvtToMsecs() ;
    unsigned long delta = msNow - lastCheck ;    

    // If first time, assume first packet in over a period
    if (lastCheck == 0)
        delta = 1000 ;
    lastCheck = msNow ;

    // Update the rate limit
    mCurrentRate = calculateRate(delta, mCurrentRate) ;

    return (mCurrentRate >= (double) mRateLimit) ;
}


bool OsNatDatagramSocket::checkHostRateLimit(UtlString* pIP)
{
    UtlContainable* pValue ;
    bool bRateLimit = false ;
    OsTime now ;    
    OsDateTime::getCurTime(now) ;

    // Release clients from Penalty box
    UtlHashMapIterator itor(mRateLimitedMap) ;
    UtlContainable* pKey ;
    while ((pKey = itor()) != NULL)
    {
        pValue = mRateLimitedMap.findValue(pKey) ;
        if (pValue)
        {
            OsTime* pExpireAt = (OsTime*) pValue ;
            if (now > *pExpireAt) 
            {
                OsSysLog::add(FAC_NET, PRI_NOTICE, "Releasing %s from rate limit (%lf)",
                        ((UtlString*)pKey)->data(), mCurrentRate) ;

                mRateLimitedMap.destroy(pKey) ;
            }
        }
    }

    // Determine if this client is still in penalty, if so increase length
    pValue = mRateLimitedMap.findValue(pIP) ;
    if (pValue != NULL)
    {
        OsTime isolationPeriod(mIgnorePeriodSecs, 0) ;
        OsTime* pExpireAt = (OsTime*) pValue ;
        *pExpireAt = now + isolationPeriod ;
        bRateLimit = true ;
    }

    return bRateLimit ;
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

bool OsNatDatagramSocket::shouldRateLimit(UtlString* pIP, int* pPort)
{   
    bool bRateLimit = false ;

    assert(pIP != NULL) ;
    assert(pPort != NULL) ;

    if (mRateLimit > 0 && pIP && pPort)
    {
        if (checkHostRateLimit(pIP))
        {
            bRateLimit = true ;
        }
        else if (checkTotalRateLimit())
        {
            OsTime now ;
            OsDateTime::getCurTime(now) ;
            OsTime isolationPeriod(mIgnorePeriodSecs, 0) ;
            OsTime* pExpireAt = new OsTime() ;
            *pExpireAt = now + isolationPeriod ;

            OsSysLog::add(FAC_NET, PRI_NOTICE, "Selected %s:%d for rate limiting (%f >= %d), penalty=%d",
                    pIP->data(), *pPort, mCurrentRate, mRateLimit, mIgnorePeriodSecs) ;
            
            mRateLimitedMap.insertKeyAndValue(new UtlString(pIP->data()), pExpireAt) ;

            bRateLimit = true ;
            if (mCurrentRate > 1)
                mCurrentRate -= 1.0 ;
        }
        else
        {
            bRateLimit = false ;
        }
    }

    return bRateLimit ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
