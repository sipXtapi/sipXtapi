//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "net/SipUserAgent.h"
#include "cp/CallManager.h"
#include "net/SipLineMgr.h"
#include "net/SipRefreshMgr.h"
#include "net/SipSubscribeClient.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlString.h"
#include "utl/UtlInt.h"
#include "utl/UtlVoidPtr.h"
#include "utl/UtlString.h"
#include "utl/UtlDListIterator.h"
#include "os/OsLock.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/SipXHandleMap.h"
#include "net/Url.h"
#include "net/SipUserAgent.h"
#include "cp/CallManager.h"
#include "mi/CpMediaInterfaceFactory.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// GLOBAL VARIABLES
SipXHandleMap* gpCallHandleMap = new SipXHandleMap();  /**< Global Map of call handles */
SipXHandleMap* gpLineHandleMap = new SipXHandleMap() ;  /**< Global Map of line handles */
SipXHandleMap* gpConfHandleMap = new SipXHandleMap() ;  /**< Global Map of conf handles */
SipXHandleMap* gpInfoHandleMap = new SipXHandleMap() ;  /**< Global Map of info handles */
SipXHandleMap* gpPubHandleMap = new SipXHandleMap() ;  /**< Global Map of Published (subscription server) event data handles */
SipXHandleMap* gpSubHandleMap = new SipXHandleMap() ;  /**< Global Map of Subscribed (client) event data handles */


UtlDList*  gpSessionList  = new UtlDList() ;    /**< List of sipX sessions (to be replaced 
                                                     by handle map in the future */
OsMutex*	gpSessionLock = new OsMutex(OsMutex::Q_FIFO);
static int      gSessions = 0;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

SIPX_CALL sipxCallLookupHandle(const UtlString& callID, const void* pSrc)
{
    gpCallHandleMap->lock() ;

    UtlHashMapIterator iter(*gpCallHandleMap);
   
    UtlInt* pIndex = NULL;
    UtlVoidPtr* pObj = NULL;
    SIPX_CALL hCall = 0 ;
   
    while (pIndex = dynamic_cast<UtlInt*>( iter() ) )       
    {
        pObj = dynamic_cast<UtlVoidPtr*>(gpCallHandleMap->findValue(pIndex));
        SIPX_CALL_DATA* pData = NULL ;
        if (pObj)
        {
            pData = (SIPX_CALL_DATA*) pObj->getValue() ;
        }
         
        if (pData && 
                (pData->callId->compareTo(callID) == 0 ||
                (pData->sessionCallId && (pData->sessionCallId->compareTo(callID) == 0))) && 
                pData->pInst->pCallManager == pSrc)
        {
            hCall = pIndex->getValue() ;
            break ;
        }
    }

    gpCallHandleMap->unlock() ;

    return hCall;
}


void sipxCallObjectFree(const SIPX_CALL hCall)
{
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE) ;
    
    if (pData)
    {
        const void* pRC = gpCallHandleMap->removeHandle(hCall); 
        if (pRC)
        {
            destroyCallData(pData) ;   
        }
        else
        {
            sipxCallReleaseLock(pData, SIPX_LOCK_WRITE) ;
        }
    }
}


SIPX_CALL_DATA* sipxCallLookup(const SIPX_CALL hCall, SIPX_LOCK_TYPE type)
{
    SIPX_CALL_DATA* pRC ;    

    gpCallHandleMap->lock() ;

    pRC = (SIPX_CALL_DATA*) gpCallHandleMap->findHandle(hCall) ;
    if (validCallData(pRC))
    {
        switch (type)
        {
            case SIPX_LOCK_READ:
                // TODO: What happens if this fails?
                pRC->pMutex->acquireRead() ;
                break ;
            case SIPX_LOCK_WRITE:
                // TODO: What happens if this fails?
                pRC->pMutex->acquireWrite() ;
                break ;
        }
    }
    else
    {
        pRC = NULL ;
    }

    gpCallHandleMap->unlock() ;

    return pRC ;
}


UtlBoolean validCallData(SIPX_CALL_DATA* pData)
{
    return (pData && pData->callId && 
            pData->lineURI  && 
            pData->pInst &&
            pData->pInst->pCallManager && 
            pData->pInst->pRefreshManager &&
            pData->pInst->pLineManager &&
            pData->pMutex) ;
}


void sipxCallReleaseLock(SIPX_CALL_DATA* pData, SIPX_LOCK_TYPE type) 
{
    if ((type != SIPX_LOCK_NONE) && validCallData(pData))
    {
        switch (type)
        {
            case SIPX_LOCK_READ:
                // TODO: What happens if this fails?
                pData->pMutex->releaseRead() ;
                break ;
            case SIPX_LOCK_WRITE:
                // TODO: What happens if this fails?
                pData->pMutex->releaseWrite() ;
                break ;
        }
    }
}

