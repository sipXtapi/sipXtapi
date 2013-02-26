//  
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_MPBUFPOOL_H // [
#define _INCLUDED_MPBUFPOOL_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

struct MpBuf;
struct MpBufList;
class MpFlowGraphBase;
class UtlString;

/// Pool of buffers.
class MpBufPool {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Creates pool with numBlocks in it. Each block have size blockSize.
    MpBufPool(unsigned blockSize, unsigned numBlocks, const UtlString& poolName);

    /// Destroys pool.
    virtual
    ~MpBufPool();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Get free block from pool.
    MpBuf *getBuffer();
    /**<
    * @return If there are no free blocks in pool invalid pointer returned.
    */

    /// Bring this buffer back to pool.
    void releaseBuffer(MpBuf *pBuffer);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return size of the one block in the pool (in bytes).
    unsigned getBlockSize() const {return mBlockSize;};

    /// Return number of blocks in the pool.
    unsigned getNumBlocks() const {return mNumBlocks;};

    /// Return number of the buffer in the pool. Use this for debug output.
    int getBufferNumber(MpBuf *pBuf) const;

    /// Return the number of free buffers
    int getFreeBufferCount();

    /// Scan for orphan buffers
    int scanBufPool(MpFlowGraphBase *pFG);

    /// Syslog a histigram of buffers used by flowgraph
    int profileFlowgraphPoolUsage();

    /// Get the name label for this pool
    const UtlString& getName();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// Return pointer to the byte after end of the pool data.
    void *getPoolDataEnd() {return mpPoolData + mBlockSpan*mNumBlocks;}

    /// Return pointer to the block, next to this.
    char *getNextBlock(char *pBlock) {return pBlock + mBlockSpan;}
    
    void appendFreeList(MpBuf *pBuf);

    UtlString  mPoolName;      ///< label or name for debug
    unsigned   mBlockSize;     ///< Requested size of each block in pool (in bytes).
    unsigned   mBlockSpan;     ///< Actual size of each block.  >= mBlockSize for alignment
    unsigned   mNumBlocks;     ///< Number of blocks in pool.
    unsigned   mPoolBytes;     ///< Size of all pool in bytes.
    char      *mpPoolData;     ///< Pointer to allocated memory.
                               ///<  May be padded to match align rules.
    MpBufList *mpFreeList;     ///< Begin of the free blocks list.
                               ///<  NULL if there are no free blocks availiable.
    OsMutex    mMutex;         ///< Mutex to avoid concurrent access to the pool.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    unsigned   mNumGets;       ///For statistics
    unsigned   mNumFrees;      ///For statistics
    unsigned   mNumFree;       ///For statistics
    unsigned   mMinFree;       ///For statistics
 
};


#endif // _INCLUDED_MPBUFPOOL_H ]
