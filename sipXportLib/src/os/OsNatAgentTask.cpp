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

// APPLICATION INCLUDES
#include "os/OsNatAgentTask.h"
#include "os/OsNatAgentContext.h"
#include "os/IOsNatSocket.h"
#include "os/TurnMessage.h"
#include "os/StunMessage.h"
#include "os/StunUtils.h"

#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "os/OsEvent.h"
#include "os/OsWriteLock.h"
#include "os/OsReadLock.h"
#include "os/OsTime.h"
#include "os/OsTimer.h"
#include "os/OsQueuedEvent.h"
#include "utl/UtlVoidPtr.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlSListIterator.h"

#ifndef _WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern const char* scBindingTypes[] ;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
OsMutex OsNatAgentTask::sLock(OsMutex::Q_FIFO) ;
OsNatAgentTask* OsNatAgentTask::spInstance = NULL ;

int OsNatAgentTask::sStunTimeoutMS = NAT_RESPONSE_TIMEOUT_MS_START ;
int OsNatAgentTask::sTurnTimeoutMS = NAT_RESPONSE_TIMEOUT_MS_START ;

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define NAT_DEBUG 0

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsNatAgentTask::OsNatAgentTask()
    : OsServerTask("OsNatAgentTask-%d")
    , mMapsLock(OsMutex::Q_FIFO)
    , mExternalBindingMutex(OsRWMutex::Q_FIFO)
{
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentTask::~OsNatAgentTask()
{
    OsLock lock(mMapsLock) ;
    UtlVoidPtr* pKey ;

    // Wait for the thread to shutdown
    waitUntilShutDown() ;
    
    // Clear Context map
    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        mContextMap.destroy(pKey) ;
        if (pContext->pErrorTimer)
            releaseTimer(pContext->pErrorTimer) ;
        if (pContext->pRefreshTimer)
            releaseTimer(pContext->pRefreshTimer) ;
        
        delete pContext ;
    }

    // Clear Timers 
    UtlSListIterator timerIterator(mTimerPool);
    while (pKey = (UtlVoidPtr*)timerIterator())
    {
        OsTimer* pTimer = (OsTimer*) pKey->getValue() ;
        mTimerPool.destroy(pKey) ;        
        delete pTimer ;
    }

    // Clear external bindings
    UtlSListIterator xbindIterator(mExternalBindingsList);
    while (pKey = (UtlVoidPtr*)xbindIterator())
    {
        NAT_AGENT_EXTERNAL_CONTEXT* pContext = 
                (NAT_AGENT_EXTERNAL_CONTEXT*) pKey->getValue() ;
        mExternalBindingsList.destroy(pKey) ;        
        delete pContext ;
    }
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentTask* OsNatAgentTask::getInstance() 
{
    OsLock lock(sLock) ;
    if (spInstance == NULL) 
    {
        spInstance = new OsNatAgentTask() ;
        spInstance->start() ;
    }

    return spInstance ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::releaseInstance() 
{
    OsLock lock(sLock) ;

    if (spInstance != NULL)
    {
        delete spInstance ;
        spInstance = NULL ;
    }
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean OsNatAgentTask::handleMessage(OsMsg& rMsg) 
{
    UtlBoolean bHandled = false ;    

    switch (rMsg.getMsgType())
    {
        case NAT_MSG_TYPE:
            if (((NatMsg&) rMsg).getType() == NatMsg::STUN_MESSAGE)
            {
                handleStunMessage((NatMsg&) rMsg) ;
            }
            else if (((NatMsg&) rMsg).getType() == NatMsg::TURN_MESSAGE)
            {
                handleTurnMessage((NatMsg&) rMsg) ;
            }
            else
            {
                // Unknown message type
                assert(false) ;
            }
            bHandled = true ;
            break ;
        case SYNC_MSG_TYPE:
            handleSynchronize((OsRpcMsg&) rMsg) ;
            bHandled = true ;
            break ;
        case OsMsg::OS_EVENT:
            {
                // Pull out context
                OsTimer* pTimer = NULL ;
                OsNatAgentContext* pContext = NULL ;
                OsStatus rc = ((OsEventMsg&)rMsg).getUserData((int&) pContext) ;
                assert(rc == OS_SUCCESS) ;
                rc = ((OsEventMsg&)rMsg).getEventData((int&) pTimer) ;
                assert(rc == OS_SUCCESS) ;
                
                if ((rc == OS_SUCCESS) && (pContext != NULL))
                {
                    handleTimerEvent(pContext, pTimer) ;
                }
                else
                {
                    assert(false) ;
                }
            }
            bHandled = true ;
            break ;
    }
    return bHandled ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::handleStunMessage(NatMsg& rMsg) 
{   
    OsLock lock(mMapsLock) ;

    StunMessage    msg ;

    UtlString      sendToAddress ;
    unsigned short sendToPort ;

    size_t         nBuffer = rMsg.getLength() ;
    char*          pBuffer = rMsg.getBuffer() ;
    IOsNatSocket*  pSocket = rMsg.getSocket() ;

    if (OsSysLog::willLog(FAC_NAT, PRI_DEBUG))
    {
        if (pBuffer != NULL && nBuffer > 0)
        {
            UtlString tmp, output, localHostIp ;
            pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
            StunUtils::debugDump(pBuffer, nBuffer, tmp) ;
            output.format("STUN IN %s:%d <- %s:%d, relay=%s:%d\n%s",
                    localHostIp.data(),
                    pSocket->getSocket()->getLocalHostPort(),
                    rMsg.getReceivedIp().data(),
                    rMsg.getReceivedPort(),                    
                    rMsg.getRelayIp().data(),
                    rMsg.getRelayPort(),                                    
                    tmp.data()) ;

            OsSysLog::add(FAC_NAT, PRI_DEBUG, "%s", output.data()) ;
#if NAT_DEBUG >= 2
            printf("%s\n", output.data()) ;
#endif
        }
    }

    if ((nBuffer > 0) && pBuffer && pSocket && msg.parse(pBuffer, nBuffer))
    {
        switch (msg.getType())
        {
            case MSG_STUN_BIND_REQUEST:
                {
                    StunMessage         respMsg ;
                    STUN_TRANSACTION_ID transactionId ;
                    STUN_MAGIC_ID       magicId ;
                    unsigned short unknownAttributes[STUN_MAX_UNKNOWN_ATTRIBUTES] ;
                    size_t         nUnknownAttributes ;

                    // Copy over IDs
                    msg.getTransactionId(&transactionId) ;
                    respMsg.setTransactionId(transactionId) ;
                    msg.getMagicId(&magicId) ;
                    respMsg.setMagicId(magicId) ;

                    // Check for unknown attributes
                    if (msg.getUnknownParsedAttributes(unknownAttributes, 
                            STUN_MAX_UNKNOWN_ATTRIBUTES, nUnknownAttributes) &&
                            (nUnknownAttributes > 0))
                    {
                        respMsg.setType(MSG_STUN_BIND_ERROR_RESPONSE) ;
                        for (size_t i = 0; i<nUnknownAttributes; i++)
                        {
                            respMsg.addUnknownAttribute(unknownAttributes[i]) ;
                        }
                    }
                    else
                    {
                        // TODO: Send Error if changed port/ip is requested

                        // Set Response type
                        respMsg.setType(MSG_STUN_BIND_RESPONSE) ;                        

                         // Obey XOR request
                        if (msg.getRequestXorOnly())
                            respMsg.setSendXorOnly() ;

                        // Set Mapped && Source Addresses
                        respMsg.setMappedAddress(rMsg.getReceivedIp(), rMsg.getReceivedPort()) ;
                        respMsg.setSourceAddress(pSocket->getSocket()->getLocalIp(), pSocket->getSocket()->getLocalHostPort()) ;

                        // Check for response address
                        char cResponseAddress[64] ;
                        unsigned short responsePort ;
                        if (msg.getResponseAddress(cResponseAddress, responsePort))
                        {
                            respMsg.setReflectedFrom(pSocket->getSocket()->getLocalIp(), pSocket->getSocket()->getLocalHostPort()) ;
                            sendToAddress = cResponseAddress ;
                            sendToPort = responsePort ;
                        }
                        else
                        {
                            sendToAddress = rMsg.getReceivedIp() ;
                            sendToPort = rMsg.getReceivedPort() ;
                        }

                        // Check if this came over our relay
                        if (rMsg.getRelayPort() != 0 && rMsg.getRelayIp().length() > 0)
                        {
                            // We are assuming that if something comes in as data indication that we 
                            // should send the response back via our turn binding.  We *should* validate 
                            // the relay address against the turn server address in the binding.
                            char cEncoded[NAT_MAX_STURN_MSG_LENGTH] ;
                            size_t length ;
                            if (respMsg.encode(cEncoded, sizeof(cEncoded), length))
                                sendTurnSendRequest(pSocket, sendToAddress, sendToPort, cEncoded, length) ;
                            else
                                assert(false) ;
                        }
                        else
                            sendMessage(&respMsg, pSocket, sendToAddress, sendToPort, STUN_DISCOVERY_PACKET) ;

                        // Look for new peer-reflective addresses
                        bool bFoundMatch = false ;
                        bool bFoundProbe = false ;
                        OsNatAgentContext* pMatchingContext = NULL ;
                        {
                            UtlHashMapIterator iterator(mContextMap);
                            UtlVoidPtr* pKey;

                            while (pKey = (UtlVoidPtr*)iterator())
                            {        
                                OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
                                if ((pContext->pSocket == pSocket) && (pContext->type == STUN_PROBE))
                                {
                                    bFoundProbe = true ;
                                    if (    (pContext->serverAddress.compareTo(rMsg.getReceivedIp()) == 0) && 
                                            (pContext->serverPort == rMsg.getReceivedPort()) &&
                                            (pContext->relayAddress.compareTo(rMsg.getReceivedIp()) == 0))
                                    {
                                        bFoundMatch = true ;
                                        break ;
                                    }
                                    else if (pContext->serverAddress.compareTo(rMsg.getReceivedIp()) == 0)
                                    {
                                        pMatchingContext = pContext ;
                                    }
                                }
                            }

                            unsigned long priority = 0 ;
                            if (!bFoundMatch && msg.getPriority(priority))
                            {                                    
                                if (pMatchingContext && priority == 0)
                                    priority = pMatchingContext->priority ;
                                if (priority == 0)
                                    priority = 10 ; 

                                OsSysLog::add(FAC_NAT, PRI_INFO, "Possible peer-reflective address: %s:%d (relay: %s:%d) pri=%d\n",
                                        rMsg.getReceivedIp().data(), rMsg.getReceivedPort(), 
                                        rMsg.getRelayIp().data(), rMsg.getRelayPort(),
                                        priority) ;
#if NAT_DEBUG >= 1
                                printf("Possible peer-reflective address: %s:%d (relay: %s:%d) pri=%d\n",
                                        rMsg.getReceivedIp().data(), rMsg.getReceivedPort(), 
                                        rMsg.getRelayIp().data(), rMsg.getRelayPort(),
                                        priority) ;
#endif
                                sendStunProbe(pSocket, rMsg.getReceivedIp(), rMsg.getReceivedPort(), 
                                        rMsg.getRelayIp(), rMsg.getRelayPort(), 
                                        priority);
                            }
                        }
                    }
                }
                break ;
            case MSG_STUN_BIND_RESPONSE:
                {                           
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    OsNatAgentContext* pContext = getBinding(&transactionId) ;
                    if (pContext)
                    {
                        char mappedAddress[64] ;
                        unsigned short mappedPort ;

                        if (msg.getMappedAddress(mappedAddress, mappedPort))
                        {
                            UtlString receivedIp = rMsg.getReceivedIp() ;
                            int receivedport = rMsg.getReceivedPort() ;
#if NAT_DEBUG >= 1
                            printf("Received STUN response for %s:%d / %s:%d\n", 
                                    mappedAddress, mappedPort, receivedIp.data(), receivedport) ;
#endif
                            

                            char changedAddress[64] ;
                            unsigned short  changedPort ;

                            if (!msg.getChangedAddress(changedAddress, changedPort))
                            {
                                memset(changedAddress, 0, sizeof(changedAddress)) ;
                                changedPort = PORT_NONE ;
                            }

                            pContext->markSuccess(mappedAddress, mappedPort, 
                                    receivedIp, receivedport,
                                    changedAddress, changedPort) ;
                        }
                        else
                        {
                            assert(false) ;
                            pContext->markFailure(NAT_FAILURE_PROTOCOL) ;
                        }                   
                    }
                }
                break ;
            case MSG_STUN_BIND_ERROR_RESPONSE:
                {
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    OsNatAgentContext* pContext = getBinding(&transactionId) ;
                    if (pContext)
                        pContext->markFailure(NAT_FAILURE_UNKNOWN) ;
                }
                break ;
            case MSG_STUN_SHARED_SECRET_REQUEST:
                {
                    StunMessage         respMsg ;
                    STUN_TRANSACTION_ID transactionId ;
                    STUN_MAGIC_ID       magicId ;

                    // Not supported on client
                    msg.getTransactionId(&transactionId) ;
                    respMsg.setTransactionId(transactionId) ;
                    msg.getMagicId(&magicId) ;
                    respMsg.setMagicId(magicId) ;
                    respMsg.setError(STUN_ERROR_GLOBAL_CODE, STUN_ERROR_GLOBAL_TEXT) ;
                    sendMessage(&respMsg, pSocket, rMsg.getReceivedIp(), rMsg.getReceivedPort(), TURN_PACKET) ;
                }
                break ;
            case MSG_STUN_SHARED_SECRET_RESPONSE:
            case MSG_STUN_SHARED_SECRET_ERROR_RESPONSE:
                break ;
        }
    }

    if (pBuffer)
        free(pBuffer) ;

    return true ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::handleTurnMessage(NatMsg& rMsg) 
{
    OsLock lock(mMapsLock) ;

    TurnMessage         msg ;
    TurnMessage         respMsg ;
    size_t              nBuffer = rMsg.getLength() ;
    char*               pBuffer = rMsg.getBuffer() ;
    IOsNatSocket*       pSocket = rMsg.getSocket() ;
    STUN_TRANSACTION_ID transactionId ;
    STUN_MAGIC_ID       magicId ;

    if (OsSysLog::willLog(FAC_NAT, PRI_DEBUG))
    {
        if (pBuffer != NULL && nBuffer > 0)
        {
            UtlString tmp, output, localHostIp ;
            pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
            StunUtils::debugDump(pBuffer, nBuffer, tmp) ;
            output.format("TURN IN %s:%d <- %s:%d, relay=%s:%d\n%s",
                    localHostIp.data(),
                    pSocket->getSocket()->getLocalHostPort(),
                    rMsg.getReceivedIp().data(),
                    rMsg.getReceivedPort(),                    
                    rMsg.getRelayIp().data(),
                    rMsg.getRelayPort(),                                    
                    tmp.data()) ;

            OsSysLog::add(FAC_NAT, PRI_DEBUG, "%s", output.data()) ;
#if NAT_DEBUG >= 2
            printf("%s\n", output.data()) ;
#endif
        }
    }

    if ((nBuffer > 0) && pBuffer && pSocket && msg.parse(pBuffer, nBuffer))
    {
        switch (msg.getType())
        {
            case MSG_TURN_ALLOCATE_REQUEST:
                // Not supported on client
                msg.getTransactionId(&transactionId) ;
                respMsg.setTransactionId(transactionId) ;
                msg.getMagicId(&magicId) ;
                respMsg.setMagicId(magicId) ;
                respMsg.setError(STUN_ERROR_GLOBAL_CODE, STUN_ERROR_GLOBAL_TEXT) ;
                sendMessage(&respMsg, pSocket, rMsg.getReceivedIp(), rMsg.getReceivedPort(), TURN_PACKET) ;
                break ;
            case MSG_TURN_ALLOCATE_RESPONSE:
                {
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    OsNatAgentTurnContext* pContext = getTurnBinding(&transactionId) ;
                    if ((pContext) && pContext->type == TURN_ALLOCATION)
                    {
                        char relayAddress[64] ;
                        unsigned short relayPort ;

                        if (msg.getMappedAddress(relayAddress, relayPort))
                        {
                            pContext->markSuccess(
                                relayAddress, relayPort,
                                rMsg.getReceivedIp().data(),
                                rMsg.getReceivedPort(),
                                relayAddress, relayPort) ;
                        }
                        else
                        {
                            pContext->markFailure(NAT_FAILURE_PROTOCOL) ;
                        }
                    }
                }
                break ;
            case MSG_TURN_ALLOCATE_ERROR_RESPONSE:
                {
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    OsNatAgentTurnContext* pContext = getTurnBinding(&transactionId) ;
                    if (pContext)
                    {
                        if (pContext->status == SUCCESS)
                        {
                            // Ignore failure responses for successful transactions 
                            // (work-around for servers which incorrectly handle
                            // resends).
                            OsSysLog::add(FAC_NAT, PRI_INFO, 
                                "Ignoring turn allocation error for valid transaction (%s:%d)",
                                pContext->address.data(),
                                pContext->port) ;
                        }
                        else if (pContext->type == TURN_ALLOCATION)
                            pContext->markFailure(NAT_FAILURE_UNKNOWN) ;
                    }
                }
                break ;
            case MSG_TURN_SEND_REQUEST:
                // Not supported on client                                
                msg.getTransactionId(&transactionId) ;
                respMsg.setTransactionId(transactionId) ;
                msg.getMagicId(&magicId) ;
                respMsg.setMagicId(magicId) ;
                respMsg.setError(STUN_ERROR_GLOBAL_CODE, STUN_ERROR_GLOBAL_TEXT) ;
                sendMessage(&respMsg, pSocket, rMsg.getReceivedIp(), rMsg.getReceivedPort(), TURN_PACKET) ;
                break ;
            case MSG_TURN_SEND_RESPONSE:
                {
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    OsNatAgentContext* pContext = getBinding(&transactionId) ;
                    if (pContext && pContext->type == TURN_SEND_REQUEST)
                    {
                        pContext->status = SUCCESS ;
                        pContext->lEndTimestamp = OsDateTime::getCurTimeInMS() ;
                    }
                }
                break ;
            case MSG_TURN_SEND_ERROR_RESPONSE:
                {                    
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    OsNatAgentContext* pContext = getBinding(&transactionId) ;

                    assert(pContext->type == TURN_SEND_REQUEST) ;
                    if (pContext->type == TURN_SEND_REQUEST)
                    {
                        pContext->status = FAILED ;
                        pContext->lEndTimestamp = OsDateTime::getCurTimeInMS() ;
                    }
                }

                // Drop error response -- not much we can do right now.
                break ;
            case MSG_TURN_DATA_INDICATION:
                // Data indications should be recursed upon in OsNatSocketBaseImpl
                assert(false);
                break ;
            case MSG_TURN_ACTIVE_DESTINATION_RESPONSE:
                {
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    OsNatAgentContext* pContext = getBinding(&transactionId) ;
                    if (pContext && pContext->type == TURN_SET_ACTIVE_DEST)
                    {
                        pContext->status = SUCCESS ;
                        pContext->lEndTimestamp = OsDateTime::getCurTimeInMS() ;
                    }
                }
                break ;
            case MSG_TURN_ACTIVE_DESTINATION_ERROR_RESPONSE:
                {
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    OsNatAgentContext* pContext = getBinding(&transactionId) ;
                    if (pContext && pContext->type == TURN_SET_ACTIVE_DEST)
                    {
                        pContext->status = SUCCESS ;
                        pContext->lEndTimestamp = OsDateTime::getCurTimeInMS() ;
                    }
                }
                break ;
            default:
                // Unknown message type
                assert(false) ;
                break ;
        }
    }

    if (pBuffer)
        free(pBuffer) ;

    return true ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::handleSynchronize(OsRpcMsg& rMsg) 
{
    OsEvent* pEvent = rMsg.getEvent() ;
    pEvent->signal(0) ;

    return true ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::handleTimerEvent(OsNatAgentContext* pContext, OsTimer* pTimer) 
{    
    OsLock lock(mMapsLock) ;

    // Refresh the socket
    if ((pContext) && (pContext = getBinding(pContext))) 
    {
        bool bProcessTimeout = true ;

        // Santity Checking the timeout expiration
        switch (pContext->type)
        {
            case STUN_PROBE:    
            case STUN_DISCOVERY:
            case TURN_ALLOCATION:
            case TURN_SEND_REQUEST:
            case NAT_CLASSIFICATION:
                // Even though we stop the error timer, that event could be 
                // queued for handling.  Never process an error timer if we
                // are in the success case.
                if ((pContext->status == SUCCESS) && (pTimer == pContext->pErrorTimer))
                    bProcessTimeout = false ;
                break ;
            default:
                break ;
        }

        // Process
        switch (pContext->type)
        {
            case STUN_PROBE:    
            case STUN_DISCOVERY:
            case CRLF_KEEPALIVE:
            case STUN_KEEPALIVE:
            case NAT_CLASSIFICATION:
                if (bProcessTimeout)
                    pContext->markTimeout() ;
                break ;
            case TURN_ALLOCATION:
            case TURN_SEND_REQUEST:
            case TURN_SET_ACTIVE_DEST:
                if (bProcessTimeout)
                {
                    OsNatAgentTurnContext* pTurnContext = getTurnBinding(pContext) ;
                    if (pTurnContext)
                        pTurnContext->markTimeout() ;
                    else
                        assert(false) ;
                }
                break ;
            default:
                assert(false) ;
                break ;
        }
    }

    return true ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::sendStunProbe(IOsNatSocket* pSocket,
                                         const char*   stunServer,
                                         int           stunPort,
                                         const char*   relayAddress,
                                         int           relayPort,
                                         int           priority) 
{
    OsLock lock(mMapsLock) ;

    bool bSuccess = false ;
    StunMessage msgSend ;
    UtlString serverAddress ;

    if (!doesProbeBindingExist(pSocket, stunServer, stunPort, relayAddress, relayPort))
    {
        UtlString localHostIp ;
        pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
        OsSysLog::add(FAC_NAT, PRI_INFO, "Stun probe for %s:%d with server %s:%d priority=%d",
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                stunServer,
                stunPort,
                priority) ;

        assert(pSocket) ;
        assert(portIsValid(stunPort)) ;
        assert(stunServer != NULL) ;
        assert(strlen(stunServer) > 0) ;

        if (    pSocket && portIsValid(stunPort) && stunServer && (strlen(stunServer) > 0) &&
                OsSocket::getHostIpByName(stunServer, &serverAddress) && 
                OsSocket::isIp4Address(serverAddress))
        {
            OsNatAgentContext* pContext = new OsNatAgentContext(this) ;
            if (pContext)
            {
                pContext->type = STUN_PROBE ;
                pContext->status = SENDING ;
                pContext->serverAddress = serverAddress ;
                pContext->serverPort = stunPort ;
                pContext->relayAddress = relayAddress ;
                pContext->relayPort = relayPort ;
                pContext->options = 0 ;
                memset(&pContext->transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;
                pContext->pSocket = pSocket ;
                pContext->pErrorTimer = getTimer(pContext) ;
                pContext->pRefreshTimer = getTimer(pContext) ;
                pContext->keepAliveSecs = NAT_DEFAULT_KEEPALIVE_SEC ;
                pContext->abortCount = NAT_PROBE_HARDABORT_COUNT ;
                pContext->errorTimeoutMSec = MAX(sStunTimeoutMS, sTurnTimeoutMS) ;
                pContext->refreshErrors = 0 ;            
                pContext->port = PORT_NONE ;
                pContext->priority = priority ;
                pContext->pKeepaliveListener = NULL ;
                pContext->lStartTimestamp = OsDateTime::getCurTimeInMS() ;

                mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;
                if (sendStunRequest(pContext, true))
                {
                    bSuccess = true ;
                }
                else
                {
                    // Unable to Send; let timeouts handle the cleanup
                }            
            }
            else
            {
                // Unable to allocate context
                assert(false) ;
            }
        }
    }
    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::enableStun(IOsNatSocket*        pSocket,
                                      const UtlString&     stunServer,
                                      int                  stunPort,                                      
                                      const int            stunOptions,
                                      int                  keepAliveSecs) 
{    
    OsLock lock(mMapsLock) ;

    bool bSuccess = false ;
    StunMessage msgSend ;
    UtlString serverAddress ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "Stun enabled for %s:%d with server %s:%d options=%d, keepAlive=%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort(),
            stunServer.data(),
            stunPort,
            stunOptions,
            keepAliveSecs) ;

    assert(pSocket) ;
    assert(portIsValid(stunPort)) ;
    assert(stunServer.length() > 0) ;

    if (    pSocket && portIsValid(stunPort) && (stunServer.length() > 0) &&
            OsSocket::getHostIpByName(stunServer, &serverAddress) && 
            OsSocket::isIp4Address(serverAddress))
    {
        OsNatAgentContext* pContext = new OsNatAgentContext(this) ;
        if (pContext)
        {
            pContext->type = STUN_DISCOVERY ;
            pContext->status = SENDING ;
            pContext->serverAddress = serverAddress ;
            pContext->serverPort = stunPort ;
            pContext->options = stunOptions ;
            memset(&pContext->transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;            
            pContext->pSocket = pSocket ;
            pContext->pErrorTimer = getTimer(pContext) ;
            pContext->pRefreshTimer = getTimer(pContext) ;
            pContext->keepAliveSecs = keepAliveSecs;
            pContext->abortCount = NAT_INITIAL_ABORT_COUNT ;
            pContext->errorTimeoutMSec = sStunTimeoutMS ;
            pContext->refreshErrors = 0 ;
            pContext->port = PORT_NONE ;
            pContext->priority = 0 ;
            pContext->pKeepaliveListener = NULL ;
            pContext->lStartTimestamp = OsDateTime::getCurTimeInMS() ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;

            if (sendStunRequest(pContext, true))
            {
                bSuccess = true ;
            }
            else
            {
                // Unable to Send
                destroyBinding(pContext) ;
                if (pSocket)
                    pSocket->markStunFailure() ;
            }            
        }
        else
        {
            // Unable to allocate context
            assert(false) ;
        }
    }
    else
    {
        if (pSocket)
            pSocket->markStunFailure() ;
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::disableStun(IOsNatSocket* pSocket) 
{
    OsLock lock(mMapsLock) ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "Stun disabled for %s:%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort()) ;

    OsNatAgentContext* pBinding = getBinding(pSocket, STUN_DISCOVERY) ;
    if (pBinding)
        destroyBinding(pBinding) ;

    return true ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::enableTurn(IOsNatSocket*        pSocket,
                                      const UtlString&     turnServer,
                                      int                  turnPort,
                                      int                  keepAliveSecs,
                                      const UtlString&     username,
                                      const UtlString&     password) 
{
    OsLock lock(mMapsLock) ;
    bool bSuccess = false ;
    TurnMessage msgSend ;
    UtlString serverAddress ;

    // Logging
    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "Turn enabled for %s:%d with server %s:%d, keepalive=%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort(),
            turnServer.data(),
            turnPort,
            keepAliveSecs) ;

    // Sanitiy checking
    assert(pSocket) ;    
    assert(turnServer.length() > 0) ;
    assert(portIsValid(turnPort)) ;

    if (    pSocket && portIsValid(turnPort) && (turnServer.length() > 0) &&
            OsSocket::getHostIpByName(turnServer, &serverAddress) && 
            OsSocket::isIp4Address(serverAddress))
    {
        // Create/Init Context
        OsNatAgentTurnContext* pContext = new OsNatAgentTurnContext(this) ;
        if (pContext)
        {
            pContext->type = TURN_ALLOCATION ;
            pContext->status = SENDING ;
            pContext->serverAddress = serverAddress ;
            pContext->serverPort = turnPort ;
            pContext->pSocket = pSocket ;
            pContext->pErrorTimer = getTimer(pContext) ;
            pContext->pRefreshTimer = getTimer(pContext) ;
            pContext->keepAliveSecs = keepAliveSecs;   
            pContext->abortCount = NAT_INITIAL_ABORT_COUNT ;
            pContext->errorTimeoutMSec = sTurnTimeoutMS ;
            pContext->username = username ;
            pContext->password = password ;
            pContext->lStartTimestamp = OsDateTime::getCurTimeInMS() ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;

            if (sendTurnRequest(pContext, true))
            {
                bSuccess = true ;
            }
            else
            {
                // Unable to Send
                destroyBinding(pContext) ;
                if (pSocket)
                    pSocket->markTurnFailure() ;
            }            
        }
        else
        {
            // Unable to allocate context
            assert(false) ;
        }
    }
    else
    {
        if (pSocket)
            pSocket->markTurnFailure() ;
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::primeTurnReception(IOsNatSocket* pSocket,
                                              const         char* szAddress,
                                              int           iPort)
{
    UtlBoolean bRC = false ;
    OsLock lock(mMapsLock) ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "Turn Priming %s:%d -> %s:%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort(),
            szAddress,
            iPort) ;

    OsNatAgentTurnContext* pBinding = (OsNatAgentTurnContext*) getBinding(pSocket, TURN_ALLOCATION) ;
    if (pBinding)
    {
        OsNatAgentTurnContext* pContext = new OsNatAgentTurnContext(this) ;
        if (pContext)
        {
            pContext->type = TURN_SEND_REQUEST ;
            pContext->status = SENDING ;
            pContext->serverAddress = szAddress ;
            pContext->serverPort = iPort ;
            pContext->pSocket = pSocket ;
            pContext->pErrorTimer = getTimer(pContext) ;
            pContext->abortCount = NAT_INITIAL_ABORT_COUNT ;
            pContext->errorTimeoutMSec = sTurnTimeoutMS ;
            pContext->pPayload = strdup("\r\n\r\n") ;
            pContext->nPayload = 4 ;
            pContext->lStartTimestamp = OsDateTime::getCurTimeInMS() ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;
            if (doSendTurnSendRequest(pContext, true))
            {
                bRC = true ;
            }
        }
    }
    else
    {
        //OsSysLog::flush() ;
        //assert(false) ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::doSendTurnSendRequest(OsNatAgentTurnContext* pContext, bool bNewTransaction) 
{
    UtlBoolean bRC = false ;
    // Build TURN send request
    TurnMessage msgSend ;

    if (bNewTransaction)   
    {
        msgSend.allocTransactionId() ;
        msgSend.getTransactionId(&pContext->transactionId) ;
    }
    else
    {
        msgSend.allocTransactionId() ;
        msgSend.setTransactionId(pContext->transactionId) ;
    }

    msgSend.setType(MSG_TURN_SEND_REQUEST) ;
    msgSend.setDestinationAddress(pContext->serverAddress, pContext->serverPort) ;    
    msgSend.setData(pContext->pPayload, pContext->nPayload) ;

    OsNatAgentTurnContext* pTurnBinding = getTurnBinding(pContext->pSocket, TURN_ALLOCATION) ;
    if (pTurnBinding)
    {
        if (!pTurnBinding->username.isNull())
            msgSend.setUsername(pTurnBinding->username) ;
        if (!pTurnBinding->password.isNull()) 
            msgSend.setPassword(pTurnBinding->password) ;

        bRC = sendMessage(&msgSend, pTurnBinding->pSocket, pTurnBinding->serverAddress, pTurnBinding->serverPort, TURN_PACKET) ;
        armErrorTimer(pContext) ;
    }
    else
    {
        assert(FALSE) ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::setTurnDestination(IOsNatSocket* pSocket,
                                              const char*   szAddress,
                                              int           iPort ) 
{
    UtlBoolean bRC = false ;
    OsLock lock(mMapsLock) ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "Turn destination %s:%d -> %s:%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort(),
            szAddress,
            iPort) ;

    OsNatAgentTurnContext* pBinding = (OsNatAgentTurnContext*) getBinding(pSocket, TURN_ALLOCATION) ;
    if (pBinding)
    {
        OsNatAgentTurnContext* pContext = new OsNatAgentTurnContext(this) ;
        if (pContext)
        {
            pContext->type = TURN_SET_ACTIVE_DEST ;
            pContext->status = SENDING ;
            pContext->serverAddress = szAddress ;
            pContext->serverPort = iPort ;
            pContext->pSocket = pSocket ;
            pContext->pErrorTimer = getTimer(pContext) ;
            pContext->abortCount = NAT_INITIAL_ABORT_COUNT ;
            pContext->errorTimeoutMSec = sTurnTimeoutMS ;
            pContext->lStartTimestamp = OsDateTime::getCurTimeInMS() ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;
            if (doSendTurnSetDestRequest(pContext, true))
            {
                armErrorTimer(pContext) ;
                bRC = true ;
            }
            else
            {
                // Unable to Send
                destroyBinding(pContext) ;
                assert(false) ;
            }
        }
    }
    else
    {
        // we should be able fail gracefully when trying to set the destination,
        // even when there is no binding.
        //assert(false) ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::doSendTurnSetDestRequest(OsNatAgentTurnContext* pContext, bool bNewTransaction) 
{
    UtlBoolean bRC = false ;

    // Build Turn request
    TurnMessage msgSend ;
    if (bNewTransaction)   
    {
        msgSend.allocTransactionId() ;
        msgSend.getTransactionId(&pContext->transactionId) ;
    }
    else
    {
        msgSend.allocTransactionId() ;
        msgSend.setTransactionId(pContext->transactionId) ;
    }

    msgSend.setType(MSG_TURN_ACTIVE_DESTINATION_REQUEST) ;
    msgSend.setDestinationAddress(pContext->serverAddress, pContext->serverPort) ;    

    OsNatAgentTurnContext* pTurnBinding = (OsNatAgentTurnContext*) 
            getBinding(pContext->pSocket, TURN_ALLOCATION) ;
    if (pTurnBinding)
    {
        if (!pTurnBinding->username.isNull())
            msgSend.setUsername(pTurnBinding->username) ;
        if (!pTurnBinding->password.isNull()) 
            msgSend.setPassword(pTurnBinding->password) ;

        bRC = sendMessage(&msgSend, pTurnBinding->pSocket, pTurnBinding->serverAddress, pTurnBinding->serverPort, TURN_PACKET) ;

        // Setup resend-timer for no-response
        if (bRC)
            armErrorTimer(pContext) ;
    }
    else
    {
        assert(FALSE) ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::disableTurn(IOsNatSocket* pSocket) 
{
    OsNatAgentTurnContext* pBinding ;

    OsLock lock(mMapsLock) ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "Turn disabled for %s:%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort()) ;

    // Disable any pending TURN requests (SEND_REQUEST or SET_ACTIVE_DEST)
    while (pBinding = (OsNatAgentTurnContext*) getBinding(pSocket, TURN_SEND_REQUEST))
        destroyBinding(pBinding) ;
    while (pBinding = (OsNatAgentTurnContext*)getBinding(pSocket, TURN_SET_ACTIVE_DEST))
        destroyBinding(pBinding) ;

    // Disable actual TURN_ALLOCATION
    pBinding = (OsNatAgentTurnContext*) getBinding(pSocket, TURN_ALLOCATION) ;
    if (pBinding)
    {
        switch (pBinding->status)
        {
            case SUCCESS:
            case SENDING:
            case SENDING_ERROR:
            case RESENDING:
            case RESENDING_ERROR:
                {
                    // Release Binding ...
                    TurnMessage msgSend ;

                    msgSend.allocTransactionId() ;
                    msgSend.setType(MSG_TURN_ALLOCATE_REQUEST) ;
                    msgSend.setRequestXorOnly() ;
                    msgSend.setLifetime(0) ;

                    if (!pBinding->username.isNull()) 
                        msgSend.setUsername(pBinding->username) ;        
                    if (!pBinding->password.isNull()) 
                        msgSend.setPassword(pBinding->password) ;

                    sendMessage(&msgSend, pSocket, pBinding->serverAddress, pBinding->serverPort, TURN_PACKET) ;
                }
                break ;            
            case FAILED:
                // No need to release binding
                break ;
            default:
                assert(false) ;
                break ;
        }
        destroyBinding(pBinding) ;
    }    
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::addCrLfKeepAlive(IOsNatSocket*           pSocket, 
                                            const UtlString&        remoteIp,
                                            int                     remotePort,
                                            int                     keepAliveSecs,
                                            OsNatKeepaliveListener* pListener)
{
    OsLock lock(mMapsLock) ;
    UtlBoolean bSuccess = false ;
    UtlString serverAddress ;

    if (!doesBindingExist(pSocket, CRLF_KEEPALIVE, remoteIp, remotePort))
    {
        UtlString localHostIp ;
        pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
        OsSysLog::add(FAC_NAT, PRI_INFO, "Enabled CrLf keep alive %s:%d --> %s:%d every %d secs" ,
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                remoteIp.data(),
                remotePort,
                keepAliveSecs) ;

        OsNatAgentContext* pContext = new OsNatAgentContext(this) ;
        if (pContext)
        {
            pContext->type = CRLF_KEEPALIVE ;
            pContext->status = SUCCESS ;
            pContext->serverAddress = remoteIp ;
            pContext->serverPort = remotePort ;
            pContext->options = 0 ;
            memset(&pContext->transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;
            pContext->pSocket = pSocket ;
            pContext->pErrorTimer = getTimer(pContext) ;
            pContext->pRefreshTimer = getTimer(pContext) ;
            pContext->keepAliveSecs = keepAliveSecs;   
            pContext->abortCount = NAT_INITIAL_ABORT_COUNT ;
            pContext->errorTimeoutMSec = 0 ;
            pContext->refreshErrors = 0 ;
            pContext->port = PORT_NONE ;
            pContext->priority = 0 ;
            pContext->pKeepaliveListener = pListener ;
            pContext->lStartTimestamp = OsDateTime::getCurTimeInMS() ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;

            if (pContext->pKeepaliveListener)
            {
                pContext->pKeepaliveListener->OnKeepaliveStart(
                        populateKeepaliveEvent(pContext)); 
            }

            pContext->markTimeout() ;
    
            bSuccess = true ;
            if (keepAliveSecs > 0)
            {
                OsTime refreshAt(keepAliveSecs, 0) ;
                pContext->pRefreshTimer->periodicEvery(refreshAt, refreshAt) ;
            }
            else
            {
                destroyBinding(pContext) ;
            }
        }
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::removeCrLfKeepAlive(IOsNatSocket*    pSocket, 
                                               const UtlString& remoteIp,
                                               int              remotePort) 
{
    OsNatAgentContext* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bSuccess = FALSE ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if (    (pContext->pSocket == pSocket) &&
                (pContext->type == CRLF_KEEPALIVE) &&
                (pContext->serverPort == remotePort) &&
                (pContext->serverAddress.compareTo(remoteIp) == 0))
        {
            pRC = pContext ;
            break ;
        }
    }

    if (pRC)
    {
        UtlString localHostIp ;
        pSocket->getSocket()->getLocalHostIp(&localHostIp) ;      
        OsSysLog::add(FAC_NAT, PRI_INFO, "Disable CrLf keep alive %s:%d --> %s:%d" ,
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                remoteIp.data(),
                remotePort) ;

        destroyBinding(pRC) ;
        bSuccess = true ;
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::addStunKeepAlive(IOsNatSocket*           pSocket, 
                                            const UtlString&        remoteIp,
                                            int                     remotePort,
                                            int                     keepAliveSecs,
                                            OsNatKeepaliveListener* pListener) 
{
    OsLock lock(mMapsLock) ;
    UtlBoolean bSuccess = false ;

    if (!doesBindingExist(pSocket, STUN_KEEPALIVE, remoteIp, remotePort))
    {
        UtlString localHostIp ;
        OsSocket* pActualSocket = pSocket->getSocket();
        
        pActualSocket->getLocalHostIp(&localHostIp) ;
        int port = pActualSocket->getLocalHostPort();
        OsSysLog::add(FAC_NAT, PRI_INFO, "Enabled STUN keep alive %s:%d --> %s:%d every %d secs" ,
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                remoteIp.data(),
                remotePort,
                keepAliveSecs) ;     

        OsNatAgentContext* pContext = new OsNatAgentContext(this) ;
        if (pContext)
        {
            pContext->type = STUN_KEEPALIVE ;
            pContext->status = SUCCESS ;
            pContext->serverAddress = remoteIp ;
            pContext->serverPort = remotePort ;
            pContext->options = 0 ;
            memset(&pContext->transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;
            pContext->pSocket = pSocket ;
            pContext->pErrorTimer = getTimer(pContext) ;
            pContext->pRefreshTimer = getTimer(pContext) ;
            pContext->keepAliveSecs = keepAliveSecs;   
            pContext->abortCount = NAT_INITIAL_ABORT_COUNT ;
            pContext->errorTimeoutMSec = 0 ;
            pContext->refreshErrors = 0 ;
            pContext->port = PORT_NONE ;
            pContext->priority = 0 ;
            pContext->pKeepaliveListener = pListener ;
            pContext->lStartTimestamp = OsDateTime::getCurTimeInMS() ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;

            if (pContext->pKeepaliveListener)
            {
                pContext->pKeepaliveListener->OnKeepaliveStart(
                        populateKeepaliveEvent(pContext)); 
            }

            pContext->markTimeout() ;

            bSuccess = true ;
            if (keepAliveSecs > 0)
            {
                OsTime refreshAt(keepAliveSecs, 0) ;
                pContext->pRefreshTimer->periodicEvery(refreshAt, refreshAt) ;
            }
            else
            {
                destroyBinding(pContext) ;
            }
        }
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::removeStunKeepAlive(IOsNatSocket*    pSocket, 
                                               const UtlString& remoteIp,
                                               int              remotePort) 
{
    OsNatAgentContext* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bSuccess = FALSE ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if (    (pContext->pSocket == pSocket) &&
                (pContext->type == STUN_KEEPALIVE) &&
                (pContext->serverPort == remotePort) &&
                (pContext->serverAddress.compareTo(remoteIp) == 0))
        {
            pRC = pContext ;
            break ;
        }
    }

    if (pRC)
    {
        UtlString localHostIp ;
        pSocket->getSocket()->getLocalHostIp(&localHostIp) ;      
        OsSysLog::add(FAC_NAT, PRI_INFO, "Disable STUN keep alive %s:%d --> %s:%d" ,
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                remoteIp.data(),
                remotePort) ;

        destroyBinding(pRC) ;
        bSuccess = true ;
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::removeKeepAlives(IOsNatSocket* pSocket) 
{
    OsNatAgentContext* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bSuccess = FALSE ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if (    (pContext->pSocket == pSocket) &&
                ((pContext->type == STUN_KEEPALIVE) || (pContext->type == CRLF_KEEPALIVE)))
        {
            UtlString localHostIp ;
            pSocket->getSocket()->getLocalHostIp(&localHostIp) ;      
            OsSysLog::add(FAC_NAT, PRI_INFO, "Disable %s keep alive %s:%d --> %s:%d",
                    pContext->type == STUN_KEEPALIVE ? "STUN" : "CrLf",
                    localHostIp.data(),         
                    pSocket->getSocket()->getLocalHostPort(),
                    pContext->serverAddress.data(),
                    pContext->serverPort) ;

            destroyBinding(pContext) ;
            bSuccess = true ;
        }
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::removeStunProbes(IOsNatSocket* pSocket) 
{
    OsNatAgentContext* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bSuccess = FALSE ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if (    (pContext->pSocket == pSocket) &&
                (pContext->type == STUN_PROBE))
        {
            UtlString localHostIp ;
            pSocket->getSocket()->getLocalHostIp(&localHostIp) ;      
            OsSysLog::add(FAC_NAT, PRI_INFO, "Disable stun probe %s:%d --> %s:%d",
                    localHostIp.data(),         
                    pSocket->getSocket()->getLocalHostPort(),
                    pContext->serverAddress.data(),
                    pContext->serverPort) ;

            destroyBinding(pContext) ;
            bSuccess = true ;
        }
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::synchronize() 
{
    OsLock lock(sLock) ;

    if (isStarted() && (getCurrentTask() != this))
    {        
        // Send an event to ourself and wait for that message to be processed.
        OsEvent event ;
        OsRpcMsg msg(SYNC_MSG_TYPE, 0, event) ;
        if (postMessage(msg) == OS_SUCCESS)
        {
            event.wait() ;    
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

// Determines if probes of a higher priority are still outstanding
UtlBoolean OsNatAgentTask::areProbesOutstanding(IOsNatSocket* pSocket, int priority, bool bLongWait) 
{
    OsNatAgentContext* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bOutstanding = false ;
    UtlBoolean bAnySuccess = false ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    
    // First, check to see if we have any success cases.  If so, we
    // can exit earlier (if !bLongWait)
/*
    if (!bLongWait)
    {
        while (pKey = (UtlVoidPtr*)iterator())
        {        
            OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
            if (    (pContext->pSocket == pSocket) &&
                    (pContext->type == STUN_PROBE) &&
                    (pContext->status == SUCCESS))
            {
                bAnySuccess = true ;
                break ;
            }
        }
    }
    iterator.reset() ;
*/

    while (pKey = (UtlVoidPtr*)iterator())
    {        
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if (    (pContext->pSocket == pSocket) &&
                (pContext->type == STUN_PROBE))
        {
            if (    (pContext->priority > priority) &&
                    (pContext->status != FAILED))
            {
                if (pContext->refreshErrors < NAT_PROBE_SOFTABORT_COUNT)
                {                    
                    bOutstanding = true ;
                    break ;
                }
            }
        }
    }

    return bOutstanding ;
}

//////////////////////////////////////////////////////////////////////////////

// Dumps the probe results to the log file
void OsNatAgentTask::logProbeResults(IOsNatSocket* pSocket) 
{
    const char* cStatus[] =
    {
        "SUCCESS",
        "SENDING",
        "SENDING_ERROR",
        "RESENDING",
        "RESENDING_ERROR",
        "FAILED",
    } ;

    OsNatAgentContext* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlString output; 

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if (    (pContext->pSocket == pSocket) &&
                (pContext->type == STUN_PROBE))
        {
            UtlString lineItem ;
            const char* cStatusText = "N/A" ;

            if (pContext->status >= 0 && pContext->status < 6)
            {
                cStatusText = cStatus[pContext->status] ;
            }
            
            UtlString localIp ;
            pSocket->getSocket()->getLocalHostIp(&localIp) ;
            lineItem.format("%s %d %s:%d->%s:%d m=%s:%d r=%s:%d ec=%d ac=%d RTT=%d\n",
                cStatusText,
                pContext->priority,
                localIp.data(),
                pSocket->getSocket()->getLocalHostPort(),
                pContext->serverAddress.data(),
                pContext->serverPort,
                pContext->address.data(),
                pContext->port,
                pContext->relayAddress.data(),
                pContext->relayPort,
                pContext->refreshErrors,
                pContext->abortCount,
                (pContext->lEndTimestamp == 0) ? -1 : MAX(pContext->lEndTimestamp - pContext->lStartTimestamp, 16)) ;
                              
            output.append(lineItem.data()) ;
        }            
    }
    OsSysLog::add(FAC_NAT, PRI_INFO, "Probe Results:\n%s", output.data()) ;      
#if NAT_DEBUG >= 1
    printf("Probe Results:\n%s\n", output.data()) ;
#endif

}

//////////////////////////////////////////////////////////////////////////////

// Does a binding of the designated type/server exist 
UtlBoolean OsNatAgentTask::doesBindingExist(IOsNatSocket*          pSocket,
                                            NAT_AGENT_BINDING_TYPE type, 
                                            const UtlString&       serverIp,
                                            int                    serverPort)
{
    OsNatAgentContext* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bFound = false ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if (    (pContext->pSocket == pSocket) &&
                (pContext->type == type) &&
                (pContext->serverAddress.compareTo(serverIp) == 0) &&
                (pContext->serverPort == serverPort)    )
        {
            bFound = true ;
            break ;
        }        
    }

    return bFound ;
}

/* ============================ ACCESSORS ================================= */

UtlBoolean OsNatAgentTask::findContactAddress(const UtlString& destHost, 
                                              int              destPort, 
                                              UtlString*       pContactHost, 
                                              int*             pContactPort,
                                              int              iTimeoutMs) 
{
    OsNatAgentContext* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean  bFound = false ;
    int         iAttempts = 0 ;
    UtlBoolean  bTryAgain = false ;
    UtlBoolean  bTimedOut = false ;
    int         iMaxAttempts = 1 ;

    if (iTimeoutMs > 0)
    {
        // Figure out max attempts, rounding to next highest polling chunk (50ms)
        iMaxAttempts = (iTimeoutMs + (NAT_FIND_BINDING_POOL_MS-1)) / 
                NAT_FIND_BINDING_POOL_MS ;
    }

    bFound = findExternalBinding(destHost, destPort, pContactHost, pContactPort, iTimeoutMs, &bTimedOut) ;
    if (!bFound)
    {
        // Poll if the record isn't ready
        while ((iAttempts == 0 || bTryAgain) && iAttempts < iMaxAttempts)
        {
            bTryAgain = false ;
            iAttempts++ ;

            mMapsLock.acquire() ;
            UtlHashMapIterator iterator(mContextMap);
            while (pKey = (UtlVoidPtr*)iterator())
            {        
                OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();

                // Ignore uninteresting contexts
                if (    (pContext->type == TURN_ALLOCATION) || 
                        (pContext->type == CRLF_KEEPALIVE)  ||
                        (pContext->status == FAILED)    )
                {
                    continue ;
                }

                // Search for a match
                if (    (destPort == pContext->serverPort) &&
                        (destHost.compareTo(pContext->serverAddress, UtlString::ignoreCase) == 0))
                {
                    if (pContext->port != PORT_NONE)
                    {
                        if (pContactHost)
                        {
                            *pContactHost = pContext->address ;
                        }
                        if (pContactPort)
                        {
                            *pContactPort = pContext->port ;
                        }

                        bTryAgain = false ;
                        bFound = true ;
                        break ;
                    }
                    else
                    {
                        // If findExternalBinding timed out -- don't bother
                        // waiting any additional time
                        if (!bTimedOut && iMaxAttempts > 0)
                        {
                            bTryAgain = true ;
                        }
                    }        
                }
            }
            mMapsLock.release() ;

            if (bTryAgain && iAttempts < iMaxAttempts) 
            {
                OsTask::delay(NAT_FIND_BINDING_POOL_MS) ;
            }
        }
    }

    return bFound ;
}

//////////////////////////////////////////////////////////////////////////////

/**
 * Add an external binding (used for findContactAddress)
 */
void OsNatAgentTask::addExternalBinding(OsSocket*  pSocket,
                                        UtlString  remoteAddress,
                                        int        remotePort,
                                        UtlString  contactAddress,
                                        int        contactPort) 
{
    OsTime now ;
    OsTime expiration ;

    if ((remoteAddress.compareTo(contactAddress) == 0) && (remotePort == contactPort))
    {
        // Transport error
        return ;
    }

    UtlBoolean bUpdated = false ;    
   
    // Expire and external binding after 60 seconds
    OsDateTime::getCurTime(now) ;
    OsDateTime::getCurTime(expiration) ;
    if (contactAddress.isNull())
    {
        // If this is a place holder, expire in 1s (default timeout)
        expiration += OsTime(1, 0) ;
    }
    else
    {
        expiration += OsTime(NAT_BINDING_EXPIRATION_SECS, 0) ;
    }

    OsWriteLock lock(mExternalBindingMutex) ;

    // Update contact if found and GC old ones 
    UtlSListIterator itor(mExternalBindingsList) ;
    while (UtlContainable* pCont = itor())
    {
        NAT_AGENT_EXTERNAL_CONTEXT* pContext = (NAT_AGENT_EXTERNAL_CONTEXT*)
            ((UtlVoidPtr*) pCont)->getValue() ;

        if (    (pContext->pSocket == pSocket) &&
                (pContext->remoteAddress.compareTo(remoteAddress) == 0) &&
                (pContext->remotePort == remotePort)    )
        {
            pContext->contactAddress = contactAddress ;
            pContext->contactPort = contactPort ;
            pContext->expiration = expiration ;
            bUpdated = true ;
        }
        else if (pContext->expiration < now)
        {
            mExternalBindingsList.destroy(pCont) ;
            delete pContext ;
        }
    }

    if (!bUpdated)
    {
        NAT_AGENT_EXTERNAL_CONTEXT* pContext = new 
                NAT_AGENT_EXTERNAL_CONTEXT ;

        pContext->pSocket = pSocket ;
        pContext->remoteAddress = remoteAddress ;
        pContext->remotePort = remotePort ;
        pContext->contactAddress = contactAddress ;
        pContext->contactPort = contactPort ;
        pContext->expiration = expiration ;

        mExternalBindingsList.append(new UtlVoidPtr(pContext)) ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::clearExternalBinding(OsSocket*  pSocket,
                                          UtlString  remoteAddress,
                                          int        remotePort,
                                          bool       bOnlyIfEmpty) 
{    
    OsWriteLock lock(mExternalBindingMutex) ;

    UtlSListIterator itor(mExternalBindingsList) ;
    while (UtlContainable* pCont = itor())
    {
        NAT_AGENT_EXTERNAL_CONTEXT* pContext = (NAT_AGENT_EXTERNAL_CONTEXT*)
                ((UtlVoidPtr*) pCont)->getValue() ;

        if (    (pContext->remoteAddress.compareTo(remoteAddress) == 0) &&
                    (pContext->remotePort == remotePort)    )
        {
            if (bOnlyIfEmpty)
            {
                if (!pContext->contactAddress.isNull())
                {
                    mExternalBindingsList.destroy(pCont) ;
                    delete pContext ;                   
                }
            }
            else
            {
                mExternalBindingsList.destroy(pCont) ;
                delete pContext ;                   
            }
            break ;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::findExternalBinding(const UtlString& destHost, 
                                               int              destPort, 
                                               UtlString*       pContactHost, 
                                               int*             pContactPort,
                                               int              iTimeoutMs,
                                               UtlBoolean*      pTimedOut)
{
    UtlBoolean  bFound = false ;
    int         iAttempts = 0 ;
    UtlBoolean  bTryAgain = false ;
    int         iMaxAttempts = 1 ;
    
    if (iTimeoutMs > 0)
    {
        // Figure out max attempts, rounding to next highest polling chunk (50ms)
        iMaxAttempts = (iTimeoutMs + (NAT_FIND_BINDING_POOL_MS-1)) / 
                NAT_FIND_BINDING_POOL_MS ;
    }

    // Poll for upto 1s if the record isn't ready
    while ((iAttempts == 0 || bTryAgain) && iAttempts < iMaxAttempts)
    {
        bTryAgain = false ;
        iAttempts++ ;               
        OsTime now ;
        OsDateTime::getCurTime(now) ;
    
        mExternalBindingMutex.acquireRead() ;
        // Finding matching contact
        UtlSListIterator itor(mExternalBindingsList) ;
        while (UtlContainable* pCont = itor())
        {
            NAT_AGENT_EXTERNAL_CONTEXT* pContext = (NAT_AGENT_EXTERNAL_CONTEXT*)
                ((UtlVoidPtr*) pCont)->getValue() ;

            if (    (pContext->expiration > now) &&
                    (pContext->remoteAddress.compareTo(destHost) == 0) &&
                    (pContext->remotePort == destPort)    )
            {
                if (pContext->contactAddress.isNull())
                {
                    if (iMaxAttempts > 0)
                    {
                        bTryAgain = true ;
                    }
                }
                else
                {
                    if (pContactHost)
                        *pContactHost = pContext->contactAddress ;
                    if (pContactPort)
                        *pContactPort = pContext->contactPort ;

                    bFound = true ;
                    bTryAgain = false ;
                }
                break ;
            }
        }
        mExternalBindingMutex.releaseRead() ;

        if (bTryAgain && iAttempts < iMaxAttempts) 
        {
            OsTask::delay(NAT_FIND_BINDING_POOL_MS) ;
        }
    }

    // Set the timed out flag
    if (pTimedOut)
    {
        *pTimedOut = (bTryAgain && !bFound) ;
    }

    return bFound ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


UtlBoolean OsNatAgentTask::sendTurnSendRequest(IOsNatSocket* pSocket,
                                               const char*   szFinalAddress,
                                               int           iFinalPort,
                                               const char*   pPayload,
                                               int           nPayload) 
{

    UtlBoolean bRC = false ;
    OsLock lock(mMapsLock) ;

    OsNatAgentTurnContext* pBinding = (OsNatAgentTurnContext*) 
            getBinding(pSocket, TURN_ALLOCATION) ;
    if (pBinding)
    {
        UtlString localHostIp ;
        pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
        OsSysLog::add(FAC_NAT, PRI_INFO, "Sending Turn Request %s:%d -> %s:%d -> %s:%d",
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                pBinding->serverAddress.data(), 
                pBinding->serverPort,
                szFinalAddress,
                iFinalPort) ;

        TurnMessage msgSend ;

        msgSend.allocTransactionId() ;
        msgSend.setType(MSG_TURN_SEND_REQUEST) ;
        msgSend.setDestinationAddress(szFinalAddress, iFinalPort) ;    
        if (!pBinding->username.isNull())
            msgSend.setUsername(pBinding->username) ;
        if (!pBinding->password.isNull()) 
            msgSend.setPassword(pBinding->password) ;

        msgSend.setData(pPayload, nPayload) ;
        bRC = sendMessage(&msgSend, pSocket, pBinding->serverAddress, pBinding->serverPort, TURN_PACKET) ;
    }
    else
    {
        UtlString localHostIp ;
        pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
        OsSysLog::add(FAC_NAT, PRI_ERR, "Failed to send Turn Request %s:%d -> TURN SERVER -> %s:%d, no binding",
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                szFinalAddress,
                iFinalPort) ;
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::sendMessage(StunMessage*       pMsg, 
                                       IOsNatSocket*      pSocket, 
                                       const UtlString&   toAddress, 
                                       unsigned short     toPort,
                                       OS_NAT_PACKET_TYPE packetType)
{
    UtlBoolean bSuccess = false ;

    char cEncoded[NAT_MAX_STURN_MSG_LENGTH] ;
    size_t length ;

    if (pMsg->encode(cEncoded, sizeof(cEncoded), length))
    {
        if (OsSysLog::willLog(FAC_NAT, PRI_DEBUG))
        {
            UtlString tmp, output, localHostIp ;
            pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
            StunUtils::debugDump(cEncoded, length, tmp) ;
            output.format("STUN/TURN OUT %s:%d -> %s:%d:\n%s",
                    localHostIp.data(),
                    pSocket->getSocket()->getLocalHostPort(),
                    toAddress.data(),
                    toPort,
                    tmp.data()) ;
            OsSysLog::add(FAC_NAT, PRI_DEBUG, "%s", output.data()) ;
#if NAT_DEBUG >= 2
            printf("%s\n", output.data()) ;
#endif
        }

        // The IOsNatSocket keeps track of 
        // the last read/write times, but we need to write to the socket
        // without updating the timestamp.  Added a new function to the
        // IOsNatSocket interface that should be implemented as
        // just a write to the socket, without timestamping.
        if (pSocket->socketWrite(cEncoded, (int) length, toAddress, toPort, packetType) > 0)
        {
            bSuccess = true ;
        }
    }
    else
    {
        assert(false) ;
    }

    // According to latest STUN drafts -- we are not supposed to send out 
    // new STUN requests faster then every 20ms due to limitations in some
    // NAT/Firewalls.  For now, we are throttling all initial requests by 
    // delaying here.
    if (pMsg->isRequest())
    {
        unsigned short type = pMsg->getType() ;
        if (    type != MSG_TURN_SEND_REQUEST &&
                type != MSG_TURN_ACTIVE_DESTINATION_REQUEST &&
                type != MSG_TURN_CLOSE_BINDING_REQUEST &&
                type != MSG_STUN_SHARED_SECRET_REQUEST  )
        {
            OsTask::delay(20) ;
        }
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentContext* OsNatAgentTask::getBinding(IOsNatSocket* pSocket, NAT_AGENT_BINDING_TYPE type) 
{
    OsNatAgentContext* pRC = NULL ;
    UtlHashMapIterator iterator(mContextMap);
    UtlVoidPtr* pKey;

    while (pKey = (UtlVoidPtr*)iterator())
    {        
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if ((pContext->pSocket == pSocket) && (pContext->type == type))
        {
            pRC = pContext ;
            break ;
        }
    }
    return pRC ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentContext* OsNatAgentTask::getBinding(OsNatAgentContext* pBinding) 
{
    OsNatAgentContext* pRC = NULL ;
    UtlHashMapIterator iterator(mContextMap);
    UtlVoidPtr* pKey;

    while (pKey = (UtlVoidPtr*)iterator())
    {
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if (pBinding == pContext)
        {
            pRC = pContext ;
            break ;
        }
    }

    return pRC ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentContext* OsNatAgentTask::getBinding(STUN_TRANSACTION_ID* pId) 
{
    OsNatAgentContext* pRC = NULL ;
    UtlHashMapIterator iterator(mContextMap);
    UtlVoidPtr* pKey;

    while (pKey = (UtlVoidPtr*)iterator())
    {
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if (memcmp(&pId->id[0], &pContext->transactionId.id[0], sizeof(STUN_TRANSACTION_ID)) == 0)
        {
            pRC = pContext ;
            break ;
        }
    }

    return pRC ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentTurnContext* OsNatAgentTask::getTurnBinding(STUN_TRANSACTION_ID* pId) 
{
    return dynamic_cast<OsNatAgentTurnContext*>(getBinding(pId)) ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentTurnContext* OsNatAgentTask::getTurnBinding(IOsNatSocket* pSocket, NAT_AGENT_BINDING_TYPE type)
{
    return dynamic_cast<OsNatAgentTurnContext*>(getBinding(pSocket, type)) ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentTurnContext* OsNatAgentTask::getTurnBinding(OsNatAgentContext* pContext) 
{
    return dynamic_cast<OsNatAgentTurnContext*>(getBinding(pContext)) ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentNCContext* OsNatAgentTask::getNCBinding(STUN_TRANSACTION_ID* pId) 
{
    return dynamic_cast<OsNatAgentNCContext*>(getBinding(pId)) ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentNCContext* OsNatAgentTask::getNCBinding(IOsNatSocket* pSocket, NAT_AGENT_BINDING_TYPE type) 
{
    return dynamic_cast<OsNatAgentNCContext*>(getBinding(pSocket, type)) ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatAgentNCContext* OsNatAgentTask::getNCBinding(OsNatAgentContext* pContext) 
{
    return dynamic_cast<OsNatAgentNCContext*>(getBinding(pContext)) ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::doesProbeBindingExist(IOsNatSocket* pSocket, 
                                                 const char*   szIp, 
                                                 int           port, 
                                                 const char*   szRelayIp, 
                                                 int           relayPort) 
{
    UtlBoolean bFound = false ;
    UtlHashMapIterator iterator(mContextMap);
    UtlVoidPtr* pKey;

    while (pKey = (UtlVoidPtr*)iterator())
    {        
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if ((pContext->pSocket == pSocket) && (pContext->type == STUN_PROBE))
        {
            if (    (pContext->serverAddress.compareTo(szIp) == 0) && 
                    (pContext->serverPort == port))
            {
                if ((szRelayIp == NULL) || (pContext->relayAddress.compareTo(szRelayIp) == 0))
                {
                    bFound = true ;
                    break ;
                }
            }
        }
    }
    return bFound ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::destroyBinding(OsNatAgentContext* pBinding) 
{
    UtlHashMapIterator iterator(mContextMap);
    UtlVoidPtr* pKey;   
    
    while (pKey = (UtlVoidPtr*)iterator())
    {
        OsNatAgentContext* pContext = (OsNatAgentContext*) pKey->getValue();
        if (pContext == pBinding)
        {                
            mContextMap.destroy(pKey) ;
            if (pBinding->pErrorTimer)
                releaseTimer(pBinding->pErrorTimer) ;            
            if (pBinding->pRefreshTimer) 
                releaseTimer(pBinding->pRefreshTimer) ;
            
            if (pBinding->pKeepaliveListener)
            {
                pBinding->pKeepaliveListener->OnKeepaliveStop(
                        populateKeepaliveEvent(pBinding)) ; 
            }

            delete pBinding ;
            break ;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

OsTimer* OsNatAgentTask::getTimer(OsNatAgentContext* pBinding) 
{
    OsTimer* pTimer = NULL ;

    UtlVoidPtr* pWrappedTimer = (UtlVoidPtr*) mTimerPool.get() ;
    if (pWrappedTimer)
    {
        pTimer = (OsTimer*) pWrappedTimer->getValue() ;
        delete pWrappedTimer ;
    }
    else
    {
        pTimer = new OsTimer(getMessageQueue(), 0) ;
    }

    assert(pTimer != NULL) ;
    if (pTimer)
    {

        OsQueuedEvent* pEvent = (OsQueuedEvent*) pTimer->getNotifier() ;
        pEvent->setUserData((int) pBinding) ;
    }

    return pTimer ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::performNatClassification(IOsNatSocket*              pSocket, 
                                              const UtlString&           stunServer,
                                              int                        stunPort,     
                                              int                        refreshPeriod,
                                              OsNatAgentContextListener* pListener) 
{
    OsLock lock(mMapsLock) ;

    UtlString serverAddress ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NAT, PRI_INFO, "NAT classification check for %s:%d with server %s:%d (refresh=%d)",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort(),
            stunServer.data(),
            stunPort,
            refreshPeriod) ;

    assert(pSocket) ;

    if (    pSocket && portIsValid(stunPort) && (stunServer.length() > 0) &&
            OsSocket::getHostIpByName(stunServer, &serverAddress) && 
            OsSocket::isIp4Address(serverAddress))
    {
        OsNatAgentNCContext* pContext = new OsNatAgentNCContext(this,
                pSocket, pListener) ;
        if (pContext)
        {
            pContext->serverAddress = serverAddress ;
            pContext->serverPort = stunPort ;
            pContext->lStartTimestamp = OsDateTime::getCurTimeInMS() ;
            pContext->keepAliveSecs = refreshPeriod ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;

            pContext->start() ;
        }
        else
        {
            // Unable to allocate context
            assert(false) ;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::trace(OsSysLogPriority priority, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    /* Guess we need no more than 128 bytes. */
    int n, size = 128;
    char *p;

    p = (char*) malloc(size) ;
    
    while (p != NULL)
    {
        /* Try to print in the allocated space. */
#ifdef _WIN32
        n = _vsnprintf (p, size, format, args);
#else
        n = vsnprintf (p, size, format, args);
#endif

        /* If that worked, return the string. */
        if (n > -1 && n < size)
        {
            break;
        }
        /* Else try again with more space. */
        if (n > -1)    /* glibc 2.1 */
            size = n+1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        if ((p = (char*) realloc (p, size)) == NULL)
        {
            break;
        }
    }

    if (p != NULL)
    {
        OsSysLog::add(FAC_NAT, priority, "%s", p) ;
#if NAT_DEBUG >= 1
        printf("%s\n", p) ;
#endif
        free(p) ;
    }

    va_end(args) ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::releaseTimer(OsTimer* pTimer) 
{
    assert(pTimer) ;
    if (pTimer)
    {
        if (pTimer->getState() == OsTimer::STARTED)
        {
            pTimer->stop() ;
            purgeTimers(pTimer, NULL) ;

        }
        mTimerPool.insert(new UtlVoidPtr(pTimer)) ;
    }
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::sendStunRequest(OsNatAgentContext* pBinding, UtlBoolean bNewTransaction) 
{
    UtlBoolean bSuccess = false ;
    StunMessage msgSend ;

    if (bNewTransaction)   
    {
        msgSend.allocTransactionId() ;
        msgSend.getTransactionId(&pBinding->transactionId) ;
    }
    else
    {
        msgSend.allocTransactionId() ;
        msgSend.setTransactionId(pBinding->transactionId) ;
    }

    msgSend.setType(MSG_STUN_BIND_REQUEST) ;
    msgSend.setRequestXorOnly() ;

    if (pBinding->options & ATTR_CHANGE_FLAG_PORT)
    {
        msgSend.setChangePort(true) ;
    }

    if (pBinding->options & ATTR_CHANGE_FLAG_IP)
    {
        msgSend.setChangeIp(true) ;
    }

    if (pBinding->priority)
        msgSend.setPriority(pBinding->priority) ;

    if ((pBinding->relayAddress.length() && pBinding->relayPort > 0))
    {   
        char cEncoded[NAT_MAX_STURN_MSG_LENGTH] ;
        size_t length ;
        if (msgSend.encode(cEncoded, sizeof(cEncoded), length))
        {
            if (sendTurnSendRequest(pBinding->pSocket, pBinding->serverAddress, pBinding->serverPort, cEncoded, length))
            {
                bSuccess = true ;
            }
            // Setup resend-timer for no-response
            armErrorTimer(pBinding) ;
        }
        else
        {
            assert(false) ;
        }
    }
    else
    {
        // Send message
        if (sendMessage(&msgSend, pBinding->pSocket, pBinding->serverAddress, pBinding->serverPort, STUN_PROBE_PACKET))
        {
            bSuccess = true ;
        }
        // Setup resend-timer for no-response
        armErrorTimer(pBinding) ;
    }

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::sendTurnRequest(OsNatAgentTurnContext* pBinding, UtlBoolean bNewTransaction) 
{
    UtlBoolean bSuccess = false ;
    TurnMessage msgSend ;

    if (bNewTransaction)
    {
        msgSend.allocTransactionId() ;
        msgSend.getTransactionId(&pBinding->transactionId) ;
    }
    else
    {   
        msgSend.allocTransactionId() ;
        msgSend.setTransactionId(pBinding->transactionId) ;
    }

    msgSend.setType(MSG_TURN_ALLOCATE_REQUEST) ;
    msgSend.setRequestXorOnly() ;
    msgSend.setLifetime(pBinding->keepAliveSecs * 2) ;

    if (!pBinding->username.isNull()) 
        msgSend.setUsername(pBinding->username) ;    
    if (!pBinding->password.isNull()) 
        msgSend.setPassword(pBinding->password) ;
       
    if (sendMessage(&msgSend, pBinding->pSocket, pBinding->serverAddress, pBinding->serverPort, TURN_PACKET))
    {
        bSuccess = true ;
    }

    // Setup resend-timer for no-response
    armErrorTimer(pBinding) ;

    return bSuccess ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::armErrorTimer(OsNatAgentContext* pBinding)
{
    UtlBoolean bSet = false ;

    if (pBinding->errorTimeoutMSec > 0)
    {
        // Calc backoff timeout (set ceiling at 1/2 keepalive period)
        int timeoutMsec = pBinding->errorTimeoutMSec ;
        for (int i=0; i<pBinding->refreshErrors; i++)
        {
            timeoutMsec *=2 ;

            // Never wait longer then 1/2 the keepAlivePeriod
            if (pBinding->keepAliveSecs > 0)
            {
                if (timeoutMsec > (pBinding->keepAliveSecs * 500))
                {
                    timeoutMsec = pBinding->keepAliveSecs * 500 ;
                    break ;
                }
            }
            else if (timeoutMsec > (NAT_DEFAULT_KEEPALIVE_SEC*250))
            {
                timeoutMsec = NAT_DEFAULT_KEEPALIVE_SEC * 250 ;
                break ;
            }
        }

        // Reset the timer
        pBinding->pErrorTimer->stop();
        purgeTimers(pBinding->pErrorTimer, pBinding) ;
        OsTime errorAt(0, timeoutMsec * OsTime::USECS_PER_MSEC) ;
        OsQueuedEvent* pEvent = (OsQueuedEvent*) pBinding->pErrorTimer->getNotifier() ;
        pEvent->setUserData((int) pBinding) ;
        pBinding->pErrorTimer->oneshotAfter(errorAt) ;

        bSet = true ;
    }

    return bSet ;
}

//////////////////////////////////////////////////////////////////////////////

OsNatKeepaliveEvent OsNatAgentTask::populateKeepaliveEvent(OsNatAgentContext* pContext)
{
    OsNatKeepaliveEvent event ;

    if (pContext)
    {
        if (pContext->type == CRLF_KEEPALIVE)
            event.type = OS_NAT_KEEPALIVE_CRLF ;
        else if (pContext->type == STUN_KEEPALIVE)
            event.type = OS_NAT_KEEPALIVE_STUN ;
        else
            event.type = OS_NAT_KEEPALIVE_INVALID ;

        event.remoteAddress = pContext->serverAddress ;
        event.remotePort = pContext->serverPort ;
        event.keepAliveSecs = pContext->keepAliveSecs ;
        event.mappedAddress = pContext->address ;
        event.mappedPort = pContext->port ;
    }

    return event ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::dumpContext(UtlString* pResults, OsNatAgentContext* pBinding) 
{
    const char* cStatus[] =
    {
        "SUCCESS",
        "SENDING",
        "SENDING_ERROR",
        "RESENDING",
        "RESENDING_ERROR",
        "FAILED",
    } ;

    char cBuf[2048] ;

    if (pBinding)
    {
        UtlString socketHostIp ;
        int socketPort = -1 ;

        if (pBinding->pSocket)
        {
            pBinding->pSocket->getSocket()->getLocalHostIp(&socketHostIp) ;
            socketPort = pBinding->pSocket->getSocket()->getLocalHostPort() ;
        }

        sprintf(cBuf, 
                " BindingType: %d\n"
                " Status     : %s\n"
                " Server Addr: %s\n"
                " Server Port: %d\n"
                " Servers Opt: %d\n"
                " Transaction: %s\n"
                " Socket     : %p (%s:%d)\n"
                " Error Timer: %p\n"
                " Refresh T  : %p\n"
                " KeepAlive  : %d\n"
                " Errors     : %d\n"
                " Address    : %s\n"
                " Port       : %d\n"
//                " Username   : %s\n"
//                " Password   : %s\n"
                " Priority   : %d\n",
                pBinding->type,
                cStatus[pBinding->status],
                pBinding->serverAddress.data(),
                pBinding->serverPort,
                pBinding->options,
                "TBD",
                pBinding->pSocket,
                socketHostIp.data(),
                socketPort,
                pBinding->pErrorTimer,
                pBinding->pRefreshTimer,
                pBinding->keepAliveSecs,
                pBinding->refreshErrors,
                pBinding->address.data(),
                pBinding->port,
//                pBinding->username.data(),
//                pBinding->password.data(),
                pBinding->priority) ;

        pResults->append(cBuf) ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::purgeTimers(OsTimer* pTimer, OsNatAgentContext* pContext) 
{
    OsMsgQ* pMsgQ = getMessageQueue() ;
    if (pMsgQ)
        pMsgQ->purge(purgeMsgQCallback, pTimer, pContext) ;
}

//////////////////////////////////////////////////////////////////////////////

UtlBoolean OsNatAgentTask::purgeMsgQCallback(const OsMsg& rMsg, 
                                             void* pUserData1,      // event data / timer
                                             void* pUserData2)      // user data / context
{
    UtlBoolean bPurge = false ;

    if (rMsg.getMsgType() == OsMsg::OS_EVENT)
    {
        void* pEventData = NULL ;
        void* pUserData = NULL ;
        if (    (((OsEventMsg&)rMsg).getUserData((int&) pUserData) == OS_SUCCESS) &&
                (((OsEventMsg&)rMsg).getEventData((int&) pEventData) == OS_SUCCESS))
        {
            if (pUserData2 == NULL)
            {
                bPurge = (pUserData1 == pEventData) ;
            }
            else
            {
                bPurge = ((pUserData1 == pEventData) && (pUserData2 == pUserData)) ;
            }
            
        }
    }
    return bPurge ;
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::onStunRTT(long ms)
{
    int origTimeout = sStunTimeoutMS ;

    if (ms <= 0)
        ms = NAT_RESPONSE_TIMEOUT_MS_MIN ;

    if (ms < sStunTimeoutMS)
    {
        sStunTimeoutMS += (int) ((1.0/5.0) * (ms - sStunTimeoutMS)) ;
    }
    else if (ms > sStunTimeoutMS)
    {
        sStunTimeoutMS = ms ;
    }

    if (sStunTimeoutMS < NAT_RESPONSE_TIMEOUT_MS_MIN)
        sStunTimeoutMS = NAT_RESPONSE_TIMEOUT_MS_MIN ;

    if (sStunTimeoutMS > NAT_RESPONSE_TIMEOUT_MS_MAX)
        sStunTimeoutMS = NAT_RESPONSE_TIMEOUT_MS_MAX ;

    if (origTimeout != sStunTimeoutMS)
    {
        OsSysLog::add(FAC_NAT, PRI_INFO, "Stun RTT changed from %d to %d",
            origTimeout, sStunTimeoutMS) ;
#if NAT_DEBUG >= 1
        printf("Stun RTT changed from %d to %d\n",
            origTimeout, sStunTimeoutMS) ;
#endif
    }
}

//////////////////////////////////////////////////////////////////////////////

void OsNatAgentTask::onTurnRTT(long ms)
{
    int origTimeout = sTurnTimeoutMS ;

    if (ms <= 0)
        ms = NAT_RESPONSE_TIMEOUT_MS_MIN ;

    if (ms < sTurnTimeoutMS)
    {
        sTurnTimeoutMS += (int) ((1.0/5.0) * (ms - sTurnTimeoutMS)) ;
    }
    else if (ms > sTurnTimeoutMS)
    {
        sTurnTimeoutMS = ms ;
    }

    if (sTurnTimeoutMS < NAT_RESPONSE_TIMEOUT_MS_MIN)
        sTurnTimeoutMS = NAT_RESPONSE_TIMEOUT_MS_MIN ;

    if (sTurnTimeoutMS > NAT_RESPONSE_TIMEOUT_MS_MAX)
        sTurnTimeoutMS = NAT_RESPONSE_TIMEOUT_MS_MAX ;

    if (origTimeout != sTurnTimeoutMS)
    {
        OsSysLog::add(FAC_NAT, PRI_INFO, "Turn RTT changed from %d to %d",
            origTimeout, sTurnTimeoutMS) ;
#if NAT_DEBUG >= 1
        printf("Turn RTT changed from %d to %d\n",
            origTimeout, sTurnTimeoutMS) ;
#endif
    }
}



/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


