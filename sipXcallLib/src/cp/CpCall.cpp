//
// Copyright (C) 2006-2018 SIPez LLC.  All rights reserved.
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
#include <assert.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsQueuedEvent.h>
#include <os/OsEventMsg.h>
#include "os/OsSysLog.h"
#include <cp/CpCall.h>
#include <mi/CpMediaInterface.h>
#include <cp/CpMultiStringMessage.h>
#include <cp/CpIntMessage.h>
#include <mp/MprVoiceActivityNotifier.h>
#include <mp/MpResourceTopology.h>
#include "ptapi/PtConnection.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtTerminalConnection.h"
#include "tao/TaoProviderAdaptor.h"
#include "tao/TaoListenerEventMessage.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CALL_STACK_SIZE (128*1024)    // 128K stack for the call task
#       define LOCAL_ONLY 0
#       define LOCAL_AND_REMOTE 1
#define UI_TERMINAL_CONNECTION_STATE "TerminalConnectionState"
#define UI_CONNECTION_STATE "ConnectionState"

//#define debugNew(x)   osPrintf("new 0x%08x: %s/%d\n", (int)x, __FILE__, __LINE__);
//#define debugDelete(x)        osPrintf("delete 0x%08x: %s/%d\n", (int)x, __FILE__, __LINE__);

// STATIC VARIABLE INITIALIZATIONS
OsLockingList *CpCall::spCallTrackingList = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpCall::CpCall(CpCallManager* manager,
               CpMediaInterface* callMediaInterface,
               int callIndex,
               const char* callId,
               int holdType)
: OsServerTask("Call-%d", NULL, DEF_MAX_MSGS, DEF_PRIO, DEF_OPTIONS, CALL_STACK_SIZE)
, mCallIdMutex(OsMutex::Q_FIFO)
, mMediaMsgDispatcher(&mIncomingQ)
{
    // add the call task name to a list so we can track leaked calls.
    UtlString strCallTaskName = getName();
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG,
            "Constructing %s",
            strCallTaskName.data());
#endif
    addToCallTrackingList(strCallTaskName);

    mCallInFocus = FALSE;
    mRemoteDtmf = FALSE;
    mDtmfEnabled = FALSE;

    mpManager = manager;

    mDropping = FALSE;
    mLocalHeld = FALSE;

    mCallIndex = callIndex;
    if(callId && callId[0])
    {
        setCallId(callId);
    }
    mHoldType = holdType;
    if(mHoldType < CallManager::NEAR_END_HOLD ||
        mHoldType > CallManager::FAR_END_HOLD)
    {
        mHoldType = CallManager::NEAR_END_HOLD;
    }

    // Create the media processing channel
    mpMediaInterface = callMediaInterface;

    // Set the media notification dispatcher we created on the media
    // interface and turn on notifications, so that the CpCall queue 
    // will receive the media notifications in its queue.
    if (mpMediaInterface)
    {
       mpMediaInterface->setNotificationDispatcher(&mMediaMsgDispatcher);
       mpMediaInterface->setNotificationsEnabled(true);
    }

    mCallState = PtCall::IDLE;
    mLocalConnectionState = PtEvent::CONNECTION_IDLE;
    mLocalTermConnectionState = PtTerminalConnection::IDLE;

    // Meta event initialization
    mMetaEventId = 0;
    mMetaEventType = PtEvent::META_EVENT_NONE;
    mNumMetaEventCalls = 0;
    mpMetaEventCallIds = NULL;
    mMessageEventCount = -1;

    OsSysLog::add(FAC_CP, PRI_DEBUG, "%s Call constructed: %s\n", strCallTaskName.data(), mCallId.data());
}

// Destructor
CpCall::~CpCall()
{
    if (isStarted())
    {
        waitUntilShutDown();
    }
    // remove the call task name from the list (for tracking leaked calls)
    UtlString strCallTaskName = getName();
    removeFromCallTrackingList(strCallTaskName);

    if(mpMediaInterface)
    {
        mpMediaInterface->release();
        mpMediaInterface = NULL;
    }

    if(mpMetaEventCallIds)
    {
        //for(int i = 0; i < mNumMetaEventCalls; i++)
        //{
        //    if(mpMetaEventCallIds[i]) delete mpMetaEventCallIds[i];
        //    mpMetaEventCallIds[1] = NULL;
        //}
        delete[] mpMetaEventCallIds;
        mpMetaEventCallIds = NULL;
    }

    UtlString name = getName();
#ifdef TEST_PRINT
    OsSysLog::add(FAC_CP, PRI_DEBUG, "%s destructed: %s\n", name.data(), mCallId.data());
    osPrintf("%s destructed: %s\n", name.data(), mCallId.data());
#endif
    name.remove(0);
    mCallId.remove(0);
    mOriginalCallId.remove(0);
    mTargetCallId.remove(0);

}

