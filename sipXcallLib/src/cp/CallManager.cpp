//
// Copyright (C) 2005-2017 SIPez LLC. All rights reserved.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Daniel Petrie dpetrie AT SIPez DOT com

// SYSTEM INCLUDES

#include <time.h>
#include <assert.h>

// APPLICATION INCLUDES
#include <cp/CallManager.h>
#include <net/SipMessageEvent.h>
#include <net/SipUserAgent.h>
#include <sdp/SdpCodec.h>
#include <sdp/SdpCodecList.h>
#include <net/Url.h>
#include <net/SipSession.h>
#include <net/SipDialog.h>
#include <net/SipLineMgr.h>
#include <cp/CpIntMessage.h>
#include <cp/CpStringMessage.h>
#include <cp/CpMultiStringMessage.h>
#include <cp/Connection.h>
#include <mi/CpMediaInterfaceFactory.h>
#include <utl/UtlRegex.h>
#include <os/OsUtil.h>
#include <os/OsConfigDb.h>
#include <os/OsEventMsg.h>
#include <os/OsTimer.h>
#include <os/OsQueuedEvent.h>
#include <os/OsEvent.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <utl/UtlNameValueTokenizer.h>
#include <cp/CpPeerCall.h>
#include "tao/TaoMessage.h"
#include "tao/TaoProviderAdaptor.h"
#include "tao/TaoString.h"
#include "tao/TaoPhoneComponentAdaptor.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtEvent.h"

// TO_BE_REMOVED
#ifndef EXCLUDE_STREAMING
#include "mp/MpPlayer.h"
#include "mp/MpStreamMsg.h"
#include "mp/MpStreamPlayer.h"
#include "mp/MpStreamQueuePlayer.h"
#include "mp/MpStreamPlaylistPlayer.h"
#include <mp/MpMediaTask.h> 
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAXIMUM_CALLSTATE_LOG_SIZE 100000
#define CALL_STATUS_FIELD "status"
#define SEND_KEY '#'
char CONVERT_TO_STR[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
'*', '#', 'A', 'B', 'C', 'D', 'F'};
/*      _________________________
0--9                0--9
*                     10
#                     11
A--D              12--15
Flash                 16
*/

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CallManager::CallManager(UtlBoolean isRequredUserIdMatch,
                         SipLineMgr* pLineMgrTask,
                         UtlBoolean isEarlyMediaFor180Enabled,
                         SdpCodecList* pCodecFactory,
                         int rtpPortStart,
                         int rtpPortEnd,
                         const char* localAddress,
                         const char* publicAddress,
                         SipUserAgent* userAgent,
                         int sipSessionReinviteTimer,
                         PtMGCP* mgcpStackTask,
                         const char* defaultCallExtension,
                         int availableBehavior,
                         const char* unconditionalForwardUrl,
                         int forwardOnNoAnswerSeconds,
                         const char* forwardOnNoAnswerUrl,
                         int busyBehavior,
                         const char* sipForwardOnBusyUrl,
                         OsConfigDb* pConfigDb,
                         CallTypes phonesetOutgoingCallProtocol,
                         int numDialPlanDigits,
                         int holdType,
                         int offeringDelay,
                         const char* locale,
                         int inviteExpireSeconds,
                         int expeditedIpTos,
                         int maxCalls,
                         CpMediaInterfaceFactory* pMediaFactory,
                         int   internalSamplerate) 
: CpCallManager("CallManager-%d", "c", rtpPortStart, rtpPortEnd, localAddress,
                publicAddress, internalSamplerate)
, mIsEarlyMediaFor180(TRUE)
, mpMediaFactory(NULL)
{
    OsStackTraceLogger(FAC_CP, PRI_DEBUG, "CallManager");

    dialing = FALSE;
    mOffHook = FALSE;
    speakerOn = FALSE;
    flashPending = FALSE;
    mIsEarlyMediaFor180 = isEarlyMediaFor180Enabled;
    mNumDialPlanDigits = numDialPlanDigits;
    mHoldType = holdType;
    mnTotalIncomingCalls = 0;
    mnTotalOutgoingCalls = 0;
    mMaxCalls = maxCalls ;
    
    if (pMediaFactory)
    {
        mpMediaFactory = pMediaFactory;
    }
    else
    {
        assert(false);
    }

    // Instruct the factory to use the specified port range
    mpMediaFactory->getFactoryImplementation()->setRtpPortRange(rtpPortStart, rtpPortEnd) ;

    mLineAvailableBehavior = availableBehavior;
    mOfferedTimeOut = offeringDelay;
    mNoAnswerTimeout = forwardOnNoAnswerSeconds;
    if(forwardOnNoAnswerUrl)
    {
        mForwardOnNoAnswer = forwardOnNoAnswerUrl;
        if (mNoAnswerTimeout < 0)
            mNoAnswerTimeout = 24;  // default
    }
    if(unconditionalForwardUrl)
        mForwardUnconditional = unconditionalForwardUrl;
    mLineBusyBehavior = busyBehavior;
    if(sipForwardOnBusyUrl)
    {
        mSipForwardOnBusy.append(sipForwardOnBusyUrl);
    }
#ifdef TEST
    OsSysLog::add(FAC_CP, PRI_DEBUG, "SIP forward on busy URL: %s\nSIP unconditional forward URL: %s\nSIP no answer timeout:%d URL: %s\n",
        mSipForwardOnBusy.data(), mForwardUnconditional.data(),
        forwardOnNoAnswerSeconds, mForwardOnNoAnswer.data());
#endif

    mLocale = locale ? locale : "";

    if (inviteExpireSeconds > 0 && inviteExpireSeconds < CP_MAXIMUM_RINGING_EXPIRE_SECONDS)
        mInviteExpireSeconds = inviteExpireSeconds;
    else
        mInviteExpireSeconds = CP_MAXIMUM_RINGING_EXPIRE_SECONDS;

    mpLineMgrTask = pLineMgrTask;
    mIsRequredUserIdMatch = isRequredUserIdMatch;
    mExpeditedIpTos = expeditedIpTos;

    // Register with the SIP user agent
    sipUserAgent = userAgent;
    if(sipUserAgent)
    {
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_INVITE_METHOD,
            TRUE, // want to get requests
            TRUE, // and responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_BYE_METHOD,
            TRUE, // want to get requests
            TRUE, // and responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_CANCEL_METHOD,
            TRUE, // want to get requests
            TRUE, // and responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_ACK_METHOD,
            TRUE, // want to get requests
            FALSE, // no such thing as a ACK response
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_REFER_METHOD,
            TRUE, // want to get requests
            TRUE, // and responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_OPTIONS_METHOD,
            FALSE, // don't want to get requests
            TRUE, // do want responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_NOTIFY_METHOD,
            TRUE, // do want to get requests
            TRUE, // do want responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        sipUserAgent->addMessageObserver(*(this->getMessageQueue()),
            SIP_INFO_METHOD,
            TRUE, // do want to get requests
            TRUE, // do want responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages

        // Allow the "replaces" extension, because CallManager
        // implements the INVITE-with-Replaces logic.
        sipUserAgent->allowExtension(SIP_REPLACES_EXTENSION);

        int sipExpireSeconds = sipUserAgent->getDefaultExpiresSeconds();
        if (mInviteExpireSeconds > sipExpireSeconds) mInviteExpireSeconds = sipExpireSeconds;

    }
    mSipSessionReinviteTimer = sipSessionReinviteTimer;

    if(defaultCallExtension)
    {
        mOutboundLine = defaultCallExtension;
    }

    // MGCP stack
    mpMgcpStackTask = mgcpStackTask;

    infocusCall = NULL;
    mOutGoingCallType = phonesetOutgoingCallProtocol;
    mLocalAddress = localAddress;

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager: localAddress: %s mLocalAddress: %s publicAddress: %s mPublicAddress %s\n",
        localAddress, mLocalAddress.data(), publicAddress,
        mPublicAddress.data());
#endif

    mpCodecFactory = pCodecFactory;

    // Pre-allocate all of the history memory to minimze fragmentation
    for(int h = 0; h < CP_CALL_HISTORY_LENGTH ; h++)
        mCallManagerHistory[h].capacity(256);

    mMessageEventCount = -1 ;
    mStunPort = PORT_NONE ;
    mStunKeepAlivePeriodSecs = 0 ;

    mTurnPort = PORT_NONE ;
    mTurnKeepAlivePeriodSecs = 0 ;
}

// Copy constructor
CallManager::CallManager(const CallManager& rCallManager) :
CpCallManager("CallManager-%d", "call")
{
}

// Destructor
CallManager::~CallManager()
{
    OsStackTraceLogger stackLogger(FAC_CP, PRI_DEBUG, "~CallManager");
    while(getCallStackSize())
    {
        delete popCall();
    }

    waitUntilShutDown();   

    // do not delete the codecFactory it is not owned here

}

/* ============================ MANIPULATORS ============================== */

void CallManager::setOutboundLine(const char* lineUrl)
{
    mOutboundLine = lineUrl ? lineUrl : "";
}

