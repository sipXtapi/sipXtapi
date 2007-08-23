//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpProgressResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message object used to communicate with the media processing task

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpProgressResourceMsg::MpProgressResourceMsg(MpResourceMsgType msgType,
                                         const UtlString& msgDestName, 
                                         int32_t updatePeriodMS)
: MpResourceMsg(msgType, msgDestName)
{
   // Only set mPeriodMS if updatePeriodMS passed in is positive.
   assert(updatePeriodMS > 0);
   if(updatePeriodMS > 0)
   {
      mPeriodMS = updatePeriodMS;
   }
};

// Copy constructor
MpProgressResourceMsg::MpProgressResourceMsg(const MpProgressResourceMsg& rMsg)
: MpResourceMsg(rMsg)
, mPeriodMS(rMsg.mPeriodMS)
{}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MpProgressResourceMsg::createCopy(void) const 
{
   return new MpProgressResourceMsg(*this); 
}

// Destructor
MpProgressResourceMsg::~MpProgressResourceMsg() { /* No work required */ };


/* ============================ MANIPULATORS ============================== */

// Assignment operator
MpProgressResourceMsg& MpProgressResourceMsg::operator=(const MpProgressResourceMsg& rhs)
{
   if (this == &rhs) 
      return *this;  // handle the assignment to self case

   MpResourceMsg::operator=(rhs);  // assign fields for parent class
   mPeriodMS = rhs.mPeriodMS;
   return *this;
}

// Set the tone that is associated with this resource.
OsStatus MpProgressResourceMsg::setUpdatePeriodMS(int32_t updatePeriodMS) 
{
   OsStatus stat = OS_FAILED;

   assert(updatePeriodMS > 0);
   if(updatePeriodMS > 0)
   {
      mPeriodMS = updatePeriodMS;
      stat = OS_SUCCESS;
   }
   return stat;
}

/* ============================ ACCESSORS ================================= */

// Get the tone that is associated with this resource.
int32_t MpProgressResourceMsg::getUpdatePeriodMS(void) const 
{ 
   return mPeriodMS; 
}

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
