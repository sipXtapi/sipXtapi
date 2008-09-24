//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_MPARRAYBUF_H // [
#define _INCLUDED_MPARRAYBUF_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

///  Stores data right after the header.
/**
*  This class adds two features to MpBuf - pointer to data, located right after
*  the MpArrayBuf object, and its size. Size of the data may be set by user to
*  any acceptable value. See setDataSize() for more details about data size
*  setting.
*/
struct MpArrayBuf : public MpBuf
{
    friend class MpArrayBufPtr;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{


//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Set current data size
    bool setDataSize( unsigned size ///< new data size
                    );
    /**<
     * New data size could not be greater then allocated space (provided by
     * getMaxDataSize()).
     * @return <b>false</b> if size is greater then allocated space.
     *                      In this case size is set to maximum possible).
     * @return <b>true</b> is returned in either case.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Get pointer to the buffer data with intent to write/change it.
    char *getDataWritePtr() {return mpData;}

    /// Get read only pointer to the buffer data.
    const char *getDataPtr() const {return mpData;}

    /// Get size of #MpArrayBuf without data (in bytes).
    static unsigned getHeaderSize() {return sizeof(MpArrayBuf)-sizeof(char);}

    /// Get maximum allowed payload size (in bytes).
    unsigned getMaxDataSize() const
    {return mpPool->getBlockSize()-getHeaderSize();}

    /// Get current data size.
    unsigned getDataSize() const {return mDataSize;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    unsigned  mDataSize;   ///< Size of the following data (in bytes).
    char      mpData[1];   ///< Pointer to the data, following this header.

    /// This is called in place of constructor.
    void init();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Disable copy (and other) constructor.
    MpArrayBuf(const MpBuf &);
    /**<
    * This struct will be initialized by init() member.
    */

    /// Disable assignment operator.
    MpArrayBuf &operator=(const MpBuf &);
    /**<
    * Buffers may be copied. But do we need this?
    */
};

///  Smart pointer to MpArrayBuf.
/**
*  You should only use this smart pointer, not #MpArrayBuf* itself.
*  The goal of this smart pointer is to care about reference counter and
*  buffer deallocation.
*/
class MpArrayBufPtr : public MpBufPtr {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Default constructor - construct invalid pointer.
    MPBUF_DEFAULT_CONSTRUCTOR(MpArrayBuf)

    /// This constructor owns MpBuf object.
    MPBUF_FROM_BASE_CONSTRUCTOR(MpArrayBuf, MP_BUF_ARRAY, MpBuf)

    /// Copy object from base type with type check.
    MPBUF_TYPECHECKED_COPY(MpArrayBuf, MP_BUF_ARRAY, MpBuf)

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return pointer to MpArrayBuf.
    MPBUF_MEMBER_ACCESS_OPERATOR(MpArrayBuf)

    /// Return readonly pointer to MpArrayBuf.
    MPBUF_CONST_MEMBER_ACCESS_OPERATOR(MpArrayBuf)

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


#endif // _INCLUDED_MPARRAYBUF_H ]
