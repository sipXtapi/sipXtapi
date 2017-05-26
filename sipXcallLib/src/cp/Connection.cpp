//
// Copyright (C) 2005-2017 SIPez LLC.  All rights reserved.
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

// APPLICATION INCLUDES
#include <cp/Connection.h>
#include <cp/CpGhostConnection.h>
#include <mi/CpMediaInterface.h>
#include <cp/CpMultiStringMessage.h>
#include <cp/CpCall.h>
#include <sdp/SdpCodec.h>
#include <net/SipSession.h>
#include <os/OsLock.h>
#include <os/OsMsg.h>
#include <os/OsDatagramSocket.h>
#include <os/OsServerTask.h>
#include <os/OsQueuedEvent.h>
#include <os/OsTimer.h>
#include "os/OsDateTime.h"
#include "os/OsUtil.h"
#include <tao/TaoObjectMap.h>
#include <tao/TaoReference.h>
#include <tao/TaoListenerEventMessage.h>
#include <ptapi/PtConnection.h>
#include <net/TapiMgr.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CONN_DELETE_DELAY_SECS  10    // Number of seconds to wait before a
                                      // connection should be removed from a
                                      // call and deleted.

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType Connection::TYPE = "Connection";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Connection::Connection(CpCallManager* callMgr,
                       CpCall* call,
                       CpMediaInterface* mediaInterface,
                       //UiContext* callUiContext,
                       int offeringDelayMilliSeconds,
                       int availableBehavior,
                       const char* forwardUnconditionalUrl,
                       int busyBehavior, const char* forwardOnBusyUrl,
                       int forwardOnNoAnswerSeconds)
   : mConnectionId(CpMediaInterface::getInvalidConnectionId())
   , callIdMutex(OsMutex::Q_FIFO)
   , mDeleteAfter(OsTime::OS_INFINITY)
{
#ifdef TEST_PRINT
    UtlString callId;

    if (call) {
       call->getCallId(callId);
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Connection constructed: %s\n", callId.data());
    } else
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Connection constructed: call is Null\n");
#endif

    mOfferingDelay = offeringDelayMilliSeconds;
    mLineAvailableBehavior = availableBehavior;
    if(mLineAvailableBehavior == FORWARD_UNCONDITIONAL &&
        forwardUnconditionalUrl != NULL)
    {
        mForwardUnconditional.append(forwardUnconditionalUrl);
    }
    mLineBusyBehavior = busyBehavior;
    if(mLineBusyBehavior == FORWARD_ON_BUSY &&
        forwardOnBusyUrl != NULL)
    {
        mForwardOnBusy.append(forwardOnBusyUrl);
    }
    mForwardOnNoAnswerSeconds = forwardOnNoAnswerSeconds;

    mRemoteIsCallee = FALSE;
    mRemoteRequestedHold = FALSE;
    remoteRtpPort = PORT_NONE;
    remoteRtcpPort = PORT_NONE;
    remoteVideoRtpPort = PORT_NONE;
    remoteVideoRtcpPort = PORT_NONE;
    mLocalConnectionState = CONNECTION_IDLE;
    mRemoteConnectionState = CONNECTION_IDLE;
    mConnectionStateCause = CONNECTION_CAUSE_NORMAL;
    mTerminalConnState = PtTerminalConnection::IDLE;
    mHoldState = TERMCONNECTION_NONE;
    mResponseCode = 0;
    mResponseText.remove(0);

    mpCallManager = callMgr;
    mpCall = call;
    mpMediaInterface = mediaInterface;
    //mpCallUiContext = callUiContext;

    m_eLastMajor = (SIPX_CALLSTATE_EVENT) -1 ;
    m_eLastMinor = (SIPX_CALLSTATE_CAUSE) -1 ;

    mpCallManager->getNewSessionId(this) ;
    mbTransferHeld = false ;


#ifdef TEST_PRINT
    if (!callId.isNull())
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving Connection constructed: %s\n", callId.data());
    else
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving Connection constructed: call is Null\n");
#endif
}