UtlBoolean CallManager::handleMessage(OsMsg& eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();
    UtlBoolean messageProcessed = TRUE;
    UtlString holdCallId;
    UtlBoolean messageConsumed = FALSE;
    CpMediaInterface* pMediaInterface;

    switch(msgType)
    {
    case OsMsg::PHONE_APP:
        {
            char eventDescription[100];
            UtlString subTypeString;
            getEventSubTypeString((enum CpCallManager::EventSubTypes)msgSubType,
                subTypeString);
            sprintf(eventDescription, "%s (%d)", subTypeString.data(),
                msgSubType);
            addHistoryEvent(eventDescription);
        }

        switch(msgSubType)
        {
        case CP_MGCP_MESSAGE:
        case CP_MGCP_CAPS_MESSAGE:
        case CP_SIP_MESSAGE:
            {
                UtlBoolean newCallCreated = FALSE;
                OsWriteLock lock(mCallListMutex);
                CpCall* handlingCall = NULL;

                handlingCall = findHandlingCall(eventMessage);

                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "CallManager::handleMessage found call: %p", handlingCall);

                // This message does not belong to any of the calls
                // If this is an invite for a new call
                // Currently only one call can exist
                if(!handlingCall)
                {
                    UtlString callId;
                    if(msgSubType == CP_SIP_MESSAGE)
                    {
                        const SipMessage* sipMsg = ((SipMessageEvent&)eventMessage).getMessage();
                        if(sipMsg)
                        {
                           // Generate a new call id for new call.
                           getNewCallId(&callId);
                           OsSysLog::add(FAC_CP, PRI_DEBUG,
                                         "New CallID: %s\n", callId.data());
                        }

                        /////////////////
                        UtlBoolean isUserValid = FALSE;
                        UtlString method;
                        sipMsg->getRequestMethod(&method);

                        if(mpLineMgrTask && mIsRequredUserIdMatch &&
                            method.compareTo(SIP_INVITE_METHOD,UtlString::ignoreCase) == 0)
                        {
                            isUserValid = mpLineMgrTask->isUserIdDefined(sipMsg);
                            if( !isUserValid)
                            {
                                //no such user - return 404
                                SipMessage noSuchUserResponse;
                                noSuchUserResponse.setResponseData(sipMsg,
                                    SIP_NOT_FOUND_CODE,
                                    SIP_NOT_FOUND_TEXT);
                                sipUserAgent->send(noSuchUserResponse);
                            }
                        }
                        else
                        {
                            isUserValid = TRUE;
                        }
                        ////////////////

                        if( isUserValid && CpPeerCall::shouldCreateCall(
                            *sipUserAgent, eventMessage, *mpCodecFactory))
                        {
#ifdef _WIN32
                            if(IsTroubleShootingModeEnabled())
                            {
                              OsSysLog::add(FAC_CP, PRI_DEBUG, "BEGIN - DEBUGGING : Call Stack Size");

                                // Marc Chenier 31/10/2005
                                // BEGIN : log the entire call stack
                                printCalls();

                              OsSysLog::add(FAC_CP, PRI_DEBUG, "END - DEBUGGING : Call Stack Size");

                            }                           
#endif
                            // If this call would exceed the limit that we have been
                            // given for calls to handle simultaneously,
                            // send a BUSY_HERE SIP (486) message back to the sender.
                            if(getCallStackSize() >= mMaxCalls)
                            {
                                OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::handleMessage - The call stack size as reached it's limit of %d", mMaxCalls);
                                if( (sipMsg->isResponse() == FALSE) &&
                                    (method.compareTo(SIP_ACK_METHOD,UtlString::ignoreCase) != 0) )

                                {
                                    SipMessage busyHereResponse;
                                    busyHereResponse.setInviteBusyData(sipMsg);
                                    sipUserAgent->send(busyHereResponse);
                                }
                            }
                            else
                            {
                                // Create a new SIP call
                                int numCodecs;
                                SdpCodec** codecArray = NULL;
                                getCodecs(numCodecs, codecArray);
                                OsSysLog::add(FAC_CP, PRI_DEBUG, "Creating new call for incoming SIP message\n");

                                UtlString publicAddress;
                                int publicPort;
                                //always use sipUserAgent public address, not the mPublicAddress of this call manager.
                                sipUserAgent->getViaInfo(OsSocket::UDP, publicAddress, publicPort, NULL, NULL);

                                UtlString localAddress;
                                int port;
                                UtlString adapterName;

                                localAddress = sipMsg->getLocalIp();

                                getContactAdapterName(adapterName, localAddress, false);

                                SIPX_CONTACT_ADDRESS contact;
                                sipUserAgent->getContactDb().getRecordForAdapter(contact, adapterName.data(), CONTACT_LOCAL);
                                port = contact.iPort;

                                pMediaInterface = mpMediaFactory->createMediaInterface(
                                    NULL, 
                                    localAddress, numCodecs, codecArray, 
                                    mLocale.data(), mExpeditedIpTos, mStunServer, 
                                    mStunPort, mStunKeepAlivePeriodSecs, mTurnServer,
                                    mTurnPort, mTurnUsername, mTurnPassword,
                                    mTurnKeepAlivePeriodSecs, isIceEnabled(),
                                    mDefaultSampleRate);


                                int inviteExpireSeconds;
                                if (sipMsg->getExpiresField(&inviteExpireSeconds) && inviteExpireSeconds > 0)
                                {
                                    if (inviteExpireSeconds > mInviteExpireSeconds)
                                        inviteExpireSeconds = mInviteExpireSeconds;
                                }
                                else
                                    inviteExpireSeconds = mInviteExpireSeconds;

                                handlingCall = new CpPeerCall(mIsEarlyMediaFor180,
                                    this,
                                    pMediaInterface,
                                    aquireCallIndex(),
                                    callId.data(),
                                    sipUserAgent,
                                    mSipSessionReinviteTimer,
                                    mOutboundLine.data(),
                                    mHoldType,
                                    mOfferedTimeOut,
                                    mLineAvailableBehavior,
                                    mForwardUnconditional.data(),
                                    mLineBusyBehavior,
                                    mSipForwardOnBusy.data(),
                                    mNoAnswerTimeout,
                                    mForwardOnNoAnswer.data(),
                                    inviteExpireSeconds);

                                for (int i = 0; i < numCodecs; i++)
                                {
                                    delete codecArray[i];
                                }
                                delete[] codecArray;
                            }
                        }
                    }

                    // If we created a new call
                    if(handlingCall)
                    {
                        handlingCall->start();
                        pushCall(handlingCall);
                        newCallCreated = TRUE;
                    }
                }

                // Pass on the message if there is a call to process
                if(handlingCall)
                {
                    handlingCall->postMessage(eventMessage);
                    messageProcessed = TRUE;

                    // Yield to be sure that the INVITE
                    // message is handled by the Call task.  This is to
                    // mitigate the risk that another message comes in for
                    // the same call, but is not matched because the Call
                    // task has not initialized call state as it has not 
                    // had time to process the INVITE yet.
                    if(newCallCreated)
                    {
                        OsMsgQ* callMessageQueue = handlingCall->getMessageQueue();
                        // Wait until call process has started and it removes the INVITE from 
                        // its message queue
                        while(callMessageQueue &&
                              callMessageQueue->numMsgs() > 0)
                        {
                            yield();
                        }

                        while (msgSubType == CP_SIP_MESSAGE &&
                               !handlingCall->willHandleMessage(eventMessage))
                        {
                            // Wait until the call process had processed the INVITE
                            // message and has setup enough of the call state to match
                            // any future SIP messages with the same dialog
                            yield();
                        }
                    }
                }
            }
            break;

        case CP_CALL_EXITED:
            {
                CpCall* call;
                ((CpIntMessage&)eventMessage).getIntData((intptr_t&) call);

                OsSysLog::add(FAC_CP, PRI_DEBUG, "Call EXITING message received: %p infofocus: %p\r\n", 
                        (void*)call, (void*) infocusCall);

                call->stopMetaEvent();

                mCallListMutex.acquireWrite() ;                                                
                releaseCallIndex(call->getCallIndex());
                if(infocusCall == call)
                {
                    // The infocus call is not in the mCallList -- no need to 
                    // remove, but we should tell the call that it is not 
                    // longer in focus.
                    call->outOfFocus();                    
                }
                else
                {
                    call = removeCall(call);
                }
                mCallListMutex.releaseWrite() ;

                if(call)
                {
                    delete call;                        
                }

                messageProcessed = TRUE;
                break;
            }

        case CP_DIAL_STRING:
            {
                OsWriteLock lock(mCallListMutex);
                if(infocusCall && dialing)
                {
                    //OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::processMessage posting dial string to infocus call\n");
                    ((CpMultiStringMessage&)eventMessage).getString1Data(mDialString) ;
                    infocusCall->postMessage(eventMessage);
                }
                dialing = FALSE;
                messageProcessed = TRUE;
                break;
            }

        case CP_YIELD_FOCUS:
            {
                CpCall* call;
                ((CpIntMessage&)eventMessage).getIntData((intptr_t&) call);

                OsSysLog::add(FAC_CP, PRI_DEBUG, "Call YIELD FOCUS message received: %p\r\n", (void*)call);
                OsSysLog::add(FAC_CP, PRI_DEBUG, "infocusCall: %p\r\n", infocusCall);
                yieldFocus(call);
                messageConsumed = TRUE;
                messageProcessed = TRUE;
                break;
            }
        case CP_GET_FOCUS:
            {
                CpCall* call;
                ((CpIntMessage&)eventMessage).getIntData((intptr_t&) call);
                OsSysLog::add(FAC_CP, PRI_DEBUG, "Call GET FOCUS message received: %p\r\n", (void*)call);
                OsSysLog::add(FAC_CP, PRI_DEBUG, "infocusCall: %p\r\n", infocusCall);
                doGetFocus(call);
                messageConsumed = TRUE;
                messageProcessed = TRUE;
                break;
            }
        case CP_CREATE_CALL:
            {
                UtlString callId;
                int metaEventId = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                int metaEventType = ((CpMultiStringMessage&)eventMessage).getInt2Data();
                int numCalls = ((CpMultiStringMessage&)eventMessage).getInt3Data();
                UtlBoolean assumeFocusIfNoInfocusCall = ((CpMultiStringMessage&)eventMessage).getInt4Data();
                const char* metaEventCallIds[4];
                UtlString metaCallId0;
                UtlString metaCallId1;
                UtlString metaCallId2;
                UtlString metaCallId3;

                ((CpMultiStringMessage&)eventMessage).getString1Data(callId);
                ((CpMultiStringMessage&)eventMessage).getString2Data(metaCallId0);
                ((CpMultiStringMessage&)eventMessage).getString3Data(metaCallId1);
                ((CpMultiStringMessage&)eventMessage).getString4Data(metaCallId2);
                ((CpMultiStringMessage&)eventMessage).getString5Data(metaCallId3);

                OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager:: create call %s\n", callId.data());

                metaEventCallIds[0] = metaCallId0.data();
                metaEventCallIds[1] = metaCallId1.data();
                metaEventCallIds[2] = metaCallId2.data();
                metaEventCallIds[3] = metaCallId3.data();
                doCreateCall(callId.data(), metaEventId, metaEventType,
                    numCalls, metaEventCallIds, assumeFocusIfNoInfocusCall);

                messageProcessed = TRUE;
                break;
            }

        case CP_GET_CALLS:
            {
                int numCalls = 0;
                UtlString callId;
                UtlSList* callList;
                UtlVoidPtr* callCollectable;
                OsProtectedEvent* getCallsEvent = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                getCallsEvent->getIntData((intptr_t&)callList);

                if(getCallsEvent)
                {
                    OsReadLock lock(mCallListMutex);
                    // Get the callId for the infocus call
                    if(infocusCall)
                    {
                        infocusCall->getCallId(callId);
                        callList->append(new UtlString(callId));
                        numCalls++;
                    }

                    // Get the callId for the calls in the stack
                    CpCall* call = NULL;

                    UtlSListIterator iterator(callStack);
                    callCollectable = (UtlVoidPtr*) iterator();
                    while(callCollectable)
                    {
                        call = (CpCall*) callCollectable->getValue();
                        if(call)
                        {
                            call->getCallId(callId);
                            callList->append(new UtlString(callId));
                            numCalls++;
                        }
                        callCollectable = (UtlVoidPtr*) iterator();
                    }

                    // Signal the caller that we are done.
                    // If the event has already been signalled, clean up
                    if(OS_ALREADY_SIGNALED == getCallsEvent->signal(numCalls))
                    {
                        // The other end must have timed out on the wait
                        callList->destroyAll();
                        delete callList;
                        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                        eventMgr->release(getCallsEvent);
                    }
                }
                messageConsumed = TRUE;
                messageProcessed = TRUE;
                break;
            }

        case CP_CONNECT:
            {
                UtlString callId;
                UtlString addressUrl;
                UtlString desiredConnectionCallId ;
                UtlString locationHeader;
                SIPX_CONTACT_ID contactId;
                ((CpMultiStringMessage&)eventMessage).getString1Data(callId);
                ((CpMultiStringMessage&)eventMessage).getString2Data(addressUrl);
                ((CpMultiStringMessage&)eventMessage).getString4Data(desiredConnectionCallId);
                ((CpMultiStringMessage&)eventMessage).getString5Data(locationHeader);
                contactId = (SIPX_CONTACT_ID) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                void* pDisplay = (void*) ((CpMultiStringMessage&)eventMessage).getInt2Data();
                void* pSecurity = (void*) ((CpMultiStringMessage&)eventMessage).getInt3Data();
                int bandWidth = ((CpMultiStringMessage&)eventMessage).getInt4Data();
                SIPX_TRANSPORT_DATA* pTransport = (SIPX_TRANSPORT_DATA*)((CpMultiStringMessage&)eventMessage).getInt5Data();
                RtpTransportOptions rtpTransportFlags = (RtpTransportOptions)((CpMultiStringMessage&)eventMessage).getInt6Data();

                const char* locationHeaderData = (locationHeader.length() == 0) ? NULL : locationHeader.data();

                doConnect(callId.data(), addressUrl.data(), desiredConnectionCallId.data(), contactId, pDisplay, pSecurity, 
                          locationHeaderData, bandWidth, pTransport, rtpTransportFlags) ;
                messageProcessed = TRUE;
                break;
            }
        case CP_SET_INBOUND_CODEC_CPU_LIMIT:
            {
                int iLevel = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                mpCodecFactory->setCodecCPULimit(iLevel) ;
                messageConsumed = TRUE;
                messageProcessed = TRUE;
                break ;
            }
        case CP_ENABLE_STUN:
            {
                UtlString stunServer ;
                int iRefreshPeriod ;
                int iStunPort ;
                OsNotification* pNotification ;


                CpMultiStringMessage& enableStunMessage = (CpMultiStringMessage&)eventMessage;
                enableStunMessage.getString1Data(stunServer) ;
                iStunPort = enableStunMessage.getInt1Data() ;
                iRefreshPeriod = enableStunMessage.getInt2Data() ;                
                pNotification = (OsNotification*) enableStunMessage.getInt3Data() ;

                doEnableStun(stunServer, iStunPort, iRefreshPeriod, pNotification) ;
                break ;
            }
        case CP_ENABLE_TURN:
            {
                UtlString turnServer ;
                UtlString turnUsername ;
                UtlString turnPassword ;
                int iTurnPort ;
                int iRefreshPeriod ;

                CpMultiStringMessage& enableStunMessage = (CpMultiStringMessage&)eventMessage;
                enableStunMessage.getString1Data(turnServer) ;
                enableStunMessage.getString2Data(turnUsername) ;
                enableStunMessage.getString3Data(turnPassword) ;
                iTurnPort = enableStunMessage.getInt1Data() ;
                iRefreshPeriod = enableStunMessage.getInt2Data() ;                

                doEnableTurn(turnServer, iTurnPort, turnUsername, turnPassword, iRefreshPeriod) ;
                break ;
            }
        case CP_ANSWER_CONNECTION:
        case CP_DROP:
        case CP_BLIND_TRANSFER:
        case CP_CONSULT_TRANSFER:
        case CP_CONSULT_TRANSFER_ADDRESS:
        case CP_TRANSFER_CONNECTION:
        case CP_TRANSFER_CONNECTION_STATUS:
        case CP_TRANSFEREE_CONNECTION:
        case CP_TRANSFEREE_CONNECTION_STATUS:
        case CP_HOLD_TERM_CONNECTION:
        case CP_HOLD_ALL_TERM_CONNECTIONS:
        case CP_UNHOLD_ALL_TERM_CONNECTIONS:
        case CP_UNHOLD_TERM_CONNECTION:
        case CP_RENEGOTIATE_CODECS_CONNECTION:
        case CP_SILENT_REMOTE_HOLD:
        case CP_RENEGOTIATE_CODECS_ALL_CONNECTIONS:
        case CP_SET_CODEC_CPU_LIMIT:
        case CP_GET_CODEC_CPU_LIMIT:
        case CP_GET_CODEC_CPU_COST:
        case CP_SET_MIC_GAIN:
        case CP_SET_OUTPUT_MIX_WEIGHT:
        case CP_UNHOLD_LOCAL_TERM_CONNECTION:
        case CP_HOLD_LOCAL_TERM_CONNECTION:
        case CP_START_TONE_TERM_CONNECTION:
        case CP_STOP_TONE_TERM_CONNECTION:
        case CP_START_TONE_CONNECTION:
        case CP_STOP_TONE_CONNECTION:
        case CP_PLAY_AUDIO_TERM_CONNECTION:        
        case CP_STOP_AUDIO_TERM_CONNECTION:
        case CP_PLAY_AUDIO_CONNECTION:        
        case CP_STOP_AUDIO_CONNECTION:
        case CP_RECORD_AUDIO_CONNECTION_START:
        case CP_RECORD_AUDIO_CONNECTION_PAUSE:
        case CP_RECORD_AUDIO_CONNECTION_RESUME:
        case CP_RECORD_AUDIO_CONNECTION_STOP:
        case CP_RECORD_BUFFER_AUDIO_CONNECTION_START:
        case CP_SET_MEDIA_PASS_THROUGH:
        case CP_CREATE_MEDIA_CONNECTION:
        case CP_SET_RTP_DESTINATION:
        case CP_START_RTP_SEND:
        case CP_STOP_RTP_SEND:
        case CP_RECORD_BUFFER_AUDIO_CONNECTION_STOP:
        case CP_PLAY_BUFFER_TERM_CONNECTION:
        case CP_CREATE_PLAYER:
        case CP_DESTROY_PLAYER:
        case CP_CREATE_PLAYLIST_PLAYER:
        case CP_DESTROY_PLAYLIST_PLAYER:
        case CP_DESTROY_QUEUE_PLAYER:
        case CP_CREATE_QUEUE_PLAYER:
        case CP_GET_NUM_CONNECTIONS:
        case CP_GET_CONNECTIONS:
        case CP_GET_CALLED_ADDRESSES:
        case CP_GET_CALLING_ADDRESSES:
        case CP_GET_NUM_TERM_CONNECTIONS:
        case CP_GET_TERM_CONNECTIONS:
        case CP_IS_LOCAL_TERM_CONNECTION:
        case CP_ACCEPT_CONNECTION:
        case CP_REJECT_CONNECTION:
        case CP_REDIRECT_CONNECTION:
        case CP_DROP_CONNECTION:
        case CP_FORCE_DROP_CONNECTION:
        case CP_OFFERING_EXPIRED:
        case CP_RINGING_EXPIRED:
        case CP_GET_CALLSTATE:
        case CP_GET_CONNECTIONSTATE:
        case CP_GET_TERMINALCONNECTIONSTATE:
        case CP_GET_SESSION:
        case CP_CANCEL_TIMER:
        case CP_GET_NEXT_CSEQ:
        case CP_SET_OUTBOUND_LINE:
        case CP_GET_LOCAL_CONTACTS:
        case CP_GET_MEDIA_CONNECTION_ID:
        case CP_GET_MEDIA_ENERGY_LEVELS:
        case CP_GET_CALL_MEDIA_ENERGY_LEVELS:
        case CP_GET_MEDIA_RTP_SOURCE_IDS:
        case CP_GET_CAN_ADD_PARTY:
        case CP_SPLIT_CONNECTION:
        case CP_JOIN_CONNECTION:
        case CP_TRANSFER_OTHER_PARTY_HOLD:
        case CP_TRANSFER_OTHER_PARTY_JOIN:
        case CP_TRANSFER_OTHER_PARTY_UNHOLD:
        case CP_LIMIT_CODEC_PREFERENCES:
        case CP_OUTGOING_INFO:
        case CP_GET_USERAGENT:
        case CP_FLOWGRAPH_MESSAGE:
        case CP_LIMIT_CODECS:
        case CP_GET_INVITE_HEADER_VALUE:
            // Forward the message to the call
            {
                UtlString callId;
                ((CpMultiStringMessage&)eventMessage).getString1Data(callId);
                OsReadLock lock(mCallListMutex);
                CpCall* call = findHandlingCall(callId);
                if(!call)
                {
                    // The call might have been terminated by asynchronous events.
                    // But output a debugging message, so the programmer can check
                    // to see that the CallId was valid in the past.
                    OsSysLog::add(FAC_CP, PRI_DEBUG, "Cannot find CallId: %s to post message: %d\n",
                        callId.data(), msgSubType);
                    if(   msgSubType == CP_GET_NUM_CONNECTIONS ||
                        msgSubType == CP_GET_CONNECTIONS ||
                        msgSubType == CP_GET_CALLED_ADDRESSES ||
                        msgSubType == CP_GET_CALLING_ADDRESSES ||
                        msgSubType == CP_GET_NUM_TERM_CONNECTIONS ||
                        msgSubType == CP_GET_TERM_CONNECTIONS ||
                        msgSubType == CP_IS_LOCAL_TERM_CONNECTION ||
                        msgSubType == CP_GET_CALLSTATE ||
                        msgSubType == CP_GET_CONNECTIONSTATE ||
                        msgSubType == CP_GET_TERMINALCONNECTIONSTATE ||
                        msgSubType == CP_GET_NEXT_CSEQ ||
                        msgSubType == CP_GET_SESSION ||
                        msgSubType == CP_GET_CODEC_CPU_COST ||
                        msgSubType == CP_GET_CODEC_CPU_LIMIT ||
                        msgSubType == CP_CREATE_PLAYER ||
                        msgSubType == CP_CREATE_PLAYLIST_PLAYER ||
                        msgSubType == CP_CREATE_QUEUE_PLAYER ||
                        msgSubType == CP_DESTROY_PLAYER ||
                        msgSubType == CP_DESTROY_PLAYLIST_PLAYER ||
                        msgSubType == CP_DESTROY_QUEUE_PLAYER ||
                        msgSubType == CP_PLAY_BUFFER_TERM_CONNECTION ||
                        msgSubType == CP_GET_LOCAL_CONTACTS || 
                        msgSubType == CP_GET_MEDIA_CONNECTION_ID ||
                        msgSubType == CP_GET_MEDIA_ENERGY_LEVELS ||
                        msgSubType == CP_GET_CALL_MEDIA_ENERGY_LEVELS ||
                        msgSubType == CP_GET_MEDIA_RTP_SOURCE_IDS ||
                        msgSubType == CP_GET_CAN_ADD_PARTY ||
                        msgSubType == CP_RECORD_AUDIO_CONNECTION_START ||
                        msgSubType == CP_RECORD_AUDIO_CONNECTION_STOP ||
                        msgSubType == CP_RECORD_BUFFER_AUDIO_CONNECTION_START ||
                        msgSubType == CP_RECORD_BUFFER_AUDIO_CONNECTION_STOP ||
                        msgSubType == CP_OUTGOING_INFO ||
                        msgSubType == CP_GET_USERAGENT ||
                        msgSubType == CP_CREATE_MEDIA_CONNECTION ||
                        msgSubType == CP_GET_INVITE_HEADER_VALUE)
                    {
                        // Get the OsProtectedEvent and signal it to go away
                        OsProtectedEvent* eventWithoutCall = (OsProtectedEvent*)
                            ((CpMultiStringMessage&)eventMessage).getInt1Data();
                        if (eventWithoutCall)
                        {
                            OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::handleMessage Received a message subtype %d request on invalid callId '%s'; signaled event in message\n",
                                msgSubType, callId.data());

                            // Test if already signaled here and
                            // releasing the event if it is.
                            if(OS_ALREADY_SIGNALED == eventWithoutCall->signal(0))
                            {
                                OsProtectEventMgr* eventMgr =
                                    OsProtectEventMgr::getEventMgr();
                                eventMgr->release(eventWithoutCall);
                                eventWithoutCall = NULL;
                            }
                        }
                        else
                        {
                            OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::handleMessage Received a message subtype %d request on invalid callId '%s'; no event to signal\n",
                                msgSubType, callId.data());
                        }
                    }
                }
                else
                {
                    call->postMessage(eventMessage);
                }
                messageProcessed = TRUE;
                break;
            }

        default:
            {
                OsSysLog::add(FAC_CP, PRI_ERR, "Unknown PHONE_APP CallManager message subtype: %d\n", msgSubType);
                messageProcessed = TRUE;
                break;
            }
        }
        break;

        // Timer event expired
    case OsMsg::OS_EVENT:
        // Pull out the OsMsg from the user data and post it
        if(msgSubType == OsEventMsg::NOTIFY)
        {
            OsMsg* timerMsg;
            OsTimer* timer;

            ((OsEventMsg&)eventMessage).getUserData((intptr_t&)timerMsg);
            ((OsEventMsg&)eventMessage).getEventData((intptr_t&)timer);

            if(timer)
            {
#ifdef TEST_PRINT
                int eventMessageType = timerMsg->getMsgType();
                int eventMessageSubType = timerMsg->getMsgSubType();
                OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::handleMessage deleting timer for message type: %d %d\n",
                    eventMessageType, eventMessageSubType);
#endif
                timer->stop();
                delete timer;
                timer = NULL;
            }
            if(timerMsg)
            {
                postMessage(*timerMsg);
                delete timerMsg;
                timerMsg = NULL;
            }
        }
        messageProcessed = TRUE;
        break;
