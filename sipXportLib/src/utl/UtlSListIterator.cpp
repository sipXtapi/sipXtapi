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
#include "utl/UtlSListIterator.h"
#include "utl/UtlSList.h"
#include "utl/UtlContainable.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlSListIterator::UtlSListIterator(const UtlSList& list)
   : UtlListIterator(list)
{
}


/* ============================ MANIPULATORS ============================== */



// Find the next like-instance of the designated object .
UtlContainable* UtlSListIterator::findNext(const UtlContainable* containableToMatch) 
{
   UtlContainable* match = NULL;

   UtlContainer::acquireIteratorConnectionLock();
   OsLock takeContainer(mContainerRefLock);
   UtlSList* myList = dynamic_cast<UtlSList*>(mpMyContainer);
   
   if (myList != NULL) // list still valid?
   {
      OsLock take(myList->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      GList* next = (mpCurrentNode == NULL
                     ? g_list_first(myList->mpList)
                     : g_list_next(mpCurrentNode)
                     );
      while (next != NULL && match == NULL)
      {
          UtlContainable *candidate = (UtlContainable*)next->data;
          if (candidate != NULL && candidate->compareTo(containableToMatch) == 0)
          {
              mpCurrentNode = next;
              match = candidate;
          }
          else
          {
              next = g_list_next(next);
          }
      }
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   
   
   return match;
}


UtlContainable* UtlSListIterator::insertAfterPoint(UtlContainable* insertedObject) 
{
   UtlContainable*    result = NULL;

   OsLock takeContainer(mContainerRefLock);
   UtlSList* myList = dynamic_cast<UtlSList*>(mpMyContainer);

   if (myList)
   {
      OsLock take(myList->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      if (mpCurrentNode == NULL) // at the start of the list
      {
         myList->mpList = g_list_insert(myList->mpList, insertedObject, 0);
      }
      else
      {
         myList->mpList = g_list_insert_before(myList->mpList, mpCurrentNode, insertedObject);
      }

      result = insertedObject; 
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   
   
   return result;
}




/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
