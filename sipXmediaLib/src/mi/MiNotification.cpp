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
#include "mi/MiNotification.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message notification class used to communicate media notification events. 

/* //////////////////////////// PUBLIC //////////////////////////////////// */

// Connection ID that indicates invalid connection or no connection.
const int MiNotification::INVALID_CONNECTION_ID = -1;

/* ============================ CREATORS ================================== */

// Constructor
MiNotification::MiNotification(NotfType msgType, 
                               const UtlString& sourceId,
                               int connId)
   : OsMsg(OsMsg::MI_NOTF_MSG, msgType)
   , mSourceId(sourceId)
   , mConnectionId(connId)
{
}

// Copy constructor
MiNotification::MiNotification(const MiNotification& rNotf)
   : OsMsg(rNotf)
   , mSourceId(rNotf.mSourceId)
   , mConnectionId(rNotf.mConnectionId)
{
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MiNotification::createCopy(void) const
{
   return new MiNotification(*this);
}

// Destructor
MiNotification::~MiNotification()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MiNotification& 
MiNotification::operator=(const MiNotification& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mSourceId = rhs.mSourceId;
   mConnectionId = rhs.mConnectionId;

   return *this;
}

// Set the unique source identifier.
void MiNotification::setSourceId(const UtlString& sourceId)
{
   mSourceId = sourceId;
}

// Store the ID of the connection that this notification is associated with.
void MiNotification::setConnectionId(int connId)
{
   mConnectionId = connId;
}

/* ============================ ACCESSORS ================================= */

// Returns the type of the notification message
MiNotification::NotfType MiNotification::getType(void) const
{
   return (NotfType)OsMsg::getMsgSubType();
}

// Get the unique source identifier.
UtlString MiNotification::getSourceId(void) const
{
   return mSourceId;
}

// Get the ID of the connection that this notification is associated with.
int MiNotification::getConnectionId() const
{
   return mConnectionId;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
