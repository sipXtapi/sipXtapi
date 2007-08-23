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
// APPLICATION INCLUDES
#include "mp/MprnProgressMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message notification object used to communicate millisecond-based progress 
// updates from resources outward towards the flowgraph, and up through to 
// users above mediaLib and beyond.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprnProgressMsg::MprnProgressMsg(MpResNotificationMsg::RNMsgType msgType,
                                 const UtlString& namedResOriginator,
                                 unsigned posMS, unsigned totalMS)
: MpResNotificationMsg(msgType, namedResOriginator)
, mCurPositionMS(posMS)
, mTotalMS(totalMS)
{
}

// Copy constructor
MprnProgressMsg::MprnProgressMsg(const MprnProgressMsg& rMsg)
: MpResNotificationMsg(rMsg)
, mCurPositionMS(rMsg.mCurPositionMS)
, mTotalMS(rMsg.mTotalMS)
{
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MprnProgressMsg::createCopy(void) const
{
   return new MprnProgressMsg(*this);
}

// Destructor
MprnProgressMsg::~MprnProgressMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MprnProgressMsg& 
MprnProgressMsg::operator=(const MprnProgressMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MpResNotificationMsg::operator=(rhs);       // assign fields for parent class

   mCurPositionMS = rhs.mCurPositionMS;
   mTotalMS = rhs.mTotalMS;

   return *this;
}

// Set the position in milliseconds this message reports.
void MprnProgressMsg::setPositionMS(unsigned posMS)
{
   mCurPositionMS = posMS;
}

// Set the total size of the buffer in milliseconds this message reports.
void MprnProgressMsg::setTotalMS(unsigned totalMS)
{
   mTotalMS = totalMS;
}

/* ============================ ACCESSORS ================================= */

/// Get the current reported position of this progress update in MS.
unsigned MprnProgressMsg::getPositionMS(void) const
{
   return mCurPositionMS;
}

/// Get the total size of the buffer in milliseconds this message reports.
unsigned MprnProgressMsg::getTotalMS(void) const
{
   return mTotalMS;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