UtlBoolean sipxCallGetCommonData(SIPX_CALL hCall,
                                 SIPX_INSTANCE_DATA** pInst,
                                 UtlString* pStrCallId,
                                 UtlString* pStrRemoteAddress,
                                 UtlString* pLineId,
                                 UtlString* pGhostCallId) 
{
    UtlBoolean bSuccess = FALSE ;
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    if (pData)
    {
        if (pInst)
        {
            *pInst = pData->pInst ;
        }

        if (pStrCallId)
        {
            if (pData->sessionCallId)
            {
                *pStrCallId = *pData->sessionCallId ;
            }
            else
            {
                *pStrCallId = *pData->callId ;
            }
        }

        if (pStrRemoteAddress)
        {
            if (pData->remoteAddress)
            {
                *pStrRemoteAddress = *pData->remoteAddress ;
            }
            else
            {
                pStrRemoteAddress->remove(0) ;
            }
        }

        if (pLineId)
        {
            *pLineId = *pData->lineURI ;
        }
        
        if (pGhostCallId)
        {
            if (pData->ghostCallId)
            {
                *pGhostCallId = *pData->ghostCallId;
            }
        }

        bSuccess = TRUE ;

        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return bSuccess ;
}


SIPX_CONF sipxCallGetConf(SIPX_CALL hCall) 
{
    SIPX_CONF hConf = 0 ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    if (pData)
    {
        hConf = pData->hConf ;
        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return hConf ;
}


UtlBoolean sipxCallGetState(SIPX_CALL hCall, 
                            SIPX_CALLSTATE_EVENT& lastEvent,
                            SIPX_CALLSTATE_CAUSE& lastCause,
                            SIPX_INTERNAL_CALLSTATE& state) 
{
    UtlBoolean bSuccess = false ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    if (pData)
    {
        lastEvent = pData->lastCallstateEvent ;
        lastCause = pData->lastCallstateCause ;
        state = pData->state ;
        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return bSuccess ;

}


UtlBoolean sipxCallSetState(SIPX_CALL hCall, 
                            SIPX_CALLSTATE_EVENT event,
                            SIPX_CALLSTATE_CAUSE cause) 
{
    UtlBoolean bSuccess = false ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE);
    if (pData)
    {
        // Store state
        pData->lastCallstateEvent = event ;
        pData->lastCallstateCause = cause ;

        // Calculate internal state
        switch (event)
        {
            case CALLSTATE_NEWCALL:
                break ;
            case CALLSTATE_DIALTONE:
            case CALLSTATE_REMOTE_OFFERING:
            case CALLSTATE_REMOTE_ALERTING:
                pData->state = SIPX_INTERNAL_CALLSTATE_OUTBOUND_ATTEMPT ;
                break ;
            case CALLSTATE_CONNECTED:
                switch (cause)
                {
                    case CALLSTATE_CONNECTED_ACTIVE:
                        pData->state = SIPX_INTERNAL_CALLSTATE_CONNECTED ;
                        pData->bInFocus = true ;
                        break ;
                    case CALLSTATE_CONNECTED_REQUEST_NOT_ACCEPTED:
                        pData->state = SIPX_INTERNAL_CALLSTATE_CONNECTED ;
                        break ;
                    case CALLSTATE_CONNECTED_ACTIVE_HELD:                        
                        pData->state = SIPX_INTERNAL_CALLSTATE_BRIDGED ;
                        pData->bInFocus = false ;
                    case CALLSTATE_CONNECTED_INACTIVE:
                        pData->state = SIPX_INTERNAL_CALLSTATE_HELD ;
                        pData->bInFocus = false ;
                        break ;
                }
                break ;
            case CALLSTATE_DISCONNECTED:
                pData->state = SIPX_INTERNAL_CALLSTATE_DISCONNECTED ;
                pData->bInFocus = false ;
                break ;
            case CALLSTATE_OFFERING:
            case CALLSTATE_ALERTING:
                pData->state = SIPX_INTERNAL_CALLSTATE_OUTBOUND_ATTEMPT ;
                break ;                
            case CALLSTATE_DESTROYED:
                pData->bInFocus = false ;
                break ;
            case CALLSTATE_AUDIO_EVENT:
                break ;
            case CALLSTATE_TRANSFER:
                break ;
        }

        sipxCallReleaseLock(pData, SIPX_LOCK_WRITE) ;
    }

    return bSuccess ;

}



SIPX_CONTACT_TYPE sipxCallGetLineContactType(SIPX_CALL hCall) 
{
    SIPX_CONTACT_TYPE contactType = CONTACT_AUTO ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    if (pData)
    {
        SIPX_LINE_DATA* pLineData = sipxLineLookup(pData->hLine, SIPX_LOCK_READ) ;
        if (pLineData)
        {
            contactType = pLineData->contactType ;
            sipxLineReleaseLock(pLineData, SIPX_LOCK_READ) ;
        }
        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return contactType ;
}



SIPX_LINE_DATA* sipxLineLookup(const SIPX_LINE hLine, SIPX_LOCK_TYPE type)
{
    SIPX_LINE_DATA* pRC ;

    pRC = (SIPX_LINE_DATA*) gpLineHandleMap->findHandle(hLine) ;
    if (validLineData(pRC))
    {
        switch (type)
        {
            case SIPX_LOCK_READ:
                // TODO: What happens if this fails?
                pRC->pMutex->acquireRead() ;
                break ;
            case SIPX_LOCK_WRITE:
                // TODO: What happens if this fails?
                pRC->pMutex->acquireWrite() ;
                break ;
        }

    }
    else
    {
        pRC = NULL ;
    }

    return pRC ;
}

SIPX_INFO_DATA* sipxInfoLookup(const SIPX_INFO hInfo, SIPX_LOCK_TYPE type)
{
    SIPX_INFO_DATA* pRC ;

    pRC = (SIPX_INFO_DATA*) gpInfoHandleMap->findHandle(hInfo) ;
    switch (type)
    {
        case SIPX_LOCK_READ:
            // TODO: What happens if this fails?
            pRC->pMutex->acquireRead() ;
            break ;
        case SIPX_LOCK_WRITE:
            // TODO: What happens if this fails?
            pRC->pMutex->acquireWrite() ;
            break ;
    }


    return pRC ;
}

void sipxSubscribeClientSubCallback(SipSubscribeClient::SubscriptionState newState,
                                   const char* earlyDialogHandle,
                                   const char* dialogHandle,
                                   void* applicationData,
                                   int responseCode,
                                   const char* responseText,
                                   long expiration,
                                   const SipMessage* subscribeResponse)
{
    SIPX_SUB subscriptionHandle = (SIPX_SUB)applicationData;
    SIPX_SUBSCRIPTION_DATA* subscriptionData = 
        (SIPX_SUBSCRIPTION_DATA*) gpSubHandleMap->findHandle(subscriptionHandle);

    if(subscriptionData && subscriptionData->pInst)
    {
        SIPX_SUBSTATUS_INFO pInfo;
        pInfo.nSize = sizeof(SIPX_SUBSTATUS_INFO);
        UtlString userAgent;
        if(subscribeResponse)
        {
            subscribeResponse->getUserAgentField(&userAgent);
        }
        pInfo.szSubServerUserAgent = userAgent;
        pInfo.hSub = subscriptionHandle;
        // TODO: Should probably set some cause codes based upon
        // the response code from the sip message
        pInfo.cause = SUBSCRIPTION_CAUSE_NORMAL;
        UtlString errorState;

        switch(newState)
        {
        case SipSubscribeClient::SUBSCRIPTION_INITIATED: // Early dialog
            pInfo.state = SIPX_SUBSCRIPTION_PENDING;
            break;

        case SipSubscribeClient::SUBSCRIPTION_SETUP:     // Established dialog
            pInfo.state = SIPX_SUBSCRIPTION_ACTIVE;
            break;

        case SipSubscribeClient::SUBSCRIPTION_FAILED:    // Failed dialog setup or refresh
            // Could potentially differentiate between failed active
            // and failed expired based upon the expiration and the 
            // current time
            pInfo.state = SIPX_SUBSCRIPTION_FAILED;
            break;

        case SipSubscribeClient::SUBSCRIPTION_TERMINATED:
            pInfo.state = SIPX_SUBSCRIPTION_EXPIRED;
            break;

        case SipSubscribeClient::SUBSCRIPTION_UNKNOWN:
            errorState = "SUBSCRIPTION_UNKNOWN";
            pInfo.state = SIPX_SUBSCRIPTION_FAILED;
            break;

        default:
            {
                pInfo.state = SIPX_SUBSCRIPTION_FAILED;
                errorState ="unknown: ";
                char numBuf[20];
                sprintf(numBuf, "%d", newState);
                errorState.append(numBuf);
            }
            break;
        }

        // If the dialog changed from and early dialog to an 
        // established dialog, update the dialog handle in the
        // subcription data structure
        if(earlyDialogHandle && dialogHandle && 
            SipDialog::isEarlyDialog(*subscriptionData->pDialogHandle))
        {
            *(subscriptionData->pDialogHandle) = dialogHandle;
        }

        // Fire the event if it is a supported state change
        if(errorState.isNull())
        {
            sipxFireEvent(subscriptionData->pInst->pCallManager, 
                       EVENT_CATEGORY_SUB_STATUS, 
                       &pInfo);
        }
        else
        {
            OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                "sipxSubscribeClientSubCallback: invalid SubscriptionState: %s",
                errorState.data());
        }
    }

    // Cannot find subsription data for this handle
    else
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxSubscribeClientSubCallback: cannot find subscription data for handle: %p",
            applicationData);
    }
}


