// $Id$
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef SIPXTAPI_EXCLUDE

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
    sipxCallEventCallbackPtr(0),
    sipxLineEventCallbackPtr(0)
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

// setting the Line event callback pointer
void TapiMgr::setTapiLineCallback(sipxLineEventCallbackFn fp)
{
    sipxLineEventCallbackPtr = fp;
}

void TapiMgr::fireCallEvent(const void*          pSrc,
                       const char*		    szCallId,
                       SipSession*          pSession,
				       const char*          szRemoteAddress,                   
				       SIPX_CALLSTATE_MAJOR eMajorState,
				       SIPX_CALLSTATE_MINOR eMinorState)
{
    if (sipxCallEventCallbackPtr)
    {
        (*sipxCallEventCallbackPtr)(pSrc, szCallId, pSession, szRemoteAddress, eMajorState, eMinorState);
    }
    return;
}

void TapiMgr::fireLineEvent(const void* pSrc,
                        const char* szLineIdentifier,
                        SIPX_LINE_EVENT_TYPE_MAJOR major)
{
    if (sipxLineEventCallbackPtr)
    {
        (*sipxLineEventCallbackPtr)(pSrc, szLineIdentifier, major);
    }
}
#endif
