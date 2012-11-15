//  
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
//  
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_MPBUF_H // [
#define _INCLUDED_MPBUF_H

/**
 *  @todo cache align - align pool beginning, buffer sizes (must be multiple of
 *        the cache size) and data beginning in MpArrayBuf.
 *  @todo error handling - return OsStatus may be?
 */

// SYSTEM INCLUDES
#include <stdlib.h>
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpBufPool.h"

// DEFINES
// MACROS
// Uncomment MPBUF_DEBUG define to enable deep MpBuf debug with a lot of
// messages and more asserts.
//#define MPBUF_DEBUG

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// FORWARD DECLARATIONS
class MpFlowGraphBase;

// CONSTANTS
// STRUCTS

/// Enum used for runtime type checks.
/**
*  This enum is used to determine the real type of the buffer. Also this helps
*  us to decide is it possible to convert from one buffer to other.
*  E.g. MpAudioBuf could be converted to MpDataBuf, but could not be converted
*  to MpArrayBuf. This check is done inside MpBufPtr's child classes.
*/
typedef enum {
    MP_BUF,               ///< Begin of the MpBuf type
      MP_BUF_ARRAY,       ///< Begin of the MpArrayBuf type
      MP_BUF_ARRAY_END,   ///< End of the MpArrayBuf type
      MP_BUF_DATA,        ///< Begin of the MpDataBuf type
        MP_BUF_AUDIO,     ///< Begin of the MpAudioBuf type
        MP_BUF_AUDIO_END, ///< End of the MpAudioBuf type
        MP_BUF_VIDEO,     ///< Begin of the MpVideoBuf type
        MP_BUF_VIDEO_END, ///< End of the MpVideoBuf type
        MP_BUF_UDP,       ///< Begin of the MpUdpBuf type
          MP_BUF_RTP,     ///< Begin of the MpRtpBuf type
          MP_BUF_RTP_END, ///< End of the MpRtpBuf type
        MP_BUF_UDP_END,   ///< End of the MpUdpBuf type
      MP_BUF_DATA_END,    ///< End of the MpDataBuf type
    MP_BUF_END            ///< End of the MpBuf type
} MP_BUFFERS_TREE;

// TYPEDEFS

///  Base class for all media buffers.
/**
*  This class designed to be used with memory pool MpBufPool and smart pointer
*  MpBufPoolPtr. It seems to be useless without them.
*  
*  This struct itself does not provide any data storage. It contain only
*  reference counter and pointer to parent pool. To store data in this pool
*  use MpArrayBuf. To store store data in the other pool use MpDataBuf or
*  one of its children.
*/
struct MpBuf {
    friend class MpBufPtr;
    friend class MpBufPool;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{


//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Increments reference counter.
    void attach();

    /// Decrements reference counter and free buffer if needed.
    void detach();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    void setFlowGraph(MpFlowGraphBase* flowgraph)
    {
        mpFlowGraph = flowgraph;
    };

    /// Get buffer type.
    MP_BUFFERS_TREE getType() const {return mType;};

    /// Get parent pool of this buffer.
    MpBufPool *getBufferPool() const {return mpPool;};

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    MP_BUFFERS_TREE mType;     ///< Buffer class type. Used for type safety.
    int mRefCounter;           ///< Reference counter for use with MpBufPtr.
    MpBufPool* mpPool;         ///< Parent memory pool.
    MpFlowGraphBase* mpFlowGraph; ///< Debug pointer to flowgraph in which this buf is used
    void (*mpDestroy)(MpBuf*); ///< Pointer to deinitialization method. Used as
                               ///<  virtual destructor.
    void (*mpInitClone)(MpBuf*); ///< Pointer to function that initialize buffer
                                 ///<  after cloning. 

    /// @brief Function that initialize buffer after cloning. It adjusts
    /// reference counters.
    static void sInitClone(MpBuf *pBuffer);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Disable copy (and other) constructor.
    MpBuf(const MpBuf &);
    /**<
    * This struct will be initialized by init() member.
    */

    /// Disable assignment operator.
    MpBuf &operator=(const MpBuf &);
    /**<
    * Buffers may be copied. But do we need this?
    */
};

///  Smart pointer to MpBuf.
/**
*  You should only use this smart pointer, not #MpBuf* itself.
*  The goal of this smart pointer is to care about reference counter and
*  buffer deallocation.
*/
class MpBufPtr {
    friend class MpBufPool;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Default constructor - construct invalid pointer.
    MpBufPtr()
        : mpBuffer(NULL)
    {};