#ifndef EXCLUDE_STREAMING
    case OsMsg::STREAMING_MSG:
        {
            // TO_BE_REMOVED
            MpStreamMsg* pMsg = (MpStreamMsg*) &eventMessage ;
            UtlString callId = pMsg->getTarget() ;

            OsReadLock lock(mCallListMutex);
            CpCall* call = findHandlingCall(callId);
            if(!call)
            {
                // If we cannot find the call, then we must clean up
                // and unblock the caller.
                if (  (msgSubType == MpStreamMsg::STREAM_REALIZE_URL) ||
                    (msgSubType == MpStreamMsg::STREAM_REALIZE_BUFFER))
                {
                    OsNotification* pNotifyHandle = (OsNotification*) pMsg->getPtr1() ;
                    Url* pUrl = (Url*) pMsg->getInt2() ;
                    delete pUrl ;
                    pNotifyHandle->signal(0) ;
                }

            }
            else
            {
                call->postMessage(eventMessage);
            }
            
            //assert(false);
            break;
        }
#endif 

    default:
        {
            OsSysLog::add(FAC_CP, PRI_ERR, "Unknown TYPE %d of CallManager message subtype: %d\n", msgType, msgSubType);
            messageProcessed = TRUE;
            break;
        }
    }

    return(messageProcessed);
}


void CallManager::requestShutdown()
{
    // Need to put a Mutex on the call stack


    UtlSListIterator iterator(callStack);
    CpCall* call = NULL;
    UtlVoidPtr* callCollectable;

    while(! callStack.isEmpty() && ! iterator.atLast())
    {
        callCollectable = (UtlVoidPtr*) iterator();
        if(callCollectable)
        {
            call = (CpCall*) callCollectable->getValue();
            call->requestShutdown();
        }
    }

    {
        OsReadLock lock(mCallListMutex);
        if(infocusCall)
        {
            infocusCall->requestShutdown();
        }
    }

    // Pass the shut down to itself
    OsServerTask::requestShutdown();
    yield();

}


void CallManager::unhold(const char* callId)
{
    CpStringMessage unholdMessage(CP_OFF_HOLD_CALL, callId);
    postMessage(unholdMessage);
}

void CallManager::createCall(UtlString* callId,
                             int metaEventId,
                             int metaEventType,
                             int numCalls,
                             const char* callIds[],
                             UtlBoolean assumeFocusIfNoInfocusCall)
{
    if(callId->isNull())
    {
        getNewCallId(callId);
    }
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::createCall new Id: %s\n", callId->data());
    CpMultiStringMessage callMessage(CP_CREATE_CALL,
        callId->data(),
        numCalls >= 1 ? callIds[0] : NULL,
        numCalls >= 2 ? callIds[1] : NULL,
        numCalls >= 3 ? callIds[2] : NULL,
        numCalls >= 4 ? callIds[3] : NULL,
        metaEventId,
        metaEventType,
        numCalls,
        assumeFocusIfNoInfocusCall);
    postMessage(callMessage);
    mnTotalOutgoingCalls++;

}


OsStatus CallManager::getCalls(int maxCalls, int& numCalls,
                               UtlString callIds[])
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlSList* addressList = new UtlSList;
    OsProtectedEvent* callsSet = eventMgr->alloc();
    callsSet->setIntData((intptr_t) addressList);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getCallsMessage(CP_GET_CALLS, NULL, NULL, NULL,
        NULL, NULL, (intptr_t)callsSet);
    postMessage(getCallsMessage);

    // Wait until the call manager sets the callIDs
    if(callsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        int callIndex = 0;
        { // scope the iterator
            UtlSListIterator iterator(*addressList);
            UtlString* callIdCollectable;
            callIdCollectable = (UtlString*)iterator();
            returnCode = OS_SUCCESS;

            while (callIdCollectable)
            {
                if(callIndex >= maxCalls)
                {
                    returnCode = OS_LIMIT_REACHED;
                    break;
                }
                callIds[callIndex] = *callIdCollectable;
                callIndex++;
                callIdCollectable = (UtlString*)iterator();
            }
            numCalls = callIndex;
        } // end of iterator scope
#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getCalls %d calls\n",
            numCalls);
#endif
        addressList->destroyAll();
        delete addressList;
        eventMgr->release(callsSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getCalls TIMED OUT\n");

        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == callsSet->signal(0))
        {
            addressList->destroyAll();
            delete addressList;
            eventMgr->release(callsSet);
        }
        numCalls = 0;

    }

#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager:getCalls numCalls = %d\n ", numCalls) ;
#endif
    return(returnCode);
}

void CallManager::getRemoteUserAgent(const char* callId, const char* remoteAddress,
                                     UtlString& userAgent)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* agentSet = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    UtlString* pUserAgent = new UtlString;
    agentSet->setIntData((intptr_t) pUserAgent);
    CpMultiStringMessage getAgentMessage(CP_GET_USERAGENT, callId, remoteAddress, NULL,
        NULL, NULL, (intptr_t)agentSet);
    postMessage(getAgentMessage);

    // Wait until the call manager sets the callIDs
    if(agentSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        userAgent = *pUserAgent;
        delete pUserAgent;
        eventMgr->release(agentSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getRemoteUserAgent TIMED OUT\n");

        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == agentSet->signal(0))
        {
            delete pUserAgent;
            eventMgr->release(agentSet);
        }
    }
}

OsStatus CallManager::getInviteHeaderValue(const char* callId, 
                                           const char* remoteAddress,
                                           const char* headerName, 
                                           UtlString& headerValueString,
                                           UtlBoolean& inviteIsFromRemote,
                                           const int headerInstanceIndex)
{
    OsStatus status = OS_FAILED;
    inviteIsFromRemote = TRUE;

    // Construct a message containing event to notify when header value is set
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* setValueEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    UtlString* pHeaderValue = new UtlString;
    // Pass a UtlString in event to retrieve header value
    setValueEvent->setIntData((intptr_t) pHeaderValue);
    CpMultiStringMessage getValueMessage(CP_GET_INVITE_HEADER_VALUE, callId, remoteAddress, headerName,
        NULL, NULL, (intptr_t)setValueEvent, headerInstanceIndex);

    // Send the message to the peer call
    postMessage(getValueMessage);

    // Wait until the peer call handles the message
    if(setValueEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        headerValueString = *pHeaderValue;
        // Get the return code to determine if INVITE exists and header exists
        setValueEvent->getEventData((intptr_t&)status);

        // get if INVITE was from remote or local side
        setValueEvent->getIntData((intptr_t&)inviteIsFromRemote);

        delete pHeaderValue;
        eventMgr->release(setValueEvent);
    }

    // Timeout occurred and message was not handled
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getInviteHeaderValue TIMED OUT\n");
        status = OS_WAIT_TIMEOUT;

        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == setValueEvent->signal(0))
        {
            delete pHeaderValue;
            eventMgr->release(setValueEvent);
        }
    }

    return(status);
}

PtStatus CallManager::connect(const char* callId,
                              const char* toAddressString,
                              const char* fromAddressString,
                              const char* desiredCallIdString,
                              SIPX_CONTACT_ID contactId,
                              const void* pDisplay,
                              const void* pSecurity,
                              const char* locationHeader,
                              const int bandWidth,
                              SIPX_TRANSPORT_DATA* pTransportData,
                              const RTP_TRANSPORT rtpTransportOptions)
{
    UtlString toAddressUrl(toAddressString ? toAddressString : "");
    UtlString fromAddressUrl(fromAddressString ? fromAddressString : "");
    UtlString desiredCallId(desiredCallIdString ? desiredCallIdString : "") ;
    
    // create a copy of the transport data
    SIPX_TRANSPORT_DATA* pTransportDataCopy = NULL;
    if (pTransportData)
    {
        pTransportDataCopy = new SIPX_TRANSPORT_DATA;
        memcpy(pTransportDataCopy, pTransportData, sizeof(SIPX_TRANSPORT_DATA));
    }

    PtStatus returnCode = validateAddress(toAddressUrl);
    if(returnCode == PT_SUCCESS)
    {
        CpMultiStringMessage callMessage(CP_CONNECT, callId,
            toAddressUrl, fromAddressUrl, desiredCallId, locationHeader,
            contactId, (intptr_t)pDisplay, (intptr_t)pSecurity, 
            (intptr_t)bandWidth, (intptr_t)pTransportDataCopy, rtpTransportOptions);
        postMessage(callMessage);
    }
    return(returnCode);
}

PtStatus CallManager::consult(const char* idleTargetCallId,
                              const char* activeOriginalCallId, const char* originalCallControllerAddress,
                              const char* originalCallControllerTerminalId, const char* consultAddressString,
                              UtlString& targetCallControllerAddress, UtlString& targetCallConsultAddress)
{
    UtlString consultAddressUrl(consultAddressString ? consultAddressString : "");
    PtStatus returnCode = validateAddress(consultAddressUrl);
    if(returnCode == PT_SUCCESS)
    {
        // There is an implied hold for all connections in
        // the original call
        holdAllTerminalConnections(activeOriginalCallId);

        // Not sure if we should explicitly put the consultative
        // call in focus or not.
        // For now we won't
        //unholdTerminalConnection(idleTargetCallId, extension.data(), NULL);

        // Create the consultative connection on the new call
        connect(idleTargetCallId, consultAddressString);

        //targetCallControllerAddress = extension; // temporary kludge
        targetCallControllerAddress = mOutboundLine;
        targetCallConsultAddress = consultAddressUrl;
    }
    return(returnCode);
}

void CallManager::drop(const char* callId)
{
    CpMultiStringMessage callMessage(CP_DROP, callId);
    postMessage(callMessage);
}

UtlBoolean CallManager::sendInfo(const char*  callId, 
                                 const char*  szRemoteAddress,
                                 const char*  szContentType,
                                 const size_t nContentLength,
                                 const char*  szContent)
{
    UtlBoolean bRC = false ;
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* pSuccessEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);


    CpMultiStringMessage infoMessage(CP_OUTGOING_INFO, 
            callId, 
            szRemoteAddress,
            szContentType, 
            UtlString(szContent, nContentLength),
            NULL, 
            (intptr_t)pSuccessEvent) ;
    postMessage(infoMessage);

    if (pSuccessEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success ;
        pSuccessEvent->getEventData(success);
        bRC = success ;
        eventMgr->release(pSuccessEvent);
    }
    else
    {
        if(OS_ALREADY_SIGNALED == pSuccessEvent->signal(0))
        {
            eventMgr->release(pSuccessEvent);
        }
    }

    return bRC ;
}

