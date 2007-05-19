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
#include "os/IStunSocket.h"
#include "os/TurnMessage.h"
#include "os/StunMessage.h"
#include "os/StunUtils.h"

#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "os/OsEvent.h"
#include "os/OsWriteLock.h"
#include "os/OsReadLock.h"
#include "os/OsTime.h"
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
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
OsMutex OsNatAgentTask::sLock(OsMutex::Q_FIFO) ;
OsNatAgentTask* OsNatAgentTask::spInstance = NULL ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsNatAgentTask::OsNatAgentTask()
    : OsServerTask("OsNatAgentTask-%d")
    , mMapsLock(OsMutex::Q_FIFO)
    , mExternalBindingMutex(OsRWMutex::Q_FIFO)
{

}

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
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
        mContextMap.destroy(pKey) ;
        releaseTimer(pContext->pTimer) ;
        delete pContext ;
    }

    // Clear Timers 
    UtlSListIterator listIterator(mTimerPool);
    while (pKey = (UtlVoidPtr*)iterator())
    {
        OsTimer* pTimer = (OsTimer*) pKey->getValue() ;
        mTimerPool.destroy(pKey) ;        
        delete pTimer ;
    }
}


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
                bHandled = handleStunMessage((NatMsg&) rMsg) ;
            }
            else if (((NatMsg&) rMsg).getType() == NatMsg::TURN_MESSAGE)
            {
                bHandled = handleTurnMessage((NatMsg&) rMsg) ;
            }
            else if (((NatMsg&) rMsg).getType() == NatMsg::EXPIRATION_MESSAGE)
            {
                NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) ((NatMsg&) rMsg).getContext() ;
                if (pContext)
                {
                    bHandled = handleTimerEvent(pContext) ;
                }
            }
            else
            {
                // Unknown message type
                assert(false) ;
            }
            break ;
        case SYNC_MSG_TYPE:
            bHandled = handleSynchronize((OsRpcMsg&) rMsg) ;
            break ;
        case OsMsg::OS_EVENT:
            {
                // Pull out context
                NAT_AGENT_CONTEXT* pContext = NULL ;
                OsStatus rc = ((OsEventMsg&)rMsg).getUserData((int&) pContext) ;
                assert(rc == OS_SUCCESS) ;    

                if ((rc == OS_SUCCESS) && (pContext != NULL))
                {   
                    NatMsg msg(NatMsg::EXPIRATION_MESSAGE, pContext) ;
                    postMessage(msg) ;
                }
            }
            break ;
    }

    return bHandled ;
}


UtlBoolean OsNatAgentTask::handleStunMessage(NatMsg& rMsg) 
{
    OsLock lock(mMapsLock) ;

    StunMessage msg ;
    size_t nBuffer = rMsg.getLength() ;
    char* pBuffer = rMsg.getBuffer() ;
    IStunSocket* pSocket = rMsg.getSocket() ;
    UtlString sendToAddress ;
    unsigned short sendToPort ;
    unsigned short unknownAttributes[STUN_MAX_UNKNOWN_ATTRIBUTES] ;
    size_t nUnknownAttributes ;

    if (OsSysLog::willLog(FAC_NET, PRI_DEBUG))
    {
        if (pBuffer != NULL && nBuffer > 0)
        {
            UtlString output ;
            StunUtils::debugDump(pBuffer, nBuffer, output) ;
            OsSysLog::add(FAC_NET, PRI_DEBUG, "Inbound STUN message (Crude Parse):\n%s\n",
                    output.data()) ;
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
                        {
                            respMsg.setSendXorOnly() ;
                        }

                        // Set Mapped Address
                        respMsg.setMappedAddress(rMsg.getReceivedIp(), rMsg.getReceivedPort()) ;

                        // Set Source Address
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

                        sendMessage(&respMsg, pSocket, sendToAddress, sendToPort) ;
                    }
                }
                break ;
            case MSG_STUN_BIND_RESPONSE:
                {                                        
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    NAT_AGENT_CONTEXT* pContext = getBinding(&transactionId) ;

                    if (pContext)
                    {
                        char mappedAddress[64] ;
                        unsigned short mappedPort ;

                        if (msg.getMappedAddress(mappedAddress, mappedPort))
                        {
                            markStunSuccess(pContext, mappedAddress, mappedPort) ;
                        }
                        else
                        {
                            markStunFailure(pContext) ;
                        }                   
                    }
                }
                break ;
            case MSG_STUN_BIND_ERROR_RESPONSE:
                {
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    NAT_AGENT_CONTEXT* pContext = getBinding(&transactionId) ;

                    if (pContext)
                    {
                        markStunFailure(pContext) ;
                    }
                }
                break ;
            case MSG_STUN_SHARED_SECRET_REQUEST:
                    // TODO: Send error
                    break ;
            case MSG_STUN_SHARED_SECRET_RESPONSE:
            case MSG_STUN_SHARED_SECRET_ERROR_RESPONSE:
                break ;
        }
    }

    if (pBuffer)
    {
        free(pBuffer) ;
    }

    return true ;
}


