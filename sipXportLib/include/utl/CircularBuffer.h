//
// Copyright (C) 2015 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _CircularBuffer_h_
#define _CircularBuffer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsMutex.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Circular linked array of buffer data
// TBD
class CircularBuffer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
	typedef char ElementType;
/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Default constructor.
	CircularBuffer(unsigned long capacity = 0);
    /**
     * Construct circular buffer of given size in char
     * @param[in] capacity - size of buffer in char
     *
     */
      
	~CircularBuffer();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

	void          initialize(unsigned long capacity);
	bool          write(const ElementType * buffer, unsigned long bufferSize, unsigned long * newSize = 0, 
	                    unsigned long * previousSize = 0);
	bool          fill(ElementType value, unsigned long count, unsigned long * newSize = 0,
	                   unsigned long * previousSize = 0);
	unsigned long extract(ElementType * buffer, unsigned long bufferSize);
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{
	unsigned long getSize();
//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{
//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
	const unsigned char   mItemSize;
	unsigned long         mCapacity;
	unsigned long         mCount;
	OsMutex               mMutex;
	ElementType         * mBufferBegin;
	ElementType         * mBufferEnd;
	ElementType         * mHead;
	ElementType         * mTail;

	void pushBack(ElementType sample);
	void free();
	void eraseBegin(unsigned long count);
};

/* ============================ INLINE METHODS ============================ */
#endif  // _CircularBuffer_h_

