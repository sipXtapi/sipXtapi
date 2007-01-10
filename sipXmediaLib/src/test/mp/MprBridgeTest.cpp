//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MpMisc.h>
#include <mp/MprBridge.h>
#include <mp/MpBufferMsg.h>

#include "mp/MpGenericResourceTest.h"

///  Unit test for MprBridge
class MprBridgeTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUB_SUITE(MprBridgeTest, MpGenericResourceTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testDisabled);
    CPPUNIT_TEST(testEnabledNoData);
    CPPUNIT_TEST(testEnabledWithOneActiveInput);
    CPPUNIT_TEST(testEnabledWithManyActiveInputs);
    CPPUNIT_TEST_SUITE_END();

public:

   void testCreators()
   {
       MprBridge*        pBridge    = NULL;
       OsStatus          res;

       // when we have a flow graph that contains resources and links,
       // verify that destroying the flow graph also gets rid of the resources
       // and links.
       pBridge = new MprBridge("MprBridge",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);

       res = mpFlowGraph->addResource(*pBridge);
       CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   void testDisabled()
   {
       MprBridge*        pBridge    = NULL;
       MpBufPtr          pBuf;
       OsStatus          res;

       pBridge = new MprBridge("MprBridge",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       // TESTCASE 1:
       // pBridge disabled, there are no buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pBridge->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers on remote parts
       for (int i=1; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[i].isValid());

       // Mixer does not generate data on the local output, cause it can't find
       // any active audio.
       CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // TESTCASE 2:
       // pBridge disabled, there are buffers on all inputs.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pBridge->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Local microphone data (input 0) should be copied to all remote parts
       // (outputs 1..MAX_BRIDGE_PORTS)
       pBuf = mpSourceResource->mLastDoProcessArgs.outBufs[0];
       for (int i=1; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[i] == pBuf);

       // Mixer generate data on the local output, cause it try to do mixing.
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // Free stored buffer
       pBuf.release();

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledNoData()
   {
       MprBridge*        pBridge    = NULL;
       OsStatus          res;

       pBridge = new MprBridge("MprBridge",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       // pBridge enabled, there are no buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pBridge->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers on remote parts
       for (int i=1; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[i].isValid());

       // Mixer does not generate data on the local output, cause it can't find
       // any active audio.
       CPPUNIT_ASSERT(!mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledWithOneActiveInput()
   {
       MprBridge*        pBridge    = NULL;
       MpBufPtr          pBuf;
       OsStatus          res;

       pBridge = new MprBridge("MprBridge",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       // pBridge enabled, there are buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pBridge->enable());

       // Generate buffers on outputs 0 (simulate local mic) and 1 (simulate one
       // remote party).
       mpSourceResource->setGenOutBufMask(0x03);

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Local microphone data (input 0) should be copied to the first remote
       // connection (output 1)
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[1] ==
                      mpSourceResource->mLastDoProcessArgs.outBufs[0]);

       // Data from the input 1 (first remote connection) should be copied to
       // the local speaker.
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0] ==
                      mpSourceResource->mLastDoProcessArgs.outBufs[1]);

       // Other remote connections (outputs 2..MAX_BRIDGE_PORTS) should receive
       // mixed audio.
       for (int i=2; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[i].isValid());

       // Free stored buffer
       pBuf.release();

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledWithManyActiveInputs()
   {
       MprBridge*        pBridge    = NULL;
       MpBufPtr          pBuf;
       OsStatus          res;

       pBridge = new MprBridge("MprBridge",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pBridge != NULL);

       setupFramework(pBridge);

       // pBridge enabled, there are buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pBridge->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Store output buffer for convenience
       pBuf = mpSinkResource->mLastDoProcessArgs.inBufs[0];

       // We did not generated any buffers on remote parts
       for (int i=1; i<pBridge->maxOutputs(); i++)
          CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[i].isValid());

       // Mixer does not generate data on the local output, cause it can't find
       // any active audio.
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // Free stored buffer
       pBuf.release();

       // Stop flowgraph
       haltFramework();
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(MprBridgeTest);
