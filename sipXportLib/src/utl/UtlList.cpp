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
#include "utl/UtlListIterator.h"
#include "utl/UtlList.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlList::TYPE = "UtlList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlList::UtlList()
   : mpList(NULL)
{
}


// Destructor
UtlList::~UtlList()
{
   UtlContainer::acquireIteratorConnectionLock();
   
   OsLock take(mContainerLock);
      
   GLIST_SANITY_CHECK;

   invalidateIterators();

   UtlContainer::releaseIteratorConnectionLock();

   GList* node;
   while((node = g_list_first(mpList)))
   {
      removeLink(node);
   }
}

/* ============================ MANIPULATORS ============================== */


// Removes and returns the first item in the list (pop).
UtlContainable* UtlList::get() 
{
   OsLock take(mContainerLock);
   
   GLIST_SANITY_CHECK;
   UtlContainable* firstElement = NULL;

   GList* firstNode = g_list_first(mpList);
    
   if(firstNode)
   {
      firstElement = (UtlContainable*) firstNode->data;
      removeLink(firstNode);
   }
    
   return(firstElement);
}



// Removed the designated object by reference.
UtlContainable* UtlList::removeReference(const UtlContainable* containableToMatch) 
{
   UtlContainable* foundElement = NULL;

   GList* foundNode = NULL;
   GList* listNode;

   OsLock take(mContainerLock);
   
   GLIST_SANITY_CHECK;

   for(listNode = g_list_first(mpList); listNode && !foundElement; listNode = g_list_next(listNode))
   {
      if((UtlContainable*) listNode->data == containableToMatch)
      {
         foundNode = listNode;
         foundElement = (UtlContainable*) listNode->data;
      }
   }
    
   if (foundNode)
   {
      removeLink(foundNode);
   }
    
   return(foundElement);
}


void UtlList::removeLink(GList* toBeRemoved)
{
   // The caller already holds the mContainerLock.

   GList*            listNode = NULL;
   UtlListIterator* foundIterator;
   
   for (listNode = g_list_first(mpIteratorList); listNode; listNode = g_list_next(listNode))
   {
      foundIterator = (UtlListIterator*)listNode->data;
      foundIterator->removing(toBeRemoved);
   }

   mpList = g_list_delete_link(mpList, toBeRemoved);
}


// Removes and frees the designated objects.
UtlBoolean UtlList::destroy(UtlContainable* obj) 
{
   UtlBoolean result = FALSE;

   // this does not take the mContainerLock, because all the container state changes
   // are made inside the remove method, which already takes it.

   UtlContainable* removed = remove(obj); 
        
    if (removed)
    {
        result = TRUE;
        delete removed; 
    }
     
    return result; 
}


// Removes and delete all elements
void UtlList::destroyAll()
{
   GList* node;

   OsLock take(mContainerLock);
   
   GLIST_SANITY_CHECK;
   while((node = g_list_first(mpList)))
   {
      UtlContainable* theObject = (UtlContainable*)node->data;
      removeLink(node);
      if(theObject)
      {
         delete theObject;
      }
   }
}


// Remove all elements, but do not free the objects
void UtlList::removeAll()
{
   GList* node;

   OsLock take(mContainerLock);
   
   GLIST_SANITY_CHECK;
   while((node = g_list_first(mpList)))
   {
      removeLink(node);
   }
}

/* ============================ ACCESSORS ================================= */

// Return the first element (head) of the list.
UtlContainable* UtlList::first() const 
{
   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   GList* firstNode = g_list_first(mpList);
   
   return firstNode ? (UtlContainable*) firstNode->data : NULL;
}


// Return the last element (tail) of the list.
UtlContainable* UtlList::last() const 
{
   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   GList* lastNode = g_list_last(mpList);

   return lastNode ? (UtlContainable*) lastNode->data : NULL;
}


// Return the element at position N.
UtlContainable* UtlList::at(size_t N) const 
{
   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   return ((UtlContainable*) g_list_nth_data(mpList, N));
}

/* ============================ INQUIRY =================================== */

// Return the total number.
size_t UtlList::entries() const
{
   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   return g_list_length(mpList);
}


// Return true of the container is empty.
UtlBoolean UtlList::isEmpty() const 
{
   return entries() == 0; 
}


// Return true if the container includes the designated object.
UtlBoolean UtlList::contains(const UtlContainable* object) const 
{
   return(find(object) != NULL);
}


// Return true if the list contains the designated object reference .
UtlBoolean UtlList::containsReference(const UtlContainable* containableToMatch) const 
{
   GList* listNode;
   UtlBoolean isMatch = FALSE;

   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   for(listNode = g_list_first(mpList); listNode && !isMatch; listNode = g_list_next(listNode))
   {
      if((UtlContainable*)listNode->data == containableToMatch)
      {
         isMatch = TRUE;
      }
   }

   return isMatch;
}

/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType UtlList::getContainableType() const
{
   return UtlList::TYPE;
}

#ifdef GLIST_SANITY_TEST
bool UtlList::sanityCheck() const
{
   GList* thisNode;
   GList* prevNode;
   
   // The caller already holds the mContainerLock.

   for ( ( prevNode=NULL, thisNode=g_list_first(mpList) );
         thisNode;
         ( prevNode=thisNode, thisNode=g_list_next(thisNode) )
        )
   {
      if (g_list_previous(thisNode) != prevNode)
      {
         return FALSE;
      }
   }
   return TRUE;
}
#endif /* GLIST_SANITY_TEST */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