// Cosultative transfer
PtStatus CallManager::transfer(const char* targetCallId, const char* originalCallId)
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManger::transfer targetCallId=%s, originalCallId=%s\n", targetCallId, originalCallId) ;
#endif

    PtStatus returnCode =  PT_SUCCESS;

    // Make sure the consultative call is on hold
    CpMultiStringMessage allHold(CP_HOLD_ALL_TERM_CONNECTIONS, targetCallId);
    postMessage(allHold);

    // Need to get the To URL from the consultative call
    UtlString addresses[2];
    int numConnections;
    getConnections(targetCallId, 2, numConnections, addresses);

    // It is only valid to have 2 connections (local and transfer target)
    // <2 suggest the target hung up.  >2 suggest we are trying to transfer
    // a conference.
    if (numConnections == 2)
    {
        // Assume the first is always the local connection
        UtlString fromAddress;
        UtlString toAddress;
        //        getToField(targetCallId, addresses[1].data(), toAddress) ;
        toAddress = addresses[1];
        getFromField(targetCallId, addresses[1].data(), fromAddress) ;

        // Construct the replaces header info
        // SIP alert: this is SIP specific and should not be in CallManager

        // Add the Replaces header info to the consultative URL
        UtlString replacesField;
        SipMessage::buildReplacesField(replacesField, targetCallId,
            fromAddress.data(), toAddress.data());

        Url transferTargetUrl(toAddress);
        transferTargetUrl.removeFieldParameters() ;
        transferTargetUrl.setHeaderParameter(SIP_REPLACES_FIELD,
            replacesField.data());
        UtlString transferTargetUrlString;
        transferTargetUrl.toString(transferTargetUrlString);

#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::transfer transferTargetUrlString=%s, targetCallId=%s\n", transferTargetUrlString.data(), targetCallId);
#endif

        // Tell the original call to complete the
        // consultative transfer
        CpMultiStringMessage consultTransfer(CP_CONSULT_TRANSFER,
            originalCallId, transferTargetUrlString.data(), targetCallId);

        postMessage(consultTransfer);
    }
    else
    {
        returnCode = PT_INVALID_STATE;
    }

    return(returnCode);
}

// Transfer an individual participant from one end point to another using 
// REFER w/replaces.
PtStatus CallManager::transfer(const char* sourceCallId, 
                               const char* sourceAddress, 
                               const char* targetCallId,
                               const char* targetAddress) 
{
    PtStatus returnCode =  PT_SUCCESS;


    // Place connections on hold
    CpMultiStringMessage sourceHold(CP_HOLD_TERM_CONNECTION, sourceCallId, sourceAddress);
    postMessage(sourceHold);
    CpMultiStringMessage targetHold(CP_HOLD_TERM_CONNECTION, targetCallId, targetAddress);
    postMessage(targetHold);
    
    // Construct the replaces header info
    // SIP alert: this is SIP specific and should not be in CallManager
    UtlString fromAddress;
    getFromField(targetCallId, targetAddress, fromAddress) ;

    // Add the Replaces header info to the consultative URL
    UtlString replacesField;
    SipMessage::buildReplacesField(replacesField, targetCallId, fromAddress, 
            targetAddress);

    Url transferTargetUrl(targetAddress);
    transferTargetUrl.removeFieldParameters() ;
    transferTargetUrl.setHeaderParameter(SIP_REPLACES_FIELD, replacesField.data());
    UtlString transferTargetUrlString;
    transferTargetUrl.toString(transferTargetUrlString);

    // Tell the original call to complete the consultative transfer
    CpMultiStringMessage consultTransfer(CP_CONSULT_TRANSFER_ADDRESS,
    sourceCallId, sourceAddress, targetCallId, targetAddress, transferTargetUrlString);

    postMessage(consultTransfer);

    return returnCode ;
}




// Split szSourceAddress from szSourceCallId and join it to the specified 
// target call id.  The source call/connection MUST be on hold prior
// to initiating the split/join.
PtStatus CallManager::split(const char* szSourceCallId,
                            const char* szSourceAddress,
                            const char* szTargetCallId) 
{
    PtStatus status = PT_FAILED ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* splitSuccess = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage splitMessage(CP_SPLIT_CONNECTION, 
            szSourceCallId, 
            szSourceAddress,
            szTargetCallId, 
            NULL, 
            NULL,
            (intptr_t) splitSuccess);
    postMessage(splitMessage);

    // Wait until the call sets the number of connections
    if(splitSuccess->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success ;
        splitSuccess->getEventData(success);
        eventMgr->release(splitSuccess);

        if (success)
        {
            status = PT_SUCCESS  ;
        } 
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::split TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == splitSuccess->signal(0))
        {
            eventMgr->release(splitSuccess);
        }
    }

    return status ;
}


// Blind transfer?
PtStatus CallManager::transfer_blind(const char* callId, const char* transferToUrl,
                               UtlString* targetConnectionCallId,
                               UtlString* targetConnectionAddress)
{
    UtlString transferTargetUrl(transferToUrl ? transferToUrl : "");

    PtStatus returnCode = validateAddress(transferTargetUrl);

    if(returnCode == PT_SUCCESS)
    {
        if(targetConnectionAddress)
            *targetConnectionAddress = transferToUrl;
        UtlString targetCallId;
        getNewCallId(&targetCallId);
        if(targetConnectionCallId)
            *targetConnectionCallId = targetCallId;

#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::transfer type: %d transferUrl: \"%s\"\n",
            mTransferType, transferTargetUrl.data());
#endif

        // CP_BLIND_TRANSFER (i.e. two call blind transfer)
        CpMultiStringMessage transferMessage(CP_BLIND_TRANSFER,
            callId, transferTargetUrl, targetCallId.data(), NULL, NULL,
            getNewMetaEventId());

        postMessage(transferMessage);
    }
    return(returnCode);
}

void CallManager::sendFlowgraphMessage(const char* callId, OsMsg& flowgraphMessage)
{
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::sendFlowgraphMessage");
    CpMultiStringMessage message(CP_FLOWGRAPH_MESSAGE,
                                 callId, 
                                 NULL,
                                 NULL, 
                                 NULL, 
                                 NULL,
                                 (intptr_t) &flowgraphMessage);
    postMessage(message);
}

void CallManager::toneStart(const char* callId, int toneId, UtlBoolean local, UtlBoolean remote)
{
    CpMultiStringMessage startToneMessage(CP_START_TONE_TERM_CONNECTION,
        callId, NULL, NULL, NULL, NULL,
        toneId, local, remote);

    postMessage(startToneMessage);
}

void CallManager::toneStop(const char* callId)
{
    CpMultiStringMessage stopToneMessage(CP_STOP_TONE_TERM_CONNECTION, callId);
    postMessage(stopToneMessage);
}

void CallManager::audioPlay(const char* callId, const char* audioUrl, UtlBoolean repeat, UtlBoolean local, UtlBoolean remote, UtlBoolean mixWithMic, int downScaling)
{
    CpMultiStringMessage startToneMessage(CP_PLAY_AUDIO_TERM_CONNECTION,
        callId, audioUrl, NULL, NULL, NULL,
        repeat, local, remote, mixWithMic, downScaling);

    postMessage(startToneMessage);
}

void CallManager::audioStop(const char* callId)
{
    CpMultiStringMessage stopAudioMessage(CP_STOP_AUDIO_TERM_CONNECTION, callId);
    postMessage(stopAudioMessage);
}


void CallManager::toneChannelStart(const char* callId, const char* szRemoteAddress, int toneId, UtlBoolean local, UtlBoolean remote)
{
    CpMultiStringMessage startToneMessage(CP_START_TONE_CONNECTION,
        callId, szRemoteAddress, NULL, NULL, NULL,
        toneId, local, remote);

    postMessage(startToneMessage);
}

void CallManager::toneChannelStop(const char* callId, const char* szRemoteAddress)
{
    CpMultiStringMessage stopToneMessage(CP_STOP_TONE_CONNECTION, 
        callId, szRemoteAddress);
    postMessage(stopToneMessage);
}

void CallManager::audioChannelPlay(const char* callId, const char* szRemoteAddress, const char* audioUrl, UtlBoolean repeat, UtlBoolean local, UtlBoolean remote, UtlBoolean mixWithMic, int downScaling)
{
    CpMultiStringMessage startPlayMessage(CP_PLAY_AUDIO_CONNECTION,
        callId, szRemoteAddress, audioUrl, NULL, NULL,
        repeat, local, remote, mixWithMic, downScaling);

    postMessage(startPlayMessage);
}

void CallManager::audioChannelStop(const char* callId, const char* szRemoteAddress)
{
    CpMultiStringMessage stopAudioMessage(CP_STOP_AUDIO_CONNECTION, 
        callId, szRemoteAddress);
    postMessage(stopAudioMessage);
}


OsStatus CallManager::audioChannelRecordStart(const char* callId, 
                                              const char* szRemoteAddress, 
                                              const char* szFile,
                                              const CpMediaInterface::CpAudioFileFormat recordFormat,
                                              const UtlBoolean appendToFile,
                                              const int numChannels)
{
    OsStatus status = OS_FAILED ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* pEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage message(CP_RECORD_AUDIO_CONNECTION_START, 
                                 callId, 
                                 szRemoteAddress,
                                 szFile, 
                                 NULL, 
                                 NULL,
                                 (intptr_t) pEvent,
                                 recordFormat,
                                 appendToFile,
                                 numChannels);
    postMessage(message);

    // Wait for error response
    if(pEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success;
        pEvent->getEventData(success);
        eventMgr->release(pEvent);

        if (success)
        {
            status = OS_SUCCESS  ;
        } 
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::audioChannelRecordStart TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pEvent->signal(0))
        {
            eventMgr->release(pEvent);
        }
    }

    return status ;
}

OsStatus CallManager::audioChannelRecordPause(const char* callId, const char* szRemoteAddress) 
{
    OsStatus status = OS_FAILED ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* pEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage message(CP_RECORD_AUDIO_CONNECTION_PAUSE, 
                                 callId, 
                                 szRemoteAddress,
                                 NULL,
                                 NULL, 
                                 NULL,
                                 (intptr_t) pEvent);

    postMessage(message);

    // Wait for error response
    if(pEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success;
        pEvent->getEventData(success);
        eventMgr->release(pEvent);

        if (success)
        {
            status = OS_SUCCESS  ;
        } 
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::audioChannelRecordPause TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pEvent->signal(0))
        {
            eventMgr->release(pEvent);
        }
    }

    return status ;
}

OsStatus CallManager::audioChannelRecordResume(const char* callId, const char* szRemoteAddress) 
{
    OsStatus status = OS_FAILED ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* pEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage message(CP_RECORD_AUDIO_CONNECTION_RESUME, 
                                 callId, 
                                 szRemoteAddress,
                                 NULL,
                                 NULL, 
                                 NULL,
                                 (intptr_t) pEvent);

    postMessage(message);

    // Wait for error response
    if(pEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success;
        pEvent->getEventData(success);
        eventMgr->release(pEvent);

        if (success)
        {
            status = OS_SUCCESS  ;
        } 
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::audioChannelRecordResume TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pEvent->signal(0))
        {
            eventMgr->release(pEvent);
        }
    }

    return status ;
}

OsStatus CallManager::audioChannelRecordStop(const char* callId, const char* szRemoteAddress) 
{
    OsStatus status = OS_FAILED ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* pEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage message(CP_RECORD_AUDIO_CONNECTION_STOP, 
                                 callId, 
                                 szRemoteAddress,
                                 NULL,
                                 NULL, 
                                 NULL,
                                 (intptr_t) pEvent);

    postMessage(message);

    // Wait for error response
    if(pEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success;
        pEvent->getEventData(success);
        eventMgr->release(pEvent);

        if (success)
        {
            status = OS_SUCCESS  ;
        } 
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::audioChannelRecordStop TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pEvent->signal(0))
        {
            eventMgr->release(pEvent);
        }
    }

    return status ;
}

OsStatus CallManager::setMediaPassThrough(const UtlString& callId, const UtlString& remoteAddress, CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                          int mediaTypeStreamIndex, const UtlString& mediaRecieveAddress, int rtpPort, int rtcpPort)
{
    OsStatus status = OS_SUCCESS;

    CpMultiStringMessage message(CP_SET_MEDIA_PASS_THROUGH,
                                 callId,
                                 remoteAddress,
                                 mediaRecieveAddress,
                                 NULL,
                                 NULL,
                                 mediaType,
                                 mediaTypeStreamIndex,
                                 rtpPort,
                                 rtcpPort);

    status = postMessage(message);

    OsSysLog::add(FAC_CP, PRI_DEBUG, 
                  "CallManager::setMediaPassThrough postin CP_SET_MEDIA_PASS_THROUGH (%d) post returned: %d",
                  CP_SET_MEDIA_PASS_THROUGH, status);

    return(status);
}

OsStatus CallManager::createMediaConnection(const UtlString& callId, int& connectionId)
{
    OsStatus status = OS_FAILED ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* event = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage message(CP_CREATE_MEDIA_CONNECTION,
                                 callId,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 (intptr_t) event);

    postMessage(message);

    // Wait for error response
    if(event->wait(0, maxEventTime) == OS_SUCCESS)
    {
        event->getEventData((intptr_t&)connectionId);
        eventMgr->release(event);

        if (connectionId > 0)
        {
            status = OS_SUCCESS;
        } 
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::createMediaConnection TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == event->signal(0))
        {
            eventMgr->release(event);
        }
    }

    return(status);
}

OsStatus CallManager::setRtpDestination(const UtlString& callId, int connectionId, CpMediaInterface::MEDIA_STREAM_TYPE mediaType,
                                        int mediaTypeStreamIndex, const UtlString& mediaRecieveAddress, int rtpPort, int rtcpPort)
{
    OsStatus status = OS_INVALID_ARGUMENT;
    assert(!callId.isNull());
    assert(connectionId > 0);
    assert(mediaTypeStreamIndex == 0);
    assert(!mediaRecieveAddress.isNull());

    if(!callId.isNull() && 
       connectionId > 0 && 
       mediaTypeStreamIndex == 0 &&
       !mediaRecieveAddress.isNull())
    {
        CpMultiStringMessage message(CP_SET_RTP_DESTINATION,
                                     callId,
                                     mediaRecieveAddress,
                                     NULL,
                                     NULL,
                                     NULL,
                                     connectionId,
                                     mediaType,
                                     mediaTypeStreamIndex,
                                     rtpPort,
                                     rtcpPort);

        
        status = postMessage(message);
    }

    return(status);
}

