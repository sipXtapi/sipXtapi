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
#include "tapi/SipXMessageObserver.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/SipXHandleMap.h"
#include "net/SipUserAgent.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsEventMsg.h"
#include "os/OsLock.h"
#include "os/OsTimer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern UtlSList*	g_pEventListeners;
extern OsMutex*	    g_pEventListenerLock;
extern SipXHandleMap* gpInfoHandleMap ;   // sipXtapiInternal.cpp


// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */


SipXMessageObserver::SipXMessageObserver(const SIPX_INST hInst) :
    OsServerTask("SipXMessageObserver%d", NULL, 2000),
    mTestResponseCode(0),// if mTestResponseCode is set to a value other than 0,
                         // then this message observer can generate a test response.
                         // This feature is used by sipXtapiTest
    mhInst(hInst)
{
}

SipXMessageObserver::~SipXMessageObserver(void)
{
    waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean SipXMessageObserver::handleMessage(OsMsg& rMsg)
{
    UtlBoolean bRet = FALSE ;

    if (rMsg.getMsgType() == OsMsg::OS_EVENT)
    {
        OsEventMsg* pEventMsg = (OsEventMsg*) &rMsg ;
        int eventType ;
        pEventMsg->getUserData(eventType) ;

        switch (eventType)
        {
            case SIPXMO_NOTIFICATION_STUN:
                handleStunOutcome(pEventMsg) ;
                bRet = TRUE ;
                break ;
            default:
                if ( rMsg.getMsgSubType() == OsEventMsg::NOTIFY)
                {
                     SIPX_CALL_DATA* pData = NULL;
                     OsTimer* timer = NULL;
 
                     pEventMsg->getUserData((int&)pData);
                     pEventMsg->getEventData((int&)timer);
 
                     if(timer)
                     {
                         timer->stop();
                         delete timer;
                         timer = NULL;
                     }
                     if(pData)
                     {
                         if (pData->pMutex)
                         {
                            pData->pMutex->acquireWrite();
                            destroyCallData(pData) ;
                            pData = NULL;
                         }
                     }
                }
               break;                
        }                
    }
    else
    {
	    SipMessage* pSipMessage = (SipMessage*) ((SipMessageEvent&)rMsg).getMessage() ;
        UtlString method;
    
        pSipMessage->getRequestMethod(&method);
        
        if (pSipMessage && pSipMessage->isResponse())
        {
            // ok, the phone has received a response to a sent INFO message.
            bRet = handleIncomingInfoStatus(pSipMessage);
        }
        else if (pSipMessage && !pSipMessage->isResponse())
        {
            if (method == SIP_INFO_METHOD)
            {
                // ok, the phone has received an INFO message.
                bRet = handleIncomingInfoMessage(pSipMessage);
            }
        }        
    }
    return bRet;
}

bool SipXMessageObserver::handleIncomingInfoMessage(SipMessage* pMessage)
{
    bool bRet = false;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) pMessage->getResponseListenerData();
    
    if (NULL != pInst && NULL != pMessage)
    {

        if (mTestResponseCode != 0)  // for unit testing purposes.
        {
            if (mTestResponseCode == 408)   // a timeout response is being tested
            {
                // simulate a timeout ....
                OsTask::delay(1000);
                // respond to whomever sent us the message
	            SipMessage sipResponse;
	            sipResponse.setOkResponseData(pMessage);
                sipResponse.setResponseData(pMessage, mTestResponseCode, "timed out");	       
	            pInst->pSipUserAgent->send(sipResponse);
                return true ;
            }
        }
        else
        {
            // respond to whomever sent us the message
	        SipMessage sipResponse;
	        sipResponse.setOkResponseData(pMessage);
	        pInst->pSipUserAgent->send(sipResponse);
	    }
	    
        // Find Line
        UtlString lineId;
        pMessage->getToUri(&lineId);
        UtlString requestUri; 
    
        pMessage->getRequestUri(&requestUri); 
        SIPX_LINE hLine = sipxLineLookupHandle(lineId.data(), requestUri) ; 
        
        //if (0 != hLine)
        if (!pMessage->isResponse())
        {
            
        
            // find call
            UtlString callId;
            pMessage->getCallIdField(&callId);
            SIPX_CALL hCall = sipxCallLookupHandle(callId, pInst->pCallManager);

            if (0 == hCall)
            {
                // we are unaware of the call context
            }
            
            SIPX_INFO_DATA* pInfoData = new SIPX_INFO_DATA;
            
            memset((void*)pInfoData, 0, sizeof(SIPX_INFO_DATA));
            pInfoData->infoData.nSize = sizeof(SIPX_INFO_INFO);
            pInfoData->infoData.hCall = hCall;
            pInfoData->infoData.hLine = hLine;
            Url fromUrl;
            
            pInfoData->infoData.szFromURL = lineId.data();
            pInfoData->infoData.nContentLength = pMessage->getContentLength();
            
            // get and set the content type
            UtlString contentType;
            pMessage->getContentType(&contentType) ;
            pInfoData->infoData.szContentType = strdup(contentType.data());
            
            // get the user agent
            UtlString userAgent;
            pMessage->getUserAgentField(&userAgent);
            pInfoData->infoData.szUserAgent = strdup(userAgent.data());
            
            // get the content
            UtlString body;
            int dummyLength = pMessage->getContentLength();
            const HttpBody* pBody = pMessage->getBody();
            pBody->getBytes(&body, &dummyLength);    
            pInfoData->infoData.pContent = body.data();
            
            // set the Instance
            pInfoData->pInst = pInst;
            
            // Create Mutex
            pInfoData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO);

            UtlVoidPtr* ptr = NULL;
	        OsLock eventLock(*g_pEventListenerLock) ;
            UtlSListIterator eventListenerItor(*g_pEventListeners);
            while ((ptr = (UtlVoidPtr*) eventListenerItor()) != NULL)
            {
                EVENT_LISTENER_DATA *pData = (EVENT_LISTENER_DATA*) ptr->getValue();
                if (pData->pInst == pInfoData->pInst)
                {
                    pData->pCallbackProc(EVENT_CATEGORY_INFO, &(pInfoData->infoData), pData->pUserData);
                }
            }
            
            bRet = true;
        } // if (0 != hLine)
    } // if (NULL != pInst && NULL != pMessage)
    return bRet;
}