// Copy constructor
Connection::Connection(const Connection& rConnection)
    : UtlString(rConnection)
    , callIdMutex(OsMutex::Q_FIFO)
{
}

// Destructor
Connection::~Connection()
{
#ifdef TEST_PRINT
    UtlString callId;
    if (mpCall) {
       mpCall->getCallId(callId);
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Connection destructed: %s\n", callId.data());
    } else
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Connection destructed: call is Null\n");
#endif

#ifdef TEST_PRINT
    if (!callId.isNull())
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving Connection destructed: %s\n", callId.data());
    else
       OsSysLog::add(FAC_CP, PRI_DEBUG, "Leaving Connection destructed: call is Null\n");
#endif
}

/* ============================ MANIPULATORS ============================== */

void Connection::prepareForSplit()
{
    if ((mpMediaInterface) &&
        mpMediaInterface->isConnectionIdValid(mConnectionId))
    {
        mpMediaInterface->deleteConnection(mConnectionId) ;
    }

    mpCall = NULL ;
    mpMediaInterface = NULL ;
    mConnectionId = CpMediaInterface::getInvalidConnectionId();
}


void Connection::prepareForJoin(CpCall* pNewCall, const char* szLocalAddress, CpMediaInterface* pNewMediaInterface)
{
    mpCall = pNewCall ;
    mpMediaInterface = pNewMediaInterface ;

    mpMediaInterface->createConnection(mConnectionId, szLocalAddress) ;

    // VIDEO: Need to include window handle!
    // SECURITY:  What about the security attributes?
    // RTP-over-TCP:  What about rtp-over-tcp?
}


void Connection::setState(int newState, int isLocal, int newCause, int termState)
{
   UtlString oldStateString;
   UtlString newStateString;
   int currentState = isLocal ? mLocalConnectionState : mRemoteConnectionState;
   getStateString(currentState, &oldStateString);
   getStateString(newState, &newStateString);

   int metaEventId = 0;
   int metaEventType = PtEvent::META_EVENT_NONE;
   int numCalls = 0;
   const UtlString* metaEventCallIds = NULL;
   if (mpCall)
   {
      mpCall->getMetaEvent(metaEventId, metaEventType, numCalls,
                           &metaEventCallIds);
   }

   UtlString callId;
   if (mpCall) {
      mpCall->getCallId(callId);
   }
   if (callId.isNull())
      callId="null";

   UtlString strCallName;
   if (mpCall) {
      strCallName = mpCall->getName();
   }
   if (strCallName.isNull())
   {
      strCallName="null";
   }

   if (!isStateTransitionAllowed(newState, currentState))
   {
      // Under some conditions, "invalid" state changes are allowed.
      if (!(!isLocal && metaEventId > 0 && metaEventType == PtEvent::META_CALL_TRANSFERRING))
      {
         if (newState == currentState)
         {
            OsSysLog::add(FAC_CP, PRI_DEBUG, "Connection::setState: "
                          "Questionable connection state change - isLocal %d, for call "
                          "'%s' with callid '%s' from %s to %s, cause %d",
                          isLocal, strCallName.data(), callId.data(),
                          oldStateString.data(), newStateString.data(), newCause);
         }
         else
         {
            OsSysLog::add(FAC_CP, PRI_ERR, "Connection::setState: "
                          "Invalid connection state change - isLocal %d, for call "
                          "'%s' with callid '%s' from %s to %s, cause %d",
                          isLocal, strCallName.data(), callId.data(),
                          oldStateString.data(), newStateString.data(), newCause);
         }
         return;
      }
   }

   UtlBoolean bPostStateChange = FALSE;

   if (newState != currentState || newCause != CONNECTION_CAUSE_NORMAL)
   {
      if (isLocal && newState == CONNECTION_DISCONNECTED)
      {
         if ((mpCall->canDisconnectConnection(this) || newCause == CONNECTION_CAUSE_CANCELLED) &&
             metaEventType != PtEvent::META_CALL_TRANSFERRING)
         {
            bPostStateChange = TRUE;
         }
      }
      else
      {
         bPostStateChange = TRUE;
      }
   }

   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "Call %s %s state isLocal %d\nchange\nfrom %s to\n\t %s\ncause=%d\npost change to upper layer %d",
            strCallName.data(),
            callId.data(),
            isLocal,
            oldStateString.data(),
            newStateString.data(),
            newCause,
            bPostStateChange);

   if (bPostStateChange)
   {
      mConnectionStateCause = newCause;

      if (isLocal)
      {
         mLocalConnectionState = newState;
         mTerminalConnState = termState == -1 ? terminalConnectionState(newState) : termState;
      }
      else
      {
         mRemoteConnectionState = newState;
      }

      postTaoListenerMessage(newState, newCause, isLocal);
   }
}