OsStatus CallManager::startRtpSend(const UtlString& callId, int connectionId, const SdpCodecList& codecList)
{
    OsStatus status = OS_INVALID_ARGUMENT;
    assert(!callId.isNull());
    assert(connectionId > 0);

    if(!callId.isNull() && connectionId > 0)
    {
        CpMultiStringMessage message(CP_START_RTP_SEND,
                                     callId,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     connectionId,
                                     (intptr_t) new SdpCodecList(codecList)); // Copy needs to be freed in CpPeerCall

        
        status = postMessage(message);
    }

    return(status);
}

OsStatus CallManager::stopRtpSend(const UtlString& callId, int connectionId)
{
    OsStatus status = OS_INVALID_ARGUMENT;
    assert(!callId.isNull());
    assert(connectionId > 0);

    if(!callId.isNull() && connectionId > 0)
    {
        CpMultiStringMessage message(CP_STOP_RTP_SEND,
                                     callId,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     connectionId);

        status = postMessage(message);
    }

    return(status);
}

OsStatus CallManager::audioChannelRecordBufferStart(const char* callId,
                                                    const char* szRemoteAddress,
                                                    const char* pBuffer,
                                                    const int bufferSize,
                                                    const int bufferType,
                                                    const int maxRecordTime,
                                                    const int maxSilence)
{
    OsStatus status = OS_FAILED ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* pEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage message(CP_RECORD_BUFFER_AUDIO_CONNECTION_START,
                                 callId,
                                 szRemoteAddress,
                                 NULL,
                                 NULL,
                                 NULL,
                                 (intptr_t)pEvent,
                                 (intptr_t)pBuffer,
                                 bufferSize,
                                 bufferType,
                                 maxRecordTime,
                                 maxSilence);
    postMessage(message);

    // Wait for error response
    if(pEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success;
        pEvent->getEventData(success);
        eventMgr->release(pEvent);

        if (success)
        {
            status = OS_SUCCESS  ;
        } 
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::audioChannelRecordBufferStart TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pEvent->signal(0))
        {
            eventMgr->release(pEvent);
        }
    }

    return status ;
}


OsStatus CallManager::audioChannelRecordBufferStop(const char* callId,
                                                   const char* szRemoteAddress) 
{
    OsStatus status = OS_FAILED ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* pEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage message(CP_RECORD_BUFFER_AUDIO_CONNECTION_STOP, 
                                 callId, 
                                 szRemoteAddress,
                                 NULL,
                                 NULL, 
                                 NULL,
                                 (intptr_t) pEvent);

    postMessage(message);

    // Wait for error response
    if(pEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success;
        pEvent->getEventData(success);
        eventMgr->release(pEvent);

        if (success)
        {
            status = OS_SUCCESS  ;
        } 
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::audioChannelRecordBufferStop TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pEvent->signal(0))
        {
            eventMgr->release(pEvent);
        }
    }

    return status ;
}


void CallManager::bufferPlay(const char* callId, const void* audioBuf, int bufSize, int type, UtlBoolean repeat, UtlBoolean local, UtlBoolean remote)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* pEvent = eventMgr->alloc();
    // TODO: this should use the sample rate of the flowgraph not call manager
    // default rate
    int sTimeout = bufSize / mDefaultSampleRate;
    if (sTimeout < CP_MAX_EVENT_WAIT_SECONDS)
      sTimeout = CP_MAX_EVENT_WAIT_SECONDS;

    OsTime maxEventTime(sTimeout, 0);

    CpMultiStringMessage startToneMessage(CP_PLAY_BUFFER_TERM_CONNECTION,
       callId, NULL, NULL, NULL, NULL,
       (intptr_t)pEvent, repeat, local, remote, (intptr_t)audioBuf, bufSize, type);

    postMessage(startToneMessage);

    // Wait for error response
    if(pEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t success ;
        pEvent->getEventData(success);
        eventMgr->release(pEvent);

        if (success)
        {
            // Do something with this success?
        } 
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::bufferPlay TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == pEvent->signal(0))
        {
            eventMgr->release(pEvent);
        }
    }
}


#ifndef EXCLUDE_STREAMING
void CallManager::createPlayer(const char* callId,
                               MpStreamPlaylistPlayer** ppPlayer)
{
    // TO_BE_REMOVED
    int msgtype = CP_CREATE_PLAYLIST_PLAYER;;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* ev = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage msg(msgtype,
        callId, NULL, NULL, NULL, NULL, // strings
        (intptr_t)ev, (intptr_t) ppPlayer, 0); // ints

    postMessage(msg);

    // Wait until the player is created by CpCall
    if(ev->wait(0, maxEventTime) != OS_SUCCESS)
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::createPlayer(MpStreamPlaylistPlayer) TIMED OUT\n");

        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == ev->signal(0))
        {
            eventMgr->release(ev);
        }
    }
    else
    {
        eventMgr->release(ev);
    }
    
    //assert(false);
}

void CallManager::createPlayer(int type,
                               const char* callId,
                               const char* szStream,
                               int flags,
                               MpStreamPlayer** ppPlayer)
{
    // TO_BE_REMOVED
    int msgtype;

    switch (type)
    {
    case MpPlayer::STREAM_QUEUE_PLAYER:
        msgtype = CP_CREATE_QUEUE_PLAYER;
        break;

    case MpPlayer::STREAM_PLAYER:
    default:
        msgtype = CP_CREATE_PLAYER;
        break;
    }

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* ev = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage msg(msgtype,
        callId, szStream, NULL, NULL, NULL, // strings
        (intptr_t)ev, (intptr_t) ppPlayer, flags);   // ints

    postMessage(msg);

    // Wait until the player is created by CpCall
    if(ev->wait(0, maxEventTime) != OS_SUCCESS)
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::createPlayer TIMED OUT\n");

        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == ev->signal(0))
        {
            eventMgr->release(ev);
        }
    }
    else
    {
        eventMgr->release(ev);
    }
    
    //assert(false);
}


void CallManager::destroyPlayer(const char* callId, MpStreamPlaylistPlayer* pPlayer)
{
    // TO_BE_REMOVED
    int msgtype = CP_DESTROY_PLAYLIST_PLAYER;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* ev = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage msg(msgtype,
        callId, NULL, NULL, NULL, NULL, // strings
        (intptr_t) ev, (intptr_t) pPlayer);   // ints

    postMessage(msg);

    // Wait until the player is created by CpCall
    if(ev->wait(0, maxEventTime) != OS_SUCCESS)
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::destroyPlayer(MpStreamPlaylistPlayer) TIMED OUT\n");

        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == ev->signal(0))
        {
            eventMgr->release(ev);
        }
    }
    else
    {
        eventMgr->release(ev);
    }

    // Delete the object
    delete pPlayer;
    
    //assert(false);
}


void CallManager::destroyPlayer(int type, const char* callId, MpStreamPlayer* pPlayer)
{
    // TO_BE_REMOVED
    int msgtype;
    switch (type)
    {
    case MpPlayer::STREAM_QUEUE_PLAYER:
        msgtype = CP_DESTROY_QUEUE_PLAYER;
        break;

    case MpPlayer::STREAM_PLAYER:
    default:
        msgtype = CP_DESTROY_PLAYER;
        break;
    }

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* ev = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);

    CpMultiStringMessage msg(msgtype,
        callId, NULL, NULL, NULL, NULL, // strings
        (intptr_t) ev, (intptr_t) pPlayer);   // ints

    postMessage(msg);

    // Wait until the player is created by CpCall
    if(ev->wait(0, maxEventTime) != OS_SUCCESS)
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::destroyPlayer TIMED OUT\n");

        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == ev->signal(0))
        {
            eventMgr->release(ev);
        }
    }
    else
    {
        eventMgr->release(ev);
    }

    // Delete the object
    switch (type)
    {
    case MpPlayer::STREAM_QUEUE_PLAYER:
        delete (MpStreamQueuePlayer*) pPlayer ;
        break;
    case MpPlayer::STREAM_PLAYER:
    default:
        pPlayer->waitForDestruction() ;
        delete pPlayer ;
        break;
    }
    
    //assert(false);
}
#endif

void CallManager::setOutboundLineForCall(const char* callId, const char* address, SIPX_CONTACT_TYPE eType)
{
    CpMultiStringMessage outboundLineMessage(CP_SET_OUTBOUND_LINE, callId, 
            address, NULL, NULL, NULL, (int) eType);

    postMessage(outboundLineMessage);
}

void CallManager::acceptConnection(const char* callId,
                                   const char* address, 
                                   SIPX_CONTACT_ID contactId,
                                   const void* hWnd,
                                   const void* security,
                                   const char* locationHeader,
                                   const int bandWidth,
                                   UtlBoolean sendEarlyMedia)
{
    CpMultiStringMessage acceptMessage(CP_ACCEPT_CONNECTION, callId, address, NULL, NULL, locationHeader, (int) contactId, 
                                       (intptr_t) hWnd, (intptr_t) security, (intptr_t)bandWidth, sendEarlyMedia);
    postMessage(acceptMessage);
}



void CallManager::rejectConnection(const char* callId, const char* address, int errorCode, const char* errorText)
{
    CpMultiStringMessage rejectMessage(CP_REJECT_CONNECTION, callId, address, errorText, NULL, NULL, errorCode);
    postMessage(rejectMessage);
}

PtStatus CallManager::redirectConnection(const char* callId, const char* address,
                                         const char* forwardAddress)
{
    UtlString forwardAddressUrl(forwardAddress ? forwardAddress : "");
    PtStatus returnCode = validateAddress(forwardAddressUrl);

    if(returnCode == PT_SUCCESS)
    {
        CpMultiStringMessage acceptMessage(CP_REDIRECT_CONNECTION, callId, address,
            forwardAddressUrl.data());
        postMessage(acceptMessage);
    }
    return(returnCode);
}

void CallManager::dropConnection(const char* callId, const char* address)
{
    CpMultiStringMessage acceptMessage(CP_DROP_CONNECTION, callId, address);
    postMessage(acceptMessage);
}

void CallManager::getNumConnections(const char* callId, int& numConnections)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getNumMessage(CP_GET_NUM_CONNECTIONS, callId, NULL,
        NULL, NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t temp;
        numConnectionsSet->getEventData(temp);
        numConnections = temp;
        eventMgr->release(numConnectionsSet);

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getNumConnections %d connections\n",
            numConnections);
#endif
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getNumConnections TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;
    }
}

OsStatus CallManager::getConnections(const char* callId, int maxConnections,
                                     int& numConnections, UtlString addresses[])
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlSList* addressList = new UtlSList;
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    numConnectionsSet->setIntData((intptr_t)addressList);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getNumMessage(CP_GET_CONNECTIONS, callId, NULL, NULL,
        NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        {
            int addressIndex = 0;
            UtlSListIterator iterator(*addressList);
            UtlString* addressCollectable;
            addressCollectable = (UtlString*)iterator();
            returnCode = OS_SUCCESS;

            while (addressCollectable)
            {
                if(addressIndex >= maxConnections)
                {
                    returnCode = OS_LIMIT_REACHED;
                    break;
                }
                addresses[addressIndex] = *addressCollectable;
                addressIndex++;
                addressCollectable = (UtlString*)iterator();
            }
            numConnections = addressIndex;
        }

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getConnections %d connections\n",
            numConnections);
#endif

        addressList->destroyAll();
        delete addressList;
        eventMgr->release(numConnectionsSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getConnections TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            addressList->destroyAll();
            delete addressList;
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;

    }

    return(returnCode);
}

OsStatus CallManager::getCalledAddresses(const char* callId, int maxConnections,
                                         int& numConnections, UtlString addresses[])
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlSList* addressList = new UtlSList;
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    numConnectionsSet->setIntData((intptr_t) addressList);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getNumMessage(CP_GET_CALLED_ADDRESSES, callId, NULL,
        NULL, NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        int addressIndex = 0;
        {  // set the iterator scope
            UtlSListIterator iterator(*addressList);
            UtlString* addressCollectable;
            addressCollectable = (UtlString*)iterator();
            returnCode = OS_SUCCESS;

            while (addressCollectable)
            {
                if(addressIndex >= maxConnections)
                {
                    returnCode = OS_LIMIT_REACHED;
                    break;
                }
                addresses[addressIndex] = *addressCollectable;
                addressIndex++;
                addressCollectable = (UtlString*)iterator();
            }
            numConnections = addressIndex;
        } // end of interator scope
#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getCalledAddresses %d addresses\n",
            numConnections);
#endif

        addressList->destroyAll();
        delete addressList;
        eventMgr->release(numConnectionsSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getCalledAddresses TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            addressList->destroyAll();
            delete addressList;
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;

    }

    return(returnCode);
}

OsStatus CallManager::getCallingAddresses(const char* callId, int maxConnections,
                                          int& numConnections, UtlString addresses[])
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlSList* addressList = new UtlSList;
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    numConnectionsSet->setIntData((intptr_t) addressList);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getNumMessage(CP_GET_CALLING_ADDRESSES, callId, NULL,
        NULL, NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        int addressIndex = 0;
        UtlSListIterator iterator(*addressList);
        UtlString* addressCollectable;
        addressCollectable = (UtlString*)iterator();
        returnCode = OS_SUCCESS;

        while (addressCollectable)
        {
            if(addressIndex >= maxConnections)
            {
                returnCode = OS_LIMIT_REACHED;
                break;
            }
            addresses[addressIndex] = *addressCollectable;
            addressIndex++;
            addressCollectable = (UtlString*)iterator();
        }
        numConnections = addressIndex;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getCallingAddresses %d addresses\n",
            numConnections);
#endif

        addressList->destroyAll();
        delete addressList;
        eventMgr->release(numConnectionsSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getCalledAddresses TIMED OUT");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            addressList->destroyAll();
            delete addressList;
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;

    }

    return(returnCode);
}

OsStatus CallManager::getFromField(const char* callId,
                                   const char* address,
                                   UtlString& fromField)
{
    SipSession session;
    OsStatus status = getSession(callId, address, session);

    if(status == OS_SUCCESS)
    {
        Url fromUrl;
        session.getFromUrl(fromUrl);
        fromUrl.toString(fromField);

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getFromField %s\n", fromField.data());
#endif
    }

    else
    {
        fromField.remove(0);
    }

    return(status);
}
void CallManager::limitCodecs(const char* callId, const char* remoteAddr, const char* codecNames)
{
    CpMultiStringMessage message(CP_LIMIT_CODECS, 
                                 callId, 
                                 remoteAddr,
                                 codecNames);

    postMessage(message);
}

void CallManager::limitCodecPreferences(const char* callId,
                                        const char* remoteAddr,
                                        const int audioBandwidth,
                                        const int videoBandwidth,
                                        const char* szVideoCodecName)
{
    CpMultiStringMessage message(CP_LIMIT_CODEC_PREFERENCES, 
            callId, 
            remoteAddr,
            szVideoCodecName,
            NULL, 
            NULL,
            (int) audioBandwidth,
            (int) videoBandwidth);
    postMessage(message);
}

void CallManager::limitCodecPreferences(const char* callId,
                                        const int audioBandwidth,
                                        const int videoBandwidth,
                                        const char* szVideoCodecName)
{
    CpMultiStringMessage message(CP_LIMIT_CODEC_PREFERENCES, 
            callId,
            NULL,
            szVideoCodecName,
            NULL, 
            NULL,
            (int) audioBandwidth,
            (int) videoBandwidth);
    postMessage(message);
}

