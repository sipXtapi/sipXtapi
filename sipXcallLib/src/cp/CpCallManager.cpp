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
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Daniel Petrie dpetrie AT SIPez DOT com

// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include <assert.h>

// APPLICATION INCLUDES
#include <cp/CpCallManager.h>
#include <cp/CpCall.h>
#include <net/NetMd5Codec.h>
#include <os/OsLock.h>
#include <os/OsDateTime.h>
#include <os/OsSocket.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsProcess.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
#ifdef __pingtel_on_posix__ /* [ */
const int    CpCallManager::CALLMANAGER_MAX_REQUEST_MSGS = 6000;
#else
const int    CpCallManager::CALLMANAGER_MAX_REQUEST_MSGS = 1000;
#endif  

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpCallManager::CpCallManager(const char* taskName,
                             const char* callIdPrefix,
                             int rtpPortStart,
                             int rtpPortEnd,
                             const char* localAddress,
                             const char* publicAddress) 
        : OsServerTask(taskName, NULL, CALLMANAGER_MAX_REQUEST_MSGS)
        , mManagerMutex(OsMutex::Q_FIFO)
        , mCallListMutex(OsMutex::Q_FIFO)
        , mCallIndices()
{
    mDoNotDisturbFlag = FALSE;
    mMsgWaitingFlag = FALSE;
    mOfferedTimeOut = 0;
    mPChargingVectorEnabled = false ;    
   if(callIdPrefix)
   {
       mCallIdPrefix.append(callIdPrefix);
   }

   mRtpPortStart = rtpPortStart;
   mRtpPortEnd = rtpPortEnd;

   if(localAddress && *localAddress)
   {
       mLocalAddress.append(localAddress);
   }
   else
   {
       OsSocket::getHostIp(&mLocalAddress);
   }

   if(publicAddress && *publicAddress)
   {
       mPublicAddress.append(publicAddress);
   }
   else
   {
       OsSocket::getHostIp(&mPublicAddress);
   }

   mLastMetaEventId = 0;
   mbEnableICE = false ;
}

// Copy constructor
CpCallManager::CpCallManager(const CpCallManager& rCpCallManager) :
OsServerTask("badCallManagerCopy"),
mManagerMutex(OsMutex::Q_FIFO),
mCallListMutex(OsMutex::Q_FIFO)
{
    mDoNotDisturbFlag = rCpCallManager.mDoNotDisturbFlag;
    mMsgWaitingFlag = rCpCallManager.mMsgWaitingFlag;
    mOfferedTimeOut = rCpCallManager.mOfferedTimeOut;

    mLastMetaEventId = 0;
    mbEnableICE = rCpCallManager.mbEnableICE ; 
}

// Destructor
CpCallManager::~CpCallManager()
{
}

/* ============================ MANIPULATORS ============================== */

