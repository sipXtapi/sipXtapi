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
#include "os/OsMsgDispatcher.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsMsgDispatcher::OsMsgDispatcher()
   : OsMsgQ()
   , mMsgsLost(FALSE)
{}

/* ============================ MANIPULATORS ============================== */

OsStatus OsMsgDispatcher::post(const OsMsg& msg)
{
   if (numMsgs() == maxMsgs())
   {
      setMsgsLost();
      return OS_LIMIT_REACHED;
   }
   else
   {
      // Send the message, give it 1 millisecond to send.
      OsMsgQ::send(msg, OsTime(1));
      return OS_SUCCESS;
   }
};

OsStatus OsMsgDispatcher::receive(OsMsg*& rpMsg,
                                  const OsTime& rTimeout)
{
   return OsMsgQ::receive(rpMsg, rTimeout);
};

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


