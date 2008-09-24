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

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlHashMapIterator::UtlHashMapIterator(const UtlHashMap& mapSource)
   : UtlIterator(mapSource)
{
   OsLock container(const_cast<OsBSem&>(mapSource.mContainerLock));
   addToContainer(&mapSource);

   init();
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

      myHashMap->removeIterator(this);

      /*
       * A UtlHashMap cannot be resized when there is an iterator associated with it,
       * so it's possible that it has grown while this iterator existed and that it
       * needs to be resized.  Check for and do that now if needed.
       */
      myHashMap->resizeIfNeededAndSafe();

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

      if (mPosition < myHashMap->numberOfBuckets())
      {
         UtlPair* pair;
         for ( pair = (  mpCurrentPair
                       ? static_cast<UtlPair*>(mpCurrentPair->UtlChain::next)
                       : static_cast<UtlPair*>(myHashMap->mpBucket[mPosition].listHead())
                       );
               !pair && ++mPosition < myHashMap->numberOfBuckets();
               pair = static_cast<UtlPair*>(myHashMap->mpBucket[mPosition].listHead())
              )
         {
         }

         if(pair)
         {
            mpCurrentPair = pair;
            foundKey = pair->data;
         }
      }
      else
      {
         // mPosition >= myHashMap->numberOfBuckets(), so we've run off the end of the entries.
         mpCurrentPair = NULL;
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

      init();
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

      if (   (mPosition < myHashMap->numberOfBuckets())
          && (mpCurrentPair)
          && (mPairIsValid)
          )
      {
         currentKey = mpCurrentPair->data;
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

      if (   (mPosition < myHashMap->numberOfBuckets())
          && (mpCurrentPair)
          && (mPairIsValid)
          )
      {
         currentValue = (  mpCurrentPair->value != UtlHashMap::INTERNAL_NULL
                         ? mpCurrentPair->value
                         : NULL
                         );
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
void UtlHashMapIterator::removing(const UtlPair* key)
{
   // the caller already holds the mContainerLock
   if (key = mpCurrentPair)
   {
      mPairIsValid = false;
      mpCurrentPair  = static_cast<UtlPair*>(mpCurrentPair->UtlChain::prev);
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

void UtlHashMapIterator::init()   
{
   mPosition = 0;
   mpCurrentPair = NULL;
   mPairIsValid = true;
}

/* ============================ FUNCTIONS ================================= */