void Connection::setTerminalConnectionState(int newState, int isLocal, int newCause)
{
    mTerminalConnState = newState;
    mConnectionStateCause = newCause;
}


#if 1
int Connection::getState(int isLocal) const
{
   int state;

   if (mRemoteIsCallee)
      state = mRemoteConnectionState;
   else
      state = mLocalConnectionState;

   if ((mLocalConnectionState == CONNECTION_FAILED) &&
       state != mLocalConnectionState)
   {
      UtlString oldStateString, newStateString;
      getStateString(mLocalConnectionState, &oldStateString);
      getStateString(state, &newStateString);
      state = mLocalConnectionState;
   }
   else if ((mRemoteConnectionState == CONNECTION_FAILED) &&
            mRemoteConnectionState != state)
   {
      UtlString oldStateString, newStateString;
      getStateString(mRemoteConnectionState, &oldStateString);
      getStateString(state, &newStateString);
      state = mRemoteConnectionState;
   }

   return state;
}
#endif /* 1 */

int Connection::getState(int isLocal, int& cause) const
{
   cause = mConnectionStateCause;
   int state;
   if (isLocal)
      state = mLocalConnectionState;
   else
      state = mRemoteConnectionState;

   if ((mLocalConnectionState == CONNECTION_FAILED) &&
       state != mLocalConnectionState)
   {
      UtlString oldStateString, newStateString;
      getStateString(mLocalConnectionState, &oldStateString);
      getStateString(state, &newStateString);
      state = mLocalConnectionState;
   }
   else if ((mRemoteConnectionState == CONNECTION_FAILED) &&
            mRemoteConnectionState != state)
   {
      UtlString oldStateString, newStateString;
      getStateString(mRemoteConnectionState, &oldStateString);
      getStateString(state, &newStateString);
      state = mRemoteConnectionState;
   }

   return state;
}

int Connection::getTerminalState(int isLocal) const
{
    int state;

    state = mTerminalConnState;
    return state;
}

void Connection::getStateString(int state, UtlString* stateLabel)
{
    stateLabel->remove(0);

    switch(state)
    {
    case CONNECTION_IDLE:
        stateLabel->append("CONNECTION_IDLE");
        break;

    case CONNECTION_INITIATED:
        stateLabel->append("CONNECTION_INITIATED");
        break;

    case CONNECTION_QUEUED:
        stateLabel->append("CONNECTION_QUEUED");
        break;

    case CONNECTION_OFFERING:
        stateLabel->append("CONNECTION_OFFERING");
        break;

    case CONNECTION_ALERTING:
        stateLabel->append("CONNECTION_ALERTING");
        break;

    case CONNECTION_ESTABLISHED:
        stateLabel->append("CONNECTION_ESTABLISHED");
        break;

    case CONNECTION_FAILED:
        stateLabel->append("CONNECTION_FAILED");
        break;

    case CONNECTION_DISCONNECTED:
        stateLabel->append("CONNECTION_DISCONNECTED");
        break;

    case CONNECTION_DIALING:
        stateLabel->append("CONNECTION_DIALING");
        break;

    default:
        stateLabel->append("CONNECTION_UNKNOWN");
        break;

    }

}

