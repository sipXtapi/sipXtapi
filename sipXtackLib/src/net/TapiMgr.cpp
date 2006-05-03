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
#include "net/TapiMgr.h"
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
TapiMgr* TapiMgr::spTapiMgr = 0;

// Constructor - private, this is a singleton
TapiMgr::TapiMgr() :
    sipxCallEventCallbackPtr(NULL),
    sipxLineEventCallbackPtr(NULL),
    sipxMediaCallbackPtr(NULL)
{
}

// destructor - private
TapiMgr::~TapiMgr()
{
}

// static accessor for the singleton instance
TapiMgr& TapiMgr::getInstance()
{
   if (TapiMgr::spTapiMgr == 0)
   {
      TapiMgr::spTapiMgr = new TapiMgr();
   }
   return *(TapiMgr::spTapiMgr);
}

// setting the Call event callback pointer
void TapiMgr::setTapiCallCallback(sipxCallEventCallbackFn fp)
{
    sipxCallEventCallbackPtr = fp;
}

void TapiMgr::setTapiMediaCallback(sipxMediaCallbackFn fp) 
{
    sipxMediaCallbackPtr = fp;
}

// setting the Line event callback pointer
void TapiMgr::setTapiLineCallback(sipxLineEventCallbackFn fp)
{
    sipxLineEventCallbackPtr = fp;
}

void TapiMgr::setTapiCallback(sipxEventCallbackFn fp)
{
    sipxEventCallbackPtr = fp;
}

void TapiMgr::fireCallEvent(const void*          pSrc,
                            const char*		     szCallId,
                            SipSession*          pSession,
				            const char*          szRemoteAddress,                   
				            SIPX_CALLSTATE_EVENT event,
				            SIPX_CALLSTATE_CAUSE cause,
                            void*                pEventData)
{
    if (sipxCallEventCallbackPtr)
    {
        (*sipxCallEventCallbackPtr)(pSrc, szCallId, pSession, szRemoteAddress,
                                    event, cause, pEventData);
    }
    return;
}


void TapiMgr::fireMediaEvent(const void*         pSrc,
                             const char*         szCallId,
                             const char*         szRemoteAddress,
                             SIPX_MEDIA_EVENT    event,
                             SIPX_MEDIA_CAUSE    cause,
                             SIPX_MEDIA_TYPE     type,
                             void*               pEventData) 
{
    static void* lastSrc;
    static char lastCallId[256];
    static char lastRemoteAddress[256];
    static SIPX_MEDIA_EVENT lastEvent;
    static SIPX_MEDIA_CAUSE lastCause;
    static SIPX_MEDIA_TYPE lastType;
    static void* lastEventData;

    if (sipxMediaCallbackPtr)
    {
        // Commenting out for now - detection of duplicate events is handled in tapi layer
        /*if (lastSrc != pSrc ||
            strcmp(lastCallId, szCallId) != 0 ||
            strcmp(lastRemoteAddress, szRemoteAddress) != 0 ||
            lastEvent != event ||
            lastCause != cause ||
            lastType != type ||
            lastEventData != pEventData)
        {*/
            (*sipxMediaCallbackPtr)(pSrc, szCallId, szRemoteAddress, event, 
                    cause, type, pEventData);

            lastSrc = (void*)pSrc;
            strncpy(lastCallId, szCallId, sizeof(lastCallId));
            strncpy(lastRemoteAddress, szRemoteAddress, sizeof(lastRemoteAddress));
            lastEvent = event;
            lastCause = cause;
            lastType = type;
            lastEventData = pEventData;
        //}
    }
}
    

void TapiMgr::fireLineEvent(const void* pSrc,
                        const char* szLineIdentifier,
                        SIPX_LINESTATE_EVENT event,
                        SIPX_LINESTATE_CAUSE cause)
{
    if (sipxLineEventCallbackPtr)
    {
        (*sipxLineEventCallbackPtr)(pSrc, szLineIdentifier, event, cause);
    }
}

bool TapiMgr::fireEvent(const void* pSrc, const SIPX_EVENT_CATEGORY event, void *pInfo)
{
    if (sipxEventCallbackPtr)
    {
        return (*sipxEventCallbackPtr)(pSrc, event, pInfo);
    }
    else
    {
        return false;
    }
}