/* ============================ MANIPULATORS ============================== */

void CpCall::setDropState(UtlBoolean state)
{
    mDropping = state;
}

void CpCall::setCallState(int responseCode, UtlString responseText, int state, int casue)
{
    if (state != mCallState)
    {
        switch(state)
        {
        case PtCall::INVALID:
            postTaoListenerMessage(responseCode, responseText, PtEvent::CALL_INVALID, CALL_STATE, casue);
            break;

        case PtCall::ACTIVE:
            postTaoListenerMessage(responseCode, responseText, PtEvent::CALL_ACTIVE, CALL_STATE, casue);
            break;

        default:
            break;
        }
    }

    mCallState = state;
}

UtlBoolean CpCall::handleMessage(OsMsg& eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();
    //int key;
    //int hookState;
    CpMultiStringMessage* multiStringMessage = (CpMultiStringMessage*)&eventMessage;

    UtlBoolean processedMessage = TRUE;
    int qCount = mIncomingQ.numMsgs();
    int qMax = mIncomingQ.maxMsgs();
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpCall::handleMessage(%s) message type: %d subtype %d queue: %d/%d\n", 
                  mName.data(), msgType, msgSubType, qCount, qMax);

    switch(msgType)
    {
    case OsMsg::PHONE_APP:

        switch(msgSubType)
        {
            // If these cases need to be overrided,  they should
            // be broken out into virtual methods
        case CallManager::CP_START_TONE_TERM_CONNECTION:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                int toneId = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                UtlBoolean local = ((CpMultiStringMessage&)eventMessage).getInt2Data();
                UtlBoolean remote = ((CpMultiStringMessage&)eventMessage).getInt3Data();

                if(mpMediaInterface)
                {
                    mpMediaInterface->startTone(toneId,
                    local, remote);
                }
            }
            break;

        case CallManager::CP_FLOWGRAPH_MESSAGE:
        {
            OsMsg* flowgraphMessage = (OsMsg*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
            if(flowgraphMessage)
            {
                sendFlowgraphMessage(*flowgraphMessage);

                // Free up the flowgraph message
                // flowgraphMessage->releaseMsg();
            }

        }
            break;

        case CallManager::CP_STOP_TONE_TERM_CONNECTION:
            addHistoryEvent(msgSubType, multiStringMessage);
            if(mpMediaInterface)
            {
                mpMediaInterface->stopTone();
            }
            break;

        case CallManager::CP_PLAY_AUDIO_TERM_CONNECTION:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                int repeat = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                UtlBoolean local = ((CpMultiStringMessage&)eventMessage).getInt2Data();
                UtlBoolean remote = ((CpMultiStringMessage&)eventMessage).getInt3Data();
                UtlBoolean mixWithMic = ((CpMultiStringMessage&)eventMessage).getInt4Data();
                int downScaling = ((CpMultiStringMessage&)eventMessage).getInt5Data();
                UtlString url;
                ((CpMultiStringMessage&)eventMessage).getString2Data(url);

                if(mpMediaInterface)
                {
                    mpMediaInterface->playAudio(url.data(), repeat,
                        local, remote, mixWithMic, downScaling, FALSE) ;
                }
            }
            break;

        case CallManager::CP_PLAY_BUFFER_TERM_CONNECTION:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                int repeat = ((CpMultiStringMessage&)eventMessage).getInt2Data();
                UtlBoolean local = ((CpMultiStringMessage&)eventMessage).getInt3Data();
                UtlBoolean remote = ((CpMultiStringMessage&)eventMessage).getInt4Data();
                int buffer = ((CpMultiStringMessage&)eventMessage).getInt5Data();
                int bufSize = ((CpMultiStringMessage&)eventMessage).getInt6Data();
                int type = ((CpMultiStringMessage&)eventMessage).getInt7Data();
                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();

                if(mpMediaInterface)
                {
                    // Hard code the sample rate to play at 8000Hz for now,
                    // When wideband support at the callLib level is implemented,
                    // you'll want to change this to the passed in rate of the
                    // buffer.  The media interface currently understands how to
                    // process a buffer of pretty much any rate.  tested rates are:
                    // 8kHz, 16kHz, 32kHz, 48kHz.
                    mpMediaInterface->playBuffer((char*)buffer,
                    bufSize, 8000, type, repeat, local, remote, ev);
                }
            }
            break;

        case CallManager::CP_STOP_AUDIO_TERM_CONNECTION:
            addHistoryEvent(msgSubType, multiStringMessage);
            if(mpMediaInterface)
            {
                mpMediaInterface->stopAudio();
            }
            break;
        case CallManager::CP_CREATE_PLAYLIST_PLAYER:
            {
                UtlString callId;

                MpStreamPlaylistPlayer** ppPlayer = (MpStreamPlaylistPlayer **) ((CpMultiStringMessage&)eventMessage).getInt2Data();
                assert(ppPlayer != NULL);

                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "CpCall::handle creating MpStreamPlaylistPlayer ppPlayer 0x%08x ev 0x%08x",
                    (int)ppPlayer, (int)ev);