    /// This constructor owns MpBuf object.
    /**
    *  @note THIS CONSTRUCTOR ARE ONLY USABLE BY POOLS
    */
    MpBufPtr(MpBuf *buffer)
        : mpBuffer(buffer)
    {
        if (mpBuffer != NULL) {
            mpBuffer->mType = MP_BUF;
            mpBuffer->mpDestroy = NULL;
            mpBuffer->mpInitClone = MpBuf::sInitClone;
            mpBuffer->attach();
        }
#ifdef _DEBUG
        else {
            osPrintf("mpBuffer == NULL!\n");
        }
#endif
    };

    /// Destructor. It decrements buffer's reference counter.
    ~MpBufPtr()
    {
        if (mpBuffer != NULL)
            mpBuffer->detach();
    };

    /// Copy buffer pointer and increment its reference counter.
    MpBufPtr(const MpBufPtr &buffer) 
        : mpBuffer(buffer.mpBuffer)
    {
        if (mpBuffer != NULL)
            mpBuffer->attach();
    }

    MpBufPtr clone() const
    {
       MpBufPtr clone;

       // Return invalid pointer as a copy of invalid pointer.
       if (!isValid())
          return clone;

       // Get fresh buffer
       clone.mpBuffer = mpBuffer->getBufferPool()->getBuffer();
       if (!clone.isValid())
          return clone;

       // Copy raw buffer's content to new location
       memcpy(clone.mpBuffer, mpBuffer, mpBuffer->getBufferPool()->getBlockSize());

       // Init clone
       clone->mpInitClone(clone.mpBuffer);

       return clone;
    }

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Smart assignment.
    /**
    * Decrement reference counter of our old buffer and increment in new one.
    */
    MpBufPtr &operator=(const MpBufPtr &bufferPtr)
    {
        // Check for a=a case;
        if (&bufferPtr == this) {
            return *this;
        }

        if (mpBuffer != NULL)
            mpBuffer->detach();
        mpBuffer = bufferPtr.mpBuffer;
        if (mpBuffer != NULL)
            mpBuffer->attach();

        return *this;
    }

    /// Compare two smart pointers
    /**
    *  Two pointers assumed equal if they point to the same buffer.
    */
    bool operator==(const MpBufPtr &pBuffer)
    {
        return (mpBuffer == pBuffer.mpBuffer);
    }

    /// Compare two smart pointers
    /**
    *  Two pointers assumed equal if they point to the same buffer.
    */
    bool operator!=(const MpBufPtr &pBuffer)
    {
        return (mpBuffer != pBuffer.mpBuffer);
    }

    /// Release buffer we are pointing to.
    /**
    *  If no one else is pointing to this buffer it will be freed.
    */
    void release()
    {
        if (mpBuffer != NULL)
            mpBuffer->detach();
        mpBuffer = NULL;
    }

    /// Swap to buffers.
    /**
    *  This pointer will point to buffer pointed by pBuffer, and pBuffer will
    *  point to buffer, pointed by this pointer. Swap does not modify reference
    *  counters of buffers.
    */
    void swap(MpBufPtr &pBuffer)
    {
        MpBuf *temp = pBuffer.mpBuffer;
        pBuffer.mpBuffer = mpBuffer;
        mpBuffer = temp;
    }

