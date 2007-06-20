//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_MPDATABUF_H // [
#define _INCLUDED_MPDATABUF_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpBuf.h"
#include "mp/MpArrayBuf.h"

// DEFINES
// MACROS
/// This constructor owns MpBuf object.
#define MPBUFDATA_FROM_BASE_CONSTRUCTOR(classname, buffer_type, base_classname) \
    classname##Ptr( MpBuf *buffer                                           \
                  , MpBufPool *pHeaderPool=classname::smpDefaultPool)       \
        : base_classname##Ptr(buffer, pHeaderPool)                          \
    {                                                                       \
        if (mpBuffer != NULL) {                                             \
            MPBUF_FROM_BASE_CONSTRUCTOR_INIT(classname, buffer_type)        \
        }                                                                   \
    };

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

///  Stores data in the external buffer.
/**
*  This struct could be used if you want to store data separate from header. This
*  approach may be useful when you need to share data between several headers.
*  For example, you may want to pass raw audio data to RTP packet without copying.
*
*  @warning Data pointer is set in the MpDataBufPtr constructor. 
*/
struct MpDataBuf : public MpBuf
{
    friend class MpDataBufPtr;
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /// Default pool for MpDataBuf objects.
    static MpBufPool *smpDefaultPool;

/* ============================ CREATORS ================================== */
///@name Creators
//@{


//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Set new payload data.
    void setData(const MpArrayBufPtr &pData)
    { mpData = pData; };

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return pointer to payload data.
    char *getDataWritePtr();

    /// Return pointer to payload data.
    const char *getDataPtr() const;

    /// Get payload data.
    MpArrayBufPtr getData() const {return mpData;};

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    MpArrayBufPtr mpData;    ///< Payload data.

    /// This is called in place of constructor.
    void init();

    /// Destructor for MpDataBuf.
    static void sDestroy(MpBuf *pBuffer);

    /// @brief Function that initialize buffer after cloning. It makes clone
    /// of mpData.
    static void sInitClone(MpBuf *pBuffer);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Disable copy (and other) constructor.
    MpDataBuf(const MpBuf &);
    /**<
    * This struct will be initialized by init() member.
    */

    /// Disable assignment operator.
    MpDataBuf &operator=(const MpBuf &);
    /**<
    * Buffers may be copied. But do we need this?
    */
};

///  Smart pointer to MpDataBuf.
/**
*  You should only use this smart pointer, not #MpDataBuf* itself.
*  The goal of this smart pointer is to care about reference counter and
*  buffer deallocation.
*/
class MpDataBufPtr : public MpBufPtr {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Default constructor - construct invalid pointer.
    MPBUF_DEFAULT_CONSTRUCTOR(MpDataBuf)

    /// This constructor owns MpBuf object.
    MpDataBufPtr(MpBuf *pDataBuffer, MpBufPool *pHeaderPool=MpDataBuf::smpDefaultPool)
        : MpBufPtr(pHeaderPool->getBuffer())
    {
        if (mpBuffer != NULL) {
            MPBUF_FROM_BASE_CONSTRUCTOR_INIT(MpDataBuf, MP_BUF_DATA)

            // Set data pointer
            pBuffer->setData(MpArrayBufPtr(pDataBuffer));
        }
    };

    /// Construct object from base type with type check.
    MPBUF_TYPECHECKED_COPY(MpDataBuf, MP_BUF_DATA, MpBuf)

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{


//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return pointer to MpDataBuf.
    MPBUF_MEMBER_ACCESS_OPERATOR(MpDataBuf)

    /// Return readonly pointer to MpDataBuf.
    MPBUF_CONST_MEMBER_ACCESS_OPERATOR(MpDataBuf)

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // _INCLUDED_MPDATABUF_H ]
