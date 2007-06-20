//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpBuf.h>
#include <mp/MpArrayBuf.h>
#include <mp/MpDataBuf.h>
#include <mp/MpAudioBuf.h>

/**
 * Unittest for MpBuf and its successors
 */
class MpBufTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpBufTest);
   CPPUNIT_TEST(testCreators);
   CPPUNIT_TEST(testAudioBuffersWriteData);
   CPPUNIT_TEST(testDataBuffersExchangeData);
   CPPUNIT_TEST(testDataBuffersAssignment);
   CPPUNIT_TEST(testTypesafeCasts);
   CPPUNIT_TEST(testCloningAllTypes);
   CPPUNIT_TEST(testCloningWithDataCheck);
   CPPUNIT_TEST(testRequestWrite);
   CPPUNIT_TEST_SUITE_END();

#define BUFFER_SIZE   100
#define BUFFER_NUM    5

public:
   void setUp()
   {
      // Create pool for data buffers
      mpPool = new MpBufPool(BUFFER_SIZE, BUFFER_NUM);
      CPPUNIT_ASSERT(mpPool != NULL);

      // Create pool for buffer headers
      mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), BUFFER_NUM);
      CPPUNIT_ASSERT(mpHeadersPool != NULL);

      // Set mpHeadersPool as default pool for audio and data pools.
      MpAudioBuf::smpDefaultPool = mpHeadersPool;
      MpDataBuf::smpDefaultPool = mpHeadersPool;
   }

   void tearDown()
   {
      if (mpPool != NULL)
         delete mpPool;

      if (mpHeadersPool != NULL)
         delete mpHeadersPool;
   }

   void testCreators()
   {
      // Allocate (BUFFER_NUM+1) buffers. First BUFFER_NUM must be valid, other
      // must not be valid.

      MpBufPtr p1 = mpPool->getBuffer();
      CPPUNIT_ASSERT(p1.isValid());

      MpBufPtr p2 = mpPool->getBuffer();
      CPPUNIT_ASSERT(p2.isValid());

      MpBufPtr p3 = mpPool->getBuffer();
      CPPUNIT_ASSERT(p3.isValid());

      MpBufPtr p4 = mpPool->getBuffer();
      CPPUNIT_ASSERT(p4.isValid());

      MpBufPtr p5 = mpPool->getBuffer();
      CPPUNIT_ASSERT(p5.isValid());

      MpBufPtr p6 = mpPool->getBuffer();
      CPPUNIT_ASSERT(!p6.isValid());
   }

   void testAudioBuffersWriteData()
   {
      // Allocate two audio buffers and write data to them.
      unsigned i;

      MpAudioSample *pSamples;

      // Create buffer 1
      MpAudioBufPtr buf1 = mpPool->getBuffer();
      CPPUNIT_ASSERT(buf1.isValid());

      // Fill buffer 1
      pSamples = buf1->getSamplesWritePtr();
      for (i=0; i<buf1->getSamplesNumber(); i++ ) {
         pSamples[i] = (MpAudioSample)0xFACE;
      }

      // Create buffer 2
      MpAudioBufPtr buf2 = mpPool->getBuffer();
      CPPUNIT_ASSERT(buf2.isValid());

      // Fill buffer 2
      pSamples = buf2->getSamplesWritePtr();
      for (i=0; i<buf2->getSamplesNumber(); i++ ) {
         pSamples[i] = (MpAudioSample)0xCAFE;
      }

      // Check buffer 1
      pSamples = buf1->getSamplesWritePtr();
      for (i=0; i<buf1->getSamplesNumber(); i++ ) {
         CPPUNIT_ASSERT(pSamples[i] == (MpAudioSample)0xFACE);
      }

      // Check buffer 2
      pSamples = buf2->getSamplesWritePtr();
      for (i=0; i<buf2->getSamplesNumber(); i++ ) {
         CPPUNIT_ASSERT(pSamples[i] == (MpAudioSample)0xCAFE);
      }
   }

   void testDataBuffersExchangeData()
   {
      // Allocate two data buffers and exchange data between them.

      // Create buffers
      MpDataBufPtr buf1 = mpPool->getBuffer();
      CPPUNIT_ASSERT(buf1.isValid());
      MpDataBufPtr buf2 = mpPool->getBuffer();
      CPPUNIT_ASSERT(buf2.isValid());

      // Copy data from one to other:
      buf2->setData(buf1->getData());

      // Only pointer should be copied.
      CPPUNIT_ASSERT(buf1->getData() == buf2->getData());
   }

   void testDataBuffersAssignment()
   {
      // Allocate two data buffers and assign one to other.

      // Create buffers
      MpDataBufPtr buf1 = mpPool->getBuffer();
      CPPUNIT_ASSERT(buf1.isValid());
      MpDataBufPtr buf2 = mpPool->getBuffer();
      CPPUNIT_ASSERT(buf2.isValid());

      // Check assignment operator
      buf1 = buf2;

      // Only pointer should be copied.
      CPPUNIT_ASSERT(buf1 == buf2);
   }

   void testTypesafeCasts()
   {
      // *** I do not know how to implement this test with CPPUnit... ***

      MpBufPtr parent;

      // Create buffer 1
      MpAudioBufPtr buf1 = mpPool->getBuffer();
      CPPUNIT_ASSERT(buf1.isValid());

      MpBufPtr parentCopy(buf1);

      parent = buf1;

      MpAudioBufPtr same(parent);

      MpDataBufPtr middle(parent);

      MpAudioBufPtr fromMiddle(middle);

      fromMiddle = middle;

      // This will fail:
//      MpArrayBufPtr wrong(parent);
   }

   void testCloningAllTypes()
   {
      // This test is supposed to be compile-only test.
      // That is it test does all copy constructors and equality operators
      // could be found by compiler correctly. GCC seems to be the most
      // problematic here.
      {
         MpBufPtr buf1;
         MpBufPtr buf2;
         buf2 = buf1.clone();
         MpBufPtr buf3(buf1.clone());
      }

      {
         MpArrayBufPtr buf1;
         MpArrayBufPtr buf2;
         buf2 = buf1.clone();
         MpArrayBufPtr buf3(buf1.clone());
      }

      {
         MpDataBufPtr buf1;
         MpDataBufPtr buf2;
         buf2 = buf1.clone();
         MpDataBufPtr buf3(buf1.clone());
      }

      {
         MpAudioBufPtr buf1;
         MpAudioBufPtr buf2;
         buf2 = buf1.clone();
         MpAudioBufPtr buf3(buf1.clone());
      }
   }

   void testCloningWithDataCheck()
   {
      // Allocate audio buffer and make its clone.
      unsigned i;

      MpAudioSample *pSamples;
      MpAudioBufPtr buf1;
      MpAudioBufPtr buf2;

      // Create clone of invalid pointer
      CPPUNIT_ASSERT(!buf1.isValid());
      buf2 = buf1.clone();
      CPPUNIT_ASSERT(!buf1.isValid());
      CPPUNIT_ASSERT(!buf2.isValid());

      // Get real buffer
      buf1 = mpPool->getBuffer();
      CPPUNIT_ASSERT(buf1.isValid());

      // Fill buffer 1
      pSamples = buf1->getSamplesWritePtr();
      for (i=0; i<buf1->getSamplesNumber(); i++ ) {
         pSamples[i] = (MpAudioSample)0xFACE;
      }

      // Clone buffer 1 to buffer 2
      buf2 = buf1.clone();
      CPPUNIT_ASSERT(buf2.isValid());

      // Clone must be located in different buffer.
      CPPUNIT_ASSERT(buf1 != buf2);

      // Data of the clone must be located in different buffer.
      CPPUNIT_ASSERT(buf1->getData() != buf2->getData());

      // Check buffer 2: its data should be equal to the original data
      pSamples = buf2->getSamplesWritePtr();
      for (i=0; i<buf2->getSamplesNumber(); i++ ) {
         CPPUNIT_ASSERT(pSamples[i] == (MpAudioSample)0xFACE);
      }
   }

   void testRequestWrite()
   {
      MpAudioBufPtr buf1;
      MpAudioBufPtr buf2;

      // For invalid buffer requestWrite() should just return false.
      CPPUNIT_ASSERT(!buf1.requestWrite());

      // Get real buffer
      buf1 = mpPool->getBuffer();
      CPPUNIT_ASSERT(buf1.isValid());

      // With only one reference requestWrite() should just return true.
      CPPUNIT_ASSERT(buf1.requestWrite());

      // Buffer now have two references.
      buf2 = buf1;

      // This should create copy and return true.
      CPPUNIT_ASSERT(buf1.requestWrite());
      CPPUNIT_ASSERT(buf1.isValid());
      CPPUNIT_ASSERT(buf2.isValid());
      CPPUNIT_ASSERT(buf1 != buf2);
   }

protected:
   MpBufPool *mpPool;         ///< Pool for data buffers
   MpBufPool *mpHeadersPool;  ///< Pool for buffers headers

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpBufTest);
