//
//
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPXTAPI_EXCLUDE /* [ */

// SYSTEM INCLUDES
#if !defined(_WIN32)
#  include <stddef.h>
#endif
#include <assert.h>
// APPLICATION INCLUDES
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/SipXHandleMap.h"
#include "utl/UtlSList.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsMutex.h"
#include "utl/UtlString.h"
#include "os/OsLock.h"
#include "net/Url.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlString.h"
#include "net/SipSession.h"
#include "cp/CallManager.h"

// DEFINES

// #define DEBUG_SIPXTAPI_EVENTS

// GLOBAL VARIABLES
UtlSList        g_listeners ;
OsMutex         g_listenerLock(OsMutex::Q_FIFO) ;
UtlSList    g_lineListeners;
OsMutex     g_lineListenerLock(OsMutex::Q_FIFO);

// EXTERNAL VARIABLES
extern SipXHandleMap gCallHandleMap ;   // sipXtapiInternal.cpp

// EXTERNAL FUNCTIONS
// STRUCTURES

/* ============================ FUNCTIONS ================================= */

static const char* MajorEventToString(SIPX_CALLSTATE_MAJOR eMajor)
{
        const char* str = "Unknown" ;
        switch (eMajor)
        {
        case UNKNOWN:
            str = "UNKNOWN" ;
            break ;
                case NEWCALL:
                        str = "NEWCALL" ;
                        break ;
                case DIALTONE:
                        str = "DIALTONE" ;
                        break ;
                case REMOTE_OFFERING:
                        str = "REMOTE_OFFERING" ;
                        break ;
                case REMOTE_ALERTING:
                        str = "REMOTE_ALERTING" ;
                        break ;
                case CONNECTED:
                        str = "CONNECTED" ;
                        break ;
                case DISCONNECTED:
                        str = "DISCONNECTED" ;
                        break ;
                case OFFERING:
                        str = "OFFERING" ;
                        break ;
                case ALERTING:
                        str = "ALERTING" ;
                        break ;
                case DESTROYED:
                        str = "DESTROYED" ;
        }
   return str;
}

static const char* MinorEventToString(SIPX_CALLSTATE_MINOR eMinor)
{
        const char* str = "Unknown" ;
        switch (eMinor)
        {
                case NEW_CALL_NORMAL:
                        str = "NEW_CALL_NORMAL" ;
                        break ;
                case DIALTONE_UNKNOWN:
                        str = "DIALTONE_UNKNOWN" ;
                        break ;
                case DIALTONE_CONFERENCE:
                        str = "DIALTONE_CONFERENCE" ;
                        break ;
                case REMOTE_OFFERING_NORMAL:
                        str = "REMOTE_OFFERING_NORMAL" ;
                        break ;
                case REMOTE_ALERTING_NORMAL:
                        str = "REMOTE_ALERTING_NORMAL" ;
                        break ;
                case REMOTE_ALERTING_MEDIA:
                        str = "REMOTE_ALERTING_MEDIA" ;
                        break ;
                case CONNECTED_ACTIVE:
                        str = "CONNECTED_ACTIVE" ;
                        break ;
                case CONNECTED_ACTIVE_HELD:
                        str = "CONNECTED_ACTIVE_HELD" ;
                        break ;
                case CONNECTED_INACTIVE:
                        str = "CONNECTED_INACTIVE" ;
                        break ;
                case DISCONNECTED_BADADDRESS:
                        str = "DISCONNECTED_BADADDRESS" ;
                        break ;
                case DISCONNECTED_BUSY:
                        str = "DISCONNECTED_BUSY" ;
                        break ;
                case DISCONNECTED_NORMAL:
                        str = "DISCONNECTED_NORMAL" ;
                        break ;
                case DISCONNECTED_RESOURCES:
                        str = "DISCONNECTED_RESOURCES" ;
                        break ;
                case DISCONNECTED_NETWORK:
                        str = "DISCONNECTED_NETWORK" ;
                        break ;
                case DISCONNECTED_REDIRECTED:
                        str = "DISCONNECTED_REDIRECTED" ;
                        break ;
                case DISCONNECTED_NO_RESPONSE:
                        str = "DISCONNECTED_NO_RESPONSE" ;
                        break ;
                case DISCONNECTED_AUTH:
                        str = "DISCONNECTED_AUTH" ;
                        break ;
                case DISCONNECTED_UNKNOWN:
                        str = "DISCONNECTED_UNKNOWN" ;
                        break ;
                case OFFERING_ACTIVE:
                        str = "OFFERING_ACTIVE" ;
                        break ;
                case ALERTING_NORMAL:
                        str = "ALERTING_NORMAL" ;
                        break ;
                case DESTROYED_NORMAL:
                        str = "DESTROYED_NORMAL" ;
                        break ;
        }
        return str;
}