#endif

                addHistoryEvent(msgSubType, multiStringMessage);

                getCallId(callId);

                if (mpMediaInterface)
                {
                    mpMediaInterface->createPlaylistPlayer(ppPlayer, mpManager->getMessageQueue(), callId.data()) ;
                }

                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_CREATE_PLAYER:
            {
                UtlString callId;
                UtlString streamId ;

                MpStreamPlayer** ppPlayer = (MpStreamPlayer **) ((CpMultiStringMessage&)eventMessage).getInt2Data();

                assert(ppPlayer != NULL);

                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                int flags = ((CpMultiStringMessage&)eventMessage).getInt3Data();
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "CpCall::handle creating MpStreamPlayer ppPlayer 0x%08x ev 0x%08x flags %d",
                    (int)ppPlayer, (int)ev, flags);
#endif

                addHistoryEvent(msgSubType, multiStringMessage);

                ((CpMultiStringMessage&)eventMessage).getString2Data(streamId);
                getCallId(callId);

                if (mpMediaInterface)
                {
                    mpMediaInterface->createPlayer((MpStreamPlayer**)ppPlayer, streamId, flags, mpManager->getMessageQueue(), callId.data()) ;
                }
                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_CREATE_QUEUE_PLAYER:
            {
                UtlString callId;

                MpStreamPlayer** ppPlayer = (MpStreamPlayer **) ((CpMultiStringMessage&)eventMessage).getInt2Data();

                assert(ppPlayer != NULL);

                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
#ifdef TEST_PRINT
                OsSysLog::add(FAC_CP, PRI_DEBUG,
                    "CpCall::handle creating MpStreamQueuePlayer ppPlayer 0x%08x ev 0x%08x",
                    (int)ppPlayer, (int)ev);
#endif

                addHistoryEvent(msgSubType, multiStringMessage);

                getCallId(callId);

                if (mpMediaInterface)
                {
                    mpMediaInterface->createQueuePlayer((MpStreamQueuePlayer**)ppPlayer, mpManager->getMessageQueue(), callId.data()) ;
                }

                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_DESTROY_PLAYLIST_PLAYER:
            {
                MpStreamPlaylistPlayer* pPlayer ;

                addHistoryEvent(msgSubType, multiStringMessage);

                // Redispatch Request to flowgraph
                if(mpMediaInterface)
                {
                    pPlayer = (MpStreamPlaylistPlayer*) ((CpMultiStringMessage&)eventMessage).getInt2Data();
                    mpMediaInterface->destroyPlaylistPlayer(pPlayer) ;
                }

                // Signal Event so that the caller knows the work is done
                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_DESTROY_PLAYER:
            {
                MpStreamPlayer* pPlayer ;

                addHistoryEvent(msgSubType, multiStringMessage);

                // Redispatch Request to flowgraph
                if(mpMediaInterface)
                {
                    pPlayer = (MpStreamPlayer*) ((CpMultiStringMessage&)eventMessage).getInt2Data();
                    mpMediaInterface->destroyPlayer(pPlayer) ;
                }

                // Signal Event so that the caller knows the work is done
                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_DESTROY_QUEUE_PLAYER:
            {
                MpStreamPlayer* pPlayer ;

                addHistoryEvent(msgSubType, multiStringMessage);

                // Redispatch Request to flowgraph
                if(mpMediaInterface)
                {
                    pPlayer = (MpStreamPlayer*) ((CpMultiStringMessage&)eventMessage).getInt2Data();
                    mpMediaInterface->destroyQueuePlayer((MpStreamQueuePlayer*)pPlayer) ;
                }

                // Signal Event so that the caller knows the work is done
                OsProtectedEvent* ev = (OsProtectedEvent*) ((CpMultiStringMessage&)eventMessage).getInt1Data();
                if(OS_ALREADY_SIGNALED == ev->signal(0))
                {
                    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
                    eventMgr->release(ev);
                }
            }
            break;

        case CallManager::CP_DROP:
            addHistoryEvent(msgSubType, multiStringMessage);
            {
                UtlString callId;
                int metaEventId = ((CpMultiStringMessage&)eventMessage).getInt1Data();
                ((CpMultiStringMessage&)eventMessage).getString1Data(callId);

                hangUp(callId, metaEventId);                                         
            }
            break;

        case CallManager::CP_SET_OUTPUT_MIX_WEIGHT:
            addHistoryEvent(msgSubType, multiStringMessage);
            if(mpMediaInterface)
            {
               int bridgeOutputIndex = ((CpMultiStringMessage&)eventMessage).getInt1Data();
               float weight = ((CpMultiStringMessage&)eventMessage).getInt2Data();
               weight += (((double) ((CpMultiStringMessage&)eventMessage).getInt3Data()) / 
                      1000000.0f);

               OsSysLog::add(FAC_CP, PRI_DEBUG,
                  "CpCall::handleMessage CP_SET_OUTPUT_MIX_WEIGHT bridge output index: %d gain: %f",
                  bridgeOutputIndex, weight);
               assert(bridgeOutputIndex >= 0);
               assert(weight >= 0.0f);

               mpMediaInterface->setMixWeightForOutput(bridgeOutputIndex, weight);
            }
            break;

        default:
            processedMessage = handleCallMessage(eventMessage);
            break;
        }

        break;

    case OsMsg::STREAMING_MSG:
        if (mpMediaInterface)
        {
            mpMediaInterface->getMsgQ()->send(eventMessage) ;
        }
        break;

    case OsMsg::MI_NOTF_MSG:
       processedMessage = handleMiNotificationMessage((MiNotification&)eventMessage);
       break;

    default:
        processedMessage = FALSE;
        osPrintf("Unknown TYPE %d of Call message subtype: %d\n", msgType, msgSubType);
        break;
    }

    //    osPrintf("exiting CpCall::handleMessage\n");
    return(processedMessage);
}

