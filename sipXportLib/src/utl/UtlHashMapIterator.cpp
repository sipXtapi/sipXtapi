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
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlContainable.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const size_t UtlHashMapIterator::BEFORE_FIRST = (size_t)(-1);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlHashMapIterator::UtlHashMapIterator(const UtlHashMap& mapSource)
   : UtlIterator(mapSource)
{
   OsLock container(const_cast<OsBSem&>(mapSource.mContainerLock));

   addToContainer(&mapSource);
      
   init(&mapSource);
}


// Destructor
UtlHashMapIterator::~UtlHashMapIterator()
{
   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerRefLock);
   UtlHashMap* myHashMap = dynamic_cast<UtlHashMap*>(mpMyContainer);
   if (myHashMap)
   {
      OsLock container(myHashMap->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      flush();

      myHashMap->removeIterator(this);
      mpMyContainer = NULL;
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator


UtlContainable* UtlHashMapIterator::operator()()
{
   UtlContainable* foundKey = NULL;

   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerRefLock);
   UtlHashMap* myHashMap = dynamic_cast<UtlHashMap*>(mpMyContainer);
   if (myHashMap)
   {
      OsLock container(myHashMap->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      while( !foundKey && ++mPosition < mIteratorSize )
      {
         if(mpContainables[mPosition] != NULL)
         {
            foundKey = mpContainables[mPosition];
         }
      }
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   

   return foundKey;
}


void UtlHashMapIterator::reset()
{
   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerRefLock);
   UtlHashMap* myHashMap = dynamic_cast<UtlHashMap*>(mpMyContainer);
   if (myHashMap)
   {
      OsLock container(myHashMap->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      flush();
      init(myHashMap);
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   

}



/* ============================ ACCESSORS ================================= */

UtlContainable* UtlHashMapIterator::key() const
{
   UtlContainable* currentKey = NULL;
   
   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(const_cast<OsBSem&>(mContainerRefLock));
   UtlHashMap* myHashMap = dynamic_cast<UtlHashMap*>(mpMyContainer);
   if (myHashMap)
   {
      OsLock container(myHashMap->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      if (mPosition < mIteratorSize)
      {
         currentKey = mpContainables[mPosition];
      }
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   

   return currentKey;
}


UtlContainable* UtlHashMapIterator::value() const
{
   UtlContainable* currentValue = NULL;
   
   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(const_cast<OsBSem&>(mContainerRefLock));
   UtlHashMap* myHashMap = dynamic_cast<UtlHashMap*>(mpMyContainer);
   if (myHashMap)
   {
      OsLock container(myHashMap->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      if (   mPosition < mIteratorSize // iterator in bounds
          && mpContainables[mPosition] // has not been removed
          )
      {
         currentValue = myHashMap->getValue(mpContainables[mPosition]);
      }
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }   

   return currentValue;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Called by the HashMap before removing a key from the map
void UtlHashMapIterator::removing(const UtlContainable* key)
{
   // the caller already holds the mContainerLock
   size_t check;
   bool found;
   for ( check = 0, found = false; check < mIteratorSize && !found; check++ )
   {
      if ( mpContainables[check] == key )
      {
         mpContainables[check] = NULL; // prevent returning this key
         found = true;
      }
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

void UtlHashMapIterator::init(const UtlHashMap* hashMap)
{
   // caller is holding the hashMap->mContainerLock
   mIteratorSize = g_hash_table_size(hashMap->mpHashTable);

   mpContainables = new UtlContainable*[mIteratorSize];
   assert(mpContainables != NULL);

   mPosition = 0;

   g_hash_table_foreach(hashMap->mpHashTable, fillInContainables, this);

   mPosition = BEFORE_FIRST;
}

void UtlHashMapIterator::fillInContainables(gpointer key, 
                                            gpointer value, 
                                            gpointer user_data
                                            )
{
   UtlHashMapIterator* my = (UtlHashMapIterator*)user_data; // 'this' passed through C library

   assert( my->mPosition < my->mIteratorSize );

   my->mpContainables[my->mPosition++] = (UtlContainable*) key;
}


void UtlHashMapIterator::flush()
{
   // caller is holding the mContainerLock
   mIteratorSize = 0;
   mPosition = BEFORE_FIRST;
   delete[] mpContainables;
   mpContainables = NULL;
}



/* ============================ FUNCTIONS ================================= */