SIPXTAPI_API char* sipxEventToString(SIPX_CALLSTATE_MAJOR eMajor,
                                     SIPX_CALLSTATE_MINOR eMinor,
                                     char*  szBuffer,
                                     size_t nBuffer)
{
    assert(szBuffer != NULL) ;

    if (szBuffer)
    {
#ifdef WIN32
        _snprintf(szBuffer, nBuffer, "%s::%s",  MajorEventToString(eMajor), MinorEventToString(eMinor)) ;
#else
        snprintf(szBuffer, nBuffer, "%s::%s",  MajorEventToString(eMajor), MinorEventToString(eMinor)) ;
#endif
    }

    return szBuffer ;
}


void ReportCallback(SIPX_CALL hCall,
                                        SIPX_LINE hLine,
                                        SIPX_CALLSTATE_MAJOR eMajor,
                                        SIPX_CALLSTATE_MINOR eMinor,
                                        void* pUserData)
{
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;
    UtlString lineId ;
    static size_t nCnt = 0 ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, &lineId))
    {
            printf("<event i=%08X, h=%04X, c=%4d, M=%25s, m=%25s, a=%s, c=%s l=%s/>\n",
                pInst,
                hCall,
                ++nCnt,
                            MajorEventToString(eMajor),
                            MinorEventToString(eMinor),
                remoteAddress.data(),
                callId.data(),
                lineId.data()) ;
    }
}


void sipxFireEvent(const void*          pSrc,
                   const char*                  szCallId,
                   SipSession*          pSession,
                                   const char*                  szRemoteAddress,
                                   SIPX_CALLSTATE_MAJOR major,
                                   SIPX_CALLSTATE_MINOR minor)
{
        OsLock lock(g_listenerLock) ;

        SIPX_CALL_DATA* pCallData = NULL;
        SIPX_LINE hLine = NULL ;
        UtlVoidPtr* ptr = NULL;
        SIPX_CALL hCall = NULL;

    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;
    UtlString lineId ;


    // If this is an NEW inbound call (first we are hearing of it), then create
    // a call handle/data structure for it.
    if (major == NEWCALL)
    {
        pCallData = new SIPX_CALL_DATA;
        memset((void*) pCallData, 0, sizeof(SIPX_CALL_DATA));

        pCallData->callId = new UtlString(szCallId) ;
        pCallData->remoteAddress = new UtlString(szRemoteAddress) ;
        pCallData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO) ;

        Url urlFrom;
        pSession->getFromUrl(urlFrom) ;

        pCallData->lineURI = new UtlString(urlFrom.toString()) ;
        pCallData->pInst = findSessionByCallManager(pSrc) ;

        hCall = gCallHandleMap.allocHandle(pCallData) ;

        pInst = pCallData->pInst ;
        callId = szCallId ;
        remoteAddress = szRemoteAddress ;
        lineId = urlFrom.toString() ;
    }
    else
    {
        hCall = sipxCallLookupHandle(szCallId, pSrc);
        if (!sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, &lineId))
        {
            // osPrintf("event sipXtapiEvents: Unable to find call data for handle: %d\n", hCall) ;
            // osPrintf("event callid=%s address=%s", szCallId, szRemoteAddress) ;
            // osPrintf("event M=%s m=%s\n", MajorEventToString(major), MinorEventToString(minor)) ;
        }
    }


    // Find Line
    hLine = sipxLineLookupHandle(lineId.data()) ;

    // Fill in remote address
    if (szRemoteAddress)
    {
        pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE) ;
        if (pCallData)
        {
            if (pCallData->remoteAddress)
            {
                *pCallData->remoteAddress = szRemoteAddress ;
            }
            else
            {
                pCallData->remoteAddress = new UtlString(szRemoteAddress) ;
            }
            sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE) ;
        }
    }


    // Report events to subscribe listeners
        int count = 0 ;
        UtlSListIterator itor(g_listeners) ;
    while ((ptr = (UtlVoidPtr*) itor()) != NULL)
        {
            LISTENER_DATA *pData = (LISTENER_DATA*) ptr->getValue() ;
        if (pData->pInst->pCallManager == pSrc)
        {
            pData->pCallbackProc(hCall, hLine, major, minor, pData->pUserData) ;
        }
    }
