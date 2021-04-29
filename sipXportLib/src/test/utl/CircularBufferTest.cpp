//  
// Copyright (C) 2006-2010 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <os/OsIntTypes.h>
#include <sipxunittests.h>
#include <utl/CircularBuffer.h>

/**
 * Unittest for CircularBuffer
 */
class CircularBufferTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(CircularBufferTest);
   CPPUNIT_TEST(testWrite_WithZeroCapacity_BufferRemainsEmpty);
   CPPUNIT_TEST(testWrite_WithOneItemOnEmptyBuffer_TheSizeBecomesOne);
   CPPUNIT_TEST(testWrite_WithOneItemOnEmptyBuffer_TheItemIsInserted);
   CPPUNIT_TEST(testWrite_WithTwoItemsOnEmptyBuffer_TheSizeBecomesTwo);
   CPPUNIT_TEST(testWrite_WithTwoItemsOnEmptyBuffer_TheItemsAreInserted);
   CPPUNIT_TEST(testWrite_WithTwoItemsOnBufferWithCapacityOne_TheSizeBecomesOne);
   CPPUNIT_TEST(testWrite_WithTwoItemsOnBufferWithCapacityOne_TheBufferContainsOnlyTheSecondItem);
   CPPUNIT_TEST(testWrite_WithThreeItemsOnBufferWithCapacityTwo_TheSizeBecomesTwo);
   CPPUNIT_TEST(testWrite_WithThreeItemsOnBufferWithCapacityTwo_TheBufferContainsOnlyTheSecondAndThirdItems);
   CPPUNIT_TEST(testWrite_WithTenItemsOnBufferWithCapacityTwo_TheBufferContainsOnlyTheLastTwoInsertedItems);
   CPPUNIT_TEST(testExtract_OneItemOutOfAOneItemBuffer_TheBufferBecomesEmpty);
   CPPUNIT_TEST(testWrite_OneItemAfterExtractingAll_TheBufferSizeBecomesOne);
   CPPUNIT_TEST(testWrite_OneItemAfterExtractingAll_TheBufferContainsThatItem);
   CPPUNIT_TEST(testFill);
   CPPUNIT_TEST_SUITE_END();

