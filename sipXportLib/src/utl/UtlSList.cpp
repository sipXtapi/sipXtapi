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
#include "utl/UtlSListIterator.h"
#include "utl/UtlSList.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlSList::TYPE = "UtlSList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlSList::UtlSList()
   : UtlList()
{
}


/* ============================ MANIPULATORS ============================== */

// Assignment operator


// Append containable object to the end.
UtlContainable* UtlSList::append(UtlContainable* obj) 
{
   OsLock take(mContainerLock);
   
    if(obj != NULL)
    {
       GLIST_SANITY_CHECK;
       mpList = g_list_append(mpList, obj);
       GLIST_SANITY_CHECK;
    }
    
    return(obj);
}


// Insert at the designated position.
UtlContainable* UtlSList::insertAt(size_t N, UtlContainable* obj) 
{
   // :NOTE: this method is deliberately not the same as g_list_insert in that
   //        the glib routine will accept a value of N > the length of the list
   //        but this routine treats that as an error.
   UtlContainable* inserted = NULL;

   OsLock take(mContainerLock);   

   GLIST_SANITY_CHECK;
   if(N <= g_list_length(mpList) && obj != NULL)
   {
      mpList = g_list_insert(mpList, obj, N);
      inserted = obj;
   }
   GLIST_SANITY_CHECK;

   return inserted;
}


UtlContainable* UtlSList::insertAfter(GList* afterNode, UtlContainable* object)
{
   OsLock take(mContainerLock);   

   mpList = g_list_insert_before(mpList, afterNode, object);
   return object;
}


// Inserts at the end postion (tailer).
UtlContainable* UtlSList::insert(UtlContainable* obj)
{
    return append(obj); 
}


// Remove the designated object by equality.
UtlContainable* UtlSList::remove(const UtlContainable* object) 
{
   GList* listNode;
   GList* found;
   UtlContainable* foundObject = NULL;
   
   OsLock take(mContainerLock);
   
   GLIST_SANITY_CHECK;
   for(listNode = g_list_first(mpList), found = NULL; listNode && !found; listNode = g_list_next(listNode))
   {
      UtlContainable* visitNode = (UtlContainable*) listNode->data;
      if(visitNode && visitNode->compareTo(object) == 0)
      {
         found = listNode;
      }
   }

   if (found)
   {
      foundObject = (UtlContainable*)found->data;
      removeLink(found);
   }
   GLIST_SANITY_CHECK;

   return foundObject;
}


// Removes and frees the designated object.
UtlBoolean UtlSList::destroy(UtlContainable* obj) 
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


/* ============================ ACCESSORS ================================= */

// Find the first occurrence of the designated object by equality.
UtlContainable* UtlSList::find(const UtlContainable* containableToMatch) const
{
   GList* listNode;
   UtlContainable* matchElement = NULL;
   UtlContainable* visitNode;

   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   GLIST_SANITY_CHECK;
   for(listNode = g_list_first(mpList);
       listNode &&  matchElement == NULL;
       listNode = g_list_next(listNode)
       )
   {
      visitNode = (UtlContainable*) listNode->data;
      if(visitNode && visitNode->compareTo(containableToMatch) == 0) 
      {
         matchElement = visitNode;
      }
   }
   GLIST_SANITY_CHECK;

   return(matchElement);
}


/* ============================ INQUIRY =================================== */


// Return the number of occurrences of the designated object.
size_t UtlSList::occurrencesOf(const UtlContainable* containableToMatch) const 
{
   int count = 0;
   GList* listNode;
   UtlContainable* visitNode = NULL;

   OsLock take(const_cast<OsBSem&>(mContainerLock));
   
   GLIST_SANITY_CHECK;
   for(listNode = g_list_first(mpList); listNode; listNode = g_list_next(listNode))
   {
      visitNode = (UtlContainable*)listNode->data;
      if(visitNode && visitNode->compareTo(containableToMatch) == 0)
      {
         count++;
      }
   }
   GLIST_SANITY_CHECK;

   return(count);
}


// Return the list position of the designated object.
size_t UtlSList::index(const UtlContainable* containableToMatch) const 
{
    size_t matchedIndex = UTL_NOT_FOUND;
    size_t currentIndex;
    GList* listNode;
    UtlContainable* visitNode = NULL;

    OsLock take(const_cast<OsBSem&>(mContainerLock));
   
    GLIST_SANITY_CHECK;
    for(listNode = g_list_first(mpList), currentIndex = 0;
        matchedIndex == UTL_NOT_FOUND && listNode;
        listNode = g_list_next(listNode)
        )
    {
        visitNode = (UtlContainable*) listNode->data;
        if(visitNode && visitNode->compareTo(containableToMatch) == 0)
        {
           matchedIndex = currentIndex;
        }
        else
        {
           currentIndex++;
        }
    }
    GLIST_SANITY_CHECK;
 
    return matchedIndex;
}


/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType UtlSList::getContainableType() const
{
   return UtlSList::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