void sipxSubscribeClientNotifyCallback(const char* earlyDialogHandle,
                                     const char* dialogHandle,
                                     void* applicationData,
                                     const SipMessage* notifyRequest)
{
    SIPX_SUB subscriptionHandle = (SIPX_SUB)applicationData;
    SIPX_SUBSCRIPTION_DATA* subscriptionData = 
        (SIPX_SUBSCRIPTION_DATA*) gpSubHandleMap->findHandle(subscriptionHandle);

    if(subscriptionData && subscriptionData->pInst)
    {
        SIPX_NOTIFY_INFO pInfo;
        UtlString userAgent;
        UtlString contentType;
        const HttpBody* contentBody = NULL;
        int bodyLength = 0;
        const char* bodyBytes = NULL;

        // If the dialog changed from and early dialog to an 
        // established dialog, update the dialog handle in the
        // subcription data structure
        if(earlyDialogHandle && dialogHandle && 
            SipDialog::isEarlyDialog(*subscriptionData->pDialogHandle))
        {
            *(subscriptionData->pDialogHandle) = dialogHandle;
        }

        if(notifyRequest)
        {
            notifyRequest->getUserAgentField(&userAgent);
            notifyRequest->getContentType(&contentType);
            contentBody = notifyRequest->getBody();

            if(contentBody)
            {
                contentBody->getBytes(&bodyBytes, &bodyLength);
            }
        }

        pInfo.nSize = sizeof(SIPX_NOTIFY_INFO);
        pInfo.hSub = (SIPX_SUB) applicationData;
        pInfo.szNotiferUserAgent = userAgent;
        pInfo.nContentLength = bodyLength;
        pInfo.pContent = bodyBytes;
        pInfo.szContentType = contentType;
        
        sipxFireEvent(subscriptionData->pInst->pCallManager, 
           EVENT_CATEGORY_NOTIFY, 
           &pInfo);

    }

    // No data for the subscription handle
    else
    {
        OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxSubscribeClientNotifyCallback: cannot find subscription data for handle: %p",
            applicationData);
    }
}