UtlBoolean OsNatAgentTask::handleTurnMessage(NatMsg& rMsg) 
{
    OsLock lock(mMapsLock) ;

    TurnMessage msg;
    TurnMessage respMsg ;
    size_t nBuffer = rMsg.getLength() ;
    char* pBuffer = rMsg.getBuffer() ;
    IStunSocket* pSocket = rMsg.getSocket() ;
    STUN_TRANSACTION_ID transactionId ;
    STUN_MAGIC_ID       magicId ;

    if (OsSysLog::willLog(FAC_NET, PRI_DEBUG))
    {
        if (pBuffer != NULL && nBuffer > 0)
        {
            UtlString output ;
            StunUtils::debugDump(pBuffer, nBuffer, output) ;
            OsSysLog::add(FAC_NET, PRI_DEBUG, "Inbound TURN message(Crude Parse):\n%s\n",
                    output.data()) ;
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
                sendMessage(&respMsg, pSocket, rMsg.getReceivedIp(), rMsg.getReceivedPort()) ;
                break ;
            case MSG_TURN_ALLOCATE_RESPONSE:
                {
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    NAT_AGENT_CONTEXT* pContext = getBinding(&transactionId) ;

                    if ((pContext) && pContext->type == TURN_ALLOCATION)
                    {
                        char relayAddress[64] ;
                        unsigned short relayPort ;

                        if (msg.getMappedAddress(relayAddress, relayPort))
                        {
                            markTurnSuccess(pContext, relayAddress, relayPort) ;
                        }
                        else
                        {
                            markTurnFailure(pContext) ;
                        }
                    }
                }
                break ;
            case MSG_TURN_ALLOCATE_ERROR_RESPONSE:
                {
                    STUN_TRANSACTION_ID transactionId ;
                    msg.getTransactionId(&transactionId) ;
                    NAT_AGENT_CONTEXT* pContext = getBinding(&transactionId) ;

                    if ((pContext) && pContext->type == TURN_ALLOCATION)
                    {
                        markTurnFailure(pContext) ;
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
                sendMessage(&respMsg, pSocket, rMsg.getReceivedIp(), rMsg.getReceivedPort()) ;
                break ;
            case MSG_TURN_SEND_RESPONSE:
                // Drop send response -- not much we can do right now.
                break ;
            case MSG_TURN_SEND_ERROR_RESPONSE:
                // Drop error response -- not much we can do right now.
                break ;
            case MSG_TURN_DATA_INDICATION:
                // Dropping data indication -- only locking onto a single 
                // address for now
                break ;
            default:
                // Unknown message type
                assert(false) ;
                break ;
        }
    }

    if (pBuffer)
    {
        free(pBuffer) ;
    }

    return true ;
}


UtlBoolean OsNatAgentTask::handleSynchronize(OsRpcMsg& rMsg) 
{
    OsEvent* pEvent = rMsg.getEvent() ;
    pEvent->signal(0) ;

    return true ;
}


UtlBoolean OsNatAgentTask::handleTimerEvent(NAT_AGENT_CONTEXT* pContext) 
{    
    OsLock lock(mMapsLock) ;

    // Refresh the socket
    if ((pContext) && (pContext = getBinding(pContext))) 
    {
        switch (pContext->type)
        {
            case STUN_PROBE:    
            case STUN_DISCOVERY:
                handleStunTimeout(pContext) ;            
                break ;
            case TURN_ALLOCATION:
                handleTurnTimeout(pContext) ;
                break ;
            case CRLF_KEEPALIVE:
                handleCrLfKeepAlive(pContext) ;
                break ;
            case STUN_KEEPALIVE:
                handleStunKeepAlive(pContext) ;
                break ;
            default:
                assert(false) ;
                break ;
        }
    }

    return true ;
}


void OsNatAgentTask::handleStunTimeout(NAT_AGENT_CONTEXT* pContext) 
{
    assert(pContext) ;
    if (pContext)
    {
        switch (pContext->status)
        {
            case SUCCESS:
                // Send keep-alive, set state to resending
                pContext->status = RESENDING ;
                
                if (!sendStunRequest(pContext))
                {
                    markStunFailure(pContext) ;
                }
                break ;
            case SENDING:
                // Migrate to sending_error and resend
                pContext->status = SENDING_ERROR ;
                if (!sendStunRequest(pContext))
                {
                    markStunFailure(pContext) ;
                }                                
                break ;
            case SENDING_ERROR:
                // If we have hit the max resends, fail otherwise resend
                pContext->refreshErrors++ ;
                if (pContext->refreshErrors < pContext->abortCount)
                {
                    if (!sendStunRequest(pContext))
                    {
                        markStunFailure(pContext) ;
                    }
                } 
                else
                {
                    markStunFailure(pContext) ;
                }
                break ;
            case RESENDING:
                // Migrate to resend_error and resend
                pContext->status = RESENDING_ERROR ;
                pContext->refreshErrors++ ;

                if (!sendStunRequest(pContext))
                {
                    markStunFailure(pContext) ;
                }
                
                break ;
            case RESENDING_ERROR:                
                pContext->refreshErrors++ ;
                if (pContext->refreshErrors < NAT_RESEND_ABORT_COUNT)
                {                    
                    if (!sendStunRequest(pContext))
                    {
                        markStunFailure(pContext) ;
                    }                    
                }
                else
                {
                    markStunFailure(pContext) ;
                }
                break ;
            case FAILED:
                // Nothing to do
                break ;
        }
    }
}


void OsNatAgentTask::handleTurnTimeout(NAT_AGENT_CONTEXT* pContext) 
{
    OsTime errorAt(0, NAT_RESPONSE_TIMEOUT_MS * OsTime::USECS_PER_MSEC) ;
    assert(pContext) ;
    if (pContext)
    {
        switch (pContext->status)
        {
            case SUCCESS:
                // Send keep-alive, set state to resending
                pContext->status = RESENDING ;

                pContext->pTimer->stop() ;
                pContext->pTimer->periodicEvery(errorAt, errorAt) ;

                if (!sendTurnRequest(pContext))
                {
                    markTurnFailure(pContext) ;
                }
                break ;
            case SENDING:
                // Migrate to sending_error and resend
                pContext->status = SENDING_ERROR ;
                if (!sendTurnRequest(pContext))
                {
                    markTurnFailure(pContext) ;
                }                                
                break ;
            case SENDING_ERROR:
                // If we have hit the max resends, fail otherwise resend
                pContext->refreshErrors++ ;
                if (pContext->refreshErrors < pContext->abortCount)
                {
                    if (!sendTurnRequest(pContext))
                    {
                        markTurnFailure(pContext) ;
                    }
                } 
                else
                {
                    markTurnFailure(pContext) ;
                }
                break ;
            case RESENDING:
                // Migrate to resend_error and resend
                pContext->status = RESENDING_ERROR ;
                pContext->refreshErrors++ ;

                if (!sendTurnRequest(pContext))
                {
                    markTurnFailure(pContext) ;
                }
                
                break ;
            case RESENDING_ERROR:                
                pContext->refreshErrors++ ;
                if (pContext->refreshErrors < NAT_RESEND_ABORT_COUNT)
                {                    
                    if (!sendTurnRequest(pContext))
                    {
                        markTurnFailure(pContext) ;
                    }                    
                }
                else
                {
                    markTurnFailure(pContext) ;
                }
                break ;
            case FAILED:
                // Nothing to do
                break ;
        }
    }
}

#define STR_CRLF  "\r\n\0"
UtlBoolean OsNatAgentTask::handleCrLfKeepAlive(NAT_AGENT_CONTEXT* pContext) 
{
    UtlBoolean bRC = false ;
   
    // This is a bit of a hack -- The IStunSocket keeps track of the last
    // read/write times.  Instead of creating a 'special' write or some markers
    // to figure it the packet should update the last written timestamp, I am 
    // just calling the base class implementation directly.
    if (pContext->pSocket->getSocket()->write(STR_CRLF, 3, pContext->serverAddress, 
            pContext->serverPort) == 3)
    {
        bRC = true ;
    }
    else
    {
        if (pContext->pKeepaliveListener)
        {
            pContext->pKeepaliveListener->OnKeepaliveFailure(
                        populateKeepaliveEvent(pContext)); 
        }
    }

    return bRC ;
}


UtlBoolean OsNatAgentTask::handleStunKeepAlive(NAT_AGENT_CONTEXT* pContext) 
{
    UtlBoolean bRC = false ;

    if (sendStunRequest(pContext))
    {
        bRC = true ;
    }
    else
    {
        if (pContext->pKeepaliveListener)
        {
            pContext->pKeepaliveListener->OnKeepaliveFailure(
                    populateKeepaliveEvent(pContext)); 
        }
    }

    return bRC ;
}


UtlBoolean OsNatAgentTask::sendStunProbe(IStunSocket* pSocket,
                                         const UtlString&     stunServer,
                                         int                  stunPort,
                                         int                  priority) 
{
    OsLock lock(mMapsLock) ;

    bool bSuccess = false ;
    StunMessage msgSend ;
    UtlString serverAddress ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NET, PRI_INFO, "Stun probe for %s:%d with server %s:%d priority=%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort(),
            stunServer.data(),
            stunPort,
            priority) ;

    assert(pSocket) ;
    assert(portIsValid(stunPort)) ;
    assert(stunServer.length() > 0) ;

    if (    pSocket && portIsValid(stunPort) && (stunServer.length() > 0) &&
            OsSocket::getHostIpByName(stunServer, &serverAddress) && 
            OsSocket::isIp4Address(serverAddress))
    {
        NAT_AGENT_CONTEXT* pContext = new NAT_AGENT_CONTEXT ;
        if (pContext)
        {
            pContext->type = STUN_PROBE ;
            pContext->status = SENDING ;
            pContext->serverAddress = serverAddress ;
            pContext->serverPort = stunPort ;
            pContext->options = 0 ;
            memset(&pContext->transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;
            pContext->nOldTransactions = 0 ;
            for (int i=0; i<MAX_OLD_TRANSACTIONS; i++)
            {
                memset(&pContext->oldTransactionsIds[i], 0, sizeof(STUN_TRANSACTION_ID)) ;            
            }                
            pContext->pSocket = pSocket ;
            pContext->pTimer = getTimer() ;
            pContext->keepAliveSecs = 27 ;
            pContext->abortCount = NAT_PROBE_ABORT_COUNT ;
            pContext->refreshErrors = 0 ;            
            pContext->port = PORT_NONE ;
            pContext->priority = priority ;
            pContext->pKeepaliveListener = NULL ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;

            if (sendStunRequest(pContext))
            {
                OsTime errorAt(0, NAT_RESPONSE_TIMEOUT_MS * OsTime::USECS_PER_MSEC) ;
                OsQueuedEvent* pEvent = (OsQueuedEvent*) pContext->pTimer->getNotifier() ;
                pEvent->setUserData((int) pContext) ;

                pContext->pTimer->periodicEvery(errorAt, errorAt) ;
                bSuccess = true ;
            }
            else
            {
                // Unable to Send
                destroyBinding(pContext) ;
            }            
        }
        else
        {
            // Unable to allocate context
            assert(false) ;
        }
    }
    return bSuccess ;
}

UtlBoolean OsNatAgentTask::enableStun(IStunSocket* pSocket,
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
    OsSysLog::add(FAC_NET, PRI_INFO, "Stun enabled for %s:%d with server %s:%d options=%d, keepAlive=%d",
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
        NAT_AGENT_CONTEXT* pContext = new NAT_AGENT_CONTEXT ;
        if (pContext)
        {
            pContext->type = STUN_DISCOVERY ;
            pContext->status = SENDING ;
            pContext->serverAddress = serverAddress ;
            pContext->serverPort = stunPort ;
            pContext->options = stunOptions ;
            memset(&pContext->transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;            
            pContext->nOldTransactions = 0 ;
            for (int i=0; i<MAX_OLD_TRANSACTIONS; i++)
            {
                memset(&pContext->oldTransactionsIds[i], 0, sizeof(STUN_TRANSACTION_ID)) ;            
            }                
            pContext->pSocket = pSocket ;
            pContext->pTimer = getTimer() ;
            pContext->keepAliveSecs = keepAliveSecs;
            pContext->abortCount = NAT_INITIAL_ABORT_COUNT ;
            pContext->refreshErrors = 0 ;
            pContext->port = PORT_NONE ;
            pContext->priority = 0 ;
            pContext->pKeepaliveListener = NULL ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;

            if (sendStunRequest(pContext))
            {
                OsTime errorAt(0, NAT_RESPONSE_TIMEOUT_MS * OsTime::USECS_PER_MSEC) ;
                OsQueuedEvent* pEvent = (OsQueuedEvent*) pContext->pTimer->getNotifier() ;
                pEvent->setUserData((int) pContext) ;

                pContext->pTimer->periodicEvery(errorAt, errorAt) ;
                bSuccess = true ;
            }
            else
            {
                // Unable to Send
                destroyBinding(pContext) ;
            }            
        }
        else
        {
            // Unable to allocate context
            assert(false) ;
        }
    }
    return bSuccess ;
}

UtlBoolean OsNatAgentTask::disableStun(IStunSocket* pSocket) 
{
    OsLock lock(mMapsLock) ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NET, PRI_INFO, "Stun disabled for %s:%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort()) ;

    NAT_AGENT_CONTEXT* pBinding = getBinding(pSocket, STUN_DISCOVERY) ;
    if (pBinding)
    {
        destroyBinding(pBinding) ;
    }

    return true ;
}


UtlBoolean OsNatAgentTask::enableTurn(IStunSocket* pSocket,
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

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NET, PRI_INFO, "Turn enabled for %s:%d with server %s:%d, keepalive=%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort(),
            turnServer.data(),
            turnPort,
            keepAliveSecs) ;

    assert(pSocket) ;    
    assert(turnServer.length() > 0) ;
    assert(portIsValid(turnPort)) ;

    if (    pSocket && portIsValid(turnPort) && (turnServer.length() > 0) &&
            OsSocket::getHostIpByName(turnServer, &serverAddress) && 
            OsSocket::isIp4Address(serverAddress))

    {       
        NAT_AGENT_CONTEXT* pContext = new NAT_AGENT_CONTEXT ;
        if (pContext)
        {
            pContext->type = TURN_ALLOCATION ;
            pContext->status = SENDING ;
            pContext->serverAddress = serverAddress ;
            pContext->serverPort = turnPort ;
            pContext->options = 0 ;
            memset(&pContext->transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;
            pContext->nOldTransactions = 0 ;
            for (int i=0; i<MAX_OLD_TRANSACTIONS; i++)
            {
                memset(&pContext->oldTransactionsIds[i], 0, sizeof(STUN_TRANSACTION_ID)) ;            
            }                
            pContext->pSocket = pSocket ;
            pContext->pTimer = getTimer() ;
            pContext->keepAliveSecs = keepAliveSecs;   
            pContext->abortCount = NAT_INITIAL_ABORT_COUNT ;
            pContext->refreshErrors = 0 ;
            pContext->port = PORT_NONE ;
            pContext->username = username ;
            pContext->password = password ;
            pContext->priority = 0 ;
            pContext->pKeepaliveListener = NULL ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;

            if (sendTurnRequest(pContext))
            {
                OsTime errorAt(0, NAT_RESPONSE_TIMEOUT_MS * OsTime::USECS_PER_MSEC) ;
                OsQueuedEvent* pEvent = (OsQueuedEvent*) pContext->pTimer->getNotifier() ;
                pEvent->setUserData((int) pContext) ;

                pContext->pTimer->periodicEvery(errorAt, errorAt) ;
                bSuccess = true ;
            }
            else
            {
                // Unable to Send
                destroyBinding(pContext) ;
            }            
        }
        else
        {
            // Unable to allocate context
            assert(false) ;
        }
    }
    return bSuccess ;
}


UtlBoolean OsNatAgentTask::primeTurnReception(IStunSocket* pSocket,
                                              const                char* szAddress,
                                              int                  iPort)
{
    UtlBoolean bRC = false ;
    OsLock lock(mMapsLock) ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NET, PRI_INFO, "Turn Priming %s:%d -> %s:%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort(),
            szAddress,
            iPort) ;

    NAT_AGENT_CONTEXT* pBinding = getBinding(pSocket, TURN_ALLOCATION) ;
    if (pBinding)
    {
        // Release Binding ...
        TurnMessage msgSend ;

        msgSend.allocTransactionId() ;
        msgSend.setType(MSG_TURN_SEND_REQUEST) ;
        msgSend.setDestinationAddress(szAddress, iPort) ;
    
        if (!pBinding->username.isNull())
        {
            msgSend.setUsername(pBinding->username) ;
        }

        if (!pBinding->password.isNull()) 
        {
            msgSend.setPassword(pBinding->password) ;
        }

        msgSend.setData("\r\n", 2) ;          

        bRC = sendMessage(&msgSend, pSocket, pBinding->serverAddress, pBinding->serverPort) ;
    }

    return bRC ;
}


UtlBoolean OsNatAgentTask::setTurnDestination(IStunSocket* pSocket,
                                              const char* szAddress,
                                              int iPort ) 
{
    UtlBoolean bRC = false ;
    OsLock lock(mMapsLock) ;

    NAT_AGENT_CONTEXT* pBinding = getBinding(pSocket, TURN_ALLOCATION) ;
    if (pBinding)
    {
        UtlString localHostIp ;
        pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
        OsSysLog::add(FAC_NET, PRI_INFO, "Turn destination %s:%d -> %s:%d",
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                szAddress,
                iPort) ;

        // Release Binding ...
        TurnMessage msgSend ;

        msgSend.allocTransactionId() ;
        msgSend.setType(MSG_TURN_ACTIVE_DESTINATION_REQUEST) ;
        msgSend.setDestinationAddress(szAddress, iPort) ;
    
        if (!pBinding->username.isNull())
        {
            msgSend.setUsername(pBinding->username) ;
        }

        if (!pBinding->password.isNull()) 
        {
            msgSend.setPassword(pBinding->password) ;
        }

        bRC = sendMessage(&msgSend, pSocket, pBinding->serverAddress, pBinding->serverPort) ;
    }

    return bRC ;
}

void OsNatAgentTask::disableTurn(IStunSocket* pSocket) 
{
    OsLock lock(mMapsLock) ;

    UtlString localHostIp ;
    pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
    OsSysLog::add(FAC_NET, PRI_INFO, "Turn disabled for %s:%d",
            localHostIp.data(),         
            pSocket->getSocket()->getLocalHostPort()) ;


    NAT_AGENT_CONTEXT* pBinding = getBinding(pSocket, TURN_ALLOCATION) ;
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

                    sendMessage(&msgSend, pSocket, pBinding->serverAddress, pBinding->serverPort) ;
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



UtlBoolean OsNatAgentTask::addCrLfKeepAlive(IStunSocket*    pSocket, 
                                            const UtlString&     remoteIp,
                                            int                  remotePort,
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
        OsSysLog::add(FAC_NET, PRI_INFO, "Enabled CrLf keep alive %s:%d --> %s:%d every %d secs" ,
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                remoteIp.data(),
                remotePort,
                keepAliveSecs) ;

        NAT_AGENT_CONTEXT* pContext = new NAT_AGENT_CONTEXT ;
        if (pContext)
        {
            pContext->type = CRLF_KEEPALIVE ;
            pContext->status = SUCCESS ;
            pContext->serverAddress = remoteIp ;
            pContext->serverPort = remotePort ;
            pContext->options = 0 ;
            memset(&pContext->transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;
            pContext->nOldTransactions = 0 ;
            for (int i=0; i<MAX_OLD_TRANSACTIONS; i++)
            {
                memset(&pContext->oldTransactionsIds[i], 0, sizeof(STUN_TRANSACTION_ID)) ;            
            }                
            pContext->pSocket = pSocket ;
            pContext->pTimer = getTimer() ;
            pContext->keepAliveSecs = keepAliveSecs;   
            pContext->abortCount = NAT_INITIAL_ABORT_COUNT ;
            pContext->refreshErrors = 0 ;
            pContext->port = PORT_NONE ;
            pContext->priority = 0 ;
            pContext->pKeepaliveListener = pListener ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;

            if (pContext->pKeepaliveListener)
            {
                pContext->pKeepaliveListener->OnKeepaliveStart(
                        populateKeepaliveEvent(pContext)); 
            }
    
            handleCrLfKeepAlive(pContext) ;
            bSuccess = true ;
            if (keepAliveSecs > 0)
            {
                OsTime refreshAt(keepAliveSecs, 0) ;
                OsQueuedEvent* pEvent = (OsQueuedEvent*) pContext->pTimer->getNotifier() ;
                pEvent->setUserData((int) pContext) ;

                pContext->pTimer->periodicEvery(refreshAt, refreshAt) ;
            }
            else
            {
                destroyBinding(pContext) ;
            }
        }
    }

    return bSuccess ;
}


UtlBoolean OsNatAgentTask::removeCrLfKeepAlive(IStunSocket* pSocket, 
                                               const UtlString&     remoteIp,
                                               int                  remotePort) 
{
    NAT_AGENT_CONTEXT* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bSuccess = FALSE ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
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
        OsSysLog::add(FAC_NET, PRI_INFO, "Disable CrLf keep alive %s:%d --> %s:%d" ,
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                remoteIp.data(),
                remotePort) ;

        destroyBinding(pRC) ;
        bSuccess = true ;
    }

    return bSuccess ;
}


UtlBoolean OsNatAgentTask::addStunKeepAlive(IStunSocket*    pSocket, 
                                            const UtlString&     remoteIp,
                                            int                  remotePort,
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
        OsSysLog::add(FAC_NET, PRI_INFO, "Enabled STUN keep alive %s:%d --> %s:%d every %d secs" ,
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                remoteIp.data(),
                remotePort,
                keepAliveSecs) ;     

        NAT_AGENT_CONTEXT* pContext = new NAT_AGENT_CONTEXT ;
        if (pContext)
        {
            pContext->type = STUN_KEEPALIVE ;
            pContext->status = SUCCESS ;
            pContext->serverAddress = remoteIp ;
            pContext->serverPort = remotePort ;
            pContext->options = 0 ;
            memset(&pContext->transactionId, 0, sizeof(STUN_TRANSACTION_ID)) ;
            pContext->nOldTransactions = 0 ;
            for (int i=0; i<MAX_OLD_TRANSACTIONS; i++)
            {
                memset(&pContext->oldTransactionsIds[i], 0, sizeof(STUN_TRANSACTION_ID)) ;            
            }                
            pContext->pSocket = pSocket ;
            pContext->pTimer = getTimer() ;
            pContext->keepAliveSecs = keepAliveSecs;   
            pContext->abortCount = NAT_INITIAL_ABORT_COUNT ;
            pContext->refreshErrors = 0 ;
            pContext->port = PORT_NONE ;
            pContext->priority = 0 ;
            pContext->pKeepaliveListener = pListener ;

            mContextMap.insertKeyAndValue(new UtlVoidPtr(pContext), new UtlVoidPtr(pSocket)) ;

            if (pContext->pKeepaliveListener)
            {
                pContext->pKeepaliveListener->OnKeepaliveStart(
                        populateKeepaliveEvent(pContext)); 
            }

            handleStunKeepAlive(pContext) ;
            bSuccess = true ;
            if (keepAliveSecs > 0)
            {
                OsTime refreshAt(keepAliveSecs, 0) ;
                OsQueuedEvent* pEvent = (OsQueuedEvent*) pContext->pTimer->getNotifier() ;
                pEvent->setUserData((int) pContext) ;

                pContext->pTimer->periodicEvery(refreshAt, refreshAt) ;

            }
            else
            {
                destroyBinding(pContext) ;
            }
        }
    }

    return bSuccess ;
}


UtlBoolean OsNatAgentTask::removeStunKeepAlive(IStunSocket* pSocket, 
                                               const UtlString&     remoteIp,
                                               int                  remotePort) 
{
    NAT_AGENT_CONTEXT* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bSuccess = FALSE ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
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
        OsSysLog::add(FAC_NET, PRI_INFO, "Disable STUN keep alive %s:%d --> %s:%d" ,
                localHostIp.data(),         
                pSocket->getSocket()->getLocalHostPort(),
                remoteIp.data(),
                remotePort) ;

        destroyBinding(pRC) ;
        bSuccess = true ;
    }

    return bSuccess ;
}


