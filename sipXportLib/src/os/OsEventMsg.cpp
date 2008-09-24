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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
//  
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsEventMsg.h"
#include "os/OsQueuedEvent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsEventMsg::OsEventMsg(const unsigned char subType,
                       const OsQueuedEvent& rEvent,
                       const int eventData,
                       const OsTime& rTimestamp)
:  OsMsg(OsMsg::OS_EVENT, subType),
   mEventData(eventData),
   mTimestamp(rTimestamp)
{
   OsStatus res;

   intptr_t temp ;
   res = rEvent.getUserData(temp);
   mUserData = (int) temp ;
   assert(res == OS_SUCCESS);
}

// Copy constructor
OsEventMsg::OsEventMsg(const OsEventMsg& rOsEventMsg)
:  OsMsg(rOsEventMsg)
{
   mEventData = rOsEventMsg.mEventData;
   mUserData  = rOsEventMsg.mUserData;
   mTimestamp = rOsEventMsg.mTimestamp;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* OsEventMsg::createCopy(void) const
{
   return new OsEventMsg(*this);
}

// Destructor
OsEventMsg::~OsEventMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsEventMsg& 
OsEventMsg::operator=(const OsEventMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mEventData = rhs.mEventData;
   mUserData  = rhs.mUserData;
   mTimestamp = rhs.mTimestamp;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Return the size of the message in bytes.
// This is a virtual method so that it will return the accurate size for
// the message object even if that object has been upcast to the type of
// an ancestor class.
int OsEventMsg::getMsgSize(void) const
{
   return sizeof(*this);
}

// Return the event data that was signaled by the notifier task.
// Always returns OS_SUCCESS.
OsStatus OsEventMsg::getEventData(int& rEventData) const
{
   rEventData = mEventData;
   return OS_SUCCESS;
}

// Return the timestamp associated with this event.
// Always returns OS_SUCCESS.
OsStatus OsEventMsg::getTimestamp(OsTime& rTimestamp) const
{
   rTimestamp = mTimestamp;
   return OS_SUCCESS;
}

// Return the user data specified when the OsQueuedEvent was constructed.
// Always returns OS_SUCCESS.
OsStatus OsEventMsg::getUserData(int& rUserData) const
{
   rUserData = mUserData;
   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


