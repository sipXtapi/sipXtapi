//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlContainer.h"
#include "utl/UtlIterator.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlContainer::TYPE = "UtlContainer" ;

// STATIC VARIABLE INITIALIZATIONS
OsBSem UtlContainer::sIteratorConnectionLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

UtlContainer::UtlContainer()
   : mContainerLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
     mpIteratorList(NULL)
{
   if (!g_thread_supported())
   {
      g_thread_init(NULL);
   }
   
}


// Destructor
UtlContainer::~UtlContainer() 
{
}

// invalidateIterators() isn't called by the UtlContainer destructor
// (and need not be, since no iterators are defined for UtlContainer),
// but it is used by the methods for various subclasses.
void UtlContainer::invalidateIterators()
{
   GList*          listNode = NULL;
   UtlIterator*    foundIterator;

   // The caller is holding the sIteratorConnectionLock and mContainerLock.
       
   // Walk the list to notify the iterators.
   for (listNode = g_list_first(mpIteratorList);
        listNode != NULL;
        listNode = g_list_next(listNode)
        )
   {
      foundIterator = (UtlIterator*)listNode->data;
      foundIterator->invalidate();
   }

   mpIteratorList = NULL;
}

/* ============================ MANIPULATORS ============================== */




/* ============================ ACCESSORS ================================= */

// These are here because every UtlContainer is also a UtlContainable

/**
 * Calculate a unique hash code for this object.  If the equals
 * operator returns true between two objects, then both objects
 * must have the same hash code.
 */
unsigned UtlContainer::hash() const
{
   return g_direct_hash(this);
}


/**
 * Get the ContainableType for a UtlContainable derived class.
 */
UtlContainableType UtlContainer::getContainableType() const
{
   return UtlContainer::TYPE;
}

/* ============================ INQUIRY =================================== */


/**
 * Compare the this object to another like object.  Results for 
 * comparing with a non-like object are undefined.
 *
 * @returns 0 if equal, <0 if less than and >0 if greater.
 */
int UtlContainer::compareTo(const UtlContainable* otherObject) const
{
   return this == otherObject ? 0 : 1;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/// Lock the linkage between containers and iterators
void UtlContainer::acquireIteratorConnectionLock()
{
   sIteratorConnectionLock.acquire();
}

/// Unlock the linkage between containers and iterators
void UtlContainer::releaseIteratorConnectionLock()
{
   sIteratorConnectionLock.release();
}

void UtlContainer::addIterator(UtlIterator* newIterator) const
{
   // Caller must be holding this->mContainerLock.
   // But it need not be holding newIterator->mpContainerRefLock, because
   // we do not set newIterator->mpMyContainer.

   // This method is declared const because it makes no change that
   // any other method can detect in the container, but it actually
   // does make a change, so we have to cast away the const.
   UtlContainer* my = const_cast<UtlContainer*>(this);
   
    if(newIterator)
    {
       my->mpIteratorList = g_list_append(my->mpIteratorList, newIterator);
    }
}


void UtlContainer::removeIterator(UtlIterator *existingIterator) const
{
   // Caller must be holding this->mContainerLock.
   // But it need not be holding newIterator->mpContainerRefLock, because
   // we do not set newIterator->mpMyContainer.

   // This method is declared const because it makes no change that
   // any other method can detect in the container, but it actually
   // does make a change, so we have to cast away the const.
   UtlContainer *my = (UtlContainer*)this;
   
   if(existingIterator)
   {
      my->mpIteratorList = g_list_remove(my->mpIteratorList, existingIterator);
   }
}




/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
