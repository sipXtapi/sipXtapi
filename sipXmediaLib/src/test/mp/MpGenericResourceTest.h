//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _GENERIC_RESOURCE_TEST_H_ // [
#define _GENERIC_RESOURCE_TEST_H_

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MpMisc.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

/// Number of frames in one frame
#define TEST_DEFAULT_SAMPLES_PER_FRAME 80
/// Number of frames in one second
#define TEST_DEFAULT_SAMPLES_PER_SEC 8000

///  Generic framework for unit test of media resources.
/**
*  For each testing resource create subclass of this class.
*/
class MpGenericResourceTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpGenericResourceTest);
   CPPUNIT_TEST_SUITE_END();

public:

   MpGenericResourceTest()
      : CppUnit::TestCase()
      , mSamplesPerFrame(TEST_DEFAULT_SAMPLES_PER_FRAME)
      , mSamplesPerSec(TEST_DEFAULT_SAMPLES_PER_SEC)
   {}

   // Initialize test framework
   void setUp()
   {
      MpMediaTask*      pMediaTask = NULL;
      OsStatus          res;

      // Setup codec paths..
      UtlString codecPaths[] = {
#ifdef WIN32
                                "bin",
                                "..\\bin",
#elif __pingtel_on_posix__
                                "../../../../bin",
                                "../../../bin",
#else
#                               error "Unknown platform"
#endif
                                "."
      };
      int numCodecPaths = sizeof(codecPaths)/sizeof(codecPaths[0]);

      // Setup media task
      res = mpStartUp(mSamplesPerSec, mSamplesPerFrame, 6*10, 
                      NULL, numCodecPaths, codecPaths);
      CPPUNIT_ASSERT(res == OS_SUCCESS);

      mpFlowGraph = new MpFlowGraphBase(mSamplesPerFrame, mSamplesPerSec);
      CPPUNIT_ASSERT(mpFlowGraph != NULL);

      // Call getMediaTask() which causes the task to get instantiated
      pMediaTask = MpMediaTask::getMediaTask(10);
      CPPUNIT_ASSERT(pMediaTask != NULL);
   }

   // Clean up after test is done.
   void tearDown()
   {
      OsStatus          res;

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

   /**
   *  Get the samples per frame.
   *  
   *  Use this in derived classes as opposed to directly accessing the member, 
   *  as test setup and shutdown may wish to wait until a new test is started 
   *  before a new value set using setSamplesPerFrame is used.
   */
   unsigned getSamplesPerFrame() const
   {
      return mSamplesPerFrame;
   }

   /**
   *  Get the sample rate.
   *  
   *  Use this in derived classes as opposed to directly accessing the member, 
   *  as test setup and shutdown may wish to wait until a new test is started 
   *  before a new value set using setSamplesPerSec is used.
   */
   unsigned getSamplesPerSec() const
   {
      return mSamplesPerSec;
   }

   /**
   *  Set the samples per frame.
   *  
   *  @Note Make sure to put this call after you're finished testing all 
   *  resources in your test at the frame rate previously provided -- i.e. all 
   *  buffers at the old frame rate are flushed through the flowgraph.  The 
   *  flowgraph should do this for you, so this may be a non-issue.
   */
   void setSamplesPerFrame(const unsigned samplesPerFrame)
   {
      mSamplesPerFrame = samplesPerFrame;
   }

   /**
   *  Set the samples per sec.
   *  
   *  @Note Make sure to put this call after you're finished testing all 
   *  resources in your test at the frame rate previously provided -- i.e. all 
   *  buffers at the old frame rate are flushed through the flowgraph.  The 
   *  flowgraph should do this for you, so this may be a non-issue.
   */
   void setSamplesPerSec(const unsigned samplesPerSec)
   {
      mSamplesPerSec = samplesPerSec;
   }

protected:

   MpFlowGraphBase*  mpFlowGraph;
   MpTestResource*   mpSourceResource; ///< Outputs of this resource will be
                                       ///<  connected to inputs of tested
                                       ///<  resource.
   MpTestResource*   mpSinkResource;   ///< Inputs of this resource will be
                                       ///<  connected to outputs of tested
                                       ///<  resource.

   /// Setup generic resource testing framework.
   /**
   *  <ol>
   *  <li> Create source and sink supply resources.
   *  <li> Add all resources to the flowgraph
   *  <li> Let source resource generate buffers and sink resource conceal them.
   *  <li> Link all resources: sourceResource -> testResource -> sinkResource.
   *  <li> Start flowgraph.
   *  <li> Enable sink resource.
   *  </ol>
   */
   void setupFramework(MpResource *pTestResource)
   {
      OsStatus          res;
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
      mpSourceResource = new MpTestResource( "SourceResource"
                                           , 0, 0, inputsCount, inputsCount);
      CPPUNIT_ASSERT(mpSourceResource != NULL);
      mpSinkResource = new MpTestResource( "SinkResource"
                                         , outputsCount, outputsCount, 0, 0);
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
      for (i=0; i<inputsCount; i++) {
         res = mpFlowGraph->addLink(*mpSourceResource, i, *pTestResource, i);
         CPPUNIT_ASSERT(res == OS_SUCCESS);
      }
      for (i=0; i<outputsCount; i++) {
         res = mpFlowGraph->addLink(*pTestResource, i, *mpSinkResource, i);
         CPPUNIT_ASSERT(res == OS_SUCCESS);
      }

      // 5. Start the flow graph
      res = mpFlowGraph->start();
      CPPUNIT_ASSERT(res == OS_SUCCESS);

      // All resources should be disabled
      CPPUNIT_ASSERT(  !mpSourceResource->isEnabled()
                    && !pTestResource->isEnabled()
                    && !mpSinkResource->isEnabled());

      // 6. Enable sink resource
      CPPUNIT_ASSERT(mpSinkResource->enable());
   }


   /// Halt generic resource testing framework.
   /**
   *  <ol>
   *  <li> Stop flowgraph.
   *  </ol>
   */
   void haltFramework()
   {
      OsStatus          res;

       // Stop the flow graph
       res = mpFlowGraph->stop();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Request processing of another frame so that the STOP_FLOWGRAPH
       // message gets handled
       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   private:
      // Private data
      unsigned mSamplesPerSec;
      unsigned mSamplesPerFrame;
};

//CPPUNIT_TEST_SUITE_REGISTRATION(MpGenericResourceTest);

#endif // _GENERIC_RESOURCE_TEST_H_ ]
