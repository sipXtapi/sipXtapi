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
#include "os/OsIntPtrMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType OsIntPtrMsg::TYPE = "OsIntPtrMsg" ;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsIntPtrMsg::OsIntPtrMsg(const unsigned char msgType, 
                   const unsigned char msgSubType, 
                   intptr_t pData1, 
                   intptr_t pData2)
                   : OsMsg(msgType, msgSubType),
                   mpData1(pData1),
                   mpData2(pData2)
{
   // all of the required work is done by the initializers
}

// Copy constructor
OsIntPtrMsg::OsIntPtrMsg(const OsIntPtrMsg& rOsMsg) :
      OsMsg(rOsMsg),
      mpData1(rOsMsg.mpData1),
      mpData2(rOsMsg.mpData2)
{
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* OsIntPtrMsg::createCopy(void) const
{
   return new OsIntPtrMsg(*this);
}


/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsIntPtrMsg& OsIntPtrMsg::operator=(const OsIntPtrMsg& rhs)
{
   if (this != &rhs)            // handle the assignment to self case
   {
      OsMsg::operator=(rhs);
      mpData1 = rhs.mpData1;
      mpData2 = rhs.mpData2;
   }

   return *this;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */


