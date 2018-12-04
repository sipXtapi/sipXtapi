//  
// Copyright (C) 2006-2018 SIPez LLC.  All rights reserved.
//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpBufPool.h>
#include <mp/MpBuf.h>
#include <os/OsSysLog.h>
#include <os/OsLock.h>
#include <utl/UtlInt.h>
#include <utl/UtlVoidPtr.h>
#include <utl/UtlHashMapIterator.h>

// DEFINES
#if defined(MPBUF_DEBUG) || defined(_DEBUG) // [
#  define MPBUF_CLEAR_EXIT_CHECK
#endif // MPBUF_DEBUG || _DEBUG ]

/// Round 'val' to be multiply of 'align'.
#define MP_ALIGN(val, align) ((((val)+((align)-1))/(align))*(align)) 

/// @brief Block size will be aligned to this value. Other bound will be aligned
//  to it later.
#if defined(__x86_64__) || defined(_M_X64)
   /// Align block size to 8 bytes on x86_64
#  define MP_ALIGN_SIZE 8
#else // __x86_64__ || _M_X64 ][
   /// Align block size to 4 bytes elsewise to avoid crashes on ARM and
   /// performance degradation on x86.
#  define MP_ALIGN_SIZE 4
#endif // !(__x86_64__ || _M_X64) ]

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

    int length() const
    {
        int length = 0;
        MpBufList* next = (MpBufList*) mpPool;
        while(next)
        {
            length++;
            next = (MpBufList*) next->mpPool;
        }
        return(length);
    }

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

MpBufPool::MpBufPool(unsigned blockSize, unsigned numBlocks, const UtlString& poolName)
: mPoolName(poolName)
, mBlockSize(blockSize)
, mBlockSpan(MP_ALIGN(blockSize,MP_ALIGN_SIZE))
, mNumBlocks(numBlocks)
, mPoolBytes(mBlockSpan*mNumBlocks)
, mpPoolData(new char[mPoolBytes])
, mpFreeList(NULL)
, mMutex(OsMutex::Q_PRIORITY)
, mNumGets(0)
, mNumFrees(0)
, mNumFree(numBlocks)
, mMinFree(numBlocks)
{
    assert(mBlockSize >= sizeof(MpBuf));
    memset(mpPoolData, 0xff, mPoolBytes);
    
    // Init buffers
    char *pBlock = mpPoolData;
    for (int i=mNumBlocks; i>0; i--) {
        MpBuf *pBuf = (MpBufList *)pBlock;
        pBuf->mRefCounter = 0;
        // Don't set mpPool cause it is used by current implementation of free list
        //pBuf->mpPool = this;

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
            osPrintf( "Buffer %d from pool %p was not correctly freed!!!\n"
                    , getBufferNumber(pBuf)
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
    if (mpFreeList == NULL) 
    {
        profileFlowgraphPoolUsage();
        OsSysLog::add(FAC_MP, PRI_ERR,
                "MpBufPool::getBuffer pool: %s is empty.  %d buffers outstanding.",
                mPoolName.data(), mNumBlocks);
#ifdef _DEBUG
       osPrintf("!!!! Buffer pool %x is full !!!!\n", this);
#endif
        return NULL;
    }
    
    MpBuf *pFreeBuffer = mpFreeList;
    mpFreeList = mpFreeList->getNextBuf();
    pFreeBuffer->mpPool = this;
    pFreeBuffer->mpFlowGraph = NULL;
    mNumGets++;
    mNumFree--;
    if (mNumFree < mMinFree)
    {
        mMinFree = mNumFree;
        if (0 == (0x3f&mNumFree))
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                "MpBufPool::getBuffer pool: %s (%p), NumFree dropped to %d",
                mPoolName.data(), this, mNumFree); 
        }
    }


#ifdef MPBUF_DEBUG
    osPrintf("Buffer %d from pool %x have been obtained.\n",
             getBufferNumber(pFreeBuffer), this);
#endif
    
    return pFreeBuffer;
}