// Assignment operator
Connection&
Connection::operator=(const Connection& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


void Connection::setLocalAddress(const char* address)
{
    OsLock lock(callIdMutex);
    mLocalAddress.remove(0);
    mLocalAddress.append(address);
}

void Connection::unimplemented(const char* methodName) const
{
    osPrintf("%s NO IMPLEMENTED\n",methodName);
}

// Is this connection marked for deletion?
void Connection::markForDeletion()
{
   OsTime timeNow ;
   OsTime deleteAfterSecs(CONN_DELETE_DELAY_SECS, 0) ;

   OsDateTime::getCurTimeSinceBoot(deleteAfterSecs) ;

   mDeleteAfter = timeNow + deleteAfterSecs ;
}


void Connection::setMediaInterface(CpMediaInterface* pMediaInterface)
{
    mpMediaInterface = pMediaInterface ;
}



UtlBoolean Connection::validStateTransition(SIPX_CALLSTATE_EVENT eFrom, SIPX_CALLSTATE_EVENT eTo)
{
    UtlBoolean bValid = TRUE ;

    switch (eFrom)
    {
        case CALLSTATE_DISCONNECTED:
            bValid = (eTo == CALLSTATE_DESTROYED) ;
            break ;
        case CALLSTATE_DESTROYED:
            bValid = FALSE ;
            break ;
        default:
            break;
    }

    // Make sure a local focus change doesn't kick off an established event
    if ((eTo == CALLSTATE_CONNECTED) && (getLocalState() != CONNECTION_ESTABLISHED))
    {
        bValid = FALSE ;
    }

    return bValid ;
}


void Connection::fireSipXSecurityEvent(SIPX_SECURITY_INFO *pEventData)
{
    TapiMgr::getInstance().fireEvent(mpCallManager, EVENT_CATEGORY_SECURITY, pEventData);
}

void Connection::fireSipXCallEvent(SIPX_CALLSTATE_EVENT eventCode, SIPX_CALLSTATE_CAUSE causeCode, void* pEventData)
{
    UtlString callId ;
    UtlString remoteAddress ;
    SipSession session ;

    // Avoid sending duplicate events
    if ((   (eventCode != m_eLastMajor) || (causeCode != m_eLastMinor)) &&
            validStateTransition(m_eLastMajor, eventCode))
    {
        m_eLastMajor = eventCode;
        m_eLastMinor = causeCode;

        mpCall->getCallId(callId);
        getRemoteAddress(&remoteAddress);
        getSession(session) ;

        TapiMgr::getInstance().fireCallEvent(mpCallManager, callId.data(), &session, remoteAddress.data(), eventCode, causeCode, pEventData) ;
    }
}


void Connection::fireSipXMediaEvent(SIPX_MEDIA_EVENT event,
                                    SIPX_MEDIA_CAUSE cause,
                                    SIPX_MEDIA_TYPE  type,
                                    void*            pEventData)
{
    UtlString callId ;
    UtlString remoteAddress ;


    getCallId(&callId) ;
    getRemoteAddress(&remoteAddress);
    if (remoteAddress == "" || remoteAddress == "sip:")
    {
        remoteAddress = mLastToAddress;
    }

    TapiMgr::getInstance().fireMediaEvent(mpCallManager, callId.data(), remoteAddress.data(), event, cause, type, pEventData) ;
}



void Connection::setTransferHeld(UtlBoolean bHeld)
{
    mbTransferHeld = bHeld ;
}

/* ============================ ACCESSORS ================================= */

// Get the ContainableType for a UtlContainable derived class.
UtlContainableType Connection::getContainableType() const
{
    return Connection::TYPE;
}

void Connection::getLocalAddress(UtlString* address)
{
    *address = mLocalAddress;
}


void Connection::getCallId(UtlString* callId)
{
    OsLock lock(callIdMutex);

    *callId = connectionCallId ;
}

void Connection::setCallId(const char* callId)
{
    OsLock lock(callIdMutex);

    connectionCallId = callId;

    UtlString callCallId;
    if(mpCall)
    {
        mpCall->getCallId(callCallId);
    }
    OsSysLog::add(FAC_CP, PRI_DEBUG,
            "Connection::setCallId(%s) Call callId: %s for call thread: %s",
            callId, callCallId.data(), mpCall ? mpCall->getName().data() : "null call");
}

void Connection::getCallerId(UtlString* callerId)
{
    OsLock lock(callIdMutex);

    *callerId = connectionCallerId ;
}

void Connection::setCallerId(const char* callerId)
{
    OsLock lock(callIdMutex);

    connectionCallerId = callerId ;
}

void Connection::getResponseText(UtlString& responseText)
{
    responseText.remove(0);
    responseText.append(mResponseText);
}

// Get the time after which this connection can be deleted.  This timespan
// is relative to boot.
OsStatus Connection::getDeleteAfter(OsTime& time)
{
   time = mDeleteAfter ;
   return OS_SUCCESS ;
}

// Get the local state for this connection
int Connection::getLocalState() const
{
   return mLocalConnectionState ;
}

// Get the remote state for this connection
int Connection::getRemoteState() const
{
   return mRemoteConnectionState ;
}

const UtlString& Connection::getRemoteRtpAddress() const
{
    return remoteRtpAddress ;
}

OsStatus Connection::getInviteHeaderValue(const char* headerName, int headerIndex, UtlString& headerValue) const
{
    headerValue = "";
    return(OS_NOT_YET_IMPLEMENTED);
}

/* ============================ INQUIRY =================================== */

UtlBoolean Connection::isInstanceOf(const UtlContainableType type) const
{
    return((type != NULL)
            && (getContainableType() != NULL)
            && ((type == getContainableType())  ||
                 UtlString::isInstanceOf(type)));
}

UtlBoolean Connection::isRemoteCallee()
{
   return(mRemoteIsCallee);
}

UtlBoolean Connection::remoteRequestedHold()
{
   return(mRemoteRequestedHold);
}

// Determines if this connection has been marked for deletion and should be
// purged from the call.
UtlBoolean Connection::isMarkedForDeletion() const
{
   return !mDeleteAfter.isInfinite() ;
}


UtlBoolean Connection::isHeld() const
{
    return mHoldState == TERMCONNECTION_HELD ;
}

UtlBoolean Connection::isHoldInProgress() const
{
    return (mHoldState == TERMCONNECTION_HOLDING ||
            mHoldState == TERMCONNECTION_UNHOLDING) ;
}

UtlBoolean Connection::isTransferHeld() const
{
    return mbTransferHeld ;
}

UtlBoolean Connection::isLocallyInitiatedRemoteHold() const
{
    return false ;
}


UtlBoolean Connection::isInviteFromThisSide() const
{
    return(FALSE);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
void Connection::postTaoListenerMessage(int state, int newCause, int isLocal)
{
   // To be removed.
}

void Connection::setOfferingTimer(int milliSeconds)
{
    UtlString    callId;
    SipSession  session ;
    Url         urlTo ;
    UtlString    remoteAddr;

    getSession(session) ;
    session.getCallId(callId) ;
    session.getToUrl(urlTo) ;
    urlTo.toString(remoteAddr) ;

    CpMultiStringMessage* offeringExpiredMessage =
        new CpMultiStringMessage(CpCallManager::CP_OFFERING_EXPIRED,
                    callId.data(), remoteAddr.data());
    OsTimer* timer = new OsTimer((mpCallManager->getMessageQueue()),
                                 (intptr_t)offeringExpiredMessage);
    // Convert from mSeconds to uSeconds
    OsTime timerTime(milliSeconds / 1000, milliSeconds % 1000);
    timer->oneshotAfter(timerTime);
#ifdef TEST_PRINT
    osPrintf("Connection::setOfferingTimer message type: %d %d",
        OsMsg::PHONE_APP, CpCallManager::CP_OFFERING_EXPIRED);
#endif

    callId.remove(0);
    remoteAddr.remove(0);
}

CpMediaInterface* Connection::getMediaInterfacePtr()
{
    return mpMediaInterface;
}

void Connection::setRingingTimer(int seconds)
{
    UtlString callId;
    mpCall->getCallId(callId);
    UtlString remoteAddr;
    getRemoteAddress(&remoteAddr);
    CpMultiStringMessage* offeringExpiredMessage =
        new CpMultiStringMessage(CpCallManager::CP_RINGING_EXPIRED,
                    callId.data(), remoteAddr.data());
    OsTimer* timer = new OsTimer((mpCallManager->getMessageQueue()),
                                 (intptr_t)offeringExpiredMessage);

#ifdef TEST_PRINT
    osPrintf("Setting ringing timeout in %d seconds\n",
        seconds);
#endif

    OsTime timerTime(seconds, 0);
    timer->oneshotAfter(timerTime);
#ifdef TEST_PRINT
    osPrintf("Connection::setRingingTimer message type: %d %d",
        OsMsg::PHONE_APP, CpCallManager::CP_RINGING_EXPIRED);
#endif
    callId.remove(0);
    remoteAddr.remove(0);
}

UtlBoolean Connection::isStateTransitionAllowed(int newState, int oldState)
{
    UtlBoolean isAllowed = TRUE;

    switch (oldState)
    {
    case CONNECTION_IDLE:
        if (newState == CONNECTION_NETWORK_ALERTING)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_QUEUED:
    case CONNECTION_OFFERING:
        if (newState != CONNECTION_ALERTING &&
            newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_ALERTING:
        if (newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_ESTABLISHED:
        if (newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_FAILED:
        if (newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_DISCONNECTED:
        if (newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_INITIATED:
        if (newState != CONNECTION_DIALING &&
            newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_OFFERING &&
            newState != CONNECTION_ALERTING &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_DIALING:
        if (newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_NETWORK_REACHED:
        if (newState != CONNECTION_NETWORK_ALERTING &&
            newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    case CONNECTION_NETWORK_ALERTING:
        if (newState != CONNECTION_ESTABLISHED &&
            newState != CONNECTION_DISCONNECTED &&
            newState != CONNECTION_FAILED &&
            newState != CONNECTION_UNKNOWN)
        {
            isAllowed = FALSE;
        }
        break;
    default:
    case CONNECTION_UNKNOWN:
        break;
    }

    return isAllowed;
}
/* //////////////////////////// PRIVATE /////////////////////////////////// */

int Connection::terminalConnectionState(int connState)
{
    int state;

    switch(connState)
    {
    case CONNECTION_IDLE:
    case CONNECTION_OFFERING:
    case CONNECTION_INITIATED:
    case CONNECTION_DIALING:
        state = PtTerminalConnection::IDLE;
        break;

    case CONNECTION_QUEUED:
        state = PtTerminalConnection::HELD;
        break;

    case CONNECTION_ALERTING:
        state = PtTerminalConnection::RINGING;
        break;

    case CONNECTION_ESTABLISHED:
        state = PtTerminalConnection::TALKING;
        break;

    case CONNECTION_DISCONNECTED:
        state = PtTerminalConnection::DROPPED;
        break;

    case CONNECTION_FAILED:
    default:
        state = PtTerminalConnection::UNKNOWN;
        break;
    }

    return state;
}

/* ============================ FUNCTIONS ================================= */

