//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsContactList.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType OsContactList::TYPE = "OsContactList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsContactList::OsContactList()
   : UtlSList()
{
}


/* ============================ MANIPULATORS ============================== */

/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType OsContactList::getContainableType() const
{
   return OsContactList::TYPE;
}

void OsContactList::setPrimary(const OsContact& contact)
{
    // first, remove the contact if it already exists
    remove(&contact);
    
    // insert the contact at the head of the list
    insertAt(0, (UtlContainable*)&contact);
}

const OsContact* OsContactList::getPrimary()
{
    const OsContact* pContact = (OsContact*) at(0);
    return pContact;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