void sipxLineReleaseLock(SIPX_LINE_DATA* pData, SIPX_LOCK_TYPE type) 
{
    if ((type != SIPX_LOCK_NONE) && validLineData(pData))
    {
        switch (type)
        {
            case SIPX_LOCK_READ:
                // TODO: What happens if this fails?
                pData->pMutex->releaseRead() ;
                break ;
            case SIPX_LOCK_WRITE:
                // TODO: What happens if this fails?
                pData->pMutex->releaseWrite() ;
                break ;
        }
    }
}


void sipxInfoReleaseLock(SIPX_INFO_DATA* pData, SIPX_LOCK_TYPE type) 
{
    if (type != SIPX_LOCK_NONE)
    {
        switch (type)
        {
            case SIPX_LOCK_READ:
                // TODO: What happens if this fails?
                pData->pMutex->releaseRead() ;
                break ;
            case SIPX_LOCK_WRITE:
                // TODO: What happens if this fails?
                pData->pMutex->releaseWrite() ;
                break ;
        }
    }
}

UtlBoolean validLineData(const SIPX_LINE_DATA* pData) 
{
    UtlBoolean bValid = FALSE ;

    if (pData && pData->lineURI && pData->pInst && 
            pData->pInst->pCallManager && pData->pMutex)
    {
        bValid = TRUE ;
    }

    return bValid ;
}


void sipxLineObjectFree(const SIPX_LINE hLine)
{
    SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_WRITE) ;
    
    if (pData)
    {
        pData->pInst->pLock->acquire() ;
        pData->pInst->nLines-- ;
        assert(pData->pInst->nLines >= 0) ;
        pData->pInst->pLock->release() ;

        const void* pRC = gpLineHandleMap->removeHandle(hLine); 
        if (pRC)
        {            
            if (pData->lineURI)
            {
                delete pData->lineURI ;
            }

            if (pData->pMutex)
            {
                delete pData->pMutex ;
            }

            if (pData->pLineAliases)
            {
                UtlVoidPtr* pValue ;
                while (pValue = (UtlVoidPtr*) pData->pLineAliases->get())
                {
                    Url* pUri = (Url*) pValue->getValue() ;
                    if (pUri)
                    {
                        delete pUri ;
                    }
                    delete pValue ;
                }
            }

            delete pData ;
        }
        else
        {
            sipxLineReleaseLock(pData, SIPX_LOCK_WRITE) ;
        }
    }
}



void sipxInfoObjectFree(SIPX_INFO hInfo)
{

    SIPX_INFO_DATA* pData = sipxInfoLookup(hInfo, SIPX_LOCK_WRITE) ;
    
    if (pData)
    {
        const void* pRC = gpInfoHandleMap->removeHandle(hInfo); 
        if (pRC)
        {
            sipxInfoFree(pData) ;   
        }
        else
        {
            sipxInfoReleaseLock(pData, SIPX_LOCK_WRITE) ;
        }
    }
}

