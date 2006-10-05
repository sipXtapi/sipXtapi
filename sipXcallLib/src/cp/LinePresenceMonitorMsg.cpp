//
//
// Copyright 2005 Pingtel Corp.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <cp/LinePresenceBase.h>
#include <cp/LinePresenceMonitorMsg.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        LinePresenceMonitorMsg::LinePresenceMonitorMsg
//
//  SYNOPSIS:    
//
//  DESCRIPTION: Constructor for UPDATE_STATE message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LinePresenceMonitorMsg::LinePresenceMonitorMsg(eLinePresenceMonitorMsgSubTypes type, LinePresenceBase* line)
: OsMsg(USER_START, type)
{   
   mLine = line;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        LinePresenceMonitorMsg::LinePresenceMonitorMsg
//
//  SYNOPSIS:    
//
//  DESCRIPTION: Copy constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LinePresenceMonitorMsg::LinePresenceMonitorMsg(const LinePresenceMonitorMsg& rLinePresenceMonitorMsg)
: OsMsg(rLinePresenceMonitorMsg)
{
   mLine = rLinePresenceMonitorMsg.mLine;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        LinePresenceMonitorMsg::createCopy
//
//  SYNOPSIS:    
//
//  DESCRIPTION: Create a copy of this msg object (which may be of a derived type)
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

OsMsg* LinePresenceMonitorMsg::createCopy(void) const
{
   return new LinePresenceMonitorMsg(*this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        LinePresenceMonitorMsg::~LinePresenceMonitorMsg
//
//  SYNOPSIS:    
//
//  DESCRIPTION: Destructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LinePresenceMonitorMsg::~LinePresenceMonitorMsg()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

