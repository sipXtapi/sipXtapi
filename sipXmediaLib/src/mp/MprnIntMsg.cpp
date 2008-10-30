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
#include "mp/MprnIntMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MprnIntMsg::MprnIntMsg(MpResNotificationMsg::RNMsgType msgType,
                       const UtlString& namedResOriginator,
                       int value,
                       MpConnectionID connId,
                       int streamId)
: MpResNotificationMsg(msgType, namedResOriginator, connId, streamId)
, mValue(value)
{
}

MprnIntMsg::MprnIntMsg(const MprnIntMsg& rMsg)
: MpResNotificationMsg(rMsg)
, mValue(rMsg.mValue)
{
}

OsMsg* MprnIntMsg::createCopy(void) const
{
   return new MprnIntMsg(*this);
}

MprnIntMsg::~MprnIntMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

MprnIntMsg& 
MprnIntMsg::operator=(const MprnIntMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MpResNotificationMsg::operator=(rhs);       // assign fields for parent class

   mValue = rhs.mValue;

   return *this;
}

void MprnIntMsg::setValue(int value)
{
   mValue = value;
}

/* ============================ ACCESSORS ================================= */

int MprnIntMsg::getValue() const
{
   return mValue;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
