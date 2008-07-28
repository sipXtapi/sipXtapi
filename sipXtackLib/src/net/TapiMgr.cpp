// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
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
TapiMgr TapiMgr::sTapiMgr;

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
   return sTapiMgr;
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
                            const char*		 szCallId,
                            SipSession*          pSession,
                            const char*          szRemoteAddress,                   
                            SIPX_CALLSTATE_EVENT event,
                            SIPX_CALLSTATE_CAUSE cause,
                            void*                pEventData,
                            const char*          remoteAssertedIdentity)
{
    static SIPX_CALLSTATE_EVENT lastEvent = CALLSTATE_UNKNOWN;
    static SIPX_CALLSTATE_CAUSE lastCause = CALLSTATE_CAUSE_UNKNOWN;
    static char szLastCallId[256] = { 0 };
    static char szLastRemoteAddress[256]  = { 0 };
    static SipSession* pLastSession = NULL;
    
    if (sipxCallEventCallbackPtr)
    {
        if (lastEvent == event &&
            lastCause == cause &&
            strcmp(szLastRemoteAddress, szRemoteAddress) == 0 &&
            strcmp(szLastCallId, szCallId) == 0 ) 
        {
            // don't fire
        }
        else
        {
            (*sipxCallEventCallbackPtr)(pSrc, szCallId, pSession, szRemoteAddress,
                                        event, cause, pEventData,
                                        remoteAssertedIdentity);
            lastEvent = event;
            lastCause = cause;
            strncpy(szLastCallId, szCallId, sizeof(szLastCallId));
            strncpy(szLastRemoteAddress, szCallId, sizeof(szLastRemoteAddress));
            pLastSession = pSession;
        }
                                    
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
    if (sipxMediaCallbackPtr)
    {
            (*sipxMediaCallbackPtr)(pSrc, szCallId, szRemoteAddress, event, 
                    cause, type, pEventData);

    }
}
    

void TapiMgr::fireLineEvent(const void* pSrc,
                        const char* szLineIdentifier,
                        SIPX_LINESTATE_EVENT event,
                        SIPX_LINESTATE_CAUSE cause,
                        const char *bodyBytes )
{
    if (sipxLineEventCallbackPtr)
    {
        (*sipxLineEventCallbackPtr)(pSrc, 
                szLineIdentifier, 
                event, 
                cause,
                bodyBytes);
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
