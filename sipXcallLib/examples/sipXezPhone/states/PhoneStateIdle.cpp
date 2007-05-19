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
#include "../stdwx.h"
#include "../sipXmgr.h"
#include "PhoneStateIdle.h"
#include "PhoneStateDialing.h"
#include "PhoneStateRinging.h"
#include "../sipXezPhoneApp.h"
#include "../sipXezPhoneSettings.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateIdle::PhoneStateIdle(void)
{
}

PhoneStateIdle::~PhoneStateIdle(void)
{
}

PhoneState* PhoneStateIdle::OnDial(const wxString number)
{
   PhoneState* nextState = new PhoneStateDialing(number);
   
   return nextState;
}

PhoneState* PhoneStateIdle::OnRinging(SIPX_CALL hCall)
{
   PhoneState* nextState = new PhoneStateRinging(hCall);
   
   return nextState;
}

PhoneState* PhoneStateIdle::Execute()
{
   
   thePhoneApp->setStatusMessage("Idle.");
   return this;
}

PhoneState* PhoneStateIdle::OnOffer(SIPX_CALL hCall)
{
    SIPX_VIDEO_DISPLAY display;
    SIPX_SECURITY_ATTRIBUTES security;
    
    // get remote sip url
    char szIncomingID[256];
    sipxCallGetRemoteID(hCall, szIncomingID, 256);    
    
    const UtlString* pCertFile = sipXezPhoneSettings::getInstance().lookupCertificate(szIncomingID);
    
    display.handle = sipXmgr::getInstance().getVideoWindow();
#ifdef DIRECT_SHOW_RENDER    
    display.type = DIRECT_SHOW_FILTER;
#else
    display.type = SIPX_WINDOW_HANDLE_TYPE;
#endif    

    SIPX_VIDEO_DISPLAY* pDisplay = NULL;
    SIPX_SECURITY_ATTRIBUTES* pSecurity = NULL;
    
    bool bSecurity = false;
    sipXezPhoneSettings::getInstance().getSecurityEnabled(bSecurity);
    if (bSecurity && pCertFile)
    {
        int iLevel;
        UtlString srtpKey;
        UtlString publicKey;
        UtlString dbLocation;
        UtlString certDbPassword;
        UtlString certNickname;

        sipXezPhoneSettings::getInstance().getSrtpParameters(iLevel);
        sipXezPhoneSettings::getInstance().getSmimeParameters(dbLocation,
                                                            certNickname,
                                                            certDbPassword);
        sipXmgr::getInstance().getSrtpKey(srtpKey);

        UtlString der;
        sipXmgr::getInstance().readPublicKeyFile(der, pCertFile->data());
        security.setSmimeKey(der.data(), der.length());


        security.setSecurityLevel((SIPX_SRTP_LEVEL)iLevel);
        sipxConfigSetSecurityParameters(sipXmgr::getInstance().getSipxInstance(), dbLocation, certNickname, certDbPassword);
        
        // the callee does not specify the srtp key, null it out
        security.setSrtpKey("", 30);
        pSecurity = &security;
    }

	if (thePhoneApp->getFrame().getVideoVisible())
	{
	    pDisplay = &display;
    }
    SIPX_CALL_OPTIONS options;
    memset((void*)&options, 0, sizeof(SIPX_CALL_OPTIONS));
    options.cbSize = sizeof(SIPX_CALL_OPTIONS);
    options.sendLocation = sipXmgr::getInstance().isLocationHeaderEnabled();
    options.bandwidthId =  AUDIO_CODEC_BW_DEFAULT;

    sipxCallAccept(hCall, pDisplay, pSecurity, &options);

    // just accept the call
    // We don't need to explicitly change state,
    // because sipXtapi should now send us
    // an ALERTING event, which will cause a state
    // change.
    return this;
}
