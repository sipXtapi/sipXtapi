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
#include "glib.h"
#include "glib/glist.h"

#include "utl/UtlContainable.h"
#include "utl/UtlSortedList.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlSortedList::TYPE = "UtlSortedList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlSortedList::UtlSortedList()
   : UtlList()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator


// Sorted Insert according to the object comparison function
UtlContainable* UtlSortedList::insert(UtlContainable* obj)
{
   OsLock take(mContainerLock);
   
   mpList = g_list_insert_before(mpList, findNode(g_list_first(mpList), POSITION, obj), obj);
   return obj; 
}

// Remove the designated object by equality
UtlContainable* UtlSortedList::remove(const UtlContainable* obj)
{
   GList*      listNode;
   UtlContainable*  removed = NULL;
    
   OsLock take(mContainerLock);
   
   listNode = findNode(g_list_first(mpList), EXACTLY, obj);
   if (listNode != NULL)
   {
      removed = (UtlContainable*)listNode->data;
      removeLink(listNode);
   }

   return removed;
}

// Remove the object at position index
UtlContainable* UtlSortedList::removeAt(const size_t index)
{
   GList* nthNode;
   UtlContainable* removed = NULL;

   OsLock take(mContainerLock);
   
   nthNode = g_list_nth(mpList, index);
   if (nthNode)
   {
      removed = (UtlContainable*)nthNode->data;
      removeLink(nthNode);
   }

   return removed;
}

/* ============================ ACCESSORS ================================= */

// Find the first occurence of the designated object by equality.
UtlContainable* UtlSortedList::find(const UtlContainable* obj) const 
{
   GList*          listNode;
   UtlContainable* matchNode = NULL;
    
   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   listNode = findNode(g_list_first(mpList), EXACTLY, obj);
   if (listNode != NULL)
   {
      matchNode = (UtlContainable*)listNode->data;
   }

   return matchNode;
}


/* ============================ INQUIRY =================================== */

// Return the list position of the designated object.
size_t UtlSortedList::index(const UtlContainable* obj) const 
{
   size_t          index = UTL_NOT_FOUND;
   size_t          thisIndex;
   GList*          listNode;
   UtlContainable* listElement = NULL;
    
   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   for (listNode = g_list_first(mpList), thisIndex = 0;
        listNode && index == UTL_NOT_FOUND;
        listNode = g_list_next(listNode), thisIndex++)
   {
      listElement = (UtlContainable*)listNode->data;
      if(listElement && listElement->compareTo(obj) == 0)
      {
         index = thisIndex;
      }
   }
    
   return index;
}

// Return the number of occurrences of the designated object.
size_t UtlSortedList::occurrencesOf(const UtlContainable* containableToMatch) const 
{
   int count = 0;
   GList* listNode;
   UtlContainable* visitNode = NULL;
   int             comparison;

   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   for (listNode = g_list_first(mpList), comparison = 0;
        comparison <= 0 && listNode;
        listNode = g_list_next(listNode)
        )
   {
      visitNode = (UtlContainable*)listNode->data;
      if(visitNode && visitNode->compareTo(containableToMatch) == 0)
      {
         count++;
      }
   }

   return(count);
}

/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType UtlSortedList::getContainableType() const
{
   return UtlSortedList::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Return the first GList node to find.
GList* UtlSortedList::findNode(GList* start, MatchType match, const UtlContainable* obj) const
{
   GList*          listNode;
   GList*          foundNode;
   UtlContainable* listElement;
   int             comparison = 0;
   
   // the caller already holds the mContainerLock

   for (listNode = start, foundNode = NULL;
        !foundNode && listNode;
        listNode = g_list_next(listNode)
        )
   {
      listElement = (UtlContainable*)listNode->data;
      if (listElement)
      {
         comparison = listElement->compareTo(obj);
         if ( comparison >= 0 )
         {
            foundNode = listNode;
         }
      }
   }

   if (foundNode && match == EXACTLY && comparison != 0) // match not exact
   {
      foundNode = NULL; 
   }

   return foundNode;
}
