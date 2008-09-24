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
#include "os/OsSysLogMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsSysLogMsg::OsSysLogMsg(const unsigned char msgSubType, const void* pData)
   : OsMsg(OS_SYSLOG, msgSubType)
{
   mpData = pData ;
   // all of the required work is done by the initializers
}

// Copy constructor
OsSysLogMsg::OsSysLogMsg(const OsSysLogMsg& rOsSysLogMsg)
   : OsMsg(rOsSysLogMsg) 
{
   mpData = rOsSysLogMsg.mpData ;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* OsSysLogMsg::createCopy(void) const
{
   return new OsSysLogMsg(*this);
}


// Destructor
OsSysLogMsg::~OsSysLogMsg()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsSysLogMsg& OsSysLogMsg::operator=(const OsSysLogMsg& rhs)  
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);

   mpData = rhs.mpData ;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Get the message data
const void* OsSysLogMsg::getData() const 
{
   return mpData;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


