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
                               int connId,
                               int streamId)
: OsMsg(OsMsg::MI_NOTF_MSG, msgType)
, mSourceId(sourceId)
, mConnectionId(connId)
, mStreamId(streamId)
{
}

// Copy constructor
MiNotification::MiNotification(const MiNotification& rNotf)
: OsMsg(rNotf)
, mSourceId(rNotf.mSourceId)
, mConnectionId(rNotf.mConnectionId)
, mStreamId(rNotf.mStreamId)
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

void MiNotification::setStreamId(int streamId)
{
   mStreamId = streamId;
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

int MiNotification::getStreamId() const
{
   return mStreamId;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
