//
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MprnRtpStreamActivityMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MprnRtpStreamActivityMsg::MprnRtpStreamActivityMsg(const UtlString& namedResOriginator,
                                                   StreamState state,
                                                   RtpSRC ssrc,
                                                   unsigned sourceIp,
                                                   int sourcePort,
                                                   MpConnectionID connId,
                                                   int streamId)
: MpResNotificationMsg(MPRNM_RX_STREAM_ACTIVITY, namedResOriginator, connId, streamId)
, mState(state)
, mSsrc(ssrc)
, mAddress(sourceIp)
, mPort(sourcePort)
{
}

MprnRtpStreamActivityMsg::MprnRtpStreamActivityMsg(const MprnRtpStreamActivityMsg& rMsg)
: MpResNotificationMsg(rMsg)
, mState(rMsg.mState)
, mSsrc(rMsg.mSsrc)
, mAddress(rMsg.mAddress)
{
}

OsMsg* MprnRtpStreamActivityMsg::createCopy(void) const
{
   return new MprnRtpStreamActivityMsg(*this);
}

MprnRtpStreamActivityMsg::~MprnRtpStreamActivityMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

MprnRtpStreamActivityMsg& 
MprnRtpStreamActivityMsg::operator=(const MprnRtpStreamActivityMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MpResNotificationMsg::operator=(rhs);       // assign fields for parent class

   mState = rhs.mState;
   mSsrc= rhs.mSsrc;
   mAddress = rhs.mAddress;

   return *this;
}

/* ============================ ACCESSORS ================================= */

MprnRtpStreamActivityMsg::StreamState MprnRtpStreamActivityMsg::getState() const
{
   return mState;
}

unsigned MprnRtpStreamActivityMsg::getSsrc() const
{
   return mSsrc;
}

unsigned MprnRtpStreamActivityMsg::getAddress() const
{
   return mAddress;
}

int MprnRtpStreamActivityMsg::getPort() const
{
   return mPort;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