void CpCall::inFocus(int talking)
{
    mCallInFocus = TRUE;

    mLocalConnectionState = PtEvent::CONNECTION_ESTABLISHED;
    if (talking)
        mLocalTermConnectionState = PtTerminalConnection::TALKING;
    else
        mLocalTermConnectionState = PtTerminalConnection::IDLE;

    if(mpMediaInterface)
    {
        mpMediaInterface->giveFocus();
    }


}

void CpCall::outOfFocus()
{
    mCallInFocus = FALSE;

    if(mpMediaInterface)
    {
        mpMediaInterface->defocus();
    }
}

void CpCall::sendFlowgraphMessage(OsMsg& flowgraphMessage)
{
    OsSysLog::add(FAC_CP, PRI_DEBUG, "CpCall::sendFlowgraphMessage");
    if(mpMediaInterface)
    {
        OsSysLog::add(FAC_CP, PRI_DEBUG, "CpCall::sendFlowgraphMessage line: %d", __LINE__);
        OsMsgQ* messageQueue = mpMediaInterface->getMsgQ();
        if(messageQueue)
        {
            OsSysLog::add(FAC_CP, PRI_DEBUG, "CpCall::sendFlowgraphMessage line: %d", __LINE__);
            messageQueue->send(flowgraphMessage);
            OsSysLog::add(FAC_CP, PRI_DEBUG, "CpCall::sendFlowgraphMessage line: %d", __LINE__);
        }
    }
}

