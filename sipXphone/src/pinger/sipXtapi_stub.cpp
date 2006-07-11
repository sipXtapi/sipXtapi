// $Id:  $
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#include "net/SipSession.h"


enum SIPX_CALLSTATE_MAJOR
{
};

enum SIPX_CALLSTATE_MINOR
{
};

/**
 * Empty stub so that sipXcallLib will link in
 */
void sipxFireEvent(const void*          pSrc,
                   const char*			szCallId,
                   SipSession* 	        pSession,
				   const char*			szRemoteAddress,
				   SIPX_CALLSTATE_MAJOR major, 
				   SIPX_CALLSTATE_MINOR minor)
{
    return;
}