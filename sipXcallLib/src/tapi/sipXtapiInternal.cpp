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
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/SipXHandleMap.h"
#include "net/Url.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// GLOBAL VARIABLES
SipXHandleMap gCallHandleMap ;  /**< Global Map of call handles */
SipXHandleMap gLineHandleMap ;  /**< Global Map of line handles */
SipXHandleMap gConfHandleMap ;  /**< Global Map of conf handles */


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
    // osPrintf("event: killing hCall %d\n", hCall) ;
   
    destroyCallData((SIPX_CALL_DATA*) sipxCallLookup(hCall)) ;   
    gCallHandleMap.removeHandle(hCall) ;    
}

SIPX_CALL_DATA* sipxCallLookup(const SIPX_CALL hCall)
{
    return (SIPX_CALL_DATA*) gCallHandleMap.findHandle(hCall) ;
}


SIPX_LINE_DATA* sipxLineLookup(const SIPX_LINE hLine)
{
    return (SIPX_LINE_DATA*) gLineHandleMap.findHandle(hLine) ;
}


void sipxLineObjectFree(const SIPX_CALL hLine)
{
    gLineHandleMap.lock() ;

    SIPX_LINE_DATA* pData = sipxLineLookup(hLine) ;
    if (pData)
    {
        if (pData->lineURI)
        {
            delete pData->lineURI ;
        }
        delete pData ;
    }

    gLineHandleMap.removeHandle(hLine) ;
                
    gLineHandleMap.unlock() ;
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

#endif /* ] SIPXTAPI_EXCLUDE */