void CpCall::localHold()
{
    if(!mLocalHeld)
    {
        mLocalHeld = TRUE;
        mpManager->yieldFocus(this);
        /*
        // Post a message to the callManager to change focus
        CpIntMessage localHoldMessage(CallManager::CP_YIELD_FOCUS,
            (int)this);
        mLocalTermConnectionState = PtTerminalConnection::HELD;
        mpManager->postMessage(localHoldMessage);
        */
    }
}

void CpCall::hangUp(UtlString callId, int metaEventId)
{
#ifdef TEST_PRINT
    osPrintf("CpCall::hangUp\n");
#endif
    mDropping = TRUE;
    mLocalConnectionState = PtEvent::CONNECTION_DISCONNECTED;
    mLocalTermConnectionState = PtTerminalConnection::DROPPED;

    if (metaEventId > 0)
        setMetaEvent(metaEventId, PtEvent::META_CALL_ENDING, 0, 0);
    else
        startMetaEvent(mpManager->getNewMetaEventId(), PtEvent::META_CALL_ENDING, 0, 0);

    onHook();
}

void CpCall::setLocalConnectionState(int newState)
{
    mLocalConnectionState = newState;
}

/* ============================ ACCESSORS ================================= */

int CpCall::getCallIndex()
{
    return(mCallIndex);
}

int CpCall::getCallState()
{
    return(mCallState);
}

void CpCall::printCall()
{
    UtlString callId;
    getCallId(callId);
    osPrintf("call[%d] id: %s state: %d%s\n", mCallIndex,
        callId.data(), getCallState(), mDropping ? ", Dropping" : "");

    osPrintf("Call message history:\n");
    for(int historyIndex = 0; historyIndex < CP_CALL_HISTORY_LENGTH; historyIndex++)
    {
        if(mMessageEventCount - historyIndex >= 0)
        {
            osPrintf("%d) %s\n", mMessageEventCount - historyIndex,
                (mCallHistory[(mMessageEventCount - historyIndex) % CP_CALL_HISTORY_LENGTH]).data());
        }
    }
    osPrintf("=====================\n");
}

void CpCall::getCallId(UtlString& callId)
{
    OsReadLock lock(mCallIdMutex);
    callId = mCallId;
}

void CpCall::setCallId(const char* callId)
{
    OsWriteLock lock(mCallIdMutex);
    mCallId.remove(0);
    if(callId) mCallId.append(callId);
}

void CpCall::enableDtmf()
{
    mDtmfEnabled = TRUE;
}

int CpCall::getLocalConnectionState(int state)
{
    int newState;

    switch(state)
    {
    case PtEvent::CONNECTION_CREATED:
    case PtEvent::CONNECTION_INITIATED:
        newState = Connection::CONNECTION_INITIATED;
        break;

    case PtEvent::CONNECTION_ALERTING:
        newState = Connection::CONNECTION_ALERTING;
        break;

    case PtEvent::CONNECTION_DISCONNECTED:
        newState = Connection::CONNECTION_DISCONNECTED;
        break;

    case PtEvent::CONNECTION_FAILED:
        newState = Connection::CONNECTION_FAILED;
        break;

    case PtEvent::CONNECTION_DIALING:
        newState = Connection::CONNECTION_DIALING;
        break;

    case PtEvent::CONNECTION_ESTABLISHED:
        newState = Connection::CONNECTION_ESTABLISHED;
        break;

    case PtEvent::CONNECTION_NETWORK_ALERTING:
        newState = Connection::CONNECTION_NETWORK_ALERTING;
        break;

    case PtEvent::CONNECTION_NETWORK_REACHED:
        newState = Connection::CONNECTION_NETWORK_REACHED;
        break;

    case PtEvent::CONNECTION_OFFERED:
        newState = Connection::CONNECTION_OFFERING;
        break;

    case PtEvent::CONNECTION_QUEUED:
        newState = Connection::CONNECTION_QUEUED;
        break;

    default:
        newState = Connection::CONNECTION_UNKNOWN;
        break;

    }

    return newState;
}