#ifdef DEBUG_SIPXTAPI_EVENTS
        ReportCallback(hCall, hLine, major, minor, NULL) ;
#endif

    // If this is a DESTROY message, free up resources after all listeners
    // have been notified.
        if (DESTROYED == major)
        {
        sipxCallObjectFree(hCall);
        }
}


SIPXTAPI_API SIPX_RESULT sipxListenerAdd(const SIPX_INST hInst,
                                         CALLBACKPROC pCallbackProc,
                                         void* pUserData)
{
   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

        OsLock lock(g_listenerLock) ;

    if (hInst && pCallbackProc)
    {
            LISTENER_DATA *pData = new LISTENER_DATA ;
            pData->pCallbackProc = pCallbackProc ;
            pData->pUserData = pUserData ;
        pData->pInst = (SIPX_INSTANCE_DATA*) hInst ;

            g_listeners.append(new UtlVoidPtr(pData)) ;

        rc = SIPX_RESULT_SUCCESS ;
    }

        return rc;

}


SIPXTAPI_API SIPX_RESULT sipxListenerRemove(const SIPX_INST hInst,
                                            CALLBACKPROC pCallbackProc,
                                            void* pUserData)
{
    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

        OsLock lock(g_listenerLock) ;

        UtlVoidPtr* ptr ;

    if (hInst && pCallbackProc)
    {
            UtlSListIterator itor(g_listeners) ;
            while ((ptr = (UtlVoidPtr*) itor()) != NULL)
            {
                    LISTENER_DATA *pData = (LISTENER_DATA*) ptr->getValue() ;
                    if ((pData->pCallbackProc == pCallbackProc) &&
                            (pData->pUserData == pUserData) &&
                (pData->pInst == (SIPX_INSTANCE_DATA*) hInst))
                    {
                            g_listeners.removeReference(ptr) ;
                            delete pData ;
                            delete ptr ;

                rc = SIPX_RESULT_SUCCESS ;
                break ;
                    }
        }
        }

        return rc ;
}


void sipxDumpListeners()
{
        OsLock lock(g_listenerLock) ;

        UtlVoidPtr* ptr ;

        printf("Dumping sipXtapi Listener List:\n") ;

        int count = 0 ;
        UtlSListIterator itor(g_listeners) ;
        while ((ptr = (UtlVoidPtr*) itor()) != NULL)
        {
                LISTENER_DATA *pData = (LISTENER_DATA*) ptr->getValue() ;

                printf("\tListener %02d: inst=0x%08X proc=0x%08X, data=0x%08X\n",
                                count, pData->pInst, pData->pCallbackProc, pData->pUserData) ;

                count++ ;
        }
}


static const char* MajorLineEventToString(SIPX_LINE_EVENT_TYPE_MAJOR eMajor)
{
        const char* str = "Unknown" ;

        switch (eMajor)
        {
        case SIPX_LINE_EVENT_REGISTERING:
            str = "REGISTERING";
            break;
                case SIPX_LINE_EVENT_REGISTERED:
                        str = "REGISTERED";
                        break ;
                case SIPX_LINE_EVENT_UNREGISTERING:
                        str = "UNREGISTERING";
                        break ;
                case SIPX_LINE_EVENT_UNREGISTERED:
                        str = "UNREGISTERED";
                        break ;
                case SIPX_LINE_EVENT_REGISTER_FAILED:
                        str = "REGISTER_FAILED";
                        break ;
                case SIPX_LINE_EVENT_PROVISIONED:
                        str = "PROVISIONED";
                        break ;
                case SIPX_LINE_EVENT_UNREGISTER_FAILED:
                    str = "UNREGISTER_FAILED";
                    break;
        }
   return str;
}


