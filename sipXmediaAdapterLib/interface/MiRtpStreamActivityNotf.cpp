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
#include "mi/MiRtpStreamActivityNotf.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MiRtpStreamActivityNotf::MiRtpStreamActivityNotf(const UtlString& namedResOriginator,
                                                 StreamState state,
                                                 unsigned ssrc,
                                                 unsigned sourceIp,
                                                 int sourcePort,
                                                 int connId,
                                                 int streamId)
: MiNotification(MI_NOTF_RX_STREAM_ACTIVITY, namedResOriginator, connId, streamId)
, mState(state)
, mSsrc(ssrc)
, mAddress(sourceIp)
, mPort(sourcePort)
{
}

MiRtpStreamActivityNotf::MiRtpStreamActivityNotf(const MiRtpStreamActivityNotf& rMsg)
: MiNotification(rMsg)
, mState(rMsg.mState)
, mSsrc(rMsg.mSsrc)
, mAddress(rMsg.mAddress)
, mPort(rMsg.mPort)
{
}

OsMsg* MiRtpStreamActivityNotf::createCopy(void) const
{
   return new MiRtpStreamActivityNotf(*this);
}

MiRtpStreamActivityNotf::~MiRtpStreamActivityNotf()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

MiRtpStreamActivityNotf& 
MiRtpStreamActivityNotf::operator=(const MiRtpStreamActivityNotf& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MiNotification::operator=(rhs);       // assign fields for parent class

   mState = rhs.mState;
   mSsrc= rhs.mSsrc;
   mAddress = rhs.mAddress;

   return *this;
}

/* ============================ ACCESSORS ================================= */

MiRtpStreamActivityNotf::StreamState MiRtpStreamActivityNotf::getState() const
{
   return mState;
}

unsigned MiRtpStreamActivityNotf::getSsrc() const
{
   return mSsrc;
}

unsigned MiRtpStreamActivityNotf::getAddress() const
{
   return mAddress;
}

int MiRtpStreamActivityNotf::getPort() const
{
   return mPort;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