void sipxInfoFree(SIPX_INFO_DATA* pData)
{
    if (pData)
    {
        if (pData->pMutex)
        {
            delete pData->pMutex ;
        }
        free((void*)pData->infoData.pContent);
        free((void*)pData->infoData.szContentType);
        free((void*)pData->infoData.szFromURL);
        free((void*)pData->infoData.szUserAgent);
        
        delete pData;
    }
}


SIPX_LINE sipxLineLookupHandle(const char* szLineURI)
{
    gpLineHandleMap->lock() ;

    UtlHashMapIterator iter(*gpLineHandleMap);
    Url                urlLine(szLineURI) ; 
   
    UtlInt* pIndex = NULL;
    UtlVoidPtr* pObj = NULL;
    SIPX_LINE hLine = 0 ;
   
    while (pIndex = dynamic_cast<UtlInt*>( iter() ) )       
    {
        pObj = dynamic_cast<UtlVoidPtr*>(gpLineHandleMap->findValue(pIndex));
        SIPX_LINE_DATA* pData = NULL ;
        if (pObj)
        {
            pData = (SIPX_LINE_DATA*) pObj->getValue() ;
            if (pData)
            {         
                // Check main line definition
                if (urlLine.isUserHostPortEqual(*pData->lineURI))
                {
                    hLine = pIndex->getValue() ;
                    break ;
                }

                // Check for line aliases
                if (pData->pLineAliases)
                {
                    UtlVoidPtr* pValue ;
                    Url* pUrl ;
                    UtlSListIterator iterator(*pData->pLineAliases) ;

                    while (pValue = (UtlVoidPtr*) iterator()) 
                    {
                        pUrl = (Url*) pValue->getValue() ;
                        
                        if (urlLine.isUserHostPortEqual(*pUrl))
                        {
                            hLine = pIndex->getValue() ;
                            break ;
                        }
                    }
                }
            }
        }
    }

    gpLineHandleMap->unlock() ;

    return hLine;
}


UtlBoolean sipxRemoveCallHandleFromConf(const SIPX_CONF hConf, 
                                        const SIPX_CALL hCall) 
{
    //
    // WARNING: This relies on outside locking
    // 


    UtlBoolean bFound = false ;
    SIPX_CONF_DATA* pConfData = (SIPX_CONF_DATA*) gpConfHandleMap->findHandle(hConf) ;
    size_t idx ;

    // First find the handle
    for (idx=0; idx < pConfData->nCalls; idx++)
    {
        if (pConfData->hCalls[idx] == hCall)
        {
            bFound = true ;
            break ; 
        }
    }

    if (bFound)
    {
        // Next step on it.
        pConfData->nCalls-- ;

        for (; idx < pConfData->nCalls; idx++)
        {
            pConfData->hCalls[idx] = pConfData->hCalls[idx+1] ;
        }

        pConfData->hCalls[pConfData->nCalls] = SIPX_CALL_NULL ;
    }

    return bFound ;
}


UtlBoolean validConfData(const SIPX_CONF_DATA* pData) 
{
    UtlBoolean bValid = FALSE ;

    if (pData && pData->pMutex)
    {
        bValid = TRUE ;
    }

    return bValid ;
}


SIPX_CONF_DATA* sipxConfLookup(const SIPX_CONF hConf, SIPX_LOCK_TYPE type) 
{
    SIPX_CONF_DATA* pRC = (SIPX_CONF_DATA*) gpConfHandleMap->findHandle(hConf) ;

    if (validConfData(pRC))
    {
        switch (type)
        {
            case SIPX_LOCK_READ:
                // TODO: What happens if this fails?
                pRC->pMutex->acquireRead() ;
                break ;
            case SIPX_LOCK_WRITE:
                // TODO: What happens if this fails?
                pRC->pMutex->acquireWrite() ;
                break ;
        }

    }
    else
    {
        pRC = NULL ;
    }

    return pRC ;
}


void sipxConfReleaseLock(SIPX_CONF_DATA* pData, SIPX_LOCK_TYPE type) 
{
    if ((type != SIPX_LOCK_NONE) && validConfData(pData))
    {
        switch (type)
        {
            case SIPX_LOCK_READ:
                // TODO: What happens if this fails?
                pData->pMutex->releaseRead() ;
                break ;
            case SIPX_LOCK_WRITE:
                // TODO: What happens if this fails?
                pData->pMutex->releaseWrite() ;
                break ;
        }
    }
}

