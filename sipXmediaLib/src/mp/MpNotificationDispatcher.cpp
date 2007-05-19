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
#include <mp/MpNotificationDispatcher.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

OsStatus MpNotificationDispatcher::postNotification(const MpResourceNotificationMsg& msg)
{
   if (numMsgs() == maxMsgs())
   {
      setNotificationsLost();
      return OS_LIMIT_REACHED;
   }
   else
   {
      // Send the message, give it 1 millisecond to send.
      send(msg, OsTime(1));
      return OS_SUCCESS;
   }
};

OsStatus MpNotificationDispatcher::receive(MpResourceNotificationMsg*& rpMsg,
                                           const OsTime& rTimeout)
{
   return receive(rpMsg, rTimeout);
};

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

