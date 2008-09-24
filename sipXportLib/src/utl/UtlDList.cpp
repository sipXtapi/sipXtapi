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
// APPLICATION INCLUDES
#include "utl/UtlDList.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlDList::TYPE = "UtlDList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlDList::UtlDList() 
    : UtlSList()
{
}


// Copy constructor


// Destructor
UtlDList::~UtlDList()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType UtlDList::getContainableType() const
{
   return UtlDList::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

