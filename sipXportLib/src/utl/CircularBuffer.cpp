//
// Copyright (C) 2015-2019 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <os/OsIntTypes.h>
#include <utl/CircularBuffer.h>
#include <os/OsSysLog.h>
#include <os/OsLock.h>
#include <os/OsMutex.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// DEFINES
#undef min
#define min(X, Y) (X < Y ? X : Y)

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

CircularBuffer::CircularBuffer(unsigned long capacity)
	: mItemSize(sizeof(ElementType)),
      mCapacity(0),
      mCount(0),
      mMutex(OsMutexBase::Q_FIFO), 
      mBufferBegin(0),
      mBufferEnd(0),
      mHead(0),
      mTail(0)
{
	initialize(capacity);
}

CircularBuffer::~CircularBuffer()
{
	if (mBufferBegin != 0)
		delete mBufferBegin;
}

/* ============================ MANIPULATORS ============================== */

void CircularBuffer::initialize(unsigned long capacity)
{
	OsLock lock(mMutex);

	if (mBufferBegin != 0)
		delete mBufferBegin;

	mCapacity = capacity;

	mBufferBegin = mCapacity != 0 ? new ElementType[mCapacity] : 0;
	mBufferEnd = mBufferBegin + mCapacity;

	mHead = mBufferBegin;
	mTail = mHead;
	mCount = 0;
	
	OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "CircularBuffer::initialize - Capacity: %lu samples",
                  mCapacity);
}

bool CircularBuffer::write(const ElementType * buffer, unsigned long bufferSize, unsigned long * newSize,
	unsigned long * previousSize)
{
#ifdef SIPX_USE_EXCEPTIONS
	try
	{
#endif
		OsLock lock(mMutex);

		if (previousSize != 0)
			*previousSize = mCount;

		if (newSize != 0)
			*newSize = mCount;

		if (mBufferBegin == 0)
			return true;

		// if asked to write too much, write the tail only
		if (bufferSize > mCapacity)
		{
			buffer = buffer + bufferSize - mCapacity;
			bufferSize = mCapacity;
		}

		//// first, make room if needed
		unsigned long availableSlots = mCapacity - mCount;
		if (bufferSize > availableSlots)
			eraseBegin(bufferSize - availableSlots);

		// write first section
		unsigned long firstCount = min(bufferSize, (unsigned long)(mBufferEnd - mTail));
		memcpy(mTail, buffer, firstCount * mItemSize);

		// write second section
		if (bufferSize > firstCount)
		{
			unsigned long secondCount = bufferSize - firstCount;
			memcpy(mBufferBegin, buffer + firstCount, secondCount);
		}

		// update the tail
		mTail = mBufferBegin + ((mTail - mBufferBegin) + bufferSize) % mCapacity;
		mCount += bufferSize;

		if (newSize != 0)
			*newSize = mCount;

		OsSysLog::add(FAC_MP, PRI_DEBUG,
			"CircularBuffer::write - Current size: %lu samples",
			mCount);

		return true;
#ifdef SIPX_USE_EXCEPTIONS
	}
	catch (...)
	{
		OsSysLog::add(FAC_MP, PRI_ERR, 
                      "CircularBuffer::write - Exception caught");
		return false;
	}
#endif
}

bool CircularBuffer::fill(ElementType value, unsigned long count, unsigned long * newSize,
	unsigned long * previousSize)
{
#ifdef SIPX_USE_EXCEPTIONS
	try
	{
#endif
		OsLock lock(mMutex);

		if (previousSize != 0)
			*previousSize = mCount;

		for (unsigned long i = 0; i < count; i++)
			pushBack(value);

		if (newSize != 0)
			*newSize = mCount;

		OsSysLog::add(FAC_MP, PRI_DEBUG,
			"CircularBuffer::fill - Current size: %lu samples",
			mCount);

		return true;
#ifdef SIPX_USE_EXCEPTIONS
	}
	catch (...)
	{
		OsSysLog::add(FAC_MP, PRI_ERR,
			"CircularBuffer::fill - Exception caught");
		return false;
	}
#endif
}

void CircularBuffer::pushBack(ElementType sample)
{
	if (mBufferBegin == 0)
		return;

	if (mCount == mCapacity)
		eraseBegin(1);

	*mTail++ = sample;
	if (mTail - mBufferBegin == (int)mCapacity)
		mTail = mBufferBegin;

	mCount++;
}

unsigned long CircularBuffer::extract(ElementType * buffer, unsigned long bufferSize)
{
#ifdef SIPX_USE_EXCEPTIONS
	try
	{
#endif
		OsLock lock(mMutex);

		unsigned long dataSize = min(bufferSize, mCount);
		if (dataSize > 0)
		{
			// first section
			unsigned long rightSlots = mCapacity - (mHead - mBufferBegin);
			memcpy(buffer, mHead, min(dataSize, rightSlots) * mItemSize);

			// 2nd section
			if (dataSize > rightSlots)
				memcpy(buffer + rightSlots, mBufferBegin, (dataSize - rightSlots) * mItemSize);
			
			eraseBegin (dataSize);
		}
		return dataSize;
#ifdef SIPX_USE_EXCEPTIONS
	}
	catch (...)
	{
		OsSysLog::add(FAC_MP, PRI_ERR, 
                      "CircularBuffer::extract - Exception caught");
		return 0;
	}
#endif
}

void CircularBuffer::eraseBegin(unsigned long count)
{
	mHead = mBufferBegin + ((mHead - mBufferBegin) + count) % mCapacity;

	mCount -= count;
	if (mCount < 0)
		count = 0;

	if (mCount == 0)
		mTail = mHead;
}

/* ============================ ACCESSORS ================================= */

unsigned long CircularBuffer::getSize()
{
	OsLock lock(mMutex);
	return mCount;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

