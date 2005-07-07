//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "utl/UtlHashBagIterator.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const size_t UtlHashBagIterator::BEFORE_FIRST = (size_t)(-1);

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlHashBagIterator::UtlHashBagIterator(UtlHashBag& hashBag, UtlContainable* key)   
   : UtlIterator(hashBag),
     mpSubsetMatch(key)
{
   OsLock container(const_cast<OsBSem&>(hashBag.mContainerLock));

   addToContainer(mpMyContainer);

   init(&hashBag);
}


// Destructor
UtlHashBagIterator::~UtlHashBagIterator()
{
   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerRefLock);
   UtlHashBag* myHashBag = dynamic_cast<UtlHashBag*>(mpMyContainer);
   if (myHashBag)
   {
      OsLock container(myHashBag->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      myHashBag->removeIterator(this);
      mpMyContainer = NULL;

      flush();
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   
}

/* ============================ MANIPULATORS ============================== */


UtlContainable* UtlHashBagIterator::operator()()
{
   UtlContainable* foundObject = NULL;

   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerRefLock);
   UtlHashBag* myHashBag = dynamic_cast<UtlHashBag*>(mpMyContainer);
   if (myHashBag)
   {
      OsLock container(myHashBag->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      do 
      {
         if ( mpListNode )
         {
            // we are somewhere in the list mpKeyLists[mListIndex]
            mpListNode = g_list_next(mpListNode);
            if (mpListNode)
            {
               foundObject = (UtlContainable*)mpListNode->data;
            }
         }
         else
         {
            // we have reached the end of a list
            if ( ++mListIndex < mKeyListSize ) // is there another list?
            {
               mpListNode = g_list_first(mpKeyLists[mListIndex]); // get the next list header
               if ( mpListNode ) // lists can be empty due to removals...
               {
                  foundObject = (UtlContainable*)mpListNode->data;
               }
            }
         }
      } while ( !foundObject && mListIndex < mKeyListSize );
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   

   return foundObject;
}

void UtlHashBagIterator::reset()
{
   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerRefLock);
   UtlHashBag* myHashBag = dynamic_cast<UtlHashBag*>(mpMyContainer);
   if (myHashBag)
   {
      OsLock container(myHashBag->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      flush();
      init(myHashBag);
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   
}

/* ============================ ACCESSORS ================================= */

// Gets the key of the current element
UtlContainable* UtlHashBagIterator::key() const
{
   UtlContainable* current = NULL;

   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(const_cast<OsBSem&>(mContainerRefLock));
   UtlHashBag* myHashBag = dynamic_cast<UtlHashBag*>(mpMyContainer);
   if (myHashBag)
   {
      OsLock container(myHashBag->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      if (mpListNode) // current position is defined
      {
         current = (UtlContainable*)mpListNode->data;
      }
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   

   return current;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void UtlHashBagIterator::removing(const GList* node)
{
   // caller is holding the mContainerLock

   size_t keyIndex = BEFORE_FIRST;
   
   if (mpKeyLists)
   {
      // check to see if node was a cached list header
      for (size_t i = 0; keyIndex == BEFORE_FIRST && i < mKeyListSize; i++)
      {
         if (mpKeyLists[i] == node)
         {
            keyIndex = i;
         }
      }
      if (keyIndex != BEFORE_FIRST)
      {
         // the node to be removed is in our cached copy of list headers,
         // so advance the cache to the next node in the list.
         mpKeyLists[keyIndex] = g_list_next(node);
      }
      
      if (mpListNode && mpListNode == node) // removing the current node?
      {
         mpListNode = g_list_previous(mpListNode);
         if (mpListNode == NULL)
         {
            // node was the first in its list, so back up the index of which list we're in
            mListIndex--;
         }
      }
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

void UtlHashBagIterator::init(UtlHashBag* hashBag)
{
   // caller is holding the mContainerLock

   // default to a null iterator
   mpKeyLists = NULL;
   mKeyListSize = 0;

   if (hashBag)
   {
      if ( mpSubsetMatch ) // is this iterator filtered for some key?
      {      
         GList* thisKeyList = (GList*)g_hash_table_lookup(hashBag->mpHashTable, mpSubsetMatch);
      
         if ( thisKeyList )
         {
            mKeyListSize = 1;
            mpKeyLists = new GList*[mKeyListSize];
            mpKeyLists[0] = thisKeyList;
         }
         else
         {
            // this key is not in the subset; leave it a null iterator
         }
      }
      else
      {
         // this iterator is for all keys
         // create a cache of the list headers for each unique key
         mKeyListSize = g_hash_table_size(hashBag->mpHashTable); 
         mpKeyLists = new GList*[mKeyListSize];
   
         assert(mpKeyLists != NULL);

         mListIndex = 0; // set up to fill in table
         g_hash_table_foreach(hashBag->mpHashTable, fillInKeyLists, this);
      }
   }

   
   mpListNode = NULL;
   mListIndex = BEFORE_FIRST;
}

void UtlHashBagIterator::fillInKeyLists(gpointer key, 
                                        gpointer value, 
                                        gpointer user_data
                                        )
{
   UtlHashBagIterator* my = (UtlHashBagIterator*)user_data; // 'this' passed through C library

   assert( my->mListIndex < my->mKeyListSize );

   my->mpKeyLists[my->mListIndex++] = (GList*)value;
}


void UtlHashBagIterator::flush()
{
   // caller is holding mContainerLock and mContainerRefLock

   mKeyListSize = 0;
   mListIndex = BEFORE_FIRST;
   mpListNode = NULL;
   if (mpKeyLists)
   {
      delete[] mpKeyLists;
      mpKeyLists = NULL;
   }
}

/* ============================ FUNCTIONS ================================= */
