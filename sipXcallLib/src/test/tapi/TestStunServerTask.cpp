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

// APPLICATION INCLUDES
#include "TestStunServerTask.h"
#include "os/StunMessage.h"

#include "os/OsDatagramSocket.h"
#include "os/OsSocket.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlVoidPtr.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
TestStunServerTask::TestStunServerTask(OsDatagramSocket* pPrimarySocket,
                                       OsDatagramSocket* pPrimarySocket2,
                                       OsDatagramSocket* pSecondarySocket,
                                       OsDatagramSocket* pSecondarySocket2)
    : OsTask("TestStunServerTask-%d")
{
    mpPrimarySocket = pPrimarySocket ;
    mpPrimarySocket2 = pPrimarySocket2 ;
    mpSecondarySocket = pSecondarySocket ;
    mpSecondarySocket2 = pSecondarySocket2 ;
    mpEventValidator = NULL ;
    mTestMode = TEST_NORMAL ;
    miRequestNumber = 0 ;
}


// Destructor
TestStunServerTask::~TestStunServerTask()
{    
    // TODO: Clear map and destroy contents    

    waitUntilShutDown() ;
}

/* ============================ MANIPULATORS ============================== */

int TestStunServerTask::run(void* pArg) 
{    
    char cReadBuffer[10240] ;
    int nReadLength ;
    UtlString readAddress ;
    int readPort ;

    while (isStarted() && mpPrimarySocket && mpPrimarySocket->isOk()) 
    {
        if (mpPrimarySocket->isReadyToRead(500))
        {
            nReadLength = ((OsSocket*) mpPrimarySocket)->read(cReadBuffer, sizeof(cReadBuffer), &readAddress, &readPort) ;
            if (nReadLength > 0)
            {
                if (StunMessage::isStunMessage(cReadBuffer, nReadLength))
                {
                    handleStunMessage(cReadBuffer, nReadLength, readAddress, readPort) ;
                }
                else
                {
                    printf("Unexpected data received by TestStunServerTask\n") ;
                }
            }
        }
    } 

    return 0 ;
}


void TestStunServerTask::setEventValidator(EventValidator* pEventValidator) 
{
    mpEventValidator = pEventValidator ;
}


void TestStunServerTask::setTestMode(STUN_TEST_MODE testMode) 
{
    mTestMode = testMode ;

    if (mpEventValidator)
    {
        char* testModes[] =
        {
            "TEST_NORMAL",
            "TEST_NO_RESPONSE",
            "TEST_RETURN_ERROR",
            "TEST_DROP_ODD_REQUEST",
            "TEST_DROP_EVEN_REQUEST",
            "TEST_DELAY_RESPONSE"
        } ;

        char cMsg[100] ;
        sprintf(cMsg, "TestStunServerTask:setTestMode(%s)",
                testModes[testMode]) ;

        mpEventValidator->addMarker(cMsg) ;
    }
}



/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


void TestStunServerTask::handleStunMessage(const char* pBuf, int nBuf, const UtlString& fromAddress, unsigned short fromPort) 
{
    StunMessage msg ;

    if (msg.parse(pBuf, (size_t) nBuf))
    {
        switch (msg.getType())
        {
            case MSG_STUN_BIND_REQUEST:
                
                miRequestNumber++ ;

                if (mpEventValidator)
                {
                    mpEventValidator->addMessage(0, "TestStunServerTask::handleStunMessage") ;
                }
                switch (mTestMode)
                {
                    case TEST_NORMAL:
                        if (mpEventValidator)
                        {
                            mpEventValidator->addMessage(0, "TestStunServerTask::Sending Response") ;
                        }
                        handleStunBindRequest(&msg, fromAddress, fromPort) ;
                        break ;
                    case TEST_NO_RESPONSE:
                        if (mpEventValidator)
                        {
                            mpEventValidator->addMessage(0, "TestStunServerTask::Dropping Response") ;
                        }
                        break ;
                    case TEST_RETURN_ERROR:
                        {
                            if (mpEventValidator)
                            {
                                mpEventValidator->addMessage(0, "TestStunServerTask::Send Error") ;
                            }
                            StunMessage response ;
                            STUN_TRANSACTION_ID transactionId ;

                            // Copy transaction id
                            msg.getTransactionId(&transactionId) ;
                            response.setTransactionId(transactionId) ;

                            response.setType(MSG_STUN_BIND_ERROR_RESPONSE) ;
                            response.setError(400, "Forced Error") ;

                            sendResponse(&response, mpPrimarySocket, fromAddress, fromPort) ;
                        }
                        break ;
                    case TEST_DROP_EVEN_REQUEST:
                        if ((miRequestNumber %2) == 0)
                        {
                            handleStunBindRequest(&msg, fromAddress, fromPort) ;
                            if (mpEventValidator)
                            {
                                mpEventValidator->addMessage(0, "TestStunServerTask::Sending Response (even)") ;
                            }
                        }
                        else
                        {
                            if (mpEventValidator)
                            {
                                mpEventValidator->addMessage(0, "TestStunServerTask::Dropping Response (even)") ;
                            }
                        }
                        break ;

                    case TEST_DROP_ODD_REQUEST:
                        if ((miRequestNumber %2) == 1)
                        {
                            handleStunBindRequest(&msg, fromAddress, fromPort) ;
                            if (mpEventValidator)
                            {
                                mpEventValidator->addMessage(0, "TestStunServerTask::Sending Response (odd)") ;
                            }
                        }
                        else
                        {
                            if (mpEventValidator)
                            {
                                mpEventValidator->addMessage(0, "TestStunServerTask::Dropping Response (odd)") ;
                            }
                        }
                        break ;
                    case TEST_DELAY_RESPONSE:
                        OsTask::delay(300) ;
                        handleStunBindRequest(&msg, fromAddress, fromPort) ;
                        if (mpEventValidator)
                        {
                            mpEventValidator->addMessage(0, "TestStunServerTask::Sending Response (delayed)") ;
                        }
                        break ;
                    default:
                        assert(false) ;                    
                }                
                break ;
            case MSG_STUN_BIND_RESPONSE:                        
                printf("TestStunServerTask: Received unprovoked stun bind response\n") ;
                break ;
            case MSG_STUN_BIND_ERROR_RESPONSE:
                printf("TestStunServerTask: Received unprovoked stun bind error response\n") ;
                break ;
            case MSG_STUN_SHARED_SECRET_REQUEST:
                handleStunSharedSecretRequest(&msg, fromAddress, fromPort) ;
                break ;
            case MSG_STUN_SHARED_SECRET_RESPONSE:
                printf("TestStunServerTask: Received unprovoked stun shared secret response\n") ;
                break ;
            case MSG_STUN_SHARED_SECRET_ERROR_RESPONSE:
                printf("TestStunServerTask: Received unprovoked stun shared secret error response\n") ;
                break ;
            default:
                printf("TestStunServerTask: Received unknown stun message\n") ;
                break ;
        }
    }
    else
    {
        printf("TestStunServerTask: Received malformed or unsupported stun message.") ;
    }
}


