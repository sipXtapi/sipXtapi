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
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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

#ifndef _MpPlayerEvent_h_
#define _MpPlayerEvent_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/StreamDefs.h"
#include "mp/MpPlayer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpPlayer ;

//:Event encapsulating a change of player state.
class MpPlayerEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
   MpPlayerEvent() ;
     //:Default Constructor

   MpPlayerEvent(MpPlayer* pPlayer, 
                 void* pUserData, 
                 PlayerState state) ;
     //:Constructor requiring a player, user data, and state

   MpPlayerEvent(const MpPlayerEvent& rMpPlayerEvent);
     //:Copy constructor

   virtual
   ~MpPlayerEvent();
     //:Destructor

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   MpPlayerEvent& operator=(const MpPlayerEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */
   MpPlayer* getPlayer() const ;
     //: Gets the player responsible for firing this event

   void* getUserData() const ;
     //: Gets the user supplied data supplied when adding the listener

   PlayerState getState() const ;
     //: Gets the state of player snapshotted when the event was fired.

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   MpPlayer* mpPlayer ;    // Reference to the Stream Player
   void* mpUserData ;      // Reference to the user data
   PlayerState mState ;    // State snapshot when fired
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpPlayerEvent_h_

#endif