void CpCallManager::getEventSubTypeString(EventSubTypes type,
                                          UtlString& typeString)
{
    switch(type)
    {
      case CP_UNSPECIFIED:
        typeString = "CP_UNSPECIFIED";
        break;

      case CP_SIP_MESSAGE:
        typeString = "CP_SIP_MESSAGE";
        break;

      case CP_CALL_EXITED:
        typeString = "CP_CALL_EXITED";
        break;

      case CP_DIAL_STRING:
        typeString = "CP_DIAL_STRING";
        break;

      case CP_FOCUS_CALL:
        typeString = "CP_FOCUS_CALL";
        break;

      case CP_HOLD_CALL:
        typeString = "CP_HOLD_CALL";
        break;

      case CP_OFF_HOLD_CALL:
        typeString = "CP_OFF_HOLD_CALL";
        break;

      case CP_DEQUEUED_CALL:
        typeString = "CP_DEQUEUED_CALL";
        break;

      case CP_MGCP_MESSAGE:
        typeString = "CP_MGCP_MESSAGE";
        break;

      case CP_MGCP_CAPS_MESSAGE:
        typeString = "CP_MGCP_CAPS_MESSAGE";
        break;

      case CP_YIELD_FOCUS:
        typeString = "CP_YIELD_FOCUS";
        break;

      case CP_GET_FOCUS:
        typeString = "CP_GET_FOCUS";
        break;

      case CP_CREATE_CALL:
        typeString = "CP_CREATE_CALL";
        break;

      case CP_GET_CALLS:
        typeString = "CP_GET_CALLS";
        break;

      case CP_CONNECT:
        typeString = "CP_CONNECT";
        break;

      case CP_SINGLE_CALL_TRANSFER:
        typeString = "CP_SINGLE_CALL_TRANSFER";
        break;

      case CP_BLIND_TRANSFER:
        typeString = "CP_BLIND_TRANSFER";
        break;

      case CP_CONSULT_TRANSFER:
        typeString = "CP_CONSULT_TRANSFER";
        break;

      case CP_CONSULT_TRANSFER_ADDRESS:
          typeString = "CP_CONSULT_TRANSFER_ADDRESS";
        break;

      case CP_TRANSFER_CONNECTION:
        typeString = "CP_TRANSFER_CONNECTION";
        break;

      case CP_TRANSFER_CONNECTION_STATUS:
        typeString = "CP_TRANSFER_CONNECTION_STATUS";
        break;

      case CP_TRANSFEREE_CONNECTION:
        typeString = "CP_TRANSFEREE_CONNECTION";
        break;

      case CP_TRANSFEREE_CONNECTION_STATUS:
        typeString = "CP_TRANSFEREE_CONNECTION_STATUS";
        break;

      case CP_DROP:
        typeString = "CP_DROP";
        break;

      case CP_DROP_CONNECTION:
        typeString = "CP_DROP_CONNECTION";
        break;

      case CP_FORCE_DROP_CONNECTION:
        typeString = "CP_FORCE_DROP_CONNECTION";
        break;

      case CP_ANSWER_CONNECTION:
        typeString = "CP_ANSWER_CONNECTION";
        break;

      case CP_ACCEPT_CONNECTION:
        typeString = "CP_ACCEPT_CONNECTION";
        break;

      case CP_REJECT_CONNECTION:
        typeString = "CP_REJECT_CONNECTION";
        break;

      case CP_REDIRECT_CONNECTION:
        typeString = "CP_REDIRECT_CONNECTION";
        break;

      case CP_GET_NUM_CONNECTIONS:
        typeString = "CP_GET_NUM_CONNECTIONS";
        break;

      case CP_GET_CONNECTIONS:
        typeString = "CP_GET_CONNECTIONS";
        break;

      case CP_GET_CALLED_ADDRESSES:
        typeString = "CP_GET_CALLED_ADDRESSES";
        break;
 
      case CP_GET_CALLING_ADDRESSES:
        typeString = "CP_GET_CALLING_ADDRESSES";
        break;

      case CP_START_TONE_TERM_CONNECTION:
        typeString = "CP_START_TONE_TERM_CONNECTION";
        break;

      case CP_STOP_TONE_TERM_CONNECTION:
        typeString = "CP_STOP_TONE_TERM_CONNECTION";
        break;

      case CP_PLAY_AUDIO_TERM_CONNECTION:
        typeString = "CP_PLAY_AUDIO_TERM_CONNECTION";
        break;

      case CP_STOP_AUDIO_TERM_CONNECTION:
        typeString = "CP_STOP_AUDIO_TERM_CONNECTION";
        break;

      case CP_CREATE_PLAYER:
        typeString = "CP_CREATE_PLAYER";
        break;

      case CP_DESTROY_PLAYER:
        typeString = "CP_DESTROY_PLAYER";
        break;

      case CP_CREATE_PLAYLIST_PLAYER:
        typeString = "CP_CREATE_PLAYLIST_PLAYER";
        break;

      case CP_DESTROY_PLAYLIST_PLAYER:
        typeString = "CP_DESTROY_PLAYLIST_PLAYER";
        break;

      case CP_GET_NUM_TERM_CONNECTIONS:
        typeString = "CP_GET_NUM_TERM_CONNECTIONS";
        break;

      case CP_GET_TERM_CONNECTIONS:
        typeString = "CP_GET_TERM_CONNECTIONS";
        break;

      case CP_IS_LOCAL_TERM_CONNECTION:
        typeString = "CP_IS_LOCAL_TERM_CONNECTION";
        break;

      case CP_HOLD_TERM_CONNECTION:
        typeString = "CP_HOLD_TERM_CONNECTION";
        break;

      case CP_UNHOLD_TERM_CONNECTION:
        typeString = "CP_UNHOLD_TERM_CONNECTION";
        break;

      case CP_UNHOLD_LOCAL_TERM_CONNECTION:
        typeString = "CP_UNHOLD_LOCAL_TERM_CONNECTION";
        break;

      case CP_HOLD_LOCAL_TERM_CONNECTION:
        typeString = "CP_HOLD_LOCAL_TERM_CONNECTION";
        break;
 
      case CP_OFFERING_EXPIRED:
        typeString = "CP_OFFERING_EXPIRED";
        break;

      case CP_RINGING_EXPIRED:
        typeString = "CP_RINGING_EXPIRED";
        break;

      case CP_GET_CALLSTATE:
        typeString = "CP_GET_CALLSTATE";
        break;

      case CP_GET_CONNECTIONSTATE:
        typeString = "CP_GET_CONNECTIONSTATE";
        break;

      case CP_GET_TERMINALCONNECTIONSTATE:
        typeString = "CP_GET_TERMINALCONNECTIONSTATE";
        break;

      case CP_GET_SESSION:
        typeString = "CP_GET_SESSION";
        break;

     case CP_HOLD_ALL_TERM_CONNECTIONS:
        typeString = "CP_HOLD_ALL_TERM_CONNECTIONS";
        break;

     case CP_UNHOLD_ALL_TERM_CONNECTIONS:
        typeString = "CP_HOLD_ALL_TERM_CONNECTIONS";
        break ;

      case CP_CANCEL_TIMER:
        typeString = "CP_CANCEL_TIMER";
        break;
 
      case CP_GET_NEXT_CSEQ:
        typeString = "CP_GET_NEXT_CSEQ";
        break;

      default:
        typeString = "?";
        break;
    }
}