void CpCall::getStateString(int state, UtlString* stateLabel)
{
    stateLabel->remove(0);

    switch(state)
    {
    case PtEvent::CONNECTION_CREATED:
        stateLabel->append("CONNECTION_CREATED");
        break;

    case PtEvent::CONNECTION_ALERTING:
        stateLabel->append("CONNECTION_ALERTING");
        break;

    case PtEvent::CONNECTION_DISCONNECTED:
        stateLabel->append("CONNECTION_DISCONNECTED");
        break;

    case PtEvent::CONNECTION_FAILED:
        stateLabel->append("CONNECTION_FAILED");
        break;

    case PtEvent::CONNECTION_DIALING:
        stateLabel->append("CONNECTION_DIALING");
        break;

    case PtEvent::CONNECTION_ESTABLISHED:
        stateLabel->append("CONNECTION_ESTABLISHED");
        break;

    case PtEvent::CONNECTION_INITIATED:
        stateLabel->append("CONNECTION_INITIATED");
        break;

    case PtEvent::CONNECTION_NETWORK_ALERTING:
        stateLabel->append("CONNECTION_NETWORK_ALERTING");
        break;

    case PtEvent::CONNECTION_NETWORK_REACHED:
        stateLabel->append("CONNECTION_NETWORK_REACHED");
        break;

    case PtEvent::CONNECTION_OFFERED:
        stateLabel->append("CONNECTION_OFFERED");
        break;

    case PtEvent::CONNECTION_QUEUED:
        stateLabel->append("CONNECTION_QUEUED");
        break;

    case PtEvent::TERMINAL_CONNECTION_CREATED:
        stateLabel->append("TERMINAL_CONNECTION_CREATED");
        break;

    case PtEvent::TERMINAL_CONNECTION_RINGING:
        stateLabel->append("TERMINAL_CONNECTION_RINGING");
        break;

    case PtEvent::TERMINAL_CONNECTION_DROPPED:
        stateLabel->append("TERMINAL_CONNECTION_DROPPED");
        break;

    case PtEvent::TERMINAL_CONNECTION_UNKNOWN:
        stateLabel->append("TERMINAL_CONNECTION_UNKNOWN");
        break;

    case PtEvent::TERMINAL_CONNECTION_HELD:
        stateLabel->append("TERMINAL_CONNECTION_HELD");
        break;

    case PtEvent::TERMINAL_CONNECTION_IDLE:
        stateLabel->append("TERMINAL_CONNECTION_IDLE");
        break;

    case PtEvent::TERMINAL_CONNECTION_IN_USE:
        stateLabel->append("TERMINAL_CONNECTION_IN_USE");
        break;

    case PtEvent::TERMINAL_CONNECTION_TALKING:
        stateLabel->append("TERMINAL_CONNECTION_TALKING");
        break;

    case PtEvent::CALL_ACTIVE:
        stateLabel->append("CALL_ACTIVE");
        break;

    case PtEvent::CALL_INVALID:
        stateLabel->append("CALL_INVALID");
        break;

    case PtEvent::EVENT_INVALID:
        stateLabel->append("!! INVALID_STATE !!");
        break;

    case PtEvent::CALL_META_CALL_STARTING_STARTED:
        stateLabel->append("CALL_META_CALL_STARTING_STARTED");
        break;

    case PtEvent::CALL_META_CALL_STARTING_ENDED:
        stateLabel->append("CALL_META_CALL_STARTING_ENDED");
        break;

    case PtEvent::SINGLECALL_META_PROGRESS_STARTED:
        stateLabel->append("SINGLECALL_META_PROGRESS_STARTED");
        break;

    case PtEvent::SINGLECALL_META_PROGRESS_ENDED:
        stateLabel->append("SINGLECALL_META_PROGRESS_ENDED");
        break;

    case PtEvent::CALL_META_ADD_PARTY_STARTED:
        stateLabel->append("CALL_META_ADD_PARTY_STARTED");
        break;

    case PtEvent::CALL_META_ADD_PARTY_ENDED:
        stateLabel->append("CALL_META_ADD_PARTY_ENDED");
        break;

    case PtEvent::CALL_META_REMOVE_PARTY_STARTED:
        stateLabel->append("CALL_META_REMOVE_PARTY_STARTED");
        break;

    case PtEvent::CALL_META_REMOVE_PARTY_ENDED:
        stateLabel->append("CALL_META_REMOVE_PARTY_ENDED");
        break;

    case PtEvent::CALL_META_CALL_ENDING_STARTED:
        stateLabel->append("CALL_META_CALL_ENDING_STARTED");
        break;

    case PtEvent::CALL_META_CALL_ENDING_ENDED:
        stateLabel->append("CALL_META_CALL_ENDING_ENDED");
        break;

    case PtEvent::MULTICALL_META_MERGE_STARTED:
        stateLabel->append("MULTICALL_META_MERGE_STARTED");
        break;

    case PtEvent::MULTICALL_META_MERGE_ENDED:
        stateLabel->append("MULTICALL_META_MERGE_ENDED");
        break;

    case PtEvent::MULTICALL_META_TRANSFER_STARTED:
        stateLabel->append("MULTICALL_META_TRANSFER_STARTED");
        break;

    case PtEvent::MULTICALL_META_TRANSFER_ENDED:
        stateLabel->append("MULTICALL_META_TRANSFER_ENDED");
        break;

    case PtEvent::SINGLECALL_META_SNAPSHOT_STARTED:
        stateLabel->append("SINGLECALL_META_SNAPSHOT_STARTED");
        break;

    case PtEvent::SINGLECALL_META_SNAPSHOT_ENDED:
        stateLabel->append("SINGLECALL_META_SNAPSHOT_ENDED");
        break;

    default:
        stateLabel->append("STATE_UNKNOWN");
        break;

    }

}