    /// Check if buffer is writable and create copy if no.
    /**
    *  @returns <b>true</b> - on success.
    *  @returns <b>false</b> - if buffer cannot be made writable. E.g. if buffer
    *                          contain NULL pointer.
    */
    bool requestWrite()
    {
       // We already writable?
       if (isWritable())
          return true;

       // Cannot make buffer writable...
       if (mpBuffer == NULL)
          return false;

       // Create the clone and own it.
       MpBufPtr pBuf = clone();
       swap(pBuf);

       return true;
    }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return number of the buffer in the pool. Use this for debug output.
    int getBufferNumber() const
    {
        if (mpBuffer == NULL)
            return -1;
        else
            return mpBuffer->mpPool->getBufferNumber(mpBuffer);
    };

    void setFlowGraph(MpFlowGraphBase* flowgraph)
    {
        if(mpBuffer != NULL)
        {
            mpBuffer->setFlowGraph(flowgraph);
        }
    };

    /// Return pointer to MpBuf.
    MpBuf *operator->() {assert(mpBuffer!=NULL); return mpBuffer;};

    /// Return readonly pointer to MpBuf.
    const MpBuf *operator->() const {assert(mpBuffer!=NULL); return mpBuffer;};

    MP_BUFFERS_TREE getType() const 
    {
        if(mpBuffer) return(mpBuffer->getType());
        else return((MP_BUFFERS_TREE)-1);
    };
//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

    /// @brief Can this pointer be dereferenced? Use this function instead of
    /// NULL comparison.
    bool isValid() const {return mpBuffer != NULL;};

    /// You should write to the buffer if and only if this function return true.
    /**
    * isWritable() check are you the only owner of this buffer.
    */
    bool isWritable() {return (mpBuffer != NULL) && (mpBuffer->mRefCounter == 1);};

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend struct MpDataBuf;

    MpBuf *mpBuffer;  ///< Pointer to real buffer.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/// Default constructor - construct invalid pointer.
#define MPBUF_DEFAULT_CONSTRUCTOR(classname)                                \
    classname##Ptr() {};

#define MPBUF_FROM_BASE_CONSTRUCTOR_INIT(classname, buffer_type)            \
            classname *pBuffer = (classname*)mpBuffer;                      \
            assert(pBuffer->mpPool->getBlockSize() >= sizeof(classname));   \
            pBuffer->mType = buffer_type;                                   \
            pBuffer->init();                                                \

/// This constructor owns MpBuf object.
#define MPBUF_FROM_BASE_CONSTRUCTOR(classname, buffer_type, base_classname) \
    classname##Ptr(MpBuf *buffer)                                           \
        : base_classname##Ptr(buffer)                                       \
    {                                                                       \
        if (mpBuffer != NULL) {                                             \
            MPBUF_FROM_BASE_CONSTRUCTOR_INIT(classname, buffer_type)        \
        }                                                                   \
    };

/// Copy object from base type with type check.
#define MPBUF_TYPECHECKED_COPY(classname, buffer_type, base_classname)      \
    classname##Ptr(const MpBufPtr &buffer)                                  \
        : base_classname##Ptr(buffer)                                       \
    {                                                                       \
        assert( (!buffer.isValid())                                         \
              || (  buffer->getType() >= buffer_type                        \
                 && buffer->getType() < buffer_type##_END));                \
    };

/// Return pointer to buffer.
#define MPBUF_MEMBER_ACCESS_OPERATOR(classname)                             \
   classname *operator->()                                                  \
      {assert(mpBuffer!=NULL); return (classname*)mpBuffer;};

/// Return readonly pointer to buffer.
#define MPBUF_CONST_MEMBER_ACCESS_OPERATOR(classname)                       \
   const classname *operator->() const                                      \
      {assert(mpBuffer!=NULL); return (classname*)mpBuffer;};

#endif // _INCLUDED_MPBUF_H  ]

