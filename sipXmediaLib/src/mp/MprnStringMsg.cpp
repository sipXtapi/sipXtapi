//
// Copyright (C) 2008-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MprnStringMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MprnStringMsg::MprnStringMsg(MpResNotificationMsg::RNMsgType msgType,
                       const UtlString& namedResOriginator,
                       const UtlString& value,
                       MpConnectionID connId,
                       int streamId)
: MpResNotificationMsg(msgType, namedResOriginator, connId, streamId)
, mValue(value)
{
}

MprnStringMsg::MprnStringMsg(const MprnStringMsg& rMsg)
: MpResNotificationMsg(rMsg)
, mValue(rMsg.mValue)
{
}

OsMsg* MprnStringMsg::createCopy(void) const
{
   return new MprnStringMsg(*this);
}

MprnStringMsg::~MprnStringMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

MprnStringMsg& 
MprnStringMsg::operator=(const MprnStringMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MpResNotificationMsg::operator=(rhs);       // assign fields for parent class

   mValue = rhs.mValue;

   return *this;
}

void MprnStringMsg::setValue(const UtlString& value)
{
   mValue = value;
}

/* ============================ ACCESSORS ================================= */

void MprnStringMsg::getValue(UtlString& value) const
{
   value = mValue;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
