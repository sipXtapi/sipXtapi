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
#include "mi/MiDtmfNotf.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message notification class used to communicate DTMF signaling.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

const int32_t MiDtmfNotf::DURATION_NOT_APPLICABLE = -1;

/* ============================ CREATORS ================================== */

// Constructor
MiDtmfNotf::MiDtmfNotf(const UtlString& sourceId, int connId, KeyCode key, 
                       KeyPressState pressState, int32_t duration)
: MiNotification(MI_NOTF_DTMF_RECEIVED, sourceId, connId)
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
MiDtmfNotf::MiDtmfNotf(const MiDtmfNotf& rMsg)
: MiNotification(rMsg)
, mKey(rMsg.mKey)
, mPressState(rMsg.mPressState)
, mDuration(rMsg.mDuration)
{
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MiDtmfNotf::createCopy(void) const
{
   return new MiDtmfNotf(*this);
}

// Destructor
MiDtmfNotf::~MiDtmfNotf()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MiDtmfNotf& 
MiDtmfNotf::operator=(const MiDtmfNotf& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MiNotification::operator=(rhs);       // assign fields for parent class

   mKey = rhs.mKey;
   mPressState = rhs.mPressState;
   mDuration = rhs.mDuration;

   return *this;
}

// Set the specific DTMF value that this notification represents.
void MiDtmfNotf::setKeyCode(KeyCode key)
{
   mKey = key;
}

// Set the key press state for this DTMF notification -- down or up.
void MiDtmfNotf::setKeyPressState(KeyPressState pressState)
{
   mPressState = pressState;
}

// Set the duration of this DTMF notification.
void MiDtmfNotf::setDuration(int32_t duration)
{
   mDuration = duration;
}

/* ============================ ACCESSORS ================================= */

// Get the specific DTMF value that this notification represents.
MiDtmfNotf::KeyCode MiDtmfNotf::getKeyCode() const
{
   return mKey;
}

// Get the key press state for this DTMF notification -- down or up.
MiDtmfNotf::KeyPressState MiDtmfNotf::getKeyPressState() const
{
   return mPressState;
}

// Get the duration of this DTMF notification.
int32_t MiDtmfNotf::getDuration() const
{
   return mDuration;
}


/* ============================ INQUIRY =================================== */

UtlBoolean MiDtmfNotf::isPressed() const
{
   return mPressState == KEY_DOWN;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
