//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "../stdwx.h"
#include "../sipXmgr.h"
#include "PhoneStateIdle.h"
#include "PhoneStateDialing.h"
#include "PhoneStateRinging.h"
#include "../sipXezPhoneApp.h"

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
    
    display.handle = sipXmgr::getInstance().getVideoWindow();
    display.type = SIPX_WINDOW_HANDLE_TYPE;
    
    sipxCallAccept(hCall, &display);
                             // just accept the call
                            // We don't need to explicitly change state,
                            // because sipXtapi should now send us
                            // an ALERTING event, which will cause a state
                            // change.
                            
                        
    return this;
}