SIPXTAPI_API SIPX_RESULT sipxLineListenerAdd(const SIPX_INST hInst,
                                         LINECALLBACKPROC pCallbackProc,
                                         void* pUserData)
{
   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

        OsLock lock(g_lineListenerLock) ;

    if (hInst && pCallbackProc)
    {
            LINE_LISTENER_DATA *pData = new LINE_LISTENER_DATA ;
            pData->pCallbackProc = pCallbackProc ;
            pData->pUserData = pUserData ;
        pData->pInst = (SIPX_INSTANCE_DATA*) hInst ;

            g_lineListeners.append(new UtlVoidPtr(pData)) ;

        rc = SIPX_RESULT_SUCCESS ;
    }

        return rc;
}

SIPXTAPI_API SIPX_RESULT sipxLineListenerRemove(const SIPX_INST hInst,
                                            LINECALLBACKPROC pCallbackProc,
                                            void* pUserData)
{
   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

        OsLock lock(g_lineListenerLock) ;

        UtlVoidPtr* ptr ;

    if (hInst && pCallbackProc)
    {
            UtlSListIterator itor(g_lineListeners) ;
            while ((ptr = (UtlVoidPtr*) itor()) != NULL)
            {
                    LINE_LISTENER_DATA *pData = (LINE_LISTENER_DATA*) ptr->getValue() ;
                    if ((pData->pCallbackProc == pCallbackProc) &&
                            (pData->pUserData == pUserData) &&
                (pData->pInst == (SIPX_INSTANCE_DATA*) hInst))
                    {
                            g_lineListeners.removeReference(ptr) ;
                            delete pData ;
                            delete ptr ;

                rc = SIPX_RESULT_SUCCESS ;
                break ;
                    }
        }
        }

        return rc ;
}

SIPXTAPI_API char* sipxLineEventToString(SIPX_LINE_EVENT_TYPE_MAJOR lineTypeMajor,
                                         char*  szBuffer,
                                         size_t nBuffer)
{
#ifdef WIN32
    _snprintf(szBuffer, nBuffer, MajorLineEventToString(lineTypeMajor)) ;
#else
    snprintf(szBuffer, nBuffer, MajorLineEventToString(lineTypeMajor)) ;
#endif

    return szBuffer;
}

void sipxFireLineEvent(const void* pSrc,
                        const char* szLineIdentifier,
                        SIPX_LINE_EVENT_TYPE_MAJOR major)
{
        OsLock lock(g_lineListenerLock) ;

        SIPX_LINE_DATA* pLineData = NULL;
        SIPX_LINE hLine = NULL ;

    hLine = sipxLineLookupHandle(szLineIdentifier);
    pLineData = sipxLineLookup(hLine, SIPX_LOCK_READ) ;
    if (pLineData)
    {
        UtlVoidPtr* ptr;
        // Report events to subscribe listeners
            int count = 0 ;
            UtlSListIterator itor(g_lineListeners) ;
        while ((ptr = (UtlVoidPtr*) itor()) != NULL)
            {
                LINE_LISTENER_DATA *pData = (LINE_LISTENER_DATA*) ptr->getValue() ;
            if (pData->pInst->pRefreshManager == pSrc)
            {
                pLineData->pInst = pData->pInst ;
                pData->pCallbackProc(hLine, major, pData->pUserData) ;
            }
        }
        if (major == SIPX_LINE_EVENT_UNREGISTERED)
        {
            sipxLineReleaseLock(pLineData, SIPX_LOCK_READ) ;
            pLineData = sipxLineLookup(hLine, SIPX_LOCK_WRITE) ;
            sipxLineObjectFree(pLineData) ;
        }
        else
        {
            sipxLineReleaseLock(pLineData, SIPX_LOCK_READ) ;
        }
    }
}

#endif /* ] SIPXTAPI_EXCLUDE */
