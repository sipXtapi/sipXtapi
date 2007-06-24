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
#include <mp/MprMixer.h>
#include <mp/MpBufferMsg.h>

#include "mp/MpGenericResourceTest.h"

///  Unit test for MprMixer
class MprMixerTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUB_SUITE(MprMixerTest, MpGenericResourceTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testDisabled);
    CPPUNIT_TEST(testMuted);
    CPPUNIT_TEST(testEnabledNoData);
    CPPUNIT_TEST(testEnabledWithOneWeight);
    CPPUNIT_TEST(testEnabledWithManyWeights);
    CPPUNIT_TEST_SUITE_END();

public:

   void testCreators()
   {
       MprMixer*         pMixer     = NULL;
       OsStatus          res;

       // when we have a flow graph that contains resources and links,
       // verify that destroying the flow graph also gets rid of the resources
       // and links.
       pMixer = new MprMixer("MprMixer", 2,
                             TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);

       res = mpFlowGraph->addResource(*pMixer);
       CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   void testDisabled()
   {
       MprMixer*         pMixer     = NULL;
       OsStatus          res;

       pMixer = new MprMixer("MprMixer", 2,
                             TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pMixer != NULL);

       setupFramework(pMixer);

       // TESTCASE 1:
       // pMixer disabled, there are no buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pMixer->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSourceResource->mLastDoProcessArgs.outBufs[1].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid()
                     );

       // TESTCASE 2:
       // pMixer disabled, there are buffers on the input 0 and 1.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pMixer->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer from input 0 should be passed through to the output.
       CPPUNIT_ASSERT(mpSourceResource->mLastDoProcessArgs.outBufs[0] ==
                      mpSinkResource->mLastDoProcessArgs.inBufs[0]);

       // Stop flowgraph
       haltFramework();
   }

   void testMuted()
   {
       MprMixer*         pMixer     = NULL;
       OsStatus          res;

       pMixer = new MprMixer("MprMixer", 2,
                             TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pMixer != NULL);

       setupFramework(pMixer);

       // pMixer enabled, there are buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pMixer->enable());

       // Set all weights to 0
       CPPUNIT_ASSERT(pMixer->setWeight(0,0));
       CPPUNIT_ASSERT(pMixer->setWeight(0,1));

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // No buffers should be processed
       CPPUNIT_ASSERT(  mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && mpSourceResource->mLastDoProcessArgs.outBufs[1].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid()
                     );

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledNoData()
   {
       MprMixer*         pMixer     = NULL;
       OsStatus          res;

       pMixer = new MprMixer("MprMixer", 2,
                             TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pMixer != NULL);

       setupFramework(pMixer);

       // pMixer enabled, there are no buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pMixer->enable());

       // Set all weights to 1
       CPPUNIT_ASSERT(pMixer->setWeight(1,0));
       CPPUNIT_ASSERT(pMixer->setWeight(1,1));

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Mixer generate new buffer even if no input is available.
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSourceResource->mLastDoProcessArgs.outBufs[1].isValid()
                     && mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid()
                     );

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledWithOneWeight()
   {
       MprMixer*         pMixer     = NULL;
       OsStatus          res;

       pMixer = new MprMixer("MprMixer", 2,
                             TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pMixer != NULL);

       setupFramework(pMixer);

       // pMixer enabled, there are buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pMixer->enable());

       // Set only first weight to 1
       CPPUNIT_ASSERT(pMixer->setWeight(1,0));
       CPPUNIT_ASSERT(pMixer->setWeight(0,1));

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer from input 0 should be passed through to the output.
       CPPUNIT_ASSERT(mpSourceResource->mLastDoProcessArgs.outBufs[0] ==
                      mpSinkResource->mLastDoProcessArgs.inBufs[0]);

       // Set only second weight to 1
       CPPUNIT_ASSERT(pMixer->setWeight(0,0));
       CPPUNIT_ASSERT(pMixer->setWeight(1,1));

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer from input 1 should be passed through to the output.
       CPPUNIT_ASSERT(mpSourceResource->mLastDoProcessArgs.outBufs[1] ==
                      mpSinkResource->mLastDoProcessArgs.inBufs[0]);

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledWithManyWeights()
   {
       MprMixer*         pMixer     = NULL;
       MpBufPtr          pBuf;
       OsStatus          res;

       pMixer = new MprMixer("MprMixer", 2,
                             TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pMixer != NULL);

       setupFramework(pMixer);

       // pMixer enabled, there are buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pMixer->enable());

       // Set all weights to 1
       CPPUNIT_ASSERT(pMixer->setWeight(1,0));
       CPPUNIT_ASSERT(pMixer->setWeight(1,1));

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Store output buffer for convenience
       pBuf = mpSinkResource->mLastDoProcessArgs.inBufs[0];

       // New buffer should be generated.
       CPPUNIT_ASSERT(  pBuf.isValid()
                     && (mpSourceResource->mLastDoProcessArgs.outBufs[0] != pBuf)
                     && (mpSourceResource->mLastDoProcessArgs.outBufs[1] != pBuf)
                     );

       // Free stored buffer
       pBuf.release();

       // Stop flowgraph
       haltFramework();
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(MprMixerTest);
