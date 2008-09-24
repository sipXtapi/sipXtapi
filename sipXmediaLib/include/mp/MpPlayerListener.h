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

#ifndef _MpPlayerListener_h_
#define _MpPlayerListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpPlayerEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpPlayer; 


//:Listener interface for the MpPlayer object. 
class MpPlayerListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   virtual void playerRealized(MpPlayerEvent& event) = 0 ;
     //: The player has been realized

   virtual void playerPrefetched(MpPlayerEvent& event) = 0 ;
     //: The player's data source has been prefetched

   virtual void playerPlaying(MpPlayerEvent& event) = 0 ;
     //: The player has begun playing.

   virtual void playerPaused(MpPlayerEvent& event) = 0 ;
     //: The player has been paused

   virtual void playerStopped(MpPlayerEvent& event) = 0 ;
     //: The player has been stopped

   virtual void playerFailed(MpPlayerEvent& event) = 0 ;
     //: The player has failed

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpPlayerListener_h_

#endif