void sipxConfFree(const SIPX_CONF hConf) 
{
    SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_WRITE) ;
    
    if (pData)
    {
        UtlString callId ;
        SIPX_INSTANCE_DATA* pInst = NULL ;
        const void* pRC = gpConfHandleMap->removeHandle(hConf); 
        if (pRC)
        {
            pData->pInst->pLock->acquire() ;
            pData->pInst->nConferences-- ;
            assert(pData->pInst->nConferences >= 0) ;
            pData->pInst->pLock->release() ;
            
            callId = *pData->strCallId ;
            pInst = pData->pInst ;
            
            delete pData->pMutex ;
            delete pData->strCallId;
            delete pData ;
        }
        else
        {
            sipxConfReleaseLock(pData, SIPX_LOCK_WRITE) ;
        }
        
        if (pInst && !callId.isNull())
        {
            pInst->pCallManager->drop(callId) ;
        }
    }
}

SIPX_INSTANCE_DATA* findSessionByCallManager(const void* pCallManager)
{
    SIPX_INSTANCE_DATA *pInst = NULL ;

    UtlDListIterator iter(*gpSessionList);
   
    UtlVoidPtr* pObj = NULL;
   
    while (pObj = dynamic_cast<UtlVoidPtr*>(iter()))       
    {
        SIPX_INSTANCE_DATA* pTest = (SIPX_INSTANCE_DATA*) pObj->getValue() ;
        if ((pTest) && (pTest->pCallManager == pCallManager))
        {
            pInst = pTest ;
            break ;
        }
    }
    
    return pInst ;
}

// Get the external host and port given the contact preference
void sipxGetContactHostPort(SIPX_INSTANCE_DATA* pData, 
                            SIPX_CONTACT_TYPE contactType, 
                            Url& uri)
{
    UtlBoolean bSet = FALSE  ;
    UtlString useIp ;
    int       usePort ;

    if (contactType == CONTACT_RELAY)
    {
        // Relay is not supported yet -- default to AUTO for now.
        contactType = CONTACT_AUTO  ;
    }

    // Use configured address first
    if ((contactType == CONTACT_AUTO) || (contactType == CONTACT_CONFIG))
    {
        if (pData->pSipUserAgent->getConfiguredPublicAddress(&useIp, &usePort))
        {
            uri.setHostAddress(useIp) ;
            uri.setHostPort(usePort) ;
            bSet = TRUE ;
        }
    }

    // Use NAT_MAPPED next
    if (!bSet && ((contactType == CONTACT_AUTO) || (contactType == CONTACT_NAT_MAPPED)))
    {
        if (pData->pSipUserAgent->getNatMappedAddress(&useIp, &usePort))
        {
            uri.setHostAddress(useIp) ;
            uri.setHostPort(usePort) ;
            bSet = TRUE ;
        }
    }

    // Lastly, use local
    if (!bSet)
    {
        if (pData->pSipUserAgent->getLocalAddress(&useIp, &usePort))
        {
            uri.setHostAddress(useIp) ;
            uri.setHostPort(usePort) ;
            bSet = TRUE ;
        }
    }
}
  

SIPXTAPI_API void sipxLogEntryAdd(OsSysLogPriority priority, 
                     const char *format,
                     ...)
{
    va_list ap;
    va_start(ap, format);

    int threadId;
    OsTask::getCurrentTaskId(threadId) ;
    OsSysLog::vadd("sipXtapi", threadId, FAC_SIPXTAPI, priority, format, ap);  
     
    va_end(ap);
}

SIPXTAPI_API SIPX_RESULT sipxConfigAllowMethod(const SIPX_INST hInst, const char* method, const bool bAllow)
{
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;
    
    if (pInst)
    {
        pInst->pSipUserAgent->allowMethod(method, bAllow);
    }
    return SIPX_RESULT_SUCCESS;
}

void sipxIncSessionCount()
{
    OsLock lock(*gpSessionLock);

    ++gSessions;
}

void sipxDecSessionCount()
{
    OsLock lock(*gpSessionLock);
    --gSessions;
}

int sipxGetSessionCount()
{
    return gSessions;
}


UtlBoolean sipxIsCallInFocus() 
{
    UtlBoolean inFocus = false ;
    gpCallHandleMap->lock() ;

    UtlHashMapIterator iter(*gpCallHandleMap);
   
    UtlInt* pIndex = NULL;
    UtlVoidPtr* pObj = NULL;
    SIPX_CALL hCall = 0 ;
   
    while (pIndex = dynamic_cast<UtlInt*>( iter() ) )       
    {
        pObj = dynamic_cast<UtlVoidPtr*>(gpCallHandleMap->findValue(pIndex));
        SIPX_CALL_DATA* pData = NULL ;
        if (pObj)
        {
            pData = (SIPX_CALL_DATA*) pObj->getValue() ;
            if (pData->bInFocus)
            {
                inFocus = true ;
                break ;
            }
        }
    }

    gpCallHandleMap->unlock() ;

    return inFocus ;
}