bool SipXMessageObserver::handleIncomingInfoStatus(SipMessage* pSipMessage)
{
    OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "SipXMessageObserver::handleIncomingInfoStatus");

    if (NULL == pSipMessage)
    {
        // something went wrong
        return false;
    }
    
    SIPX_INFO hInfo = (SIPX_INFO)pSipMessage->getResponseListenerData();
    if (hInfo)
    {
        SIPX_INFOSTATUS_INFO infoStatus;
        
        memset((void*) &infoStatus, 0, sizeof(SIPX_INFOSTATUS_INFO));
        
        infoStatus.hInfo = hInfo;
        SIPX_INFO_DATA* pInfoData = sipxInfoLookup(hInfo, SIPX_LOCK_READ, stackLogger);
        infoStatus.nSize = sizeof(SIPX_INFOSTATUS_INFO);
        infoStatus.responseCode = pSipMessage->getResponseStatusCode();
        infoStatus.event = INFOSTATUS_RESPONSE;
        
        int statusCode = pSipMessage->getResponseStatusCode();
        if (statusCode < 400)
        {
        infoStatus.status = SIPX_MESSAGE_OK;
        }
        else if (statusCode < 500)
        {
            infoStatus.status = SIPX_MESSAGE_FAILURE;
        }
        else if (statusCode < 600)
        {
            infoStatus.status = SIPX_MESSAGE_SERVER_FAILURE;
        }
        else 
        {
            infoStatus.status = SIPX_MESSAGE_GLOBAL_FAILURE;
        }
        
        UtlString sResponseText;
        pSipMessage->getResponseStatusText(&sResponseText);
        infoStatus.szResponseText = sResponseText.data();
        
        UtlVoidPtr* ptr = NULL;
	    OsLock eventLock(*g_pEventListenerLock) ;
        UtlSListIterator eventListenerItor(*g_pEventListeners);
        while ((ptr = (UtlVoidPtr*) eventListenerItor()) != NULL)
        {
            EVENT_LISTENER_DATA *pData = (EVENT_LISTENER_DATA*) ptr->getValue();
            if (pInfoData->pInst == pData->pInst)
            {
                pData->pCallbackProc(EVENT_CATEGORY_INFO_STATUS, &infoStatus, pData->pUserData);
            }
        }
        
        pInfoData->pInst->pSipUserAgent->removeMessageObserver(*(this->getMessageQueue()), (void*)hInfo);
        
        // release lock
        sipxInfoReleaseLock(pInfoData, SIPX_LOCK_READ, stackLogger);
        // info message has been handled, so go ahead and delete the object    
        sipxInfoObjectFree(hInfo);
     }
     return true;
}


