// 
// 
//
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/sipXtapi.h>
#include <tapi/sipXtapiEvents.h>
#include <tapi/sipXtapiInternal.h>
#include <tapi/SipXtapiEventDispatcher.h>
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

//__________________________________________________________________________//
/* ============================= P U B L I C ============================== */

/* ============================ C R E A T O R S =========================== */

// Constructor
SipXtapiEventDispatcher::SipXtapiEventDispatcher(SIPX_INSTANCE_DATA& instance)
{
    mpInstance = &instance;
}

// Copy constructor
SipXtapiEventDispatcher::SipXtapiEventDispatcher(const SipXtapiEventDispatcher& rSipXtapiEventDispatcher)
{
}

// Destructor
SipXtapiEventDispatcher::~SipXtapiEventDispatcher()
{
}

/* ======================== M A N I P U L A T O R S ======================= */

// Assignment operator
SipXtapiEventDispatcher& SipXtapiEventDispatcher::operator=(const SipXtapiEventDispatcher& rhs)
{
    if (this == &rhs)            // handle the assignment to self case
    {
        // Do nothing
    }

    else
    {
    }

   return *this;
}

UtlBoolean SipXtapiEventDispatcher::handleMessage(OsMsg& message)
{
    int messageType = message.getMsgType(); 
    int messageSubType = message.getMsgSubType(); 

    if (messageType == OsMsg::TAO_MSG)
    {
        if(messageSubType == TaoMessage::EVENT)
        {
            TaoMessage* taoMessage = (TaoMessage*) (&message) ; 

            TaoString args = TaoString(taoMessage->getArgList(), TAOMESSAGE_DELIMITER); 
            UtlString callId = args[0];
            UtlString toneInfo = args[1];
            UtlString connectionId = args[2];

            unsigned int toneInfoBits = atoi(toneInfo.data());

            int dtmfButton = (toneInfoBits >> 16);
            int pressDuration = (toneInfoBits & 0xFFFF);
            int buttonUp = (toneInfoBits & 0x80000000);

            if(buttonUp)
            {
                dtmfButton = dtmfButton & 0x0003FFF;
            }
  
            // Fire the appropriate sipXtapi event
            SIPX_CALLSTATE_MINOR sipXtapiDtmfMinorEvent;
            dtmfToSipXtapiMinorEvent(dtmfButton, sipXtapiDtmfMinorEvent);
            osPrintf("SipXtapiEventDispatcher::handleMessage DTMF %s for key: %d sipXtapi minor event: %d on call: %s\n",
                buttonUp ? "up" : "down",
                dtmfButton,
                sipXtapiDtmfMinorEvent,
                callId.data());

            sipxFireEvent(mpInstance->pCallManager,
                          callId,
                          NULL, //SipSession*
                          NULL,
                          buttonUp ? DTMF_UP : DTMF_DOWN,
                          sipXtapiDtmfMinorEvent);


        }
    }


    return(TRUE);
}

void SipXtapiEventDispatcher::dtmfToSipXtapiMinorEvent(int dtmfButton, 
                                                       SIPX_CALLSTATE_MINOR& sipxTapiMinorEvent)
{
    switch(dtmfButton)
    {
    case 0:
        sipxTapiMinorEvent = DTMF_0;
        break;
    case 1:
        sipxTapiMinorEvent = DTMF_1;
        break;
    case 2:
        sipxTapiMinorEvent = DTMF_2;
        break;
    case 3:
        sipxTapiMinorEvent = DTMF_3;
        break;
    case 4:
        sipxTapiMinorEvent = DTMF_4;
        break;
    case 5:
        sipxTapiMinorEvent = DTMF_5;
        break;
    case 6:
        sipxTapiMinorEvent = DTMF_6;
        break;
    case 7:
        sipxTapiMinorEvent = DTMF_7;
        break;
    case 8:
        sipxTapiMinorEvent = DTMF_8;
        break;
    case 9:
        sipxTapiMinorEvent = DTMF_9;
        break;
    case 10:
        sipxTapiMinorEvent = DTMF_STAR;
        break;
    case 11:
        sipxTapiMinorEvent = DTMF_POUND;
        break;
    }
}


/* ========================== A C C E S S O R S =========================== */

/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */

//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */

/* ========================== F U N C T I O N S =========================== */

//__________________________________________________________________________//
