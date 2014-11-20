//
// Copyright (C) 2006-2014 SIPez LLC.  Al rights reserved.
//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Robert J. Andreasen, Jr.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "tapi/SipXEventDispatcher.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsPtrMsg.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "utl/UtlHashMapIterator.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
#define SIPX_EVENT_MSG      OsMsg::USER_START + 1
// STATIC VARIABLE INITIALIZATIONS
// MACROS    

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */


SipXEventDispatcher::SipXEventDispatcher(SIPX_INST hInst) 
    : OsServerTask("SipXEventDispatcher-%d")
    , mListenerLock(OsMutex::Q_FIFO)
{
    mhInst = hInst ;

    __sipxEventListenerAdd(mhInst, EventCallBack, this) ;
}


SipXEventDispatcher::~SipXEventDispatcher(void)
{   
    __sipxEventListenerRemove(mhInst, EventCallBack, this) ;

    waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean SipXEventDispatcher::handleMessage(OsMsg& rMsg)
{
    UtlBoolean bRet = false ;

    switch (rMsg.getMsgType())
    {
        case SIPX_EVENT_MSG:
            {
                SIPX_EVENT_CATEGORY category ;
                void* pDataCopy ;
                SIPX_RESULT rc ;

                category = (SIPX_EVENT_CATEGORY) rMsg.getMsgSubType() ;
                pDataCopy = ((OsPtrMsg&) rMsg).getPtr() ;

                serviceListeners(category, pDataCopy) ;

                rc = sipxFreeDuplicatedEvent(category, pDataCopy) ;
                assert(rc == SIPX_RESULT_SUCCESS) ;
            }
            bRet = true ;
            break ;
        default:
            break ;
    }
    return bRet;
}


bool SipXEventDispatcher::addListener(SIPX_EVENT_CALLBACK_PROC  pCallbackProc,
                                      void*                     pUserData) 
{
    OsWriteLock lock(mListenerLock) ;

    assert(pCallbackProc != NULL) ;    

    SIPX_EVENT_LISTENER_CONTEXT* pContext = new SIPX_EVENT_LISTENER_CONTEXT ;
    pContext->pCallbackProc = pCallbackProc ;
    pContext->pUserData = pUserData ;

    mListeners.insert(new UtlVoidPtr(pContext)) ;

    return true ;
}


bool SipXEventDispatcher::removeListener(SIPX_EVENT_CALLBACK_PROC  pCallbackProc,
                                         void*                     pUserData) 
{
    OsWriteLock lock(mListenerLock) ;

    assert(pCallbackProc != NULL) ;    

    UtlHashMapIterator itor(mListeners) ;
    UtlVoidPtr* pValue ;
    SIPX_EVENT_LISTENER_CONTEXT* pContext = NULL ;
    bool bRC = false ;

    while ((pValue = (UtlVoidPtr*) itor()))
    {
        pContext = (SIPX_EVENT_LISTENER_CONTEXT*) pValue->getValue() ;
        assert(pContext != NULL) ;
        if (pContext)
        {
            if (pContext->pCallbackProc == pCallbackProc &&
                pContext->pUserData == pUserData)
            {
                mListeners.destroy(pValue) ;
                // mListeners.destroy will only delete the UtlVoidPtr object, not pContext
                delete pContext;
                bRC = true ;
                break ;
            }
        }
    }

    return bRC ;
}

void SipXEventDispatcher::removeAllListeners() 
{
    OsWriteLock lock(mListenerLock) ;

    UtlHashMapIterator itor(mListeners) ;
    UtlVoidPtr* pValue ;
    SIPX_EVENT_LISTENER_CONTEXT* pContext = NULL ;

    while ((pValue = (UtlVoidPtr*) itor()))
    {
       pContext = (SIPX_EVENT_LISTENER_CONTEXT*) pValue->getValue() ;
       // delete tests for NULL value automatically
       delete pContext;
    }
    mListeners.destroyAll();
}


void SipXEventDispatcher::serviceListeners(SIPX_EVENT_CATEGORY category, 
                                           void*               pInfo)
{
    OsReadLock lock(mListenerLock) ;

    assert(pInfo) ;

    UtlHashMapIterator itor(mListeners) ;
    UtlVoidPtr* pValue ;
    SIPX_EVENT_LISTENER_CONTEXT* pContext = NULL ;
    while ((pValue = (UtlVoidPtr*) itor()))
    {
        pContext = (SIPX_EVENT_LISTENER_CONTEXT*) pValue->getValue() ;
        assert(pContext != NULL) ;
        if (pContext)
        {
            assert(pContext->pCallbackProc) ;
            if (pContext->pCallbackProc)
            {
                OsTime before;
                OsDateTime::getCurTime(before);
                pContext->pCallbackProc(category, pInfo, pContext->pUserData) ;
                OsTime after;
                OsDateTime::getCurTime(after);
                OsTime lapse = after - before;
                if(lapse.seconds() || lapse.usecs() > 100000) // more than 0.1 seconds
                {
                    OsSysLogPriority priority = PRI_DEBUG;
                    if(lapse.seconds())
                    {
                        priority = PRI_ERR;
                    }
                    else //if(lapse.usecs() > 10000)
                    {
                        priority = PRI_WARNING;
                    }
                    OsSysLog::add(FAC_SIPXTAPI, priority,
                                  "sipXtapi callback event handler(%p) was slow to return (%f seconds)."
                                  "  Care should be taken not to block in the callback for so long.",
                                  pContext->pCallbackProc,
                                  ((double) lapse.seconds()) + (((double) lapse.usecs()) / 1000000.0));
                }
            }
        }
    }
}


void SipXEventDispatcher::setInstanceHandle(SIPX_INST hNew) 
{
    mhInst = hNew ;
}


bool SIPX_CALLING_CONVENTION SipXEventDispatcher::EventCallBack(SIPX_EVENT_CATEGORY category, 
                                                                void*               pInfo, 
                                                                void*               pUserData)
{
    SipXEventDispatcher* pDispatcher = (SipXEventDispatcher*) pUserData ;
    assert(pDispatcher != NULL) ;
    if (pDispatcher != NULL)
    {
        void* pDataCopy = NULL ;
        SIPX_RESULT rc = sipxDuplicateEvent(category, pInfo, &pDataCopy) ;
        assert(rc == SIPX_RESULT_SUCCESS) ;
        if (rc == SIPX_RESULT_SUCCESS)
        {
            OsPtrMsg msg(SIPX_EVENT_MSG, (unsigned char) category, pDataCopy) ;
            if (pDispatcher->postMessage(msg) != OS_SUCCESS)
            {
                rc = sipxFreeDuplicatedEvent(category, pDataCopy) ;
                assert(rc == SIPX_RESULT_SUCCESS) ;
            }
        }
    }
       
    return true ;
}
