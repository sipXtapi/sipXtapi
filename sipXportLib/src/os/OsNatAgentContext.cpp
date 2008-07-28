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
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
//

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsNatAgentContext.h"
#include "os/OsNatAgentTask.h"
#include "os/OsQueuedEvent.h"
#include "os/OsTimer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* scBindingTypes[] =
{
    "INVALID_BINDING_TYPE",
    "STUN_DISCOVERY",
    "STUN_PROBE",
    "CRLF_KEEPALIVE",
    "STUN_KEEPALIVE"
    "TURN_ALLOCATION",
    "TURN_SEND_REQUEST",
    "TURN_SET_ACTIVE_DEST",
    "NAT_CLASSIFICATION"
} ;

// MACROS
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

//////////////////////////////////////////////////////////////////////////////


OsNatAgentContext::OsNatAgentContext(OsNatAgentTask* pAgentTask)
{
    mpAgentTask = pAgentTask ;

    type = INVALID_BINDING_TYPE ;
    status = SUCCESS ;
    serverPort = PORT_NONE ;
    relayPort = PORT_NONE ;
    options = 0 ;
    memset(&transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;
    pSocket = NULL ;
    pErrorTimer = NULL ;
    pRefreshTimer = NULL ;
    keepAliveSecs = 0 ;
    abortCount = 0 ;
    refreshErrors = 0 ;
    errorTimeoutMSec = 0 ;
    port = PORT_NONE ;
    priority = 0 ; 
    pKeepaliveListener = NULL ;
    lStartTimestamp = 0 ;
    lEndTimestamp = 0 ;        
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentContext::~OsNatAgentContext()
{

}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentContext::markFailure(NAT_FAILURE_CAUSE cause) 
{
    assert(pErrorTimer != NULL) ;
    assert(pRefreshTimer != NULL) ;
    assert(pSocket != NULL) ;

    lEndTimestamp = OsDateTime::getCurTimeInMS() ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "Stun failure #%d for %s:%d (%s) -> %s:%d",
                cause,
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                scBindingTypes[type],
                serverAddress.data(),
                serverPort) ;

#if NAT_DEBUG >= 1
    printf("Stun failure for %s:%d (%s) -> %s:%d\n",
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                scBindingTypes[type],
                serverAddress.data(),
                serverPort) ;
#endif
    
    status = FAILED ;

    // Clear timers
    if (pErrorTimer->getState() == OsTimer::STARTED)
        pErrorTimer->stop() ;
    if (pRefreshTimer->getState() == OsTimer::STARTED)
        pRefreshTimer->stop() ;
    mpAgentTask->purgeTimers(pErrorTimer, this) ;
    mpAgentTask->purgeTimers(pRefreshTimer, this) ;               

    // Notify Socket
    if (type == STUN_DISCOVERY)
    {
        UtlString empty ;
        pSocket->setStunAddress(empty, PORT_NONE) ;
        pSocket->markStunFailure() ;   
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentContext::markSuccess(const UtlString& mappedAddress, 
                                    int mappedPort, 
                                    const UtlString& receivedIp, 
                                    int receivedPort,
                                    const UtlString& changeIp, 
                                    int changePort)
{
    bool bAddressChanged = false ;
    OsTime refreshPeriod(keepAliveSecs, 0) ;

    assert(pErrorTimer != NULL) ;
    assert(pRefreshTimer != NULL) ;
    assert(pSocket != NULL) ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "Stun success for %s:%d (%s), mapped address: %s:%d (from %s:%d)",
            localHostIp.data(),
            pSocket->getSocket()->getLocalHostPort(),
            scBindingTypes[type],
            mappedAddress.data(),
            mappedPort,
            receivedIp.data(),
            receivedPort) ;

    // Check for a change in mapped IP/PORT
    if (    (status == RESENDING) || 
            (status == RESENDING_ERROR)   )
    {
        if (    (mappedAddress.compareTo(address) != 0) ||
                (mappedPort != port)  )
        {
            // This is really a critial problem -- we are likely not able
            // to communicate if our NAT binding is bouncing around.
            OsSysLog::add(FAC_NAT, PRI_CRIT, 
                    "Stun binding changed for %s:%d, %s:%d is now %s:%d",
                    localHostIp.data(),
                    pSocket->getSocket()->getLocalHostPort(),
                    address.data(),
                    port,
                    mappedAddress.data(),
                    mappedPort) ;

            bAddressChanged = true ;
        }
    }

    // Update State
    address = mappedAddress ;
    port = mappedPort ;
    status = SUCCESS ;
    refreshErrors = 0 ;

    if (type == STUN_PROBE)
        errorTimeoutMSec = MAX(OsNatAgentTask::sStunTimeoutMS, OsNatAgentTask::sTurnTimeoutMS) ;
    else
        errorTimeoutMSec = OsNatAgentTask::sStunTimeoutMS ;

    lEndTimestamp = OsDateTime::getCurTimeInMS() ;
    memset(&transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;

    // Reset Timers
    if (pErrorTimer->getState() == OsTimer::STARTED)
        pErrorTimer->stop() ;
    mpAgentTask->purgeTimers(pErrorTimer, this) ;
    if (pRefreshTimer->getState() == OsTimer::STARTED)
        pRefreshTimer->stop() ;
    mpAgentTask->purgeTimers(pRefreshTimer, this) ;

    if (keepAliveSecs > 0)
    {
        pRefreshTimer->periodicEvery(refreshPeriod, refreshPeriod) ;          
    }

    // Notify Socket
    if (type == STUN_DISCOVERY)
    {
        pSocket->setStunAddress(mappedAddress, mappedPort) ;
        pSocket->markStunSuccess(bAddressChanged) ;
        mpAgentTask->onStunRTT(lEndTimestamp - lStartTimestamp) ;
    } 
    else if (type == STUN_PROBE)
    {
        bool bViaOurRelay = (relayAddress.length() > 0) ;
        
        // Fire back a stun message to the originating to help w/ NAT 
        // bindings.  This is needed when using our TURN relay
        if (!bViaOurRelay)
        {
            mpAgentTask->addStunKeepAlive(pSocket, receivedIp, receivedPort, 0, NULL) ;
        }

        // Inform the socket of 'good' match.
        pSocket->evaluateDestinationAddress(serverAddress,
                serverPort, priority, bViaOurRelay) ;
    }
    else if (type == STUN_KEEPALIVE)
    {
        if (pKeepaliveListener)
        {
            pKeepaliveListener->OnKeepaliveFeedback(
                    mpAgentTask->populateKeepaliveEvent(this)); 
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentContext::markTimeout()
{
    switch (type)
    {
        case STUN_DISCOVERY:
        case STUN_PROBE:
            handleStunTimeout() ;
            break;
        case CRLF_KEEPALIVE:
            handleCrLfKeepaliveTimeout() ;
            break;
        case STUN_KEEPALIVE:
            handleStunKeepaliveTimeout() ;
            break;
        default:
            assert(false);
            break ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentContext::handleStunTimeout() 
{
    switch (status)
    {
        case SUCCESS:
            // Send keep-alive, set state to resending
            lStartTimestamp = OsDateTime::getCurTimeInMS() ;
            status = RESENDING ;
            if (!mpAgentTask->sendStunRequest(this, true))
                markFailure(NAT_FAILURE_NETWORK) ;
            break ;
        case SENDING:
            // Migrate to sending_error and resend                
            status = SENDING_ERROR ;
            refreshErrors++ ;
            if (!mpAgentTask->sendStunRequest(this, false))
                markFailure(NAT_FAILURE_NETWORK) ;
            break ;
        case SENDING_ERROR:
            // If we have hit the max resends, fail otherwise resend
            refreshErrors++ ;
            if (refreshErrors < abortCount)
            {
                if (!mpAgentTask->sendStunRequest(this, false))
                    markFailure(NAT_FAILURE_NETWORK) ;
            } 
            else
            {
                markFailure(NAT_FAILURE_TIMEOUT) ;
            }
            break ;
        case RESENDING:
            // Migrate to resend_error and resend
            status = RESENDING_ERROR ;
            if (!mpAgentTask->sendStunRequest(this, false))
                markFailure(NAT_FAILURE_NETWORK) ;
            
            break ;
        case RESENDING_ERROR:                
            refreshErrors++ ;
            if (refreshErrors < NAT_RESEND_ABORT_COUNT)
            {                    
                if (!mpAgentTask->sendStunRequest(this, false))
                    markFailure(NAT_FAILURE_NETWORK) ;
            }
            else
            {
                markFailure(NAT_FAILURE_TIMEOUT) ;
            }
            break ;
        case FAILED:
            // Nothing to do
            break ;
    }
}


//////////////////////////////////////////////////////////////////////////////

#define STR_CRLF  "\r\n\0"
bool OsNatAgentContext::handleCrLfKeepaliveTimeout() 
{
    bool bRC = false ;
   
    // just calling the socketWrite function, which does not do timestamping
    if (pSocket->socketWrite(STR_CRLF, 3, serverAddress, serverPort, 
            CRLF_KEEPALIVE_PACKET) == 3)
    {
        bRC = true ;
    }
    else
    {
        if (pKeepaliveListener)
        {
            pKeepaliveListener->OnKeepaliveFailure(
                    mpAgentTask->populateKeepaliveEvent(this)); 
        }
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool OsNatAgentContext::handleStunKeepaliveTimeout() 
{
    UtlBoolean bRC = false ;

    if (mpAgentTask->sendStunRequest(this, true))
    {
        bRC = true ;
    }
    else
    {
        if (pKeepaliveListener)
        {
            pKeepaliveListener->OnKeepaliveFailure(
                    mpAgentTask->populateKeepaliveEvent(this)); 
        }
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentTurnContext::OsNatAgentTurnContext(OsNatAgentTask* pAgentTask)
    : OsNatAgentContext(pAgentTask)
{
    pPayload = NULL ;
    nPayload = 0 ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentTurnContext::~OsNatAgentTurnContext()
{
    if (pPayload)
        free(pPayload) ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTurnContext::markFailure(NAT_FAILURE_CAUSE cause) 
{
    assert(pErrorTimer != NULL) ;
    assert(pRefreshTimer != NULL) ;
    assert(pSocket != NULL) ;        

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "Turn failure #%d for %s:%d -> %s:%d",
                cause,
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                serverAddress.data(),
                serverPort) ;

    status = FAILED ;
    lEndTimestamp = OsDateTime::getCurTimeInMS() ;

    // Clear timers
    if (pErrorTimer->getState() == OsTimer::STARTED)
        pErrorTimer->stop() ;
    mpAgentTask->purgeTimers(pErrorTimer, this) ;
    if (pRefreshTimer->getState() == OsTimer::STARTED)
        pRefreshTimer->stop() ;
    mpAgentTask->purgeTimers(pRefreshTimer, this) ;
    
    // Notify Socket
    UtlString empty ;
    pSocket->setTurnAddress(empty, PORT_NONE) ;
    pSocket->markTurnFailure() ;   
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTurnContext::markSuccess(const UtlString& mappedAddress, 
                                        int              mappedPort, 
                                        const UtlString& receivedIp, 
                                        int              receivedPort,
                                        const UtlString& relayIp, 
                                        int              relayPort)
{
    OsTime refreshPeriod(keepAliveSecs, 0) ;

    assert(pErrorTimer != NULL) ;
    assert(pRefreshTimer != NULL) ;
    assert(pSocket != NULL) ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "Turn success for %s:%d, relay address: %s:%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort(),
            relayIp.data(),
            relayPort) ;
#if NAT_DEBUG >= 1
    printf("Turn success for %s:%d, relay address: %s:%d\n",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort(),
            relayIp.data(),
            relayPort) ;
#endif

    // Update State
    address = relayIp ;
    port = relayPort ;
    status = SUCCESS ;
    refreshErrors = 0 ;
    errorTimeoutMSec = OsNatAgentTask::sTurnTimeoutMS ;
    lEndTimestamp = OsDateTime::getCurTimeInMS() ;

    // Reset Timers
    if (pErrorTimer->getState() == OsTimer::STARTED)
        pErrorTimer->stop() ;
    mpAgentTask->purgeTimers(pErrorTimer, this) ;
    if (pRefreshTimer->getState() == OsTimer::STARTED)
        pRefreshTimer->stop() ;
    mpAgentTask->purgeTimers(pRefreshTimer, this) ;

    if (keepAliveSecs > 0)
    {
        pRefreshTimer->periodicEvery(refreshPeriod, refreshPeriod) ;          
    }

    mpAgentTask->onTurnRTT(lEndTimestamp - lStartTimestamp) ;

    // Notify Socket
    pSocket->setTurnAddress(relayIp, relayPort) ;
    pSocket->markTurnSuccess() ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTurnContext::markTimeout()
{
    switch (type)
    {
        case TURN_ALLOCATION:
            handleTurnAllocTimeout() ;
            break ;
        case TURN_SEND_REQUEST:
            handleTurnSendRequestTimeout() ;
            break ;
        case TURN_SET_ACTIVE_DEST:
            handleTurnSetDestTimeout() ;
            break ;
        default:
            assert(false) ;
            break;
    }
}

void OsNatAgentTurnContext::handleTurnAllocTimeout() 
{
    switch (status)
    {
        case SUCCESS:
            // Send keep-alive, set state to resending
            lStartTimestamp = OsDateTime::getCurTimeInMS() ;
            status = RESENDING ;
            if (!mpAgentTask->sendTurnRequest(this, true))
                markFailure(NAT_FAILURE_NETWORK) ;
            break ;
        case SENDING:
            // Migrate to sending_error and resend
            status = SENDING_ERROR ;
            if (!mpAgentTask->sendTurnRequest(this, false))
                markFailure(NAT_FAILURE_NETWORK) ;
            break ;
        case SENDING_ERROR:
            // If we have hit the max resends, fail otherwise resend
            refreshErrors++ ;
            if (refreshErrors < abortCount)
            {
                if (!mpAgentTask->sendTurnRequest(this, false))
                    markFailure(NAT_FAILURE_NETWORK) ;
            } 
            else
            {
                markFailure(NAT_FAILURE_TIMEOUT) ;
            }
            break ;
        case RESENDING:
            // Migrate to resend_error and resend
            status = RESENDING_ERROR ;
            refreshErrors++ ;
            if (!mpAgentTask->sendTurnRequest(this, false))
                markFailure(NAT_FAILURE_NETWORK) ;
            
            break ;
        case RESENDING_ERROR:                
            refreshErrors++ ;
            if (refreshErrors < NAT_RESEND_ABORT_COUNT)
            {                    
                if (!mpAgentTask->sendTurnRequest(this, false))
                    markFailure(NAT_FAILURE_NETWORK) ;
            }
            else
            {
                markFailure(NAT_FAILURE_TIMEOUT) ;
            }
            break ;
        case FAILED:
            // Nothing to do
            break ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTurnContext::handleTurnSendRequestTimeout() 
{
    switch (status)
    {
        case SUCCESS:
            // Clear binding
            mpAgentTask->destroyBinding(this) ;
            break ;
        case SENDING:
            refreshErrors++ ;
            if (refreshErrors < abortCount)
            {
                mpAgentTask->doSendTurnSendRequest(this, false) ;                    
            }
            else
            {
                UtlString localHost("Unknown") ;
                int localPort = -1 ;
                if (pSocket && pSocket->getSocket())
                {
                    pSocket->getSocket()->getLocalHostIp(&localHost) ;
                    localPort = pSocket->getSocket()->getLocalHostPort() ;
                }

                OsSysLog::add(FAC_NAT, PRI_INFO, "Timeout waiting for send request response (%s:%d->%s:%d (via relay)",
                        localHost.data(),
                        localPort,
                        serverAddress.data(),
                        serverPort) ;
#if NAT_DEBUG >= 1
                printf("Timeout waiting for send request response (%s:%d->%s:%d (via relay)\n",
                        localHost.data(),
                        localPort,
                        serverAddress.data(),
                        serverPort) ;
#endif

                // Failure; clear binding
                mpAgentTask->destroyBinding(this) ;
            }
            break ;
        case FAILED:
            mpAgentTask->destroyBinding(this) ;
            break ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTurnContext::handleTurnSetDestTimeout() 
{
    switch (status)
    {
        case SUCCESS:
            // Clear binding
            mpAgentTask->destroyBinding(this) ;
            break ;
        case SENDING:
            refreshErrors++ ;
            if (refreshErrors < abortCount)
            {
                mpAgentTask->doSendTurnSetDestRequest(this, false) ;                    
            }
            else
            {
                UtlString localHost("Unknown") ;
                int localPort = -1 ;
                if (pSocket && pSocket->getSocket())
                {
                    pSocket->getSocket()->getLocalHostIp(&localHost) ;
                    localPort = pSocket->getSocket()->getLocalHostPort() ;
                }

                OsSysLog::add(FAC_NAT, PRI_INFO, "Timeout waiting for turn set dest request response: %s:%d->%s:%d (via relay)",
                        localHost.data(),
                        localPort,
                        serverAddress.data(),
                        serverPort) ;
#if NAT_DEBUG >= 1
                printf("Timeout waiting for turn set dest request response: %s:%d->%s:%d (via relay)\n",
                        localHost.data(),
                        localPort,
                        serverAddress.data(),
                        serverPort) ;
#endif

                // Failure; clear binding
                mpAgentTask->destroyBinding(this) ;
            }
            break ;
        case FAILED:
            mpAgentTask->destroyBinding(this) ;
            break ;
    }
}

static const char* scNAT_CLASSIFICATION_STAGE[] =
{
    "NAT_STAGE_TEST1",
    "NAT_STAGE_FW_TEST2",
    "NAT_STAGE_NAT_TEST2",
    "NAT_STAGE_NAT_TEST1",
    "NAT_STAGE_NAT_TEST3",
    "NAT_STAGE_DONE"
} ;

static const char* scNAT_CLASSIFICATION_TYPE[] =
{
    "NAT_CLASSIFICATION_SERVER_ERROR",
    "NAT_CLASSIFICATION_BLOCKED",
    "NAT_CLASSIFICATION_OPEN",
    "NAT_CLASSIFICATION_FIREWALL_SYMMETRIC",
    "NAT_CLASSIFICATION_NAT_FULL_CONE",
    "NAT_CLASSIFICATION_NAT_SYMMETRIC",
    "NAT_CLASSIFICATION_NAT_RESTRICTED",
    "NAT_CLASSIFICATION_NAT_PORT_RESTRICTED",
} ;


//////////////////////////////////////////////////////////////////////////////


OsNatAgentNCContext::OsNatAgentNCContext(OsNatAgentTask* pAgentTask,
                                         IOsNatSocket* pNatSocket,
                                         OsNatAgentContextListener* pListener)
    : OsNatAgentContext(pAgentTask)
{
    mStage = NAT_STAGE_TEST1 ;
    mpListener = pListener ;
    pSocket = pNatSocket ;
    mChangePort = PORT_NONE ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentNCContext::~OsNatAgentNCContext()
{

}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentNCContext::markFailure(NAT_FAILURE_CAUSE cause) 
{
    OsNatAgentContext::markFailure(cause) ;

    switch (mStage)
    {
        case NAT_STAGE_TEST1:
            // UDP is blocked
            setClassification(NAT_CLASSIFICATION_BLOCKED) ;
            setStage(NAT_STAGE_DONE) ;
            break ;
        case NAT_STAGE_FW_TEST2:
            setClassification(NAT_CLASSIFICATION_BLOCKED) ;
            setStage(NAT_STAGE_DONE) ;
            break ;
        case NAT_STAGE_NAT_TEST2:
            setStage(NAT_STAGE_NAT_TEST1) ;
            break ;
        case NAT_STAGE_NAT_TEST1:
            setClassification(NAT_CLASSIFICATION_SERVER_ERROR) ;
            setStage(NAT_STAGE_DONE) ;
            break ;
        case NAT_STAGE_NAT_TEST3:
            setClassification(NAT_CLASSIFICATION_NAT_PORT_RESTRICTED) ;
            setStage(NAT_STAGE_DONE) ;            
            break ;
        case NAT_STAGE_DONE:
        default:
            assert(false) ;
            break ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentNCContext::markSuccess(const UtlString& mappedAddress, 
                                      int              mappedPort, 
                                      const UtlString& receivedIp, 
                                      int              receivedPort,
                                      const UtlString& changeIp, 
                                      int              changePort)
{
    UtlString prevMappedAddress = address ;
    int       prevMappedPort = port ;

    OsNatAgentContext::markSuccess(mappedAddress, mappedPort, receivedIp, receivedPort, changeIp, changePort) ;

    UtlString localHostIp ;
    int       localPort ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    localPort = pSocket->getSocket()->getLocalHostPort() ;
    
    switch (mStage)
    {
        case NAT_STAGE_TEST1:
            mChangeIp = changeIp ;
            mChangePort = changePort ;

            // Validate that we received a change ip/port
            if ((mChangeIp.length() == 0) || mChangePort <= 0)
            {
                setClassification(NAT_CLASSIFICATION_SERVER_ERROR) ;
                setStage(NAT_STAGE_DONE) ;
            }
            else
            {
                if (    localPort == mappedPort && 
                        localHostIp.compareTo(mappedAddress, UtlString::ignoreCase) == 0)
                    setStage(NAT_STAGE_FW_TEST2) ;
                else
                    setStage(NAT_STAGE_NAT_TEST2) ;
            }
            break ;
        case NAT_STAGE_FW_TEST2:
            setClassification(NAT_CLASSIFICATION_OPEN) ;
            setStage(NAT_STAGE_DONE) ;
            break ;
        case NAT_STAGE_NAT_TEST2:
            setClassification(NAT_CLASSIFICATION_NAT_FULL_CONE) ;
            setStage(NAT_STAGE_DONE) ;
            break ;
        case NAT_STAGE_NAT_TEST1:
            if (    prevMappedPort == mappedPort && 
                    prevMappedAddress.compareTo(mappedAddress, UtlString::ignoreCase) == 0)
            {
                setStage(NAT_STAGE_NAT_TEST3) ;
            }
            else
            {
                setClassification(NAT_CLASSIFICATION_NAT_SYMMETRIC) ;
                setStage(NAT_STAGE_DONE) ;
            }
                
            break ;
        case NAT_STAGE_NAT_TEST3:
            setClassification(NAT_CLASSIFICATION_NAT_RESTRICTED) ;
            setStage(NAT_STAGE_DONE) ;
            break ;
        case NAT_STAGE_DONE:
        default:
            assert(false) ;
            break ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentNCContext::markTimeout()
{
    if (mStage == NAT_STAGE_DONE)
    {        
        // We should only ever hit this state when keepAlive/retry is requested
        assert(keepAliveSecs > 0) ;
        setStage(NAT_STAGE_TEST1) ;
    }
    else
    {
        handleStunTimeout() ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentNCContext::start() 
{
    mOrigServer = serverAddress ;
    mOrigPort = serverPort ;

    setStage(NAT_STAGE_TEST1) ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentNCContext::setStage(NAT_CLASSIFICATION_STAGE stage)
{
    OsNatAgentTask::trace(PRI_DEBUG, "NAT classification moving from %s to %s",
            scNAT_CLASSIFICATION_STAGE[mStage],
            scNAT_CLASSIFICATION_STAGE[stage]) ;

    mStage = stage ;

    switch (mStage)
    {
        case NAT_STAGE_TEST1:
            doReset() ;
            doSend() ;
            break ;
        case NAT_STAGE_FW_TEST2:
            doReset() ;
            options = (ATTR_CHANGE_FLAG_PORT | ATTR_CHANGE_FLAG_IP) ;
            doSend() ;
            break ;
        case NAT_STAGE_NAT_TEST2:
            doReset() ;
            options = (ATTR_CHANGE_FLAG_PORT | ATTR_CHANGE_FLAG_IP) ;
            doSend() ;
            break ;
        case NAT_STAGE_NAT_TEST1:
            doReset() ;
            serverAddress = mChangeIp ;
            serverPort = mChangePort ;
            doSend() ;
            break ;
        case NAT_STAGE_NAT_TEST3:
            doReset() ;
            serverAddress = mChangeIp ;
            serverPort = mChangePort ;
            options = ATTR_CHANGE_FLAG_PORT ;
            doSend() ;
            break ;
        case NAT_STAGE_DONE:
            if (keepAliveSecs <= 0)
                mpAgentTask->destroyBinding(this) ;
            else
            {           
                OsTime refreshPeriod(keepAliveSecs, 0) ;
                // Reset
                doReset() ;

                serverAddress = mOrigServer ;
                serverPort = mOrigPort ;
                pRefreshTimer->periodicEvery(refreshPeriod, refreshPeriod) ;
            }
            break ;
        default:
            assert(false) ;
            break ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentNCContext::setClassification(NAT_CLASSIFICATION_TYPE type) 
{
    OsNatAgentTask::trace(PRI_INFO, "NAT classification: %s",
            scNAT_CLASSIFICATION_TYPE[type]) ;

    if (mpListener)
        mpListener->OnClassificationComplete(type) ;
}

//////////////////////////////////////////////////////////////////////////////

// Resets all but mapped address/port and server address/port
void OsNatAgentNCContext::doReset() 
{
    type = NAT_CLASSIFICATION ;
    status = SENDING ;

    relayAddress.remove(0) ;
    relayPort = PORT_NONE ;

    options = 0 ;

    memset(&transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;

    if (pErrorTimer)
        mpAgentTask->releaseTimer(pErrorTimer) ;
    pErrorTimer = mpAgentTask->getTimer(this) ;
    if (pRefreshTimer)
        mpAgentTask->releaseTimer(pRefreshTimer) ;
    pRefreshTimer = mpAgentTask->getTimer(this) ;

    abortCount = NAT_PROBE_SOFTABORT_COUNT ;
    errorTimeoutMSec = OsNatAgentTask::sStunTimeoutMS ;
    refreshErrors = 0 ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentNCContext::doSend() 
{
    mpAgentTask->sendStunRequest(this, true) ;
}

//////////////////////////////////////////////////////////////////////////////
