//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2007 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef UtlChainPool_h__
#define UtlChainPool_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlLink.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS

/// Pool of available objects derived from UtlChain.
/**
 * This avoids excessive heap operations; rather than delete unused UtlChains, they are
 * stored on the mPool here.  To limit the heap overhead associated with allocating
 * UtlChain, they are allocated in mBlockSize blocks, which are chained on
 * mBlocks.
 *
 * The actual allocation of the blocks and initial chaining is done by the allocator
 * function supplied by the UtlChain subclass.
 */
class UtlChainPool
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   friend class UtlLink;
   friend class UtlPair;
   friend class UtlInit;

   /// Allocate blocksize instances of the subclass and chain them into the pool. 
   typedef void allocator(size_t    blocksize, ///< number of instances to allocate
      UtlChain* blockList, ///< list header for first instance
      UtlChain* pool       ///< list header for others
      );
   /**<
    * This function is supplied by the subclass to the UtlChainPool constructor.
    * It is responsible for allocating a block of blocksize instances of its subclass.
    * The first instance in each block is added to the blockList, so that the UtlChainPool
    * destructor can delete the block.  The remaining (blocksize-1) instances are
    * chained onto the pool list header.
    */

   /// Create a UtlChainPool that uses blockAllocator to create UtlChain derived objects.
   UtlChainPool(allocator* blockAllocator, size_t blockSize);

   /// Get a UtlLink with chain pointers NULL
   UtlChain* get();

   /// Return freeLink to the pool of available UtlLinks.
   void release(UtlChain* freeChain);

   /// Returns the total number of subclasses instances allocated by this pool.
   /**
    * The returned count does not include the 1 instance in each allocation that is
    * consumed to manage the pool.
    */
   size_t totalAllocated()
   {
      return mAllocations * (mBlockSize-1); // one per block is overhead
   }

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Release all dynamic memory used by the UtlLinkPool.
   ~UtlChainPool();

   OsBSem        mLock; ///< lock for all the other member variables
   size_t        mBlockSize;
   size_t        mAllocations;
   allocator*    mAllocator;
   UtlChain      mPool;     ///< list of available UtlLinks.
   UtlChain      mBlocks;   /**< list of memory blocks allocated by the mAllocator.
                             *   Each block is an mBlockSize array of objects derived from
                             *   UtlChain. The 0th element is used to form the linked list
                             *   of blocks.  The rest are made a part of the mPool.*/
};


#endif // UtlChainPool_h__
