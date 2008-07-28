// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
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


void OsContactList::destroyAllAfter(int index) 
{
    OsContact* pContact = NULL ;

    while (pContact = (OsContact*) removeAt(index+1))
    {
        delete pContact ;
    }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