void MpBufPool::releaseBuffer(MpBuf *pBuffer)
{
    OsLock lock(mMutex);
#ifdef MPBUF_DEBUG
    osPrintf("Buffer %d from pool %x have been freed.\n",
             getBufferNumber(pBuffer), this);
#endif
    assert(pBuffer->mRefCounter == 0);

    // This check is need cause we don't synchronize MpBuf's reference counter.
    // See note in MpBuf::detach().
    if (pBuffer->mpPool == this) {
        appendFreeList(pBuffer);
        pBuffer->mpFlowGraph = NULL;
        mNumFrees++;
        mNumFree++;
    } else {
#ifdef MPBUF_DEBUG
        osPrintf("Error: freeing buffer with wrong pool or freeing buffer twice!");
#endif
    }
}

/* ============================ ACCESSORS ================================= */

int MpBufPool::getBufferNumber(MpBuf *pBuf) const
{
    return ((char*)pBuf-mpPoolData)/mBlockSpan;
};

int MpBufPool::getFreeBufferCount()
{
    OsLock lock(mMutex);
    int count = 0;
    if(mpFreeList)
    {
       count = mpFreeList->length();
    }

    return(count);
}

int MpBufPool::scanBufPool(MpFlowGraphBase *pFG)
{
    char *pBlock = mpPoolData;
    int bads = 0;
    for (int i=mNumBlocks; i>0; i--) {
        MpBuf *pBuf = (MpBuf *)pBlock;
        if (pBuf->mRefCounter != 0 || pBuf->mpPool == this) {
            if (pFG == pBuf->mpFlowGraph) {
                bads++;
                OsSysLog::add(FAC_MP, PRI_ERR, "Buffer %d from pool %p (flowgraph=%p) was not correctly freed!!!\n",
                    getBufferNumber(pBuf), this, pFG);
            }
        }
        pBlock = getNextBlock(pBlock);
    }
    return bads;
}

int MpBufPool::profileFlowgraphPoolUsage()
{
    UtlHashMap flowgraphBufferCount;

    char *pBlock = mpPoolData;
    for (int i=mNumBlocks; i>0; i--) 
    {
        MpBuf *pBuf = (MpBuf *)pBlock;
        if (pBuf->mRefCounter != 0 || pBuf->mpPool == this)
        {
            UtlVoidPtr pointerKey(pBuf->mpFlowGraph);
            UtlInt* flowgraphCount = (UtlInt*) flowgraphBufferCount.findValue(&pointerKey);
            if(flowgraphCount)
            {
                flowgraphCount->setValue(flowgraphCount->getValue() + 1);
            }
            else
            {
                flowgraphBufferCount.insertKeyAndValue(new UtlVoidPtr(pBuf->mpFlowGraph), new UtlInt(1));
            }
        }
        pBlock = getNextBlock(pBlock);
    }

    OsSysLog::add(FAC_MP, PRI_ERR,
            "MpBufPool::profileFlowgraphPoolUsage pool: %p, buffer size: %d+%d, free buffer count: %d/%d",
            this, mBlockSize, mBlockSpan-mBlockSize, mNumFree, mNumBlocks); 

    UtlHashMapIterator iterator(flowgraphBufferCount);
    UtlInt* countPtr = NULL;
    UtlVoidPtr* flowgraphPtr = NULL;
    while((flowgraphPtr = (UtlVoidPtr*) iterator()))
    {
        if((countPtr = (UtlInt*) flowgraphBufferCount.findValue(flowgraphPtr)))
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MpBufPool::profileFlowgraphPoolUsage flowgraph: %p using %d buffers of size %d+%d in pool %p",
                    flowgraphPtr->getValue(), countPtr->getValue(), mBlockSize, mBlockSpan-mBlockSize, this);
        }
    }

    int flowgraphCount = flowgraphBufferCount.entries();
    flowgraphBufferCount.destroyAll();

    return(flowgraphCount);
}

const UtlString& MpBufPool::getName()
{
    return(mPoolName);
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpBufPool::appendFreeList(MpBuf *pBuffer)
{
    ((MpBufList*)pBuffer)->setNextBuf(mpFreeList);
    mpFreeList = (MpBufList*)pBuffer;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