OsStatus CallManager::getSession(const char* callId,
                                 const char* address,
                                 SipSession& session)
{
   OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getSession callId = '%s', address = '%s'",
                 callId, address);
    SipSession* sessionPtr = new SipSession;
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getSession allocated session: 0x%x",
        sessionPtr);
#endif
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getSessionEvent = eventMgr->alloc();
    getSessionEvent->setIntData((intptr_t) sessionPtr);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getFieldMessage(CP_GET_SESSION, callId, address,
        NULL, NULL, NULL,
        (intptr_t)getSessionEvent);
    postMessage(getFieldMessage);

    // Wait until the call sets the number of connections
    if(getSessionEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        returnCode = OS_SUCCESS;

        session = *sessionPtr;

        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getSession deleting session: %p",
            sessionPtr);

        delete sessionPtr;
        sessionPtr = NULL;
        eventMgr->release(getSessionEvent);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getSession TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getSessionEvent->signal(0))
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getSession deleting timed out session: 0x%x",
                sessionPtr);
#endif
            delete sessionPtr;
            sessionPtr = NULL;

            eventMgr->release(getSessionEvent);
        }
    }
    return(returnCode);
}


OsStatus CallManager::getSipDialog(const char* callId,
                                   const char* address,
                                   SipDialog& dialog)
{
    OsStatus returnCode = OS_SUCCESS;

    // For now, this is only the warp for SipSession and we need to
    // re-implement it when SipSession is deprecated.
    SipSession ssn;

    returnCode = getSession(callId, address, ssn);
    // Copy all the contents into the SipDialog
    if (returnCode == OS_SUCCESS)
    {
       UtlString call;
       ssn.getCallId(call);
       dialog.setCallId(call);
       
       Url url;
       ssn.getFromUrl(url);
       dialog.setLocalField(url);

       ssn.getToUrl(url);
       dialog.setRemoteField(url);

       ssn.getLocalContact(url);
       dialog.setLocalContact(url);

       ssn.getRemoteContact(url);
       dialog.setRemoteContact(url);

       UtlString uValue;
       ssn.getInitialMethod(uValue);
       dialog.setInitialMethod(uValue);

       ssn.getLocalRequestUri(uValue);
       dialog.setLocalRequestUri(uValue);

       ssn.getRemoteRequestUri(uValue);
       dialog.setRemoteRequestUri(uValue);

       dialog.setLastLocalCseq(ssn.getLastFromCseq());
       dialog.setLastRemoteCseq(ssn.getLastToCseq());
    }
    
    return(returnCode);
}

/**
* Gets the number of lines made available by line manager.
*/
int CallManager::getNumLines()
{
    int iLines = 0;
    if (mpLineMgrTask != NULL)
    {
        iLines = mpLineMgrTask->getNumLines() ;
    }
    return iLines;
}

/**
* maxAddressesRequested is the number of addresses requested if available
* numAddressesAvailable is the actual number of addresses available.
* "addresses" is a pre-allocated array of size maxAddressesRequested
*/
OsStatus CallManager::getOutboundAddresses(int maxAddressesRequested,
                                           int& numAddressesAvailable, UtlString** addresses)
{

    OsStatus status = OS_FAILED;

    if (mpLineMgrTask != NULL)
    {
        int iMaxLines = mpLineMgrTask->getNumLines() ;

        SipLine** apLines = new SipLine*[iMaxLines] ;

        for (int i=0; i<iMaxLines; i++)
        {
            apLines[i] = new SipLine() ;
        }

        mpLineMgrTask->getLines(iMaxLines, numAddressesAvailable, apLines) ;

        if (numAddressesAvailable > 0)
        {
            status = OS_SUCCESS;
            for (int i = 0; i < numAddressesAvailable; i++)
            {
                Url urlAddress = apLines[i]->getUserEnteredUrl();
                UtlString strAddress = urlAddress.toString();
                //if the phone is set to Factory Defaults,
                //the UserEnteredUrl comes as sip:4444@
                //in that case we want to use "identity".
                unsigned int iIndexOfAtSymbol = strAddress.last('@');
                if(iIndexOfAtSymbol == strAddress.length()-1 )
                {
                    urlAddress = apLines[i]->getIdentity() ;
                }
                *addresses[i] = urlAddress.toString() ;
            }
        }

        for (int k=0; k<iMaxLines; k++)
        {
            delete apLines[k] ;
            apLines[k] = NULL ;
        }
        delete[] apLines ;
    }
    return status;
}


OsStatus CallManager::getToField(const char* callId,
                                 const char* address,
                                 UtlString& toField)
{
    SipSession session;
    OsStatus status = getSession(callId, address, session);

    if(status == OS_SUCCESS)
    {
        Url toUrl;
        session.getToUrl(toUrl);
        toUrl.toString(toField);

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getToField %s\n", toField.data());
#endif
    }

    else
    {
        toField.remove(0);
    }

    return(status);
}

void CallManager::getNumTerminalConnections(const char* callId, const char* address, int& numConnections)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getNumMessage(CP_GET_NUM_TERM_CONNECTIONS, callId,
        address, NULL, NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t temp;
        numConnectionsSet->getEventData(temp);
        numConnections = temp;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getNumTerminalConnections %d connections\n",
            numConnections);
#endif

        eventMgr->release(numConnectionsSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getToField TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;
    }
}

OsStatus CallManager::getTerminalConnections(const char* callId, const char* address, int maxConnections,
                                             int& numConnections, UtlString terminalNames[])
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlSList* addressList = new UtlSList;
    OsProtectedEvent* numConnectionsSet = eventMgr->alloc();
    numConnectionsSet->setIntData((intptr_t) addressList);
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    OsStatus returnCode = OS_WAIT_TIMEOUT;
    CpMultiStringMessage getNumMessage(CP_GET_TERM_CONNECTIONS, callId, address,
        NULL, NULL, NULL,
        (intptr_t)numConnectionsSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(numConnectionsSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        int terminalIndex = 0;
        UtlSListIterator iterator(*addressList);
        UtlString* terminalNameCollectable;
        terminalNameCollectable = (UtlString*)iterator();
        returnCode = OS_SUCCESS;

        while (terminalNameCollectable)
        {
            if(terminalIndex >= maxConnections)
            {
                returnCode = OS_LIMIT_REACHED;
                break;
            }

#ifdef TEST_PRINT_EVENT
            OsSysLog::add(FAC_CP, PRI_DEBUG, "got terminal: %s\n", terminalNameCollectable->data());
#endif

            terminalNames[terminalIndex] = *terminalNameCollectable;
            terminalIndex++;
            terminalNameCollectable = (UtlString*)iterator();
        }
        numConnections = terminalIndex;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getTerminalConnections %d connections\n",
            numConnections);
#endif

        addressList->destroyAll();
        delete addressList;
        eventMgr->release(numConnectionsSet);
    }
    else
    {
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == numConnectionsSet->signal(0))
        {
            addressList->destroyAll();
            delete addressList;
            eventMgr->release(numConnectionsSet);
        }
        numConnections = 0;
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getTerminalConnections TIMED OUT\n");

    }

    return(returnCode);
}


// Gets the CPU cost for an individual connection within the specified call.
OsStatus CallManager::getCodecCPUCostCall(const char* callId, int& cost)
{
    OsStatus status = OS_SUCCESS ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getCodecCPUCostEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getCodecCPUCostMsg(CP_GET_CODEC_CPU_COST, callId, NULL,
        NULL, NULL, NULL,
        (intptr_t)getCodecCPUCostEvent);
    postMessage(getCodecCPUCostMsg);


    // Wait until the call sets the number of connections
    if(getCodecCPUCostEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t temp;
        getCodecCPUCostEvent->getEventData(temp);
        cost = temp;
        eventMgr->release(getCodecCPUCostEvent);
    }
    else
    {
        status = OS_BUSY ;
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getCodecCPUCostCall TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getCodecCPUCostEvent->signal(0))
        {
            eventMgr->release(getCodecCPUCostEvent);
        }
        cost = 0;
    }

    return status ;
}


// Gets the CPU limit for an individual connection within the specified call.
OsStatus CallManager::getCodecCPULimitCall(const char* callId, int& cost)
{
    OsStatus status = OS_SUCCESS ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getCodecCPULimitEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getCodecCPULimitMsg(CP_GET_CODEC_CPU_LIMIT, callId, NULL,
        NULL, NULL, NULL,
        (intptr_t)getCodecCPULimitEvent);
    postMessage(getCodecCPULimitMsg);


    // Wait until the call sets the number of connections
    if(getCodecCPULimitEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t temp;
        getCodecCPULimitEvent->getEventData(temp);
        cost = temp;
        eventMgr->release(getCodecCPULimitEvent);
    }
    else
    {
        status = OS_BUSY ;
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getCodecCPULimitCall TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getCodecCPULimitEvent->signal(0))
        {
            eventMgr->release(getCodecCPULimitEvent);
        }
        cost = 0;
    }

    return status ;
}


// Sets the CPU codec limit for a call.
OsStatus CallManager::setCodecCPULimitCall(const char* callId,
                                           int limit,
                                           UtlBoolean bRenegotiate)
{
    int iOldLevel = -1 ;

    getCodecCPUCostCall(callId, iOldLevel) ;
    if (iOldLevel != limit)
    {
        CpMultiStringMessage setCPULimitMsg(CP_SET_CODEC_CPU_LIMIT, callId, NULL, NULL, NULL, NULL, limit);
        postMessage(setCPULimitMsg);

        if (bRenegotiate)
            renegotiateCodecsAllTerminalConnections(callId) ;
    }

    return OS_SUCCESS ;
}


// Sets the inbound call CPU limit for codecs
OsStatus CallManager::setInboundCodecCPULimit(int limit)
{
    CpMultiStringMessage setInboundCPULimitMsg(CP_SET_INBOUND_CODEC_CPU_LIMIT, NULL, NULL, NULL, NULL, NULL, limit);
    postMessage(setInboundCPULimitMsg);

    return OS_SUCCESS ;
}


// Sets the Mic gain.
OsStatus CallManager::setMicGain(const char* callId,
                                 float gain)
{
    float *pGain = new float(gain);
    CpMultiStringMessage setMicGainMsg(CP_SET_MIC_GAIN, callId, NULL, NULL, NULL, NULL, (intptr_t)pGain);
    OsStatus stat = postMessage(setMicGainMsg);

    return(stat);
}

OsStatus CallManager::setOutputMixWeight(const char* callId, int bridgeOutputIndex, float gain)
{
    assert(bridgeOutputIndex >= 0);
    assert(gain >= 0.0f);
    int gainInt = (int) gain;
    int gainFrac = (int)((gain - ((float)gainInt)) * 1000000);

    CpMultiStringMessage setMixWeightMsg(CP_SET_OUTPUT_MIX_WEIGHT, callId, NULL, NULL, NULL, NULL, 
       bridgeOutputIndex, gainInt, gainFrac);

    OsStatus stat = postMessage(setMixWeightMsg);

    return(stat);
}

void CallManager::answerTerminalConnection(const char* callId, const char* address, const char* terminalId,
                                           const void* pDisplay, const void* pSecurity)
{
    SIPX_VIDEO_DISPLAY* pDisplayCopy = NULL;
    SIPX_SECURITY_ATTRIBUTES* pSecurityCopy = NULL;
    
    if (pDisplay)
    {
        pDisplayCopy = new SIPX_VIDEO_DISPLAY(*(SIPX_VIDEO_DISPLAY*)pDisplay);
    }
    if (pSecurity)
    {
        pSecurityCopy = new SIPX_SECURITY_ATTRIBUTES(*(SIPX_SECURITY_ATTRIBUTES*)pSecurity);
    }
    
    CpMultiStringMessage callConnectionMessage(CP_ANSWER_CONNECTION, callId, address, NULL, NULL, NULL, (intptr_t)pDisplayCopy, (intptr_t)pSecurityCopy);
    postMessage(callConnectionMessage);
    mnTotalIncomingCalls++;

}

void CallManager::holdTerminalConnection(const char* callId, const char* address, const char* terminalId)
{
    CpMultiStringMessage holdMessage(CP_HOLD_TERM_CONNECTION, callId, address, terminalId);
    postMessage(holdMessage);
}

void CallManager::holdAllTerminalConnections(const char* callId)
{
    CpMultiStringMessage holdMessage(CP_HOLD_ALL_TERM_CONNECTIONS, callId);
    postMessage(holdMessage);
}

void CallManager::holdLocalTerminalConnection(const char* callId)
{
    CpMultiStringMessage holdMessage(CP_HOLD_LOCAL_TERM_CONNECTION, callId);
    postMessage(holdMessage);
}

void CallManager::unholdLocalTerminalConnection(const char* callId)
{    
    CpMultiStringMessage holdMessage(CP_UNHOLD_LOCAL_TERM_CONNECTION, callId);
    postMessage(holdMessage);
}

void CallManager::unholdAllTerminalConnections(const char* callId)
{
    // Unhold all of the remote connections
    CpMultiStringMessage unholdMessage(CP_UNHOLD_ALL_TERM_CONNECTIONS, callId);
    postMessage(unholdMessage);

    unholdLocalTerminalConnection(callId) ;
}

void CallManager::unholdTerminalConnection(const char* callId, const char* address, const char* terminalId)
{
    CpMultiStringMessage unholdMessage(CP_UNHOLD_TERM_CONNECTION, callId, address, terminalId);
    postMessage(unholdMessage);
}

void CallManager::renegotiateCodecsTerminalConnection(const char* callId, const char* address, const char* terminalId)
{
    CpMultiStringMessage unholdMessage(CP_RENEGOTIATE_CODECS_CONNECTION, callId, address, terminalId);
    postMessage(unholdMessage);
}

void CallManager::silentRemoteHold(const char* callId)
{
    CpMultiStringMessage silentRemoteHoldMessage(CP_SILENT_REMOTE_HOLD, callId);
    postMessage(silentRemoteHoldMessage);
}

void CallManager::renegotiateCodecsAllTerminalConnections(const char* callId)
{
    CpMultiStringMessage renegotiateMessage(CP_RENEGOTIATE_CODECS_ALL_CONNECTIONS, callId);
    postMessage(renegotiateMessage);
}

UtlBoolean CallManager::isTerminalConnectionLocal(const char* callId, const char* address, const char* terminalId)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    UtlBoolean isLocal;
    OsProtectedEvent* isLocalSet = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getNumMessage(CP_IS_LOCAL_TERM_CONNECTION, callId,
        address, terminalId, NULL, NULL,
        (intptr_t)isLocalSet);
    postMessage(getNumMessage);

    // Wait until the call sets the number of connections
    if(isLocalSet->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t tmpIsLocal;
        isLocalSet->getEventData(tmpIsLocal);
        isLocal = tmpIsLocal;  // workaround conversion issue in newer RW library

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::isTerminalConnectionLocal %d\n",
            isLocal);
#endif

        eventMgr->release(isLocalSet);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::isTerminalConnectionLocal TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == isLocalSet->signal(0))
        {
            eventMgr->release(isLocalSet);
        }
        isLocal = FALSE;
    }
    return(isLocal);
}

