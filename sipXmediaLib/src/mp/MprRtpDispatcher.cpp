//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#include <os/OsIntTypes.h>

// SYSTEM INCLUDES
#ifdef __pingtel_on_posix__ /* [ */
#include <netinet/in.h>
#endif /* __pingtel_on_posix__ ] */

// APPLICATION INCLUDES
#include <mp/MprRtpDispatcher.h>
#include <os/OsMsgDispatcher.h>
#include <os/OsSysLog.h>

//#include "mp/MpEventMsg.h"
//#include "mp/MpAppVariables.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
  /// Default timeout to mark stream as inactive.
OsTime MprRtpDispatcher::smDefaultRtpInactiveTime(1, 0);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprRtpDispatcher::MprRtpDispatcher(const UtlString& rName, int connectionId)
: mMutex(0)
, mResourceName(rName)
, mConnectionId(connectionId)
, mpEventMsgQ(NULL)
, mNotificationsEnabled(TRUE)
, mRtpInactiveTime(smDefaultRtpInactiveTime)
{
}

// Destructor
MprRtpDispatcher::~MprRtpDispatcher()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprRtpDispatcher::MpRtpStream::timeoutDeactivate(const OsTime &timeout)
{
   OsTime curTime;

   // Skip inactive streams
   if (!isActive())
   {
      return FALSE;
   }

   // Skip streams whose inactive time is smaller then mMaxMsInactive
   OsDateTime::getCurTime(curTime);
   OsTime d = curTime - mLastPacketTime;
   if (d < timeout)
   {
      return FALSE;
   }

   // Mark stream as inactive
   deactivate();

   return TRUE;
}

OsStatus MprRtpDispatcher::MpRtpStream::postStreamNotif(OsMsgDispatcher *pMsgQ,
                                                        const UtlString &resourceName,
                                                        MpConnectionID connId,
                                                        MprnRtpStreamActivityMsg::StreamState state) const
{
   // If there is no dispatcher, OS_NOT_FOUND is used.
   OsStatus stat = OS_NOT_FOUND;

   if (OsSysLog::willLog(FAC_MP, PRI_DEBUG))
   {
      switch (state)
      {
      case MprnRtpStreamActivityMsg::STREAM_START:
         osPrintf(" ---------------> MprRtpDispatcher: NOT ACTIVE -> ACTIVE, %s:%d, ssrc = 0x%x\n", 
                  mAddress, mPort, getSSRC());
         break;
      case MprnRtpStreamActivityMsg::STREAM_STOP:
         osPrintf(" ---------------> MprRtpDispatcher: ACTIVE -> NOT ACTIVE, %s:%d, ssrc = 0x%x\n", 
                  mAddress, mPort, getSSRC());
         break;
      case MprnRtpStreamActivityMsg::STREAM_CHANGE:
         osPrintf(" ---------------> MprRtpDispatcher: CHANGE,               %s:%d, ssrc = 0x%x\n", 
                  mAddress, mPort, getSSRC());
         break;
      }
   }

   if (pMsgQ)
   {
      MprnRtpStreamActivityMsg msg(resourceName, state,
                                   getSSRC(), mAddress, mPort, connId, mStreamId);
      stat = pMsgQ->post(msg);
   }
   return stat;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

