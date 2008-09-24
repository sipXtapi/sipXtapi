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
#include "os/OsPtrMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType OsPtrMsg::TYPE = "OsPtrMsg" ;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsPtrMsg::OsPtrMsg(const unsigned char msgType, 
                   const unsigned char msgSubType, 
                   void* pData, 
                   void* pData2)
: OsMsg(msgType, msgSubType),
  mpData(pData),
  mpData2(pData2)
{
   // all of the required work is done by the initializers
}

// Copy constructor
OsPtrMsg::OsPtrMsg(const OsPtrMsg& rOsMsg) :
    OsMsg(rOsMsg),
    mpData(rOsMsg.mpData),
    mpData2(rOsMsg.mpData2)
{
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* OsPtrMsg::createCopy(void) const
{
   return new OsPtrMsg(*this);
}


/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsPtrMsg& 
OsPtrMsg::operator=(const OsPtrMsg& rhs)
{
   if (this != &rhs)            // handle the assignment to self case
   {
      OsMsg::operator=(rhs);
      mpData = rhs.mpData;
      mpData2 = rhs.mpData2;
   }

   return *this;
}


/* ============================ ACCESSORS ================================= */
void* OsPtrMsg::getPtr()
{
    return mpData;
}

void* OsPtrMsg::getPtr2()
{
    return mpData2;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */


