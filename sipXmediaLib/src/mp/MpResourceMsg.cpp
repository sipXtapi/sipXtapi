//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES

#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message object targeting specific resources used to communicate with the media processing task

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpResourceMsg::MpResourceMsg(MpResourceMsgType messageSubtype, const UtlString& msgDestName)
   : OsMsg(OsMsg::MP_RESOURCE_MSG, messageSubtype)
   , mMsgDestName(msgDestName)
{
}

// Copy constructor
MpResourceMsg::MpResourceMsg(const MpResourceMsg& rMpResourceMsg)
   : OsMsg(rMpResourceMsg)
   , mMsgDestName(rMpResourceMsg.mMsgDestName)
{
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MpResourceMsg::createCopy(void) const
{
   return new MpResourceMsg(*this);
}

// Destructor
MpResourceMsg::~MpResourceMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MpResourceMsg& 
MpResourceMsg::operator=(const MpResourceMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mMsgDestName = rhs.mMsgDestName;

   return *this;
}

// Sets the name of the intended recipient for this message.
void MpResourceMsg::setDestResourceName(const UtlString& msgDestName)
{
   mMsgDestName = msgDestName;
}

/* ============================ ACCESSORS ================================= */

// Return the type of the media resource message
int MpResourceMsg::getMsg(void) const
{
   return OsMsg::getMsgSubType();
}

// Returns the resource name that is the intended recipient for this 
// message.
UtlString MpResourceMsg::getDestResourceName(void) const
{
   return mMsgDestName;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

