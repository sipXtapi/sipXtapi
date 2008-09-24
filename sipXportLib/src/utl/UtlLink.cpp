// 
// Copyright (C) 2005 Pingtel Corp.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "assert.h"

// APPLICATION INCLUDES

/*
 * Include UtlInit so that static members are initialized. This ensures UtlLink will be
 * initialized whenever it is linked into program. If it is required to use UtlLink
 * or UtlPair in a constructor or destructor of a static object, the corresponding .cpp
 * file must also include UtlInit.h to safeguard proper constructor/destructor ordering.
 * UtlInit.h should be the first include.
 */
#include "utl/UtlInit.h"

#include "os/OsDefs.h"
#include "os/OsLock.h"
#include "utl/UtlChainPool.h"
#include "utl/UtlLink.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// The pool of available UtlLinks
UtlChainPool* UtlLink::spLinkPool;

// The pool of available UtlPairs
UtlChainPool* UtlPair::spPairPool;

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/// Insert a new item into a list before an existing entry (before NULL == at the tail).
void UtlChain::listBefore(UtlChain* list,
                          UtlChain* existing
                          )
{
   if (!existing) // before NULL means at the tail
   {
      // insert on the tail of the list
      if (list->prev)
      {
         // this list has at least one UtlLink on it, so just link this on the existing chain
         chainAfter(list->prev);
         list->prev = this;
      }
      else
      {
         list->next = this;
         list->prev = this;
      }
   }
   else
   {
      chainBefore(existing);
      if (list->next == existing) // existing was the head of the list
      {
         list->next = this;
      }
   }
}


/// Insert a new item into a list before an existing entry (after NULL == at the head).
void UtlChain::listAfter(UtlChain* list,
                         UtlChain* existing
                         )
{
   
   if (!existing) // after NULL means at the head
   {
      // insert on the head of the list
      if (list->next)
      {
         // this list has at least one UtlLink on it, so just link this on the existing chain
         chainBefore(list->next);
         list->next = this;
      }
      else
      {
         list->next = this;
         list->prev = this;
      }
   }
   else
   {
      chainAfter(existing);
      if (list->prev == existing) // existing was the tail of the list
      {
         list->prev = this;
      }
   }
}

UtlChain* UtlChain::detachFromList(UtlChain* list)
{
   if (this == list->next)
   {
      list->next = next;
   }
   if (this == list->prev)
   {
      list->prev = prev;
   }
   unchain();
   
   return this;
}


UtlContainable* UtlLink::unlink()
{
   // Take the link block out of its list, and return the data pointer
   unchain();
   UtlContainable* theData = data;
   spLinkPool->release(this);
   // :NOTE: cannot reference any member after call to release...

   return theData;
}


UtlLink* UtlLink::before(UtlChain* existing, UtlContainable* newData)
{
   UtlLink* newLink;

   newLink       = get();
   newLink->data = newData;
   newLink->hash = newData->hash();
   newLink->chainBefore(existing);
   return newLink;
}


UtlLink* UtlLink::after(UtlChain* existing, UtlContainable* newData)
{
   UtlLink* newLink;

   newLink       = get();
   newLink->data = newData;
   newLink->hash = newData->hash();
   newLink->chainAfter(existing);
   return newLink;
}

/// Insert a new item into a list before an existing entry (before NULL == at the tail).
UtlLink* UtlLink::listBefore(UtlChain* list,
                             UtlChain* existing,
                             UtlContainable* newData
                             )
{
   UtlLink* newLink;
   
   newLink       = get();
   newLink->data = newData;
   newLink->hash = newData->hash();
   newLink->UtlChain::listBefore(list, existing);
   return newLink;
}


/// Insert a new item into a list before an existing entry (after NULL == at the head).
UtlLink* UtlLink::listAfter(UtlChain* list,
                            UtlChain* existing,
                            UtlContainable* newData
                            )
{
   UtlLink* newLink;
   newLink       = get();
   newLink->data = newData;
   newLink->hash = newData->hash();
   newLink->UtlChain::listAfter(list, existing);

   return newLink;
}

UtlContainable* UtlLink::detachFrom(UtlChain* list)
{
   UtlContainable* theData;

   theData = data;
   data = NULL;
   hash = 0;
   detachFromList(list);
   release();
   
   return theData;
}

/*
 * This is for use in UtlContainer only.
 * get a UtlLink for internal iterator list use.
 */
UtlLink* UtlLink::get()
{
   return static_cast<UtlLink*>(spLinkPool->get());
}

/// Return a UtlLink to the pool.
void UtlLink::release()
{
   spLinkPool->release(this);
}


/// Recalculate the hash for this item
void UtlLink::rehash()
{
   if (data)
   {
      hash = data->hash();
   }   
}


size_t UtlLink::totalAllocated()
{
   return spLinkPool->totalAllocated();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void UtlLink::allocate(size_t    blocksize, ///< number of instances to allocate
                       UtlChain* blockList, ///< list header for first instance
                       UtlChain* pool       ///< list header for others
                       )
{
   UtlLink* newBlock = new UtlLink[blocksize];
   assert(newBlock);

   // The first UtlChain is consumed to chain the list of blocks
   //     so that the destructor can free them.
   newBlock->UtlChain::listBefore(blockList, NULL);
               
   // chain the rest of the new UtlLinks onto the mLinkPool
   for (size_t i = 1; i < blocksize; i++)
   {
      newBlock[i].UtlChain::listBefore(pool, NULL);
   }
}

void UtlPair::allocate(size_t    blocksize, ///< number of instances to allocate
                       UtlChain* blockList, ///< list header for first instance
                       UtlChain* pool       ///< list header for others
                       )
{
   UtlPair* newBlock = new UtlPair[blocksize];
   assert(newBlock);

   // The first UtlChain is consumed to chain the list of blocks
   //     so that the destructor can free them.
   newBlock->UtlChain::listBefore(blockList, NULL);
               
   // chain the rest of the new UtlLinks onto the mLinkPool
   for (size_t i = 1; i < blocksize; i++)
   {
      newBlock[i].UtlChain::listBefore(pool, NULL);
   }
}

UtlPair* UtlPair::get()
{
   return static_cast<UtlPair*>(spPairPool->get());
}

void UtlPair::release()
{
   spPairPool->release(this);
}