void CpCall::setMetaEvent(int metaEventId, int metaEventType,
                          int numCalls, const char* metaEventCallIds[])
{
    if (mMetaEventId != 0 || mMetaEventType != PtEvent::META_EVENT_NONE)
        stopMetaEvent();

    mMetaEventId = metaEventId;
    mMetaEventType = metaEventType;

    if(mpMetaEventCallIds)
    {
        delete[] mpMetaEventCallIds;
        mpMetaEventCallIds = NULL;
    }

    if (numCalls > 0)
    {
        mNumMetaEventCalls = numCalls;
        mpMetaEventCallIds = new UtlString[numCalls];
        for(int i = 0; i < numCalls; i++)
        {
            if (metaEventCallIds)
                mpMetaEventCallIds[i] = metaEventCallIds[i];
            else
                mpMetaEventCallIds[i] = mCallId.data();
        }
    }
}

void CpCall::startMetaEvent(int metaEventId,
                            int metaEventType,
                            int numCalls,
                            const char* metaEventCallIds[],
                            int remoteIsCallee)
{
    setMetaEvent(metaEventId, metaEventType, numCalls, metaEventCallIds);
    postMetaEvent(METAEVENT_START, remoteIsCallee);
}

void CpCall::getMetaEvent(int& metaEventId, int& metaEventType,
                          int& numCalls, const UtlString* metaEventCallIds[]) const
{
    metaEventId = mMetaEventId;
    metaEventType = mMetaEventType;
    numCalls = mNumMetaEventCalls;
    *metaEventCallIds = mpMetaEventCallIds;
}

void CpCall::stopMetaEvent(int remoteIsCallee)
{
    postMetaEvent(METAEVENT_END, remoteIsCallee);

    // Clear the event info
    mMetaEventId = 0;
    mMetaEventType = PtEvent::META_EVENT_NONE;

    if(mpMetaEventCallIds)
    {
        delete[] mpMetaEventCallIds;
        mpMetaEventCallIds = NULL;
    }
}

void CpCall::setCallType(int callType)
{
    mCallType = callType;
}

int CpCall::getCallType() const
{
    return(mCallType);
}

void CpCall::setTargetCallId(const char* targetCallId)
{
    if(targetCallId && * targetCallId) mTargetCallId = targetCallId;
}

void CpCall::getTargetCallId(UtlString& targetCallId) const
{
    targetCallId = mTargetCallId;
}

void CpCall::setOriginalCallId(const char* originalCallId)
{
    if(originalCallId && * originalCallId) mOriginalCallId = originalCallId;
}

void CpCall::getOriginalCallId(UtlString& originalCallId) const
{
    originalCallId = mOriginalCallId;
}
/* ============================ INQUIRY =================================== */
UtlBoolean CpCall::isQueued()
{
    return(FALSE);
}

void CpCall::getLocalAddress(char* address, int maxLen)
{
   // :TODO: Not yet implemented.
   assert(FALSE);
}