UtlBoolean OsNatAgentTask::removeKeepAlives(IStunSocket* pSocket) 
{
    NAT_AGENT_CONTEXT* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bSuccess = FALSE ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
        if (    (pContext->pSocket == pSocket) &&
                ((pContext->type == STUN_KEEPALIVE) || (pContext->type == CRLF_KEEPALIVE)))
        {
            UtlString localHostIp ;
            pSocket->getSocket()->getLocalHostIp(&localHostIp) ;      
            OsSysLog::add(FAC_NET, PRI_INFO, "Disable %s keep alive %s:%d --> %s:%d",
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

UtlBoolean OsNatAgentTask::removeStunProbes(IStunSocket* pSocket) 
{
    NAT_AGENT_CONTEXT* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bSuccess = FALSE ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
        if (    (pContext->pSocket == pSocket) &&
                (pContext->type == STUN_PROBE))
        {
            UtlString localHostIp ;
            pSocket->getSocket()->getLocalHostIp(&localHostIp) ;      
            OsSysLog::add(FAC_NET, PRI_INFO, "Disable stun probe %s:%d --> %s:%d",
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


// Determines if probes of a higher priority are still outstanding
UtlBoolean OsNatAgentTask::areProbesOutstanding(IStunSocket* pSocket, int priority) 
{
    NAT_AGENT_CONTEXT* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bOutstanding = false ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
        if (    (pContext->pSocket == pSocket) &&
                (pContext->type == STUN_PROBE) &&
                (pContext->priority > priority))
        {
            //UtlString debug ;
            //dumpContext(&debug, pContext) ;
            //OutputDebugString(debug) ;
            //OutputDebugString("\n") ;

            bOutstanding = true ;
            break ;
        }        
    }

    return bOutstanding ;
}

// Does a binding of the designated type/server exist 
UtlBoolean OsNatAgentTask::doesBindingExist(IStunSocket*   pSocket,
                                            NAT_AGENT_BINDING_TYPE type, 
                                            const UtlString&       serverIp,
                                            int                    serverPort)
{
    NAT_AGENT_CONTEXT* pRC = NULL ;
    UtlVoidPtr* pKey;
    UtlBoolean bFound = false ;

    OsLock lock(mMapsLock) ;

    UtlHashMapIterator iterator(mContextMap);
    while (pKey = (UtlVoidPtr*)iterator())
    {        
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
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
    NAT_AGENT_CONTEXT* pRC = NULL ;
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
                NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();

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

    OsWriteLock lock(mExternalBindingMutex) ;
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


UtlBoolean OsNatAgentTask::sendMessage(StunMessage* pMsg, 
                                       IStunSocket* pSocket, 
                                       const UtlString& toAddress, 
                                       unsigned short toPort)
{
    UtlBoolean bSuccess = false ;

    char cEncoded[10240] ;
    size_t length ;

    pMsg->setServer("sipXtapi (www.sipfoundry.org)") ;
    if (pMsg->encode(cEncoded, sizeof(cEncoded), length))
    {

        if (OsSysLog::willLog(FAC_NET, PRI_DEBUG))
        {
            UtlString output ;
            StunUtils::debugDump(cEncoded, length, output) ;
            OsSysLog::add(FAC_NET, PRI_DEBUG, "Outbound STUN/TURN message(Crude Parse):\n%s\n",
                    output.data()) ;
        }

        // This is a bit of a hack -- The IStunSocket keeps track of 
        // the last read/write times.  Instead of creating a 'special' write 
        // or some markers to figure it the packet should update the last 
        // written timestamp, I am just calling the base class implementation 
        // directly.
        if (pSocket->getSocket()->write(cEncoded, (int) length, toAddress, toPort) > 0)
        {
            bSuccess = true ;
        }
    }
    else
    {
        printf("OsNatAgentTask: Unable to encode message\n") ;
        assert(false) ;
    }

    return bSuccess ;
}


NAT_AGENT_CONTEXT* OsNatAgentTask::getBinding(IStunSocket* pSocket, NAT_AGENT_BINDING_TYPE type) 
{
    NAT_AGENT_CONTEXT* pRC = NULL ;
    UtlHashMapIterator iterator(mContextMap);
    UtlVoidPtr* pKey;

    while (pKey = (UtlVoidPtr*)iterator())
    {        
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
        if ((pContext->pSocket == pSocket) && (pContext->type == type))
        {
            pRC = pContext ;
            break ;
        }
    }
    return pRC ;
}


NAT_AGENT_CONTEXT* OsNatAgentTask::getBinding(NAT_AGENT_CONTEXT* pBinding) 
{
    NAT_AGENT_CONTEXT* pRC = NULL ;
    UtlHashMapIterator iterator(mContextMap);
    UtlVoidPtr* pKey;

    while (pKey = (UtlVoidPtr*)iterator())
    {
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
        if (pBinding == pContext)
        {
            pRC = pContext ;
            break ;
        }
    }

    return pRC ;
}


NAT_AGENT_CONTEXT* OsNatAgentTask::getBinding(STUN_TRANSACTION_ID* pId) 
{
    NAT_AGENT_CONTEXT* pRC = NULL ;
    UtlHashMapIterator iterator(mContextMap);
    UtlVoidPtr* pKey;

    while (pKey = (UtlVoidPtr*)iterator())
    {
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
        if (memcmp(&pId->id[0], &pContext->transactionId.id[0], sizeof(STUN_TRANSACTION_ID)) == 0)
        {
            pRC = pContext ;
            break ;
        }
        else
        {
            for (int i=0; i<pContext->nOldTransactions; i++)
            {
                if (memcmp(&pId->id[0], &pContext->oldTransactionsIds[i].id[0], sizeof(STUN_TRANSACTION_ID)) == 0)
                {
                    pRC = pContext ;
                    break ;
                }
            }
        }
    }

    return pRC ;
}


void OsNatAgentTask::destroyBinding(NAT_AGENT_CONTEXT* pBinding) 
{
    UtlHashMapIterator iterator(mContextMap);
    UtlVoidPtr* pKey;   
    
    while (pKey = (UtlVoidPtr*)iterator())
    {
        NAT_AGENT_CONTEXT* pContext = (NAT_AGENT_CONTEXT*) pKey->getValue();
        if (pContext == pBinding)
        {                
            mContextMap.destroy(pKey) ;
            releaseTimer(pBinding->pTimer) ;

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


OsTimer* OsNatAgentTask::getTimer() 
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

    return pTimer ;
}


void OsNatAgentTask::releaseTimer(OsTimer* pTimer) 
{
    assert(pTimer) ;
    if (pTimer)
    {
        pTimer->stop() ;
        mTimerPool.insert(new UtlVoidPtr(pTimer)) ;
    }
}


UtlBoolean OsNatAgentTask::sendStunRequest(NAT_AGENT_CONTEXT* pBinding) 
{
    UtlBoolean bSuccess = false ;
    StunMessage msgSend ;

    msgSend.allocTransactionId() ;
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

    // Store old transaction
    int index = 0 ;
    if (pBinding->nOldTransactions < MAX_OLD_TRANSACTIONS)
    {
        index = pBinding->nOldTransactions ;
        pBinding->nOldTransactions++ ;
    }
    else
    {
        index = MAX_OLD_TRANSACTIONS -1 ; 
        for (int i=0; i<index; i++)
        {
            memcpy(&pBinding->oldTransactionsIds[i], 
                    &pBinding->oldTransactionsIds[i+1], 
                    sizeof(STUN_TRANSACTION_ID)) ;
        }
    }

    memcpy(&pBinding->oldTransactionsIds[index], 
            &pBinding->transactionId, sizeof(STUN_TRANSACTION_ID)) ;    

    // Get new transaction Id
    msgSend.getTransactionId(&pBinding->transactionId) ;

    // Send message
    if (sendMessage(&msgSend, pBinding->pSocket, pBinding->serverAddress, pBinding->serverPort))
    {
        bSuccess = true ;
    }

    return bSuccess ;
}

UtlBoolean OsNatAgentTask::sendTurnRequest(NAT_AGENT_CONTEXT* pBinding) 
{
    UtlBoolean bSuccess = false ;
    TurnMessage msgSend ;

    // Store old transaction
    int index = 0 ;
    if (pBinding->nOldTransactions < MAX_OLD_TRANSACTIONS)
    {
        index = pBinding->nOldTransactions ;
        pBinding->nOldTransactions++ ;
    }
    else
    {
        index = MAX_OLD_TRANSACTIONS -1 ; 
        for (int i=0; i<index; i++)
        {
            memcpy(&pBinding->oldTransactionsIds[i], 
                    &pBinding->oldTransactionsIds[i+1], 
                    sizeof(STUN_TRANSACTION_ID)) ;
        }
    }

    memcpy(&pBinding->oldTransactionsIds[index], 
            &pBinding->transactionId, sizeof(STUN_TRANSACTION_ID)) ;    

    msgSend.allocTransactionId() ;
    msgSend.getTransactionId(&pBinding->transactionId) ;
    msgSend.setType(MSG_TURN_ALLOCATE_REQUEST) ;
    msgSend.setRequestXorOnly() ;
    msgSend.setLifetime(pBinding->keepAliveSecs * 2) ;

    if (!pBinding->username.isNull()) 
    {
        msgSend.setUsername(pBinding->username) ;    
    }

    if (!pBinding->password.isNull()) 
    {
        msgSend.setPassword(pBinding->password) ;
    }  
    
    if (sendMessage(&msgSend, pBinding->pSocket, pBinding->serverAddress, pBinding->serverPort))
    {
        bSuccess = true ;
    }

    return bSuccess ;
}


void OsNatAgentTask::markStunSuccess(NAT_AGENT_CONTEXT* pBinding, const UtlString& mappedAddress, int mappedPort) 
{    
    bool bAddressChanged = false ;
    OsTime refreshPeriod(pBinding->keepAliveSecs, 0) ;

    assert(pBinding != NULL) ;
    if (pBinding)
    {
        UtlString localHostIp ;
        pBinding->pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
        OsSysLog::add(FAC_NET, PRI_INFO, "Stun success for %s:%d, mapped address: %s:%d",
                localHostIp.data(),         
                pBinding->pSocket->getSocket()->getLocalHostPort(),
                mappedAddress.data(),
                mappedPort) ;

        assert(pBinding->pTimer != NULL) ;
        assert(pBinding->pSocket != NULL) ;

        // Check for a change in mapped IP/PORT
        if (    (pBinding->status == RESENDING) || 
                (pBinding->status == RESENDING_ERROR)   )
        {
            if (    (mappedAddress.compareTo(pBinding->address) != 0) ||
                    (mappedPort != pBinding->port)  )
            {
                // This is really a critial problem -- we are likely not able
                // to communicate if our NAT binding is bouncing around.
                OsSysLog::add(FAC_NET, PRI_CRIT, 
                        "Stun binding changed for %s:%d, %s:%d is now %s:%d",
                        localHostIp.data(),
                        pBinding->pSocket->getSocket()->getLocalHostPort(),
                        pBinding->address.data(),
                        pBinding->port,
                        mappedAddress.data(),
                        mappedPort) ;

                bAddressChanged = true ;
            }
        }

        // Update State
        pBinding->address = mappedAddress ;
        pBinding->port = mappedPort ;       
        pBinding->status = SUCCESS ;
        pBinding->refreshErrors = 0 ;

        // Reset Timer
        pBinding->pTimer->stop() ;
        if (pBinding->keepAliveSecs > 0)
        {
            pBinding->pTimer->periodicEvery(refreshPeriod, refreshPeriod) ;          
        }

        // Notify Socket
        if (pBinding->type == STUN_DISCOVERY)
        {
            pBinding->pSocket->setStunAddress(mappedAddress, mappedPort) ;
            pBinding->pSocket->markStunSuccess(bAddressChanged) ;
        } 
        else if (pBinding->type == STUN_PROBE)
        {                                
            pBinding->pSocket->evaluateDestinationAddress(pBinding->serverAddress,
                    pBinding->serverPort, pBinding->priority) ;
        }
        else if (pBinding->type == STUN_KEEPALIVE)
        {
            if (pBinding->pKeepaliveListener)
            {
                pBinding->pKeepaliveListener->OnKeepaliveFeedback(
                        populateKeepaliveEvent(pBinding)); 
            }
        }
        else
        {
            // assert(false) ;
        }
    }
}


void OsNatAgentTask::markStunFailure(NAT_AGENT_CONTEXT* pBinding) 
{  
    assert(pBinding != NULL) ;
    if (pBinding)
    {
        UtlString localHostIp ;
        pBinding->pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
        OsSysLog::add(FAC_NET, PRI_INFO, "Stun failure for %s:%d -> %s:%d",
                    localHostIp.data(),         
                    pBinding->pSocket->getSocket()->getLocalHostPort(),
                    pBinding->serverAddress.data(),
                    pBinding->serverPort) ;

        assert(pBinding->pTimer != NULL) ;
        assert(pBinding->pSocket != NULL) ;
        
        pBinding->status = FAILED ;

        // Clear timer
        pBinding->pTimer->stop() ; 

        // Notify Socket
        if (pBinding->type == STUN_DISCOVERY)
        {
            UtlString empty ;
            pBinding->pSocket->setStunAddress(empty, PORT_NONE) ;
            pBinding->pSocket->markStunFailure() ;   
        }
        else if (pBinding->type == STUN_PROBE)
        {
            destroyBinding(pBinding) ;
        }
        else if (pBinding->type == STUN_KEEPALIVE)
        {
            // We are currently ignore keep alive responses
        }
        else
        {
            assert(false) ;
        }
    }
}


void OsNatAgentTask::markTurnSuccess(NAT_AGENT_CONTEXT* pBinding, const UtlString& relayAddress, int relayPort) 
{
    OsTime refreshPeriod(pBinding->keepAliveSecs, 0) ;

    assert(pBinding != NULL) ;
    if (pBinding)
    {
        UtlString localHostIp ;
        pBinding->pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
        OsSysLog::add(FAC_NET, PRI_INFO, "Turn success for %s:%d, relay address: %s:%d",
                localHostIp.data(),         
                pBinding->pSocket->getSocket()->getLocalHostPort(),
                relayAddress.data(),
                relayPort) ;

        assert(pBinding->pTimer != NULL) ;
        assert(pBinding->pSocket != NULL) ;

        pBinding->address = relayAddress ;
        pBinding->port = relayPort ;

        pBinding->status = SUCCESS ;
        pBinding->refreshErrors = 0 ;

        // Reset Timer
        pBinding->pTimer->stop() ;
        if (pBinding->keepAliveSecs > 0)
        {
            pBinding->pTimer->periodicEvery(refreshPeriod, refreshPeriod) ;          
        }

        // Notify Socket
        pBinding->pSocket->setTurnAddress(relayAddress, relayPort) ;
        pBinding->pSocket->markTurnSuccess() ;
    }
}

void OsNatAgentTask::markTurnFailure(NAT_AGENT_CONTEXT* pBinding) 
{
    assert(pBinding != NULL) ;
    if (pBinding)
    {
        UtlString localHostIp ;
        pBinding->pSocket->getSocket()->getLocalHostIp(&localHostIp) ;
        OsSysLog::add(FAC_NET, PRI_INFO, "Turn failure for %s:%d -> %s:%d",
                    localHostIp.data(),         
                    pBinding->pSocket->getSocket()->getLocalHostPort(),
                    pBinding->serverAddress.data(),
                    pBinding->serverPort) ;

        assert(pBinding->pTimer != NULL) ;
        assert(pBinding->pSocket != NULL) ;
        
        pBinding->status = FAILED ;

        // Clear timer
        pBinding->pTimer->stop() ; 

        // Notify Socket
        UtlString empty ;
        pBinding->pSocket->setTurnAddress(empty, PORT_NONE) ;
        pBinding->pSocket->markTurnFailure() ;   
    }
}

OsNatKeepaliveEvent OsNatAgentTask::populateKeepaliveEvent(NAT_AGENT_CONTEXT* pContext)
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


void OsNatAgentTask::dumpContext(UtlString* pResults, NAT_AGENT_CONTEXT* pBinding) 
{
    const char* cBindingTypes[] =
    {
        "STUN_DISCOVERY",
        "STUN_PROBE",
        "TURN_ALLOCATION",
        "CRLF_KEEPALIVE",
        "STUN_KEEPALIVE"
    } ;

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
                " BindingType: %s\n"
                " Status     : %s\n"
                " Server Addr: %s\n"
                " Server Port: %d\n"
                " Servers Opt: %d\n"
                " Transaction: %s\n"
                " Socket     : %p (%s:%d)\n"
                " Timer      : %p\n"
                " KeepAlive  : %d\n"
                " Errors     : %d\n"
                " Address    : %s\n"
                " Port       : %d\n"
                " Username   : %s\n"
                " Password   : %s\n"
                " Priority   : %d\n",
                cBindingTypes[pBinding->type],
                cStatus[pBinding->status],
                pBinding->serverAddress.data(),
                pBinding->serverPort,
                pBinding->options,
                "TBD",
                pBinding->pSocket,
                socketHostIp.data(),
                socketPort,
                pBinding->pTimer,
                pBinding->keepAliveSecs,
                pBinding->refreshErrors,
                pBinding->address.data(),
                pBinding->port,
                pBinding->username.data(),
                pBinding->password.data(),
                pBinding->priority) ;

        pResults->append(cBuf) ;
    }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