SIPX_RESULT sipxGetActiveCallIds(SIPX_INST hInst, int maxCalls, int& actualCalls, UtlString callIds[]) 
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;

    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;
    if (pInst)
    {
        OsStatus status = pInst->pCallManager->getCalls(maxCalls, actualCalls, callIds) ;
        if (status == OS_SUCCESS)
        {
            rc = SIPX_RESULT_SUCCESS ;
        }
    }
    else
    {
        rc = SIPX_RESULT_INVALID_ARGS ;
    }

    return rc ;
}


SIPX_RESULT sipxCheckForHandleLeaks() 
{
    SIPX_RESULT rc = SIPX_RESULT_SUCCESS ;

    if (gpCallHandleMap->entries() != 0)
    {
        printf("\ngpCallHandleMap Leaks (%d):\n", gpCallHandleMap->entries()) ;
        gpCallHandleMap->dump() ;
        rc = SIPX_RESULT_FAILURE ;
    }

    if (gpLineHandleMap->entries() != 0)
    {
        printf("\ngpLineHandleMap Leaks (%d):\n", gpLineHandleMap->entries()) ;
        gpLineHandleMap->dump() ;
        rc = SIPX_RESULT_FAILURE ;
    }

    if (gpConfHandleMap->entries() != 0)
    {
        printf("\ngpConfHandleMap Leaks (%d):\n", gpConfHandleMap->entries()) ;
        gpConfHandleMap->dump() ;
        rc = SIPX_RESULT_FAILURE ;
    }

    if (gpInfoHandleMap->entries() != 0)
    {
        printf("\ngpInfoHandleMap Leaks (%d):\n", gpInfoHandleMap->entries()) ;
        gpInfoHandleMap->dump() ;
        rc = SIPX_RESULT_FAILURE ;
    }

    if (gpSessionList->entries() != 0)
    {
        printf("\ngSessionList leaks (%d)\n", gpSessionList->entries()) ;
        rc = SIPX_RESULT_FAILURE ;
    }

    return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetConnectionMediaInterface(const SIPX_CALL hCall,
                                                             void** ppInstData)
{
    SIPX_RESULT sr = SIPX_RESULT_FAILURE;
    int connectionId = -1;
    UtlString callId ;
    UtlString remoteAddress ;
    
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    
    assert(pData != 0);
    assert(pData->pInst != 0);
    
    if (pData && pData->callId && pData->remoteAddress)
    {
        callId = *pData->callId ;
        remoteAddress = *pData->remoteAddress ;
    }
    
    if (pData)
    {
        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }    
    
    if (!callId.isNull() && !remoteAddress.isNull())
    {
        connectionId = pData->pInst->pCallManager->getMediaConnectionId(callId, remoteAddress, ppInstData);
        if (-1 != connectionId)
        {
            sr = SIPX_RESULT_SUCCESS;
        }
    }

    return sr;
}       

#ifdef VOICE_ENGINE
#include "include\VoiceEngineMediaInterface.h"
SIPXTAPI_API GipsVoiceEngineLib* sipxCallGetVoiceEnginePtr(const SIPX_CALL hCall)
{
    VoiceEngineMediaInterface* pMediaInterface = NULL;
    GipsVoiceEngineLib* pLib = NULL;
    
    if (hCall)
    {
        sipxCallGetConnectionMediaInterface(hCall, (void**)&pMediaInterface);

        if (pMediaInterface)
        {
            pLib = pMediaInterface->getVoiceEnginePtr();
        }
    }
    return pLib;    
}

SIPXTAPI_API GipsVoiceEngineLib* sipxConfigGetVoiceEnginePtr(const SIPX_INST hInst)
{
    GipsVoiceEngineLib* ptr = NULL;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        VoiceEngineFactoryImpl* pInterface =
                static_cast<VoiceEngineFactoryImpl*>(pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation());
        if (pInterface)
        {
            ptr = pInterface->getVoiceEnginePointer();
        }
    }

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetVoiceEnginePtr hInst=%x, ptr=%08X",
        hInst, ptr);
    return ptr;
}

SIPXTAPI_API GIPSAECTuningWizard* sipxConfigGetVoiceEngineAudioWizard()
{
    GIPSAECTuningWizard& wizard = GetGIPSAECTuningWizard();
    
    return &wizard;
}

#ifdef VIDEO
SIPXTAPI_API GipsVideoEngineWindows* sipxConfigGetVideoEnginePtr(const SIPX_INST hInst)
{
    GipsVideoEngineWindows* ptr = NULL;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst ;

    if (pInst)
    {
        VoiceEngineFactoryImpl* pImpl = (VoiceEngineFactoryImpl *) pInst->pCallManager->getMediaInterfaceFactory()->getFactoryImplementation();
        if (pImpl)
        {
            ptr = pImpl->getVideoEnginePointer();
        }
    }

    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigGetVideoEnginePtr hInst=%x, ptr=%08X",
        hInst, ptr);
    return ptr;
}
#endif

void GIPSVETraceCallback(char *szMsg, int iNum)
{
    OsSysLog::add(FAC_AUDIO, PRI_DEBUG,
            "%s (%d)",
            szMsg, 
            iNum);
}