void CpCall::getLocalTerminalId(char* terminal, int maxLen)
{
   // :TODO: Not yet implemented.
   assert(FALSE);
}

UtlBoolean CpCall::isCallIdSet()
{
    OsReadLock lock(mCallIdMutex);
    return(!mCallId.isNull());
}

UtlBoolean CpCall::isLocalHeld()
{
    return mLocalHeld;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void CpCall::addHistoryEvent(const char* messageLogString)
{
    mMessageEventCount++;
    mCallHistory[mMessageEventCount % CP_CALL_HISTORY_LENGTH] =
        messageLogString;
}

void CpCall::addHistoryEvent(const int msgSubType,
                             const CpMultiStringMessage* multiStringMessage)
{
    char eventDescription[100];
    UtlString subTypeString;
    CpCallManager::getEventSubTypeString((enum CpCallManager::EventSubTypes)msgSubType,
        subTypeString);
    UtlString msgDump;
    if(multiStringMessage) multiStringMessage->toString(msgDump, ", ");
    sprintf(eventDescription, " (%d) \n\t", msgSubType);
    addHistoryEvent(subTypeString + eventDescription + msgDump);
}

void CpCall::postMetaEvent(int state, int remoteIsCallee)
{
    /// To be removed...
}

void CpCall::postTaoListenerMessage(int responseCode,
                                    UtlString responseText,
                                    int eventId,
                                    int type,
                                    int cause,
                                    int remoteIsCallee,
                                    UtlString remoteAddress,
                                    int isRemote,
                                    UtlString targetCallId)
{
    /// To be removed...
}

int CpCall::tcStateFromEventId(int eventId)
{
    int state;

    switch(eventId)
    {
    case PtEvent::TERMINAL_CONNECTION_CREATED:
    case PtEvent::TERMINAL_CONNECTION_IDLE:
        state = PtTerminalConnection::IDLE;
        break;

    case PtEvent::TERMINAL_CONNECTION_HELD:
        state = PtTerminalConnection::HELD;
        break;

    case PtEvent::TERMINAL_CONNECTION_RINGING:
        state = PtTerminalConnection::RINGING;
        break;

    case PtEvent::TERMINAL_CONNECTION_TALKING:
        state = PtTerminalConnection::TALKING;
        break;

    case PtEvent::TERMINAL_CONNECTION_IN_USE:
        state = PtTerminalConnection::IN_USE;
        break;

    case PtEvent::TERMINAL_CONNECTION_DROPPED:
        state = PtTerminalConnection::DROPPED;
        break;

    default:
        state = PtTerminalConnection::UNKNOWN;
        break;
    }

    return state;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

OsStatus CpCall::addToCallTrackingList(UtlString &rCallTaskName)
{
    OsStatus retval = OS_FAILED;

    if (!spCallTrackingList)
        spCallTrackingList = new OsLockingList();

    UtlString *tmpTaskName = new UtlString(rCallTaskName);
    spCallTrackingList->push((void *)tmpTaskName);
    retval = OS_SUCCESS; //push doesn't have a return value

    return retval;
}



//Removes a call task from the tracking list, then deletes it.
//
OsStatus CpCall::removeFromCallTrackingList(UtlString &rCallTaskName)
{
    OsStatus retval = OS_FAILED;


    UtlString *pStrFoundTaskName;

    //get an iterator handle for safe traversal
    int iteratorHandle = spCallTrackingList->getIteratorHandle();

    pStrFoundTaskName = (UtlString *)spCallTrackingList->next(iteratorHandle);
    while (pStrFoundTaskName)
    {
        // we found a Call task name that matched.  Lets remove it
        if (*pStrFoundTaskName == rCallTaskName)
        {
            spCallTrackingList->remove(iteratorHandle);
            delete pStrFoundTaskName;
            retval = OS_SUCCESS;
        }

        pStrFoundTaskName = (UtlString *)spCallTrackingList->next(iteratorHandle);
    }

    spCallTrackingList->releaseIteratorHandle(iteratorHandle);

    return retval;
}

//returns number of call tasks being tracked
int CpCall::getCallTrackingListCount()
{
    int numCalls = 0;

    if (spCallTrackingList)
        numCalls = spCallTrackingList->getCount();

    return numCalls;
}