// Assignment operator
CallManager&
CallManager::operator=(const CallManager& rhs)
{
    if (this == &rhs)            // handle the assignment to self case
        return *this;

    return *this;
}

void CallManager::dialString(const char* url)
{
    if(url && strlen(url) > 0)
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::dialString posting dial string\n");
#endif
        UtlString trimmedUrl(url);
        trimmedUrl.strip(UtlString::both);
        CpMultiStringMessage dialEvent(CP_DIAL_STRING, trimmedUrl.data());
        postMessage(dialEvent);
    }
#ifdef TEST
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "url NULL or empty string\n");
    }
#endif

}


void CallManager::setTransferType(int type)
{
    mTransferType = type;
}

// Set the maximum number of calls to admit to the system.
void CallManager::setMaxCalls(int maxCalls)
{
    mMaxCalls = maxCalls;
}

// Enable STUN for NAT/Firewall traversal
void CallManager::enableStun(const char* szStunServer, 
                             int iServerPort,
                             int iKeepAlivePeriodSecs, 
                             OsNotification* pNotification)
{
    CpMultiStringMessage enableStunMessage(CP_ENABLE_STUN, szStunServer, NULL, 
            NULL, NULL, NULL, iServerPort, iKeepAlivePeriodSecs, (intptr_t) pNotification) ;
    postMessage(enableStunMessage);
}


void CallManager::enableTurn(const char* szTurnServer,
                             int iTurnPort,
                             const char* szUsername,
                             const char* szPassword,
                             int iKeepAlivePeriodSecs) 
{
    CpMultiStringMessage enableTurnMessage(CP_ENABLE_TURN, szTurnServer, szUsername,
            szPassword, NULL, NULL, iTurnPort, iKeepAlivePeriodSecs) ;
    postMessage(enableTurnMessage);
}

/* ============================ ACCESSORS ================================= */
int CallManager::getTransferType()
{
    return(mTransferType);
}

UtlBoolean CallManager::getCallState(const char* callId, int& state)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* callState = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getCallStateMessage(CP_GET_CALLSTATE, callId, NULL, NULL,
        NULL, NULL, (intptr_t)callState);
    postMessage(getCallStateMessage);

    // Wait until the call sets the call state
    if(callState->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t temp;
        callState->getEventData(temp);
        state = temp;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getCallState state: %d\n",
            getConnectionState);
#endif

        eventMgr->release(callState);
        return TRUE;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getCallState TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == callState->signal(0))
        {
            eventMgr->release(callState);
        }
        return FALSE;
    }
}

UtlBoolean CallManager::getConnectionState(const char* callId, const char* remoteAddress, int& state)
{
    //:provides the next csequence number for a given call session (leg) if it exists.
    // Note: this does not protect against transaction
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* connectionState = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getConnectionStateMessage(CP_GET_CONNECTIONSTATE, callId, remoteAddress, NULL,
        NULL, NULL, (intptr_t)connectionState);
    postMessage(getConnectionStateMessage);

    // Wait until the call sets the call state
    if(connectionState->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t temp;
        connectionState->getEventData(temp);
        state = temp;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getConnectionState state: %d\n",
            state);
#endif

        eventMgr->release(connectionState);
        return TRUE;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getConnectionState TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == connectionState->signal(0))
        {
            eventMgr->release(connectionState);
        }
        return FALSE;
    }
}

UtlBoolean CallManager::getNextSipCseq(const char* callId, const char* remoteAddress, int& nextCseq)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* connectionState = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getConnectionStateMessage(CP_GET_NEXT_CSEQ, callId, remoteAddress, NULL,
        NULL, NULL, (intptr_t)connectionState);
    postMessage(getConnectionStateMessage);

    // Wait until the call sets the call state
    if(connectionState->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t temp;
        connectionState->getEventData(temp);
        nextCseq = temp;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getConnectionState state: %d\n",
            state);
#endif

        eventMgr->release(connectionState);
        return TRUE;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getConnectionState TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == connectionState->signal(0))
        {
            eventMgr->release(connectionState);
        }
        nextCseq = -1;
        return FALSE;
    }
}

UtlBoolean CallManager::getTermConnectionState(const char* callId,
                                               const char* address,
                                               const char* terminal,
                                               int& state)
{
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* termConnectionState = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getTermConnectionStateMessage(CP_GET_TERMINALCONNECTIONSTATE,
        callId, address, terminal,
        NULL, NULL, (intptr_t)termConnectionState);
    postMessage(getTermConnectionStateMessage);

    // Wait until the call sets the call state
    if(termConnectionState->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t temp;
        termConnectionState->getEventData(temp);
        state = temp;

#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getTermConnectionState state: %d\n",
            state);
#endif

        eventMgr->release(termConnectionState);
        return TRUE;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getTermConnectionState TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == termConnectionState->signal(0))
        {
            eventMgr->release(termConnectionState);
        }
        return FALSE;
    }
}

UtlBoolean CallManager::changeCallFocus(CpCall* callToTakeFocus)
{
    OsWriteLock lock(mCallListMutex);
    UtlBoolean focusChanged = FALSE;

    if(callToTakeFocus != infocusCall)
    {
        focusChanged = TRUE;
        if(callToTakeFocus)
        {
            callToTakeFocus = removeCall(callToTakeFocus);
            if(callToTakeFocus) callToTakeFocus->inFocus();
        }
        if(infocusCall)
        {
            // Temporary fix so that focus change has happened
            delay(20);


            infocusCall->outOfFocus();
            pushCall(infocusCall);
        }
        infocusCall = callToTakeFocus;
    }
    return(focusChanged);
}

void CallManager::pushCall(CpCall* call)
{
    callStack.insertAt(0, new UtlVoidPtr((void*)call));
}

CpCall* CallManager::popCall()
{
    CpCall* call = NULL;
    UtlVoidPtr* callCollectable = (UtlVoidPtr*) callStack.get();
    if(callCollectable)
    {
        call = (CpCall*) callCollectable->getValue();
        delete callCollectable;
        callCollectable = NULL;
    }
    return(call);
}

CpCall* CallManager::removeCall(CpCall* call)
{
    UtlVoidPtr matchCall((void*)call);
    UtlVoidPtr* callCollectable = (UtlVoidPtr*) callStack.remove(&matchCall);
    if(callCollectable)
    {
        call = (CpCall*) callCollectable->getValue();
#ifdef TEST_PRINT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Found and removed call from stack: %X\r\n", call);
#endif
        delete callCollectable;
        callCollectable = NULL;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "Failed to find call to remove from stack\r\n");
        call = NULL;
    }
    return(call);
}

int CallManager::getCallStackSize()
{
    return(callStack.entries());
}

void CallManager::addHistoryEvent(const char* messageLogString)
{
    mMessageEventCount++;
    mCallManagerHistory[mMessageEventCount % CP_CALL_HISTORY_LENGTH] =
        messageLogString;
}

CpCall* CallManager::findHandlingCall(const char* callId)
{
    CpCall* handlingCall = NULL;

    if(infocusCall)
    {
        if(infocusCall->hasCallId(callId))
        {
            handlingCall = infocusCall;
        }
    }

    if(!handlingCall)
    {
        UtlSListIterator iterator(callStack);
        UtlVoidPtr* callCollectable;
        CpCall* call;
        callCollectable = (UtlVoidPtr*)iterator();
        while(callCollectable &&
            !handlingCall)
        {
            call = (CpCall*)callCollectable->getValue();
            if(call && call->hasCallId(callId))
            {
                handlingCall = call;
            }
            callCollectable = (UtlVoidPtr*)iterator();
        }

    }

    return(handlingCall);
}

CpCall* CallManager::findHandlingCall(int callIndex)
{
    CpCall* handlingCall = NULL;

    if(infocusCall)
    {
        if(infocusCall->getCallIndex() == callIndex)
        {
            handlingCall = infocusCall;
        }
    }

    if(!handlingCall)
    {
        UtlSListIterator iterator(callStack);
        UtlVoidPtr* callCollectable;
        CpCall* call;
        callCollectable = (UtlVoidPtr*)iterator();
        while(callCollectable &&
            !handlingCall)
        {
            call = (CpCall*)callCollectable->getValue();
            if(call && call->getCallIndex() == callIndex)
            {
                handlingCall = call;
            }
            callCollectable = (UtlVoidPtr*)iterator();
        }

    }

    return(handlingCall);
}


CpCall* CallManager::findHandlingCall(const OsMsg& eventMessage)
{
    CpCall* handlingCall = NULL;
    CpCall::handleWillingness handlingWeight = CpCall::CP_WILL_NOT_HANDLE;
    CpCall::handleWillingness thisCallHandlingWeight;

    if(infocusCall)
    {
        handlingWeight = infocusCall->willHandleMessage(eventMessage);
        if(handlingWeight != CpCall::CP_WILL_NOT_HANDLE)
        {
            handlingCall = infocusCall;
        }
    }

    if(handlingWeight != CpCall::CP_DEFINITELY_WILL_HANDLE)
    {
        UtlSListIterator iterator(callStack);
        UtlVoidPtr* callCollectable;
        CpCall* call;
        callCollectable = (UtlVoidPtr*)iterator();
        while(callCollectable)
        {
            call = (CpCall*)callCollectable->getValue();
            if(call)
            {
                thisCallHandlingWeight =
                    call->willHandleMessage(eventMessage);

                if(thisCallHandlingWeight > handlingWeight)
                {
                    handlingWeight = thisCallHandlingWeight;
                    handlingCall = call;
                }

                if(handlingWeight == CpCall::CP_DEFINITELY_WILL_HANDLE)
                {
                    break;
                }
            }
            callCollectable = (UtlVoidPtr*)iterator();
        }

    }

    return(handlingCall);
}

CpCall* CallManager::findFirstQueuedCall()
{
    CpCall* queuedCall = NULL;
    UtlSListIterator iterator(callStack);
    UtlVoidPtr* callCollectable;
    CpCall* call;
    callCollectable = (UtlVoidPtr*)iterator();

    // Go all the way through, the last queued call is the first in line
    while(callCollectable)
    {
        call = (CpCall*)callCollectable->getValue();
        if(call && call->isQueued())
        {
            queuedCall = call;
        }
        callCollectable = (UtlVoidPtr*)iterator();
    }

    return(queuedCall);
}

void CallManager::printCalls()
{
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager message history:\n");
    for(int historyIndex = 0; historyIndex < CP_CALL_HISTORY_LENGTH; historyIndex++)
    {
        if(mMessageEventCount - historyIndex >= 0)
        {
            OsSysLog::add(FAC_CP, PRI_DEBUG, "%d) %s\n", mMessageEventCount - historyIndex,
                (mCallManagerHistory[(mMessageEventCount - historyIndex) % CP_CALL_HISTORY_LENGTH]).data());
        }
    }
    OsSysLog::add(FAC_CP, PRI_DEBUG, "============================\n");

    // mCallListMutex is already locked by caller.
//    OsReadLock lock(mCallListMutex);
    if(infocusCall)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "infocusCall: %p ", infocusCall);
        //OsSysLog::add(FAC_CP, PRI_DEBUG, "shutting down: %d started: %d suspended: %d\n",
        //    infocusCall->isShuttingDown(),
        //    infocusCall->isStarted(), infocusCall->isSuspended());
        infocusCall->printCall();
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "infocusCall: %p\n", infocusCall);
    }

    int callIndex = 0;

    UtlSListIterator iterator(callStack);
    UtlVoidPtr* callCollectable;
    CpCall* call;
    callCollectable = (UtlVoidPtr*)iterator();
    while(callCollectable)
    {
        call = (CpCall*)callCollectable->getValue();
        if(call)
        {
            OsSysLog::add(FAC_CP, PRI_DEBUG, "callStack[%d] = %p ", callIndex, call);
            OsSysLog::add(FAC_CP, PRI_DEBUG, "shutting down: %d started: %d suspended: %d\n",
                call->isShuttingDown(),
                call->isStarted(), call->isSuspended());
            call->printCall();
        }
        callCollectable = (UtlVoidPtr*)iterator();
        callIndex++;
    }
    if(callIndex == 0)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "No calls on the stack\n");
    }

}

void CallManager::setOutGoingCallType(int callType)
{
    switch(callType)
    {
    case SIP_CALL:
    case MGCP_CALL:
        mOutGoingCallType = callType;
        break;
    default:
        OsSysLog::add(FAC_CP, PRI_WARNING, "CallManger::setOutGoingCallType invalid call type %d\n",
            callType);
        break;
    }
}

PtStatus CallManager::validateAddress(UtlString& address)
{
    PtStatus returnCode = PT_SUCCESS;

    // Check that we are adhering to one of the address schemes
    // Currently we only support SIP URLs so everything must map
    // to a SIP URL

    RegEx ip4Address("^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$");

    // If it is all digits
    RegEx allDigits("^[0-9*]+$");

    if(allDigits.Search(address.data()))
    {
        // There must be a valid default SIP host address (SIP_DIRECTORY_SERVER)
        UtlString directoryServerAddress;
        if(sipUserAgent)
        {
            int port;
            UtlString protocol;
            sipUserAgent->getDirectoryServer(0,&directoryServerAddress, &port,
                &protocol);
        }

        // If there is no host or there is an invalid IP4 address
        // We do not validate DNS host names here so that we do not block
        if(   directoryServerAddress.isNull() // no host
            || (   ip4Address.Search(directoryServerAddress.data())
            && !OsSocket::isIp4Address(directoryServerAddress)
            ))
        {
            returnCode = PT_INVALID_SIP_DIRECTORY_SERVER;
        }

        else
        {
            address.append("@");
            //OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::transfer adding @\n");
        }
    }

    // If it is not all digits it must be a SIP URL
    else
    {
        Url addressUrl(address.data());
        UtlString urlHost;
        addressUrl.getHostAddress(urlHost);
        if(urlHost.isNull())
        {
            returnCode = PT_INVALID_SIP_URL;
        }
        else
        {
            // If the host name is an IP4 address check that it is valid
            if(   ip4Address.Search(urlHost.data())
                && !OsSocket::isIp4Address(urlHost)
                )
            {
                returnCode = PT_INVALID_IP_ADDRESS;
            }

            else
            {
                // It is illegal to have a tag in the
                // To field of an initial INVITE
                addressUrl.removeFieldParameter("tag");
                addressUrl.toString(address);
            }
        }
    }
    return(returnCode);
}

// Get the current number of calls in the system and the maximum number of
// calls to be admitted to the system.
void CallManager::getCalls(int& currentCalls, int& maxCalls)
{
    currentCalls = getCallStackSize();
    maxCalls = mMaxCalls;
}

