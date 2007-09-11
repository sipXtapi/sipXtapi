//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MprnDTMFMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message notification object used to communicate DTMF signaling from 
// resources outward towards the flowgraph, and up through to users above
// mediaLib and beyond.
// If one creates a KEY_DOWN notification, duration should not be 
// supplied, as it isn't useful until a KEY_UP event happens.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

const int32_t MprnDTMFMsg::DURATION_NOT_APPLICABLE = -1;

/* ============================ CREATORS ================================== */

// Constructor
MprnDTMFMsg::MprnDTMFMsg(const UtlString& namedResOriginator, MpConnectionID connId,
                         KeyCode key, KeyPressState pressState, int32_t duration)
: MpResNotificationMsg(MPRNM_DTMF_RECEIVED, namedResOriginator, connId)
, mKey(key)
, mPressState(pressState)
, mDuration(duration)
{
   if(key == KEY_DOWN)
   {
      assert(mDuration == DURATION_NOT_APPLICABLE);
   }
}

// Copy constructor
MprnDTMFMsg::MprnDTMFMsg(const MprnDTMFMsg& rMsg)
: MpResNotificationMsg(rMsg)
, mKey(rMsg.mKey)
, mPressState(rMsg.mPressState)
, mDuration(rMsg.mDuration)
{
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MprnDTMFMsg::createCopy(void) const
{
   return new MprnDTMFMsg(*this);
}

// Destructor
MprnDTMFMsg::~MprnDTMFMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MprnDTMFMsg& 
MprnDTMFMsg::operator=(const MprnDTMFMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MpResNotificationMsg::operator=(rhs);       // assign fields for parent class

   mKey = rhs.mKey;
   mPressState = rhs.mPressState;
   mDuration = rhs.mDuration;

   return *this;
}

// Set the specific DTMF value that this event represents.
void MprnDTMFMsg::setKeyCode(KeyCode key)
{
   mKey = key;
}

// Set the key press state for this DTMF message -- down or up.
void MprnDTMFMsg::setKeyPressState(KeyPressState pressState)
{
   mPressState = pressState;
}

// Set the duration of this DTMF event.
void MprnDTMFMsg::setDuration(int32_t duration)
{
   mDuration = duration;
}

/* ============================ ACCESSORS ================================= */

// Get the specific DTMF value that this event represents.
MprnDTMFMsg::KeyCode MprnDTMFMsg::getKeyCode() const
{
   return mKey;
}

// Get the key press state for this DTMF message -- down or up.
MprnDTMFMsg::KeyPressState MprnDTMFMsg::getKeyPressState() const
{
   return mPressState;
}

// Get the duration of this DTMF event.
int32_t MprnDTMFMsg::getDuration() const
{
   return mDuration;
}


/* ============================ INQUIRY =================================== */

UtlBoolean MprnDTMFMsg::isPressed() const
{
   return mPressState == KEY_DOWN;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