void TestStunServerTask::handleStunBindRequest(StunMessage* pMsg, const UtlString& fromAddress, unsigned short fromPort) 
{
    unsigned short unknownAttributes[STUN_MAX_UNKNOWN_ATTRIBUTES] ;
    size_t nUnknownAttributes ;
    StunMessage response ;
    STUN_TRANSACTION_ID transactionId ;
    UtlString sendToAddress ;
    unsigned short sendToPort ;
    OsDatagramSocket* pSendSocket ;

    // Copy transaction id
    pMsg->getTransactionId(&transactionId) ;
    response.setTransactionId(transactionId) ;

    // Check for unknown attributes
    if (pMsg->getUnknownParsedAttributes(unknownAttributes, STUN_MAX_UNKNOWN_ATTRIBUTES, nUnknownAttributes) && 
            (nUnknownAttributes > 0))
    {
        response.setType(MSG_STUN_BIND_ERROR_RESPONSE) ;
        for (size_t i = 0; i<nUnknownAttributes; i++)
        {
            response.addUnknownAttribute(unknownAttributes[i]) ;
        }
    }
    else
    {
        response.setType(MSG_STUN_BIND_RESPONSE) ;

        // Obey XOR request
        if (pMsg->getRequestXorOnly())
        {
            response.setSendXorOnly() ;
        }

        // Set Mapped Address
        response.setMappedAddress(fromAddress, fromPort) ;

        // Set Changed address
        UtlString changeIp = mpSecondarySocket2->getLocalIp() ;
        unsigned short changePort = (unsigned short) mpSecondarySocket2->getLocalHostPort() ;
        response.setChangedAddress(changeIp, changePort) ;

        // Check for response address
        char cResponseAddress[64] ;
        unsigned short responsePort ;
        if (pMsg->getResponseAddress(cResponseAddress, responsePort))
        {
            UtlString sourceIp = mpPrimarySocket->getLocalIp() ;
            unsigned short sourcePort = (unsigned short) mpPrimarySocket->getLocalHostPort() ;

            response.setReflectedFrom(sourceIp, sourcePort) ;
            sendToAddress = cResponseAddress ;
            sendToPort = responsePort ;
        }
        else
        {
            sendToAddress = fromAddress ;
            sendToPort = fromPort ;
        }

        if (pMsg->getChangeIp())
        {
            if (pMsg->getChangePort())
            {
                pSendSocket = mpSecondarySocket2 ;
            }
            else
            {
                pSendSocket = mpSecondarySocket ;
            }
        }
        else
        {
            if (pMsg->getChangePort())
            {
                pSendSocket = mpPrimarySocket2 ;
            }
            else
            {
                pSendSocket = mpPrimarySocket ;
            }
        }
        
        // Set source address depending on change-request attributes
        UtlString sourceIp = pSendSocket->getLocalIp() ;
        unsigned short sourcePort = (unsigned short) pSendSocket->getLocalHostPort() ;
        response.setSourceAddress(sourceIp, sourcePort) ;

        sendResponse(&response, pSendSocket, sendToAddress, sendToPort) ;
    }
}


void TestStunServerTask::handleStunSharedSecretRequest(StunMessage* pMsg, const UtlString& fromAddress, unsigned short fromPort) 
{
    StunMessage response ;
    STUN_TRANSACTION_ID transactionId ;

    // Copy transaction id
    pMsg->getTransactionId(&transactionId) ;
    response.setTransactionId(transactionId) ;

    response.setType(MSG_STUN_SHARED_SECRET_ERROR_RESPONSE) ;
    response.setError(433, "Not Implemented") ;

    sendResponse(&response, mpPrimarySocket, fromAddress, fromPort) ;
}


void TestStunServerTask::sendResponse(StunMessage* pMsg, OsDatagramSocket* pSocket, const UtlString& toAddress, unsigned short toPort) 
{
    char cEncoded[4096] ;
    size_t length ;

    pMsg->setServer("sipXnatServer") ;

    if (pMsg->encode(cEncoded, sizeof(cEncoded), length))
    {
        pSocket->write(cEncoded, (int) length, toAddress, toPort) ;
    }
    else
    {
        printf("TestStunServerTask: Unable to encode message\n") ;
        assert(false) ;
    }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


