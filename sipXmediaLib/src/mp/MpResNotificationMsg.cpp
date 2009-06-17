//
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpResNotificationMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message notification object used to communicate information from resources
// outward towards the flowgraph, and up through to users above mediaLib and beyond.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpResNotificationMsg::MpResNotificationMsg(RNMsgType msgType, 
                                           const UtlString& namedResOriginator,
                                           MpConnectionID connId,
                                           int streamId)
   : OsMsg(OsMsg::MP_RES_NOTF_MSG, msgType)
   , mMsgOriginatorName(namedResOriginator)
   , mConnectionId(connId)
   , mStreamId(streamId)
{
}

// Copy constructor
MpResNotificationMsg::MpResNotificationMsg(const MpResNotificationMsg& rMpResNotifyMsg)
   : OsMsg(rMpResNotifyMsg)
   , mMsgOriginatorName(rMpResNotifyMsg.mMsgOriginatorName)
   , mConnectionId(rMpResNotifyMsg.mConnectionId)
   , mStreamId(rMpResNotifyMsg.mStreamId)
{
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MpResNotificationMsg::createCopy(void) const
{
   return new MpResNotificationMsg(*this);
}

// Destructor
MpResNotificationMsg::~MpResNotificationMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MpResNotificationMsg& 
MpResNotificationMsg::operator=(const MpResNotificationMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mMsgOriginatorName = rhs.mMsgOriginatorName;
   mConnectionId = rhs.mConnectionId;
   mStreamId = rhs.mStreamId;

   return *this;
}

// Sets the name of the intended recipient for this message.
void MpResNotificationMsg::setOriginatingResourceName(const UtlString& resOriginator)
{
   mMsgOriginatorName = resOriginator;
}

// Set the connection ID that this message is associated with.
void MpResNotificationMsg::setConnectionId(MpConnectionID connId)
{
   mConnectionId = connId;
}

void MpResNotificationMsg::setStreamId(int streamId)
{
   mStreamId = streamId;
}

/* ============================ ACCESSORS ================================= */

// Return the type of the media resource message
int MpResNotificationMsg::getMsg(void) const
{
   return OsMsg::getMsgSubType();
}

// Returns the resource name that is the intended recipient for this 
// message.
UtlString MpResNotificationMsg::getOriginatingResourceName(void) const
{
   return mMsgOriginatorName;
}

// Get the connection ID that this message is associated with.
MpConnectionID MpResNotificationMsg::getConnectionId() const
{
   return mConnectionId;
}

int MpResNotificationMsg::getStreamId() const
{
   return mStreamId;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
