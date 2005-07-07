// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <listener.h>
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>
#include <cp/CallManager.h>
#include <net/Url.h>
#include <os/OsFS.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   3

#define DEBUGGING

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Listener::Listener(CallManager* callManager, UtlString playfile)
   : mPlayfile(playfile)
{
   mpCallManager = callManager;
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean Listener::handleMessage(OsMsg& rMsg)
{
    // React to telephony events
    if(rMsg.getMsgSubType()== TaoMessage::EVENT)
    {
        TaoMessage* taoMessage = (TaoMessage*)&rMsg;
		int taoEventId = taoMessage->getTaoObjHandle();
        UtlString argList(taoMessage->getArgList());
        TaoString arg(argList, TAOMESSAGE_DELIMITER);

#ifdef DEBUGGING
        dumpTaoMessageArgs(taoEventId, arg) ;
#endif        
        UtlBoolean localConnection = atoi(arg[TAO_OFFER_PARAM_LOCAL_CONNECTION]);
        UtlString  callId = arg[TAO_OFFER_PARAM_CALLID] ;
        UtlString  address = arg[TAO_OFFER_PARAM_ADDRESS] ;
        UtlString  prompt; 

        switch (taoEventId) 
        {
            case PtEvent::CONNECTION_OFFERED:
#ifdef DEBUGGING
                osPrintf("Answering callId: %s address: %s\n", 
                        callId.data(), address.data());
#endif
                mpCallManager->acceptConnection(callId, address);
                mpCallManager->answerTerminalConnection(callId, address, "*");
                break;
            case PtEvent::CONNECTION_ESTABLISHED:
                if (!localConnection) 
                {
                    prompt = chooseAudioGreeting(callId, address) ;
#ifdef DEBUGGING
                    osPrintf("Playing prompt: %s\n", prompt.data());
#endif
                    mpCallManager->audioPlay(callId, 
                        prompt,
                        TRUE, // repeat
                        FALSE, // local
                        TRUE); // remote
                }
                break;
            case PtEvent::CONNECTION_DISCONNECTED:
            case PtEvent::CONNECTION_FAILED:
#ifdef DEBUGGING
                osPrintf("Dropping call: %s\n", callId.data());
#endif
                mpCallManager->drop(callId);
                break;
        }
    }
    return(TRUE);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


void Listener::dumpTaoMessageArgs(unsigned char eventId, TaoString& args) 
{
    osPrintf("===>\nMessage type: %d args:\n\n", eventId) ;
        
    int argc = args.getCnt();
    for(int argIndex = 0; argIndex < argc; argIndex++)
    {
        osPrintf("\targ[%d]=\"%s\"\n", argIndex, args[argIndex]);
    }
}


UtlString Listener::chooseAudioGreeting(UtlString callId, UtlString address)
{
    UtlString userIdGreeting ;
    UtlString targetAddress;
    UtlString greeting(mPlayfile) ;

    // This should really use the userID from the URI ...
    mpCallManager->getFromField(callId, address, targetAddress);
    Url toUrl(targetAddress);   
    toUrl.getUserId(userIdGreeting);
    userIdGreeting.append(".wav") ;  
    OsFile fileCheck(userIdGreeting) ;
    if (fileCheck.exists()) 
    {
        greeting = userIdGreeting ;
    }

    return greeting ;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

