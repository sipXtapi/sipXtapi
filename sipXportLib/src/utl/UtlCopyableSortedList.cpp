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
#include "utl/UtlCopyableSortedList.h"
#include "utl/UtlSortedListIterator.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlCopyableSortedList::TYPE = "UtlCopyableSortedList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlCopyableSortedList::UtlCopyableSortedList() 
    : UtlSortedList()
{
}


// Copy constructor
UtlCopyableSortedList::UtlCopyableSortedList(const UtlCopyableSortedList& rhs)
{
   operator=(rhs);  
}

// Destructor
UtlCopyableSortedList::~UtlCopyableSortedList()
{
   destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
UtlCopyableSortedList& UtlCopyableSortedList::operator=(const UtlCopyableSortedList& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   destroyAll();
   UtlSortedListIterator it(rhs);
   UtlCopyableContainable* item;
   while(item = dynamic_cast<UtlCopyableContainable*>(it()))
   {
      insert(item->clone());
   }

   return *this;
}


UtlContainable* UtlCopyableSortedList::insert(UtlCopyableContainable* obj)
{
   return UtlSortedList::insert(obj);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType UtlCopyableSortedList::getContainableType() const
{
   return UtlCopyableSortedList::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