public:
   void setUp()
   {
	   _circularBuffer = new CircularBuffer();
	   CPPUNIT_ASSERT(_circularBuffer != NULL);
   }

   void tearDown()
   {
	   if (_circularBuffer != NULL)
		   delete _circularBuffer;
   }

   void testWrite_WithZeroCapacity_BufferRemainsEmpty()
   {
	   // arrange
	   CircularBuffer::ElementType samples[2] = { 0x11, 0x22 };

	   // act
	   _circularBuffer->write(samples, 2);

	   // assert
	   CPPUNIT_ASSERT(_circularBuffer->getSize() == 0);
   }

   void testWrite_WithOneItemOnEmptyBuffer_TheSizeBecomesOne()
   {
	   // arrange
	   _circularBuffer->initialize(1);
	   CircularBuffer::ElementType samples[1] = { 0x11 };

	   // act
	   _circularBuffer->write(samples, 1);

	   // assert
	   CPPUNIT_ASSERT(_circularBuffer->getSize() == 1);
   }

   void testWrite_WithTwoItemsOnEmptyBuffer_TheSizeBecomesTwo()
   {
	   // arrange
	   _circularBuffer->initialize(2);
	   CircularBuffer::ElementType samples[2] = { 0x11, 0x22 };

	   // act
	   _circularBuffer->write(samples, 2);

	   // assert
	   CPPUNIT_ASSERT(_circularBuffer->getSize() == 2);
   }

   void testWrite_WithOneItemOnEmptyBuffer_TheItemIsInserted()
   {
	   // arrange
	   _circularBuffer->initialize(1);
	   CircularBuffer::ElementType samples[1] = { 0x11 };

	   // act
	   _circularBuffer->write(samples, 1);

	   // assert
	   CircularBuffer::ElementType extractedSamples[1] = { 0 };
	   _circularBuffer->extract(extractedSamples, 1);
	   CPPUNIT_ASSERT(extractedSamples[0] == 0x11);
   }

   void testWrite_WithTwoItemsOnEmptyBuffer_TheItemsAreInserted()
   {
	   // arrange
	   _circularBuffer->initialize(2);
	   CircularBuffer::ElementType samples[2] = { 0x11, 0x22 };

	   // act
	   _circularBuffer->write(samples, 2);

	   // assert
	   CircularBuffer::ElementType extractedSamples[2] = { 0, 0 };
	   _circularBuffer->extract(extractedSamples, 2);
	   CPPUNIT_ASSERT(extractedSamples[0] == 0x11 && extractedSamples[1] == 0x22);
   }

   void testWrite_WithTwoItemsOnBufferWithCapacityOne_TheSizeBecomesOne()
   {
	   // arrange
	   _circularBuffer->initialize(1);
	   CircularBuffer::ElementType samples[2] = { 0x11, 0x22 };

	   // act
	   _circularBuffer->write(samples, 2);

	   // assert
	   CPPUNIT_ASSERT(_circularBuffer->getSize() == 1);
   }

   void testWrite_WithTwoItemsOnBufferWithCapacityOne_TheBufferContainsOnlyTheSecondItem()
   {
	   // arrange
	   _circularBuffer->initialize(1);
	   CircularBuffer::ElementType samples[2] = { 0x11, 0x22 };

	   // act
	   _circularBuffer->write(samples, 2);

	   // assert
	   CircularBuffer::ElementType extractedSamples[1] = { 0 };
	   _circularBuffer->extract(extractedSamples, 1);
	   CPPUNIT_ASSERT(extractedSamples[0] == 0x22);
   }

   void testWrite_WithThreeItemsOnBufferWithCapacityTwo_TheSizeBecomesTwo()
   {
	   // arrange
	   _circularBuffer->initialize(2);
	   CircularBuffer::ElementType samples[3] = { 0x11, 0x22, 0x33 };

	   // act
	   _circularBuffer->write(samples, 3);

	   // assert
	   CPPUNIT_ASSERT(_circularBuffer->getSize() == 2);
   }

   void testWrite_WithThreeItemsOnBufferWithCapacityTwo_TheBufferContainsOnlyTheSecondAndThirdItems()
   {
	   // arrange
	   _circularBuffer->initialize(2);
	   CircularBuffer::ElementType samples[3] = { 0x11, 0x22, 0x33 };

	   // act
	   _circularBuffer->write(samples, 3);

	   // assert
	   CircularBuffer::ElementType extractedSamples[2] = { 0, 0 };
	   _circularBuffer->extract(extractedSamples, 2);
	   CPPUNIT_ASSERT(extractedSamples[0] == 0x22 && extractedSamples[1] == 0x33);
   }

   void testWrite_WithTenItemsOnBufferWithCapacityTwo_TheBufferContainsOnlyTheLastTwoInsertedItems()
   {
	   // arrange
	   _circularBuffer->initialize(2);
	   CircularBuffer::ElementType samples[10] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10 };

	   // act
	   _circularBuffer->write(samples, 10);

	   // assert
	   CircularBuffer::ElementType extractedSamples[2] = { 0, 0 };
	   _circularBuffer->extract(extractedSamples, 2);
	   CPPUNIT_ASSERT(extractedSamples[0] == 0x09 && extractedSamples[1] == 0x10);
   }

   void testExtract_OneItemOutOfAOneItemBuffer_TheBufferBecomesEmpty()
   {
	   // arrange
	   _circularBuffer->initialize(1);
	   CircularBuffer::ElementType samples[1] = { 0x11 };
	   _circularBuffer->write(samples, 1);
	   
	   // act
	   CircularBuffer::ElementType extractedSamples[1] = { 0 };
	   _circularBuffer->extract(extractedSamples, 1);

	   // assert
	   CPPUNIT_ASSERT(_circularBuffer->getSize() == 0);
   }

   void testWrite_OneItemAfterExtractingAll_TheBufferSizeBecomesOne()
   {
	   // arrange
	   _circularBuffer->initialize(1);
	   CircularBuffer::ElementType samples[1] = { 0x11 };
	   _circularBuffer->write(samples, 1);
	   CircularBuffer::ElementType extractedSamples[1] = { 0 };
	   _circularBuffer->extract(extractedSamples, 1);

	   // act
	   _circularBuffer->write(samples, 1);

	   // assert
	   CPPUNIT_ASSERT(_circularBuffer->getSize() == 1);
   }

   void testWrite_OneItemAfterExtractingAll_TheBufferContainsThatItem()
   {
	   // arrange
	   _circularBuffer->initialize(1);
	   CircularBuffer::ElementType samples[1] = { 0x11 };
	   _circularBuffer->write(samples, 1);
	   CircularBuffer::ElementType extractedSamples[1] = { 0 };
	   _circularBuffer->extract(extractedSamples, 1);

	   // act
	   CircularBuffer::ElementType samples2[1] = { 0x22 };
	   _circularBuffer->write(samples2, 1);

	   // assert
	   CircularBuffer::ElementType extractedSamples2[1] = { 0 };
	   _circularBuffer->extract(extractedSamples2, 1);
	   CPPUNIT_ASSERT(extractedSamples2[0] == 0x22);
   }

   void testFill()
   {
	   // arrange
	   _circularBuffer->initialize(2);

	   // act
	   _circularBuffer->fill(0x11, 3);

	   // assert
	   CircularBuffer::ElementType extractedSamples[2] = { 0, 0 };
	   _circularBuffer->extract(extractedSamples, 2);
	   CPPUNIT_ASSERT(extractedSamples[0] == 0x11 && extractedSamples[1] == 0x11);
   }

protected:
   CircularBuffer * _circularBuffer;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CircularBufferTest);
