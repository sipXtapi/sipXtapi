// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifdef MP_STREAMING

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpPlayerEvent.h"
#include "mp/MpPlayer.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor requiring a player, user data, and state
MpPlayerEvent::MpPlayerEvent()
{
   mpPlayer = NULL ;
   mpUserData = NULL ;
   mState = PlayerUnrealized ;
}


// Constructor
MpPlayerEvent::MpPlayerEvent(MpPlayer* pPlayer, 
                             void* pUserData, 
                             PlayerState state)
{
   mpPlayer = pPlayer ;
   mpUserData = pUserData ;
   mState = state ;
}

// Copy constructor
MpPlayerEvent::MpPlayerEvent(const MpPlayerEvent& rMpPlayerEvent)
{
   mpPlayer = rMpPlayerEvent.mpPlayer ;
   mpUserData = rMpPlayerEvent.mpUserData ;
   mState = rMpPlayerEvent.mState ;
}

// Destructor
MpPlayerEvent::~MpPlayerEvent()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MpPlayerEvent& 
MpPlayerEvent::operator=(const MpPlayerEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   mpPlayer = rhs.mpPlayer ;
   mpUserData = rhs.mpUserData ;
   mState = rhs.mState ;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Gets the player responsible for firing this event
MpPlayer* MpPlayerEvent::getPlayer() const
{
   return mpPlayer ;
}
     

// Gets the user supplied data supplied when adding the listener
void* MpPlayerEvent::getUserData() const
{
   return mpUserData ;
}


// Gets the state of player snapshotted when the event was fired.
PlayerState MpPlayerEvent::getState() const
{
   return mState ;
}
     
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

#endif