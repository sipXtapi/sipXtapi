// $Id$
//
// Copyright (C) 2005 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPXTAPI_EXCLUDE /* [ */

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHashMap.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlString.h"
#include "utl/UtlInt.h"
#include "utl/UtlVoidPtr.h"
#include "utl/UtlString.h"
#include "utl/UtlDListIterator.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/SipXHandleMap.h"
#include "net/Url.h"
#include "net/SipUserAgent.h"
#include "cp/CallManager.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// GLOBAL VARIABLES
SipXHandleMap gCallHandleMap ;  /**< Global Map of call handles */
SipXHandleMap gLineHandleMap ;  /**< Global Map of line handles */
SipXHandleMap gConfHandleMap ;  /**< Global Map of conf handles */
UtlDList      gSessionList ;    /**< List of sipX sessions (to be replaced 
                                     by handle map in the future */

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
    gCallHandleMap.lock() ;

    UtlHashMapIterator iter(gCallHandleMap);
   
    UtlInt* pIndex = NULL;
    UtlVoidPtr* pObj = NULL;
    SIPX_CALL hCall = 0 ;
   
    while (pIndex = dynamic_cast<UtlInt*>( iter() ) )       
    {
        pObj = dynamic_cast<UtlVoidPtr*>(gCallHandleMap.findValue(pIndex));
        SIPX_CALL_DATA* pData = NULL ;
        if (pObj)
        {
            pData = (SIPX_CALL_DATA*) pObj->getValue() ;
        }
         
        if (pData && pData->callId->compareTo(callID) == 0 && pData->pInst->pCallManager == pSrc)
        {
            hCall = pIndex->getValue() ;
            break ;
        }
    }

    gCallHandleMap.unlock() ;

    return hCall;
}


void sipxCallObjectFree(const SIPX_CALL hCall)
{
    SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE) ;
    gCallHandleMap.removeHandle(hCall) ;
    
    if (pData)
    {
        destroyCallData(pData) ;   
    }
}


SIPX_CALL_DATA* sipxCallLookup(const SIPX_CALL hCall, SIPX_LOCK_TYPE type)
{
    SIPX_CALL_DATA* pRC ;    

    gCallHandleMap.lock() ;

    pRC = (SIPX_CALL_DATA*) gCallHandleMap.findHandle(hCall) ;
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

    gCallHandleMap.unlock() ;

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
                                 UtlString* pLineId) 
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
            *pStrCallId = *pData->callId ;
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

        bSuccess = TRUE ;

        sipxCallReleaseLock(pData, SIPX_LOCK_READ) ;
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

    pRC = (SIPX_LINE_DATA*) gLineHandleMap.findHandle(hLine) ;
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


void sipxLineObjectFree(SIPX_LINE_DATA* pData)
{
    if (pData)
    {
        if (pData->lineURI)
        {
            delete pData->lineURI ;
        }
        if (pData->pMutex)
        {
            delete pData->pMutex ;
        }
        delete pData ;
    }
}


SIPX_LINE sipxLineLookupHandle(const char* szLineURI)
{
    gLineHandleMap.lock() ;

    UtlHashMapIterator iter(gLineHandleMap);
    Url                urlLine(szLineURI) ; 
   
    UtlInt* pIndex = NULL;
    UtlVoidPtr* pObj = NULL;
    SIPX_LINE hLine = 0 ;
   
    while (pIndex = dynamic_cast<UtlInt*>( iter() ) )       
    {
        pObj = dynamic_cast<UtlVoidPtr*>(gLineHandleMap.findValue(pIndex));
        SIPX_LINE_DATA* pData = NULL ;
        if (pObj)
        {
            pData = (SIPX_LINE_DATA*) pObj->getValue() ;
            if (pData)
            {
                Url urlDataLine(pData->lineURI->toString()) ;
         
                if (urlLine.isUserHostPortEqual(urlDataLine))
                {
                    hLine = pIndex->getValue() ;
                    break ;
                }
            }
        }
    }

    gLineHandleMap.unlock() ;

    return hLine;
}


UtlBoolean sipxRemoveCallHandleFromConf(const SIPX_CONF hConf, 
                                        const SIPX_CALL hCall) 
{
    //
    // WARNING: This relies on outside locking
    // 


    UtlBoolean bFound = false ;
    SIPX_CONF_DATA* pConfData = (SIPX_CONF_DATA*) gConfHandleMap.findHandle(hConf) ;
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

        pConfData->hCalls[pConfData->nCalls] = NULL ;
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
    SIPX_CONF_DATA* pRC = (SIPX_CONF_DATA*) gConfHandleMap.findHandle(hConf) ;

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
    gConfHandleMap.removeHandle(hConf) ;
    pData->pInst->pCallManager->drop(pData->strCallId->data()) ;
    delete pData->pMutex ;
    delete pData ;
}

SIPX_INSTANCE_DATA* findSessionByCallManager(const void* pCallManager)
{
    SIPX_INSTANCE_DATA *pInst = NULL ;

    UtlDListIterator iter(gSessionList);
   
    UtlVoidPtr* pObj = NULL;
    SIPX_CALL hCall = 0 ;
   
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
  

#endif /* ] SIPXTAPI_EXCLUDE */