// Assignment operator
CpCallManager& 
CpCallManager::operator=(const CpCallManager& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

    mDoNotDisturbFlag = rhs.mDoNotDisturbFlag;
    mMsgWaitingFlag = rhs.mMsgWaitingFlag;
    mOfferedTimeOut = rhs.mOfferedTimeOut;
    mbEnableICE = rhs.mbEnableICE ;

    return *this;
}

void CpCallManager::getNewCallId(UtlString* callId)
{
    getNewCallId(mCallIdPrefix, callId);
}

void CpCallManager::getNewSessionId(UtlString* callId)
{
    getNewCallId("s", callId);
}


void CpCallManager::getNewCallId(const char* callIdPrefix, UtlString* callId)
{
    SipMessage::generateCallId(callIdPrefix, callId) ;
}

CpCall* CpCallManager::findCall(const char* callId)
{
    OsReadLock lock(mCallListMutex);
    UtlDListIterator callIterator(mCallList);
    CpCall* call = NULL;
    UtlInt* callCollectable = NULL;

    while ((callCollectable = (UtlInt*) callIterator()))
    {
        call = (CpCall*) callCollectable->getValue();
        if(call && call->hasCallId(callId))
        {
            break;
        }

        call = NULL;
    }

    return(call);
}

void CpCallManager::appendCall(CpCall* call)
{
    OsWriteLock lock(mCallListMutex);
    UtlInt* callCollectable = new UtlInt((int)call);
    mCallList.append(callCollectable);
}

void CpCallManager::pushCall(CpCall* call)
{
    OsWriteLock lock(mCallListMutex);
    UtlInt* callCollectable = new UtlInt((int)call);
    mCallList.insertAt(0, callCollectable);
}


void CpCallManager::setDoNotDisturb(int flag)
{
    mDoNotDisturbFlag = flag;
}

void CpCallManager::setMessageWaiting(int flag)
{
    mMsgWaitingFlag = flag;
}

void CpCallManager::setOfferedTimeout(int milisec)
{
    mOfferedTimeOut = milisec;
}

void CpCallManager::enableIce(UtlBoolean bEnable) 
{
    mbEnableICE = bEnable ;
}


void CpCallManager::setVoiceQualityReportTarget(const char* szTargetSipUrl) 
{
    mVoiceQualityReportTarget = szTargetSipUrl ;
}

void CpCallManager::enablePChargingVector(UtlBoolean bEnable,
                                          const char* szIoi) 
{
    mPChargingVectorEnabled = bEnable ;
    mPChargingVectorIoi = szIoi ;
}
/* ============================ ACCESSORS ================================= */

int CpCallManager::getNewMetaEventId()
{
    mLastMetaEventId++;
    return(mLastMetaEventId);
}

UtlBoolean CpCallManager::isIceEnabled() const
{
    return mbEnableICE ;
}


UtlBoolean CpCallManager::getVoiceQualityReportTarget(UtlString& reportSipUrl) 
{
    UtlBoolean bRC = false ;

    if (!mVoiceQualityReportTarget.isNull())
    {
        reportSipUrl = mVoiceQualityReportTarget ;
        bRC = true ;
    }

    return bRC ;
}

void CpCallManager::getLocalAddress(UtlString& address) 
{
    address = mLocalAddress ;
}


void CpCallManager::getPChargingVectorIoi(UtlString& ioi) const
{
    ioi = mPChargingVectorIoi ;
}


/* ============================ INQUIRY =================================== */

UtlBoolean CpCallManager::isCallStateLoggingEnabled()
{
    return(mCallStateLogEnabled);
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

int CpCallManager::aquireCallIndex()
{
   int index = 0;
   UtlInt matchCallIndexColl;

   // Find the first unused slot
   UtlInt* existingCallIndex = NULL;
   do
   {
      index++;
      matchCallIndexColl.setValue(index);
      existingCallIndex = (UtlInt*) mCallIndices.find(&matchCallIndexColl);

   }
   while(existingCallIndex);

   // Insert the new one
   mCallIndices.insert(new UtlInt(matchCallIndexColl));
   return(index);
}

void CpCallManager::releaseCallIndex(int callIndex)
{
   if(callIndex > 0)
   {
      UtlInt matchCallIndexColl(callIndex);
      UtlInt* callIndexColl = NULL;
      callIndexColl = (UtlInt*) mCallIndices.remove(&matchCallIndexColl);

      if(callIndexColl) delete callIndexColl;
      callIndexColl = NULL;
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