bool SipXMessageObserver::handleStunOutcome(OsEventMsg* pMsg) 
{
    SIPX_CONTACT_ADDRESS sipxContact; // contact structure for notifying
                                      // sipxtapi event listeners
    SIPX_CONTACT_ADDRESS* pContact = NULL;
    pMsg->getEventData((int&)pContact) ;

    SIPX_CONFIG_INFO eventInfo ;
    memset(&eventInfo, 0, sizeof(SIPX_CONFIG_INFO)) ;
    eventInfo.nSize = sizeof(SIPX_CONFIG_INFO) ;
    if (pContact)
    {
        // first, find the user-agent, and add the contact to
        // the user-agent's db
        SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) mhInst;
        assert(pInst != NULL) ;
        pInst->pSipUserAgent->addContactAddress(*pContact);

        // If we have an external transport, also create a record for the 
        // external transport
        SIPX_CONTACT_ADDRESS externalTransportContact ;
        SIPX_CONTACT_ADDRESS* pNewContact = NULL ;

        // TODO: At the point where we support multiple external 
        // transports, this code needs to interate through ALL of
        // the external transports.
        if (pInst->pSipUserAgent->getContactDb().getRecordForAdapter(externalTransportContact, pContact->cInterface, CONTACT_LOCAL, TRANSPORT_CUSTOM))
        {
            pNewContact = new SIPX_CONTACT_ADDRESS(externalTransportContact) ;
            pNewContact->eContactType = CONTACT_NAT_MAPPED ;
            pNewContact->id = 0 ;            
            strcpy(pNewContact->cIpAddress, pContact->cIpAddress);
            pNewContact->iPort = pContact->iPort ;
            pInst->pSipUserAgent->addContactAddress(*pNewContact) ;
        }
                
        // Fire off an event for the STUN contact (normal)
        sipxContact.id = pContact->id;
        sipxContact.eContactType = CONTACT_NAT_MAPPED;
        strcpy(sipxContact.cInterface, pContact->cInterface);
        strcpy(sipxContact.cIpAddress, pContact->cIpAddress);
        sipxContact.iPort = pContact->iPort;               
        eventInfo.pData = &sipxContact;
        eventInfo.event = CONFIG_STUN_SUCCESS ;
        sipxFireEvent(this, EVENT_CATEGORY_CONFIG, &eventInfo) ;        
        delete pContact;

        // Fire off an event for the STUN contact (external transport)
        if (pNewContact)
        {
            sipxContact.id = pNewContact->id;
            sipxContact.eContactType = CONTACT_NAT_MAPPED;
            strcpy(sipxContact.cInterface, pNewContact->cInterface);
            strcpy(sipxContact.cIpAddress, pNewContact->cIpAddress);                        
            sipxContact.iPort = pNewContact->iPort;                
            eventInfo.pData = &sipxContact;
            eventInfo.event = CONFIG_STUN_SUCCESS ;
            sipxFireEvent(this, EVENT_CATEGORY_CONFIG, &eventInfo) ;
            delete pNewContact;
        }
    }
    else
    {
        eventInfo.event = CONFIG_STUN_FAILURE ;
        sipxFireEvent(this, EVENT_CATEGORY_CONFIG, &eventInfo) ;
    }
    

    return true ;
}
