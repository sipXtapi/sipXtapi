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
#include "os/shared/OsTimerMessage.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructors
OsTimerMessage::OsTimerMessage(OsTimer* pTimer, OsBSem* pSem)
:  OsMsg(OsMsg::OS_TIMER, ADD),
   mpTimer(pTimer),
   mpSynchSem(pSem)
{
}

OsTimerMessage::OsTimerMessage(int ID, OsBSem* pSem)
:  OsMsg(OsMsg::OS_TIMER, REMOVE),
   mID(ID),
   mpSynchSem(pSem)
{
}

OsTimerMessage::OsTimerMessage(OsBSem* pSem)
:  OsMsg(OsMsg::OS_TIMER, SHUTDOWN),
   mpSynchSem(pSem)
{
}

// Copy constructor
OsTimerMessage::OsTimerMessage(const OsTimerMessage& rOsTimerMessage)
:  OsMsg(rOsTimerMessage)
{
   mID  = rOsTimerMessage.mID;
   mpTimer    = rOsTimerMessage.mpTimer;
   mpSynchSem = rOsTimerMessage.mpSynchSem;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* OsTimerMessage::createCopy(void) const
{
   return new OsTimerMessage(*this);
}

// Destructor
OsTimerMessage::~OsTimerMessage()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsTimerMessage& 
OsTimerMessage::operator=(const OsTimerMessage& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mID  = rhs.mID;
   mpTimer    = rhs.mpTimer;
   mpSynchSem = rhs.mpSynchSem;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Return the size of the message in bytes.
// This is a virtual method so that it will return the accurate size for
// the message object even if that object has been upcast to the type of
// an ancestor class.
int OsTimerMessage::getMsgSize(void) const
{
   return sizeof(*this);
}

// Get the timer ID
int OsTimerMessage::getTimerID(void)
{
   return mID;
}

// Get the OsTimer
OsTimer* OsTimerMessage::getTimer(void)
{
   return mpTimer;
}

// Get the synchronization semaphore
OsBSem* OsTimerMessage::getSynchSem(void)
{
   return mpSynchSem;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


