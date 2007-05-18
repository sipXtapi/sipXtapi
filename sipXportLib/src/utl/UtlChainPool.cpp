//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2007 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "utl/UtlChainPool.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlChainPool::UtlChainPool( allocator* blockAllocator, size_t blockSize ) :
      mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
      mBlockSize(blockSize),
      mAllocations(0),
      mAllocator(blockAllocator)
{

}

UtlChain* UtlChainPool::get()
{
   UtlChain* newChain;
   {  // critical section for member variables
      OsLock poolLock(mLock);

      if (mPool.isUnLinked()) // are there available objects in the pool?
      {
         // no - get the subclass to allocate some more
         mAllocator(mBlockSize, &mBlocks, &mPool);
         mAllocations++;
      } 

      // pull the first UtlChain off the mPool
      newChain = mPool.listHead();
      if (newChain)
      {
         newChain->detachFromList(&mPool);
      }
   }  // end of critical section
   return newChain;
}

void UtlChainPool::release( UtlChain* freeChain )
{
   OsLock poolLock(mLock);

   // put this freed object on the tail of the pool list
   freeChain->listBefore(&mPool, NULL);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlChainPool::~UtlChainPool()
{
   OsLock poolLock(mLock);

   UtlChain* block;
   while (!mBlocks.isUnLinked()) // blocks still on block list
   {
      block = mBlocks.listHead()->detachFromList(&mBlocks);
      delete[] block;
   }
}

/* ============================ FUNCTIONS ================================= */