SIPXTAPI_API SIPX_RESULT sipxEnableAudioLogging(const SIPX_INST hInst, bool bEnable)
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE;
    GipsVoiceEngineLib* ptr = sipxConfigGetVoiceEnginePtr(hInst);

    if (NULL != ptr)
    {
        if (bEnable)
        {
	        int irc = ptr->GIPSVE_SetTraceCallback(GIPSVETraceCallback);
        }
        else
        {
	        int irc = ptr->GIPSVE_SetTraceCallback(NULL);
        }
        rc = SIPX_RESULT_SUCCESS;
    }

    return rc;
}

#endif

SIPXTAPI_API SIPX_RESULT sipxTranslateToneId(const TONE_ID toneId,
                                             TONE_ID& xlateId)
{
    SIPX_RESULT sr = SIPX_RESULT_SUCCESS;
    xlateId = (TONE_ID)0;
#ifdef VOICE_ENGINE
    if (toneId >= '0' && toneId <= '9')
    {
        xlateId = (TONE_ID)(toneId - '0');
    } 
    else if (toneId == ID_DTMF_STAR)
    {
        xlateId = (TONE_ID)10;
    }
    else if (toneId == ID_DTMF_POUND)
    {
        xlateId = (TONE_ID)11;
    }
    else if (toneId == ID_DTMF_FLASH)
    {
        xlateId = (TONE_ID)16;
    }
    else
    {
        sr = SIPX_RESULT_FAILURE;
    }
#else
    if (toneId != ID_DTMF_FLASH)
    {
        xlateId = toneId;
    }
    else
    {
        sr = SIPX_RESULT_FAILURE;
    }
#endif /* VOICE_ENGINE */

    return sr;
}

SIPXTAPI_API SIPX_RESULT sipxConfigVoicemailSubscribe(const SIPX_INST hInst,
                                                      const char* szSubscribeURL)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxConfigVoicemailSubscribe hInst=%p URL=%s",
        hInst, szSubscribeURL);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) hInst;

    assert(pInst);
    if (pInst)
    {
        assert(pInst->pRefreshManager);
        assert(pInst->pLineManager);
        assert(pInst->pSipUserAgent);
        if (pInst->pRefreshManager && pInst->pLineManager && pInst->pSipUserAgent)
        {
            if (szSubscribeURL != NULL)
            {
                UtlString userId;
                UtlString contactUri;
                UtlString outboundLine;
                Url subscribeUrl(szSubscribeURL);
                subscribeUrl.getUserId(userId);

                pInst->pLineManager->getDefaultOutboundLine(outboundLine);

                // If the PHONESET_MSG_WAITING_SUBSCRIBE setting does
                // not have a userid field specified, get the one
                // from the default outbound line and use that
                if( userId.isNull() )
                {
                    // get default outbound line from the line manager
                    Url outbound(outboundLine);
                    outbound.getUserId(userId);
                    subscribeUrl.setUserId(userId);
                }

                // make sure we associate a user with the line
                pInst->pSipUserAgent->getContactUri( &contactUri );
                Url contactForLine ( contactUri );
                contactForLine.setUserId( userId );

                UtlString contactforLineStr = contactForLine.toString();

                SipMessage* mwiSubscribeRequest = new SipMessage();

                int sipSubscribePeriodSeconds;

                // get the Subscribe Period from the RefreshManager
                sipSubscribePeriodSeconds = pInst->pRefreshManager->getSubscribeTimeout();

                mwiSubscribeRequest->setVoicemailData (
                    outboundLine.data(),           // fromField
                    outboundLine.data(),           // toField
                    subscribeUrl.toString().data(),// uri
                    contactforLineStr.data(),      // contactUrl
                    NULL,                          // callId, do not set here
                    1,                             // cseq
                    sipSubscribePeriodSeconds);    // expiresInSeconds

                // send the subscribe request to the refresh manager
                pInst->pRefreshManager->newSubscribeMsg( *mwiSubscribeRequest );
                rc = SIPX_RESULT_SUCCESS;
            }
        }
    }

    return rc;
}

UtlBoolean sipxCallSetRemoveInsteadofDrop(SIPX_CALL hCall) 
{
    UtlBoolean bSuccess = FALSE ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE);
    if (pData)
    {
        pData->bRemoveInsteadOfDrop = TRUE ;        
        bSuccess = TRUE ;

        sipxCallReleaseLock(pData, SIPX_LOCK_WRITE) ;
    }

    return bSuccess ;
}


UtlBoolean sipxCallIsRemoveInsteadOfDropSet(SIPX_CALL hCall)
{
    UtlBoolean bShouldRemove = FALSE ;

    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ);
    if (pData)
    {
        bShouldRemove = pData->bRemoveInsteadOfDrop ;                

        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
    }

    return bShouldRemove ;
}


