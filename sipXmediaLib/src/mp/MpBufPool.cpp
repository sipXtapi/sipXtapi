//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpBufPool.h"
#include "mp/MpBuf.h"
#include "os/OsLock.h"

// DEFINES
#if defined(MPBUF_DEBUG) || defined(_DEBUG) // [
#  define MPBUF_CLEAR_EXIT_CHECK
#endif // MPBUF_DEBUG || _DEBUG ]

/// Round 'val' to be multiply of 'align'.
#define MP_ALIGN(val, align) ((((val)+((align)-1))/(align))*(align)) 

/// @brief Block size will be aligned to this value. Other bound will be aligned
//  to it later.
/**
 * Align block size to 4 bytes to avoid problems on ARM.
 */
#define MP_ALIGN_SIZE 4

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/// Class for internal MpBufPool use.
/**
*  This class provides single linked list interface for MpBuf class. It uses
*  MpBuf::mpPool to store pointer to next buffer.
*/
struct MpBufList : public MpBuf {
    friend class MpBufPool;
public:

    /// Get buffer next to current.
    MpBufList *getNextBuf() {return (MpBufList*)mpPool;}

    /// Set buffer next to current.
    void setNextBuf(MpBuf *pNext) {mpPool = (MpBufPool*)pNext;}

private:

    /// Disable copy (and other) constructor.
    MpBufList(const MpBuf &);
    /**<
    * This struct will be initialized by init() member.
    */

    /// Disable assignment operator.
    MpBufList &operator=(const MpBuf &);
    /**<
    * Buffers may be copied. But do we need this?
    */
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpBufPool::MpBufPool(unsigned blockSize, unsigned numBlocks)
: mBlockSize(MP_ALIGN(blockSize,MP_ALIGN_SIZE))
, mNumBlocks(numBlocks)
, mPoolBytes(mBlockSize*mNumBlocks)
, mpPoolData(new char[mPoolBytes])
, mpFreeList(NULL)
, mMutex(OsMutex::Q_PRIORITY)
{
    assert(mBlockSize >= sizeof(MpBuf));
    
    // Init buffers
    char *pBlock = mpPoolData;
    for (int i=mNumBlocks; i>0; i--) {
        MpBuf *pBuf = (MpBufList *)pBlock;
        pBuf->mRefCounter = 0;
        // Don't set mpPool cause it is used by current implementation of free list
//        pBuf->mpPool = this;

        // Add buffer to the end of free list
        appendFreeList(pBuf);
        
        // Jump to next block
        pBlock = getNextBlock(pBlock);
    }

#ifdef MPBUF_DEBUG
    osPrintf("Data start: %X\n", mpPoolData);
#endif
}

MpBufPool::~MpBufPool()
{
#ifdef MPBUF_CLEAR_EXIT_CHECK
    char *pBlock = mpPoolData;
    for (int i=mNumBlocks; i>0; i--) {
        MpBuf *pBuf = (MpBuf *)pBlock;
        if (pBuf->mRefCounter > 0 || pBuf->mpPool == this) {
            osPrintf( "Buffer %d from pool %x did not correctly freed!!!\n"
                    , (pBlock-mpPoolData)/mBlockSize
                    , this);
        }
        pBlock = getNextBlock(pBlock);
    }
#endif

    delete[] mpPoolData;
}

/* ============================ MANIPULATORS ============================== */

MpBuf *MpBufPool::getBuffer()
{
    OsLock lock(mMutex);

    // No free blocks found.
    if (mpFreeList == NULL) {
#ifdef _DEBUG
       osPrintf("!!!! Buffer pool %x is full !!!!\n", this);
#endif
        return NULL;
    }
    
    MpBuf *pFreeBuffer = mpFreeList;
    mpFreeList = mpFreeList->getNextBuf();
    pFreeBuffer->mpPool = this;

#ifdef MPBUF_DEBUG
    osPrintf("Buffer %d from pool %x obtained.\n", getBufferNumber(pFreeBuffer)
                                                 , this);
#endif
    
    return pFreeBuffer;
}

void MpBufPool::releaseBuffer(MpBuf *pBuffer)
{
    OsLock lock(mMutex);
#ifdef MPBUF_DEBUG
    osPrintf("Buffer %d from pool %x freed.\n", getBufferNumber(pBuffer)
                                              , this);
#endif
    assert(pBuffer->mRefCounter == 0);

    // This check is need cause we don't synchronize MpBuf's reference counter.
    // See note in MpBuf::detach().
    if (pBuffer->mpPool == this) {
        appendFreeList(pBuffer);
    } else {
#ifdef MPBUF_DEBUG
        osPrintf("Error: freeing buffer with wrong pool or freeing buffer twice!");
#endif
    }
}

/* ============================ ACCESSORS ================================= */

int MpBufPool::getBufferNumber(MpBuf *pBuf) const
{
    return ((char*)pBuf-mpPoolData)/mBlockSize;
};

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpBufPool::appendFreeList(MpBuf *pBuffer)
{
    ((MpBufList*)pBuffer)->setNextBuf(mpFreeList);
    mpFreeList = (MpBufList*)pBuffer;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
