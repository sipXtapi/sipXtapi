//  
// Copyright (C) 2006-2010 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006-2017 SIPez LLC. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include "mp/MpGenericResourceTest.h"

// Setup codec paths..
#include <../test/mp/MpTestCodecPaths.h>

// Static data initialization
// Sample rates that can be used for wideband testing.
unsigned MpGenericResourceTest::sSampleRates[] = {8000, 16000, 32000, 48000};
unsigned MpGenericResourceTest::sNumRates = sizeof(MpGenericResourceTest::sSampleRates)/sizeof(unsigned);

MpGenericResourceTest::MpGenericResourceTest()
   : SIPX_UNIT_BASE_CLASS ()
   , mSamplesPerSec(TEST_DEFAULT_SAMPLES_PER_SEC)
   , mSamplesPerFrame(TEST_DEFAULT_SAMPLES_PER_FRAME)
{
}

// Initialize test framework
void MpGenericResourceTest::setUp()
{
   // Just in case the test aborted without doing a shut down, try to shut things down
   OsStatus res = mpShutdown();

   MpMediaTask* pMediaTask = NULL;

   // Setup codec paths..
   // Setup media task
   res = mpStartUp(mSamplesPerSec, mSamplesPerFrame, 512,
                   NULL, sNumCodecPaths, sCodecPaths);
   CPPUNIT_ASSERT(res == OS_SUCCESS);

   mpFlowGraph = new MpFlowGraphBase(mSamplesPerFrame, mSamplesPerSec);
   CPPUNIT_ASSERT(mpFlowGraph != NULL);

   // Create MediaTask
   pMediaTask = MpMediaTask::createMediaTask(10);
   CPPUNIT_ASSERT(pMediaTask != NULL);
}

// Clean up after test is done.
void MpGenericResourceTest::tearDown()
{
   OsStatus res;

   // This should normally be done by haltFramework, but if we aborted due
   // to an assertion the flowgraph will need to be shutdown here
   if(mpFlowGraph && mpFlowGraph->isStarted())
   {
       printf("WARNING: flowgraph found still running, shutting down\n");

       // ignore the result and keep going
       mpFlowGraph->stop();

       // Request processing of another frame so that the STOP_FLOWGRAPH
       // message gets handled
       mpFlowGraph->processNextFrame();
   }

   // Free flowgraph resources
   delete mpFlowGraph;
   mpFlowGraph = NULL;
   mpSinkResource = NULL;
   mpSourceResource = NULL;

   // Clear all Media Tasks data
   res = mpShutdown();
   CPPUNIT_ASSERT(res == OS_SUCCESS);
}

unsigned MpGenericResourceTest::getSamplesPerFrame() const
{
   return mSamplesPerFrame;
}

unsigned MpGenericResourceTest::getSamplesPerSec() const
{
   return mSamplesPerSec;
}

void MpGenericResourceTest::setSamplesPerFrame(const unsigned samplesPerFrame)
{
   mSamplesPerFrame = samplesPerFrame;
}

void MpGenericResourceTest::setSamplesPerSec(const unsigned samplesPerSec)
{
   mSamplesPerSec = samplesPerSec;
}

void MpGenericResourceTest::setupFramework(MpResource *pTestResource)
{
   OsStatus res;
   int i;
   int inputsCount;  // Number of inputs of test resource
   int outputsCount; // Number of outputs of test resource

   // Check prerequisites
   CPPUNIT_ASSERT(mpFlowGraph != NULL);
   CPPUNIT_ASSERT(pTestResource != NULL);

   // For convenience store number of inputs and outputs for test resource.
   inputsCount = pTestResource->maxInputs();
   outputsCount = pTestResource->maxOutputs();

   // 1. Create source and sink supply resources.
   mpSourceResource = new MpTestResource("SourceResource", 0, 0, 
                                         inputsCount, inputsCount);
   CPPUNIT_ASSERT(mpSourceResource != NULL);
   mpSinkResource = new MpTestResource("SinkResource",
                                       outputsCount, outputsCount, 0, 0);
   CPPUNIT_ASSERT(mpSinkResource != NULL);

   // 2. Add all resources to the flowgraph.
   res = mpFlowGraph->addResource(*mpSourceResource);
   CPPUNIT_ASSERT(res == OS_SUCCESS);
   res = mpFlowGraph->addResource(*pTestResource);
   CPPUNIT_ASSERT(res == OS_SUCCESS);
   res = mpFlowGraph->addResource(*mpSinkResource);
   CPPUNIT_ASSERT(res == OS_SUCCESS);

   // 3. For source resource, create new buffers on all output ports and
   // ignore all input buffers
   mpSourceResource->setProcessInBufMask(0x0);
   mpSourceResource->setGenOutBufMask((1<<inputsCount)-1);

   // For sink resource, process input buffers that arrive input ports.
   mpSinkResource->setProcessInBufMask((1<<outputsCount)-1);
   mpSinkResource->setGenOutBufMask(0x0);

   // 4. Link sourceResource -> testResource -> sinkResource
   for (i=0; i<inputsCount; i++) 
   {
      res = mpFlowGraph->addLink(*mpSourceResource, i, *pTestResource, i);
      CPPUNIT_ASSERT(res == OS_SUCCESS);
   }
   for (i=0; i<outputsCount; i++) 
   {
      res = mpFlowGraph->addLink(*pTestResource, i, *mpSinkResource, i);
      CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   // 5. Start the flow graph
   res = mpFlowGraph->start();
   CPPUNIT_ASSERT(res == OS_SUCCESS);

   // All resources should be disabled
   CPPUNIT_ASSERT(!mpSourceResource->isEnabled()
                  && !pTestResource->isEnabled()
                  && !mpSinkResource->isEnabled());

   // 6. Enable sink resource
   CPPUNIT_ASSERT(mpSinkResource->enable());
}


void MpGenericResourceTest::haltFramework()
{
   OsStatus res;

    // Stop the flow graph
    res = mpFlowGraph->stop();
    CPPUNIT_ASSERT(res == OS_SUCCESS);

    // Request processing of another frame so that the STOP_FLOWGRAPH
    // message gets handled
    res = mpFlowGraph->processNextFrame();
    CPPUNIT_ASSERT(res == OS_SUCCESS);

    CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->destroyResources());
}