// The available local contact addresses
OsStatus CallManager::getLocalContactAddresses(const char* callId,
                                               SIPX_CONTACT_ADDRESS addresses[],
                                               size_t  nMaxAddresses,
                                               size_t& nActaulAddresses)
{
    //:provides the next csequence number for a given call session (leg) if it exists.
    // Note: this does not protect against transaction
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getLocalContacts = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage msg(CP_GET_LOCAL_CONTACTS, callId, NULL, NULL,
        NULL, NULL, (intptr_t)getLocalContacts, (intptr_t)addresses, (intptr_t)nMaxAddresses,
        (intptr_t)&nActaulAddresses);
    postMessage(msg);

    // Wait until the call sets the call state
    if(getLocalContacts->wait(0, maxEventTime) == OS_SUCCESS)
    {
#ifdef TEST_PRINT_EVENT
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::getLocalContactAddresses state: %d\n",
            state);
#endif
        eventMgr->release(getLocalContacts);
        return OS_SUCCESS;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getLocalContactAddresses TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getLocalContacts->signal(0))
        {
            eventMgr->release(getLocalContacts);
        }
        return OS_BUSY;
    }
}

int CallManager::getMediaConnectionId(const char* szCallId, const char* szRemoteAddress, void** ppInstData)
{
    intptr_t connectionId = -1;
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getIdEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getIdMessage(CP_GET_MEDIA_CONNECTION_ID, szCallId, szRemoteAddress, NULL, NULL, NULL, (intptr_t) getIdEvent, (intptr_t) ppInstData);
    postMessage(getIdMessage);

    // Wait until the call sets the number of connections
    if(getIdEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        getIdEvent->getEventData(connectionId);
        eventMgr->release(getIdEvent);
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getMediaConnectionId TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getIdEvent->signal(0))
        {
            eventMgr->release(getIdEvent);
        }
        connectionId = -1;
    }
    return connectionId;
}


UtlBoolean CallManager::getAudioEnergyLevels(const char*   szCallId, 
                                             const char*   szRemoteAddress,
                                             int&          iInputEnergyLevel,
                                             int&          iOutputEnergyLevel,
                                             int&          nContributors,
                                             unsigned int* pContributorSRCIds,
                                             int*          pContributorEngeryLevels)
{
    UtlBoolean bSuccess = false ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getELEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getELMessage(CP_GET_MEDIA_ENERGY_LEVELS, 
            szCallId, szRemoteAddress, NULL, NULL, NULL, 
            (intptr_t) getELEvent,
            (intptr_t) &iInputEnergyLevel, 
            (intptr_t) &iOutputEnergyLevel, 
            (intptr_t) &nContributors,
            (intptr_t) pContributorSRCIds,
            (intptr_t) pContributorEngeryLevels);
    postMessage(getELMessage);

    // Wait until the call sets the number of connections
    if(getELEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t status ;
        getELEvent->getEventData(status);
        eventMgr->release(getELEvent);

        bSuccess = (UtlBoolean) status ;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getAudioEnergyLevels TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getELEvent->signal(0))
        {
            eventMgr->release(getELEvent);
        }
        bSuccess = false ;
    }
    return bSuccess ;
}

UtlBoolean CallManager::getAudioEnergyLevels(const char*   szCallId,                                            
                                             int&          iInputEnergyLevel,
                                             int&          iOutputEnergyLevel) 
{
    UtlBoolean bSuccess = false ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getELEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getELMessage(CP_GET_CALL_MEDIA_ENERGY_LEVELS, 
            szCallId, NULL, NULL, NULL, NULL, 
            (intptr_t) getELEvent,
            (intptr_t) &iInputEnergyLevel, 
            (intptr_t) &iOutputEnergyLevel);
    postMessage(getELMessage);

    // Wait until the call sets the number of connections
    if(getELEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t status ;
        getELEvent->getEventData(status);
        eventMgr->release(getELEvent);

        bSuccess = (UtlBoolean) status ;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getAudioEnergyLevels TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getELEvent->signal(0))
        {
            eventMgr->release(getELEvent);
        }
        bSuccess = false ;
    }
    return bSuccess ;
}

UtlBoolean CallManager::getAudioRtpSourceIDs(const char*   szCallId, 
                                             const char*   szRemoteAddress,
                                             unsigned int& uiSendingSSRC,
                                             unsigned int& uiReceivingSSRC) 
{
    UtlBoolean bSuccess = false ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getSourceIdEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getSourceIdMessage(CP_GET_MEDIA_RTP_SOURCE_IDS, 
            szCallId, szRemoteAddress, NULL, NULL, NULL, 
            (intptr_t) getSourceIdEvent,
            (intptr_t) &uiSendingSSRC, 
            (intptr_t) &uiReceivingSSRC);
    postMessage(getSourceIdMessage);

    // Wait until the call sets the number of connections
    if(getSourceIdEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t status ;
        getSourceIdEvent->getEventData(status);
        eventMgr->release(getSourceIdEvent);

        bSuccess = (UtlBoolean) status ;
    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::getAudioRtpSourceIDs TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getSourceIdEvent->signal(0))
        {
            eventMgr->release(getSourceIdEvent);
        }
        bSuccess = false ;
    }
    return bSuccess ;  
}


// Can a new connection be added to the specified call?  This method is 
// delegated to the media interface.
UtlBoolean CallManager::canAddConnection(const char* szCallId)
{
    UtlBoolean bCanAdd = FALSE ;

    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* getIdEvent = eventMgr->alloc();
    OsTime maxEventTime(CP_MAX_EVENT_WAIT_SECONDS, 0);
    CpMultiStringMessage getIdMessage(CP_GET_CAN_ADD_PARTY, szCallId, NULL, NULL, NULL, NULL, (intptr_t)getIdEvent);
    postMessage(getIdMessage);

    // Wait until the call sets the number of connections
    if(getIdEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t eventData ;
        getIdEvent->getEventData(eventData);
        eventMgr->release(getIdEvent);
        bCanAdd = (UtlBoolean) eventData ;

    }
    else
    {
        OsSysLog::add(FAC_CP, PRI_ERR, "CallManager::canAddConnection TIMED OUT\n");
        // If the event has already been signalled, clean up
        if(OS_ALREADY_SIGNALED == getIdEvent->signal(0))
        {
            eventMgr->release(getIdEvent);
        }        
    }

    return bCanAdd ;
   
}

// Gets the media interface factory used by the call manager
CpMediaInterfaceFactory* CallManager::getMediaInterfaceFactory() 
{
    return mpMediaFactory;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsStatus CallManager::doCreateCall(const char* callId,
                                   int metaEventId,
                                   int metaEventType,
                                   int numMetaEventCalls,
                                   const char* metaEventCallIds[],
                                   UtlBoolean assumeFocusIfNoInfocusCall)
{
    OsStatus status = OS_SUCCESS;
    OsWriteLock lock(mCallListMutex);

    CpPeerCall* call = (CpPeerCall*)findHandlingCall(callId);
    if(call)
    {
        // This is generally bad.  The call should not exist.
        OsSysLog::add(FAC_CP, PRI_ERR, "doCreateCall cannot create call. CallId: %s already exists.\n",
                      callId);
        status = OS_FAILED;
    }
    else
    {
        if(mOutGoingCallType == SIP_CALL)
        {
            int numCodecs;
            SdpCodec** codecArray = NULL;
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::doCreateCall getting codec array copy\n");
#endif
            getCodecs(numCodecs, codecArray);
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, "CallManager::doCreateCall got %d codecs, creating CpPhoneMediaInterface\n",
                numCodecs);
#endif
            UtlString publicAddress;
            int publicPort;
            //always use sipUserAgent public address, not the mPublicAddress of this call manager.
            sipUserAgent->getViaInfo(OsSocket::UDP, publicAddress, publicPort, NULL, NULL);

            UtlString localAddress;
            int dummyPort;
            
            sipUserAgent->getLocalAddress(&localAddress, &dummyPort, TRANSPORT_UDP);
            CpMediaInterface* mediaInterface = mpMediaFactory->createMediaInterface(
                publicAddress.data(), localAddress.data(),
                numCodecs, codecArray, mLocale.data(), mExpeditedIpTos,
                mStunServer, mStunPort, mStunKeepAlivePeriodSecs, 
                mTurnServer, mTurnPort, mTurnUsername, mTurnPassword, 
                mTurnKeepAlivePeriodSecs, isIceEnabled(), mDefaultSampleRate);

            OsSysLog::add(FAC_CP, PRI_DEBUG, "Creating new SIP Call, mediaInterface: %p\n", mediaInterface);
            call = new CpPeerCall(mIsEarlyMediaFor180,
                this,
                mediaInterface,
                aquireCallIndex(),
                callId,
                sipUserAgent,
                mSipSessionReinviteTimer,
                mOutboundLine.data(),
                mHoldType,
                mOfferedTimeOut,
                mLineAvailableBehavior,
                mForwardUnconditional.data(),
                mLineBusyBehavior,
                mSipForwardOnBusy.data(),
                mNoAnswerTimeout,
                mForwardOnNoAnswer.data());
            // Short term kludge: createCall invoked, this
            // implys the phone is off hook
            call->enableDtmf();
            call->start();

            if(metaEventId > 0)
            {
                call->setMetaEvent(metaEventId, metaEventType,
                    numMetaEventCalls, metaEventCallIds);
            }
            else
            {
                int type = (metaEventType != PtEvent::META_EVENT_NONE) ? metaEventType : PtEvent::META_CALL_STARTING;
                call->startMetaEvent(getNewMetaEventId(), type, numMetaEventCalls, metaEventCallIds);
            }
            
            // Make this call infocus if there currently is not infocus call
            if(!infocusCall && assumeFocusIfNoInfocusCall)
            {
                infocusCall = call;
                infocusCall->inFocus(0);
            }
            // Other wise add this call to the stack
            else
            {
                pushCall(call);
            }

            for (int i = 0; i < numCodecs; i++)
            {
                delete codecArray[i];
            }
            delete[] codecArray;

            // If Media Interface is not started, send an event
            if (mediaInterface == NULL)
            {
               OsSysLog::add(FAC_CP, PRI_WARNING,
                             "CallManager::doCreateCall() MediaInterface creation failed");
               call->fireSipXMediaEvent(MEDIA_DEVICE_FAILURE,
                                        MEDIA_CAUSE_DEVICE_UNAVAILABLE,
                                        MEDIA_TYPE_AUDIO);
            }
        }
    }
    return status;
}


void CallManager::doConnect(const char* callId,
                            const char* addressUrl, 
                            const char* desiredConnectionCallId, 
                            SIPX_CONTACT_ID contactId,
                            const void* pDisplay,
                            const void* pSecurity,
                            const char* locationHeader,
                            const int bandWidth,
                            SIPX_TRANSPORT_DATA* pTransport,
                            const RtpTransportOptions rtpTransportOptions)
{
    OsWriteLock lock(mCallListMutex);
    CpCall* call = findHandlingCall(callId);
    if(!call)
    {
        // This is generally bad.  The call should exist.
        OsSysLog::add(FAC_CP, PRI_ERR, "doConnect cannot find CallId: %s\n", callId);
    }
    else
    {
        // For now just send the call a dialString
        CpMultiStringMessage dialStringMessage(CP_DIAL_STRING, addressUrl, desiredConnectionCallId, NULL, NULL, locationHeader,
                                               contactId, (intptr_t)pDisplay, (intptr_t)pSecurity, bandWidth, (intptr_t) pTransport, rtpTransportOptions) ;
        call->postMessage(dialStringMessage);
        call->setLocalConnectionState(PtEvent::CONNECTION_ESTABLISHED);
        call->stopMetaEvent();
    }
}

void CallManager::doEnableStun(const UtlString& stunServer, 
                               int              iStunPort,
                               int              iKeepAlivePeriodSecs, 
                               OsNotification*  pNotification)
{
    mStunServer = stunServer ;
    mStunPort = iStunPort ;
    mStunKeepAlivePeriodSecs = iKeepAlivePeriodSecs ;

    if (sipUserAgent) 
    {
        sipUserAgent->enableStun(mStunServer, mStunPort, mStunKeepAlivePeriodSecs, pNotification) ;
    }
}


void CallManager::doEnableTurn(const UtlString& turnServer, 
                               int              iTurnPort,
                               const UtlString& turnUsername,
                               const UtlString& szTurnPassword,
                               int              iKeepAlivePeriodSecs)
{
    mTurnServer = turnServer ;
    mTurnPort = iTurnPort ;
    mTurnUsername = turnUsername ;
    mTurnPassword = szTurnPassword ;
    mTurnKeepAlivePeriodSecs = iKeepAlivePeriodSecs ;

    bool bEnabled = (mTurnServer.length() > 0) && portIsValid(mTurnPort) ;
    sipUserAgent->getContactDb().enableTurn(bEnabled) ;
}




UtlBoolean CallManager::disconnectConnection(const char* callId, const char* addressUrl)
{
    OsReadLock lock(mCallListMutex);
    CpCall* call = findHandlingCall(callId);
    if(!call)
    {
        // This is generally bad.  The call should exist.
        OsSysLog::add(FAC_CP, PRI_ERR, "disconnectConnect cannot find CallId: %s\n", callId);
        return FALSE;
    }
    else
    {
        dropConnection(callId, addressUrl);
    }

    return TRUE;

}

void CallManager::getCodecs(int& numCodecs, SdpCodec**& codecArray)
{
    mpCodecFactory->getCodecs(numCodecs,
        codecArray);

}

void CallManager::doGetFocus(CpCall* call)
{
    // OsWriteLock lock(mCallListMutex);
    //if(call && infocusCall != call)
    //{
    if (call)
    {
        changeCallFocus(call);
    }
    //}
}

#ifdef _WIN32
UtlBoolean CallManager::IsTroubleShootingModeEnabled()
{
   UtlBoolean bEnabled = FALSE;
   
   const char *strPathKey        = "SOFTWARE\\Pingtel\\sipxUA";
   const char *strKeyValueName   = "TroubleShootingMode";
   HKEY hKey;
   DWORD    cbData;
   DWORD    dataType;
   DWORD    dwValue;
   
   DWORD err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,   // handle to open key
              strPathKey,           // subkey name
              0,                    // reserved
              KEY_READ,             // security access mask
              &hKey                 // handle to open key
              );

   if (err == ERROR_SUCCESS)
   {
      cbData = sizeof(DWORD);
      dataType = REG_DWORD;
      
      err = RegQueryValueEx(
                  hKey,                      // handle to key
                  strKeyValueName,           // value name
                  0,                         // reserved
                  &dataType,                 // type buffer
                  (LPBYTE)&dwValue,          // data buffer
                  &cbData);                  // size of data buffer

      if (err == ERROR_SUCCESS)
      {
         if(dwValue == 1)
            bEnabled = TRUE;
      }

      RegCloseKey(hKey);
   }
   
   return bEnabled;
}
#endif

void CallManager::onCallDestroy(CpCall* call)
{
    if (call)
    {
        call->stopMetaEvent();

        mCallListMutex.acquireWrite() ;                                                
        releaseCallIndex(call->getCallIndex());
        if(infocusCall == call)
        {
            // The infocus call is not in the mCallList -- no need to 
            // remove, but we should tell the call that it is not 
            // longer in focus.
            call->outOfFocus();                    
        }
        else
        {
            call = removeCall(call);
        }
        mCallListMutex.releaseWrite() ;
    }
}

void CallManager::yieldFocus(CpCall* call)
{
    OsWriteLock lock(mCallListMutex);
    if(infocusCall == call)
    {
        infocusCall->outOfFocus();
        pushCall(infocusCall);
        infocusCall = NULL;
    }
}
/* ============================ FUNCTIONS ================================= */
