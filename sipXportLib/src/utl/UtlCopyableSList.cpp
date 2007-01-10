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
#include "utl/UtlCopyableSList.h"
#include "utl/UtlSListIterator.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlCopyableSList::TYPE = "UtlCopyableSList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlCopyableSList::UtlCopyableSList() 
    : UtlSList()
{
}


// Copy constructor
UtlCopyableSList::UtlCopyableSList(const UtlCopyableSList& rhs)
{
   operator=(rhs);  
}

// Destructor
UtlCopyableSList::~UtlCopyableSList()
{
   destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
UtlCopyableSList& UtlCopyableSList::operator=(const UtlCopyableSList& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   destroyAll();
   UtlSListIterator it(rhs);
   UtlCopyableContainable* item;
   while(item = dynamic_cast<UtlCopyableContainable*>(it()))
   {
      insert(item->clone());
   }

   return *this;
}


UtlContainable* UtlCopyableSList::append(UtlCopyableContainable* obj)
{
   return UtlSList::append(obj);
}

UtlContainable* UtlCopyableSList::insertAt(size_t N,           ///< zero-based position obj should be
                                           UtlCopyableContainable* obj ///< object to insert at N
                                           )
{
   return UtlSList::insertAt(N, obj);
}

UtlContainable* UtlCopyableSList::insert(UtlCopyableContainable* obj)
{
   return UtlSList::insert(obj);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType UtlCopyableSList::getContainableType() const
{
   return UtlCopyableSList::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

