//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MpMisc.h>
#include <mp/MprSplitter.h>
#include <mp/MpBufferMsg.h>

#include "mp/MpGenericResourceTest.h"

///  Unit test for MprSplitter
class MprSplitterTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUB_SUITE(MprSplitterTest, MpGenericResourceTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testDisabled);
    CPPUNIT_TEST(testEnabledNoData);
    CPPUNIT_TEST(testEnabledWithData);
    CPPUNIT_TEST_SUITE_END();

public:

   void testCreators()
   {
       MprSplitter*      pSplitter  = NULL;
       OsStatus          res;

       // when we have a flow graph that contains resources and links,
       // verify that destroying the flow graph also gets rid of the resources
       // and links.
       pSplitter = new MprSplitter("MprSplitter", 2);

       res = mpFlowGraph->addResource(*pSplitter);
       CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   void testDisabled()
   {
       MprSplitter*      pSplitter  = NULL;
       OsStatus          res;

       pSplitter = new MprSplitter("MprSplitter", 2);
       CPPUNIT_ASSERT(pSplitter != NULL);

       setupFramework(pSplitter);

       // TESTCASE 1:
       // pSplitter disabled, there are no buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pSplitter->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[1].isValid()
                     );

       // TESTCASE 2:
       // pSplitter disabled, there are buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pSplitter->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer should be passed through to the output 0.
       CPPUNIT_ASSERT(  mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && (mpSourceResource->mLastDoProcessArgs.outBufs[0] ==
                         mpSinkResource->mLastDoProcessArgs.inBufs[0])
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[1].isValid()
                     );

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledNoData()
   {
       MprSplitter*      pSplitter  = NULL;
       OsStatus          res;

       pSplitter = new MprSplitter("MprSplitter", 2);
       CPPUNIT_ASSERT(pSplitter != NULL);

       setupFramework(pSplitter);

       // pSplitter enabled, there are no buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pSplitter->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // No buffers processed
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[1].isValid()
                     );

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledWithData()
   {
      MprSplitter* pSplitter = NULL;
      MpBufPtr pBuf;
      OsStatus res;

      size_t i;
      for(i = 0; i < sNumRates; i++)
      {
         printf("Test %d Hz\n", sSampleRates[i]);

         // For this test, we want to modify the sample rate and samples per frame
         // so we need to de-inititialize what has already been initialized for us
         // by cppunit, or by a previous loop.
         tearDown();

         // Set the sample rates 
         setSamplesPerSec(sSampleRates[i]);
         setSamplesPerFrame(sSampleRates[i]/100);
         setUp();

         // Set up the splitter and framework
         pSplitter = new MprSplitter("MprMixer", 2);
         CPPUNIT_ASSERT(pSplitter != NULL);
         setupFramework(pSplitter);

         // pSplitter enabled
         CPPUNIT_ASSERT(mpSourceResource->enable());
         CPPUNIT_ASSERT(pSplitter->enable());

         res = mpFlowGraph->processNextFrame();
         CPPUNIT_ASSERT(res == OS_SUCCESS);

         // Store input buffer for convenience
         pBuf = mpSourceResource->mLastDoProcessArgs.outBufs[0];
         CPPUNIT_ASSERT(pBuf.isValid());

          // Buffer is sent to all outputs
          CPPUNIT_ASSERT(   (mpSinkResource->mLastDoProcessArgs.inBufs[0] == pBuf)
                         && (mpSinkResource->mLastDoProcessArgs.inBufs[1] == pBuf)
                        );

         // Free stored buffer
         pBuf.release();

         // Stop flowgraph
         haltFramework();

         // No need to delete splitter, as haltFramework deletes all resources
         // in the flowgraph.
      }
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MprSplitterTest);
