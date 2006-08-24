//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_MPBUFPOOL_H // [
#define _INCLUDED_MPBUFPOOL_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

struct MpBuf;
struct MpBufList;

/// Pool of buffers.
class MpBufPool {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Creates pool with numBlocks in it. Each block have size blockSize.
    MpBufPool(unsigned blockSize, unsigned numBlocks);

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

    /// Return number of the buffer in the pool. Use this for debug ouput.
    int getBufferNumber(MpBuf *pBuf) const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// Return pointer to the byte after end of the pool data.
    void *getPoolDataEnd() {return mpPoolData + mBlockSize*mNumBlocks;}

    /// Return pointer to the block, next to this.
    char *getNextBlock(char *pBlock) {return pBlock + mBlockSize;}
    
    void appendFreeList(MpBuf *pBuf);

    unsigned   mBlockSize;     ///< Size of one block in pool (in bytes).
    unsigned   mNumBlocks;     ///< Number of blocks in pool.
    unsigned   mPoolBytes;     ///< Size of all pool in bytes.
    char      *mpPoolData;     ///< Pointer to allocated memory.
                               ///<  May be padded to match align rules.
    MpBufList *mpFreeList;     ///< Begin of the free blocks list.
                               ///<  NULL if there are no free blocks availiable.
    OsMutex    mMutex;         ///< Mutex to avoid concurrent access to the pool.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};


#endif // _INCLUDED_MPBUFPOOL_H ]
