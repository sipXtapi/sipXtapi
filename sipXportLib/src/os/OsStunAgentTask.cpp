// $Id$
//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsStunAgentTask.h"
#include "os/OsStunDatagramSocket.h"
#include "os/OsStunQueryAgent.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "os/OsEvent.h"
#include "utl/UtlVoidPtr.h"
#ifndef _WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
OsMutex OsStunAgentTask::sLock(OsMutex::Q_FIFO) ;
OsStunAgentTask* OsStunAgentTask::spInstance = NULL ;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsStunAgentTask::OsStunAgentTask()
    : OsServerTask("OsStunAgentTask-%d")
    , mNotifyMapLock(OsMutex::Q_FIFO)
{

}

OsStunAgentTask::~OsStunAgentTask()
{
    // Wait for the thread to shutdown
    waitUntilShutDown() ;
}


OsStunAgentTask* OsStunAgentTask::getInstance() 
{
    OsLock lock(sLock) ;

    if (spInstance == NULL) 
    {
        spInstance = new OsStunAgentTask() ;
        spInstance->start() ;
    }

    return spInstance ;

}


void OsStunAgentTask::releaseInstance() 
{
    OsLock lock(sLock) ;

    if (spInstance != NULL)
    {
        delete spInstance ;
        spInstance = NULL ;
    }
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean OsStunAgentTask::handleMessage(OsMsg& rMsg) 
{
    UtlBoolean bHandled = false ;

    switch (rMsg.getMsgType())
    {
        case STUN_MSG_TYPE:
            bHandled = handleStunMessage((StunMsg&) rMsg) ;
            break ;
        case SYNC_MSG_TYPE:
            bHandled = handleSynchronize((OsRpcMsg&) rMsg) ;
            break ;
        case OsMsg::OS_EVENT:
            bHandled = handleStunTimerEvent((OsEventMsg&) rMsg) ;
            break ;
    }

    return bHandled ;
}


UtlBoolean OsStunAgentTask::handleStunMessage(StunMsg& rMsg) 
{
    StunMessage msg;
    size_t nBuffer = rMsg.getLength() ;
    char* pBuffer = rMsg.getBuffer() ;
    OsStunDatagramSocket* pSocket = rMsg.getSocket() ;
    unsigned int mappedAddress ;
    
    if ((nBuffer > 0) && pBuffer && pSocket)
    {
        memset(&msg, 0, sizeof(StunMessage)) ;
        msg.parseMessage(pBuffer, nBuffer) ;

        switch (msg.msgHdr.msgType)
        {
            case BindRequestMsg:
                {
                    StunMessage respMsg ;
                    memset(&respMsg, 0, sizeof(StunMessage));

                    // Copy over ID
                    respMsg.msgHdr.msgType = BindResponseMsg;
                    for (int i=0; i<16; i++)
                    {
                        respMsg.msgHdr.id.octet[i] = msg.msgHdr.id.octet[i];
                    }

                    respMsg.hasMappedAddress = true;
                    respMsg.mappedAddress.ipv4.port = htons(rMsg.getReceivedPort()) ;
                    respMsg.mappedAddress.ipv4.addr = htonl(inet_addr(rMsg.getReceivedIp().data())) ;

                    UtlString hostIp ;
                    pSocket->getLocalHostIp(&hostIp) ;
                    respMsg.hasSourceAddress = true;
                    respMsg.sourceAddress.ipv4.port = htons(pSocket->getLocalHostPort()) ;
                    respMsg.sourceAddress.ipv4.addr = htonl(inet_addr(hostIp.data())) ;

                    if (msg.hasResponseAddress)
                    {
                        // add reflected from
                        respMsg.hasReflectedFrom = true ;
                        respMsg.reflectedFrom.ipv4.port =respMsg.mappedAddress.ipv4.port ;
                        respMsg.reflectedFrom.ipv4.addr = respMsg.mappedAddress.ipv4.addr ;
                    }

                    char buf[STUN_MAX_MESSAGE_SIZE];
                    int len = STUN_MAX_MESSAGE_SIZE;

                    len = respMsg.encodeMessage(buf, len);
                    if (msg.hasResponseAddress)
                    {
                        UINT addr=htonl(msg.responseAddress.ipv4.addr) ;
                        pSocket->write(buf, len, inet_ntoa((*((in_addr*)&addr))), msg.responseAddress.ipv4.port) ;
                    }
                    else
                    {                        
                        pSocket->write(buf, len, rMsg.getReceivedIp(), rMsg.getReceivedPort()) ;
                    }
                }
                break ;
            case BindResponseMsg:
                {                                        
                    UtlString address ;
                    int iPort ;

                    // TODO: Validate that we actually have a mapped address
                    mappedAddress=htonl (msg.mappedAddress.ipv4.addr) ;
                    address = inet_ntoa (*((in_addr*)&mappedAddress)) ;
                    iPort = msg.mappedAddress.ipv4.port ;

                    pSocket->setStunAddress(address, iPort) ;
                    signalStunOutcome(pSocket, true) ;
                }
                break ;
            case BindErrorResponseMsg:
                {                    
                    pSocket->setStunAddress(NULL, -1) ;
                    signalStunOutcome(pSocket, false) ;
                }
                break ;
            case SharedSecretRequestMsg:
            case SharedSecretResponseMsg:
            case SharedSecretErrorResponseMsg:
                break ;
        }
    }
    return true ;
}


UtlBoolean OsStunAgentTask::handleSynchronize(OsRpcMsg& rMsg) 
{
    OsEvent* pEvent = rMsg.getEvent() ;
    pEvent->signal(0) ;

    return true ;
}


UtlBoolean OsStunAgentTask::handleStunTimerEvent(OsEventMsg& rMsg) 
{
    OsStunDatagramSocket* pSocket ;
    OsStatus rc ;

    // Pull out socket
    rc = rMsg.getUserData((int&) pSocket) ;
    assert(rc == OS_SUCCESS) ;

    // Refresh the socket
    if ((rc == OS_SUCCESS) && pSocket)
    {
        pSocket->refreshStunBinding() ;
    }

    return true ;
}


UtlBoolean OsStunAgentTask::sendStunRequest(OsStunDatagramSocket* pSocket,
                                            const UtlString& stunServer,
                                            int stunPort,
                                            OsEvent* pNotify) 
{    
    OsLock lock(mNotifyMapLock) ;

    assert(pSocket) ;
    assert(stunPort > 0) ;
    assert(stunServer.length() > 0) ;
    if (pSocket && (stunPort > 0) && (stunServer.length() > 0))
    {        
        // Remove any existing maps
        UtlVoidPtr* pKey = new UtlVoidPtr(pSocket) ;
        assert(pKey) ;
        if (pKey)
        {
            mNotifyMap.destroy(pKey) ;

            // Add new binding
            if (pNotify)
            {
                UtlVoidPtr* pValue = new UtlVoidPtr(pNotify) ;
                assert(pValue) ;
                if (pValue)
                {
                    mNotifyMap.insertKeyAndValue(pKey, pValue) ;
                }
                else
                {
                    delete pKey ;
                }
            }
            else
            {
                delete pKey ;
            }
        }
        
        // Force a refresh
        OsStunQueryAgent agent;
        if (agent.setServer(stunServer, stunPort))
        {
            agent.sendStunRequest(pSocket) ;
        }
        else 
        {
            signalStunOutcome(pSocket, false) ;
        }
    }
    
    return false ;
}


void OsStunAgentTask::synchronize() 
{
    OsLock lock(sLock) ;

    if (isStarted())
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


void OsStunAgentTask::removeNotify(OsStunDatagramSocket* pSocket) 
{
    OsLock lock(mNotifyMapLock) ;

    // Remove contents if it exists
    UtlVoidPtr key(pSocket) ;
    mNotifyMap.destroy(&key) ;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


void OsStunAgentTask::signalStunOutcome(OsStunDatagramSocket* pSocket, 
                                        UtlBoolean bSuccess) 
{
    OsLock lock(mNotifyMapLock) ;

    UtlVoidPtr key(pSocket) ;
    UtlVoidPtr* pValue = (UtlVoidPtr*) mNotifyMap.findValue(&key) ;
    if (pValue)
    {
        OsEvent* pNotify = (OsEvent*) pValue->getValue() ;
        if (pNotify)
        {
            pNotify->signal(bSuccess) ;
        }
    }
    mNotifyMap.destroy(&key) ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


