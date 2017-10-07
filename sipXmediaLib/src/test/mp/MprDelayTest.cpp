//  
// Copyright (C) 2008-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#include <os/OsIntTypes.h>
#include <sipxunittests.h>

// Setup codec paths..
#include <../test/mp/MpTestCodecPaths.h>

#include "MpTestResource.h"
#include "mp/MprToneGen.h"
#include "mp/MprSplitter.h"
#include "mp/MprNull.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MpMisc.h"
#include "mp/MpMediaTask.h"
#include "mp/MprDelay.h"

#include "os/OsTask.h"
#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#else
#  define RTL_START(x)
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#  define RTL_WRITE(x)
#  define RTL_STOP
#endif


#define TEST_SAMPLES_PER_FRAME        480    ///< in samples
#define TEST_SAMPLES_PER_SECOND       48000  ///< in samples/sec (Hz)
#define TEST_DELAY_FRAMES             10     ///< Length of test delay buffer.

class MprDelayTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(MprDelayTest);
   CPPUNIT_TEST(testMprDelay);
   CPPUNIT_TEST_SUITE_END();

public:

   // This function will be called before every test to setup framework.
   void setUp()
   {
      //enableConsoleOutput(1);

      // Setup media task
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                           mpStartUp(TEST_SAMPLES_PER_SECOND,
                                     TEST_SAMPLES_PER_FRAME, 3*100, 0,
                                     sNumCodecPaths, sCodecPaths));

      // Create flowgraph
      mpFlowGraph = new MpFlowGraphBase(TEST_SAMPLES_PER_FRAME,
                                        TEST_SAMPLES_PER_SECOND);
      CPPUNIT_ASSERT(mpFlowGraph != NULL);

      // Turn on notifications
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->setNotificationsEnabled(true));
   }

   // This function will be called after every test to clean up framework.
   void tearDown()
   {
      // This should normally be done by haltFramework, but if we aborted due
      // to an assertion the flowgraph will need to be shutdown here
      if (mpFlowGraph && mpFlowGraph->isStarted())
      {
         //osPrintf("WARNING: flowgraph found still running, shutting down\n");

         // ignore the result and keep going
         mpFlowGraph->stop();

         // Request processing of another frame so that the STOP_FLOWGRAPH
         // message gets handled
         mpFlowGraph->processNextFrame();
      }

      // Free flowgraph
      delete mpFlowGraph;
      mpFlowGraph = NULL;

      // Clear all media processing data and delete MpMediaTask instance.
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpShutdown());
   }

   void testMprDelay()
   {
      RTL_START(10000000);
      int i;

      // Create source (input) and sink (output) resources.
      MpTestResource sourceResource("TestSource", 0, 0, 1, 1);
      sourceResource.setGenOutBufMask(1);
      sourceResource.setOutSignalType(MpTestResource::MP_SINE_SAW);
      sourceResource.setSignalAmplitude(0, 16000);
      sourceResource.setSpeechType(0, MP_SPEECH_SILENT);

      MpTestResource sinkResource("TestSink", 1, 1, 0, 0);
      UtlString   buffer;

      MprDelay delayResource("TestDelay", TEST_DELAY_FRAMES);      

      try 
      {
         // Add source and sink resources to flowgraph and link them together.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(sourceResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(sinkResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addResource(delayResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(sourceResource, 0, delayResource, 0));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->addLink(delayResource, 0, sinkResource, 0));

         // Create a queue..
         OsMsg* pMsg;
         OsMsgDispatcher notfDisp;

         // Now we enable the flowgraph..  Which should enable resources.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->enable());
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->start());

         mpFlowGraph->setNotificationDispatcher(&notfDisp);

         // TESTING BEGINS

         // Generate NULL frames
         for (i = 0; i < 2*TEST_DELAY_FRAMES; i++)
         {
            mpFlowGraph->processNextFrame();
            CPPUNIT_ASSERT_EQUAL(0, notfDisp.numMsgs());
         }
         CPPUNIT_ASSERT_EQUAL(0, delayResource.getDelayMs());

         // Generate inactive frames
         sourceResource.setGenOutBufMask(1);
         sourceResource.setSpeechType(0, MP_SPEECH_SILENT);
         for (i = 0; i < 2*TEST_DELAY_FRAMES; i++)
         {
            mpFlowGraph->processNextFrame();
            CPPUNIT_ASSERT_EQUAL(0, notfDisp.numMsgs());
         }
         CPPUNIT_ASSERT_EQUAL(0, delayResource.getDelayMs());

         // Generate first active frame
         sourceResource.setGenOutBufMask(1);
         sourceResource.setSpeechType(0, MP_SPEECH_ACTIVE);
         mpFlowGraph->processNextFrame();
         CPPUNIT_ASSERT_EQUAL(1, notfDisp.numMsgs());
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*&)pMsg, OsTime(10)));
         CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_DELAY_SPEECH_STARTED, 
            (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());

         // Add few frames to check getDelayFrames() correctness.
         for (i = 0; i < TEST_DELAY_FRAMES/2-1; i++)
         {
            if (i%2 == 1)
               sourceResource.setGenOutBufMask(0);
            else
               sourceResource.setGenOutBufMask(1);

            mpFlowGraph->processNextFrame();
            CPPUNIT_ASSERT_EQUAL(0, notfDisp.numMsgs());
         }
         sourceResource.setGenOutBufMask(1);
         CPPUNIT_ASSERT_EQUAL(TEST_DELAY_FRAMES/2, delayResource.getDelayFrames());

         // Start Play
         MprDelay::startPlay("TestDelay", *mpFlowGraph->getMsgQ());

         // Push frames to test behaviour during the play.
         for (i = 0; i < TEST_DELAY_FRAMES; i++)
         {
            if (i%2 == 1)
               sourceResource.setGenOutBufMask(0);
            else
               sourceResource.setGenOutBufMask(1);

            mpFlowGraph->processNextFrame();
            CPPUNIT_ASSERT_EQUAL(0, notfDisp.numMsgs());
         }
         CPPUNIT_ASSERT_EQUAL(TEST_DELAY_FRAMES/2, delayResource.getDelayFrames());

         // Test quiescent state during the play.
         sourceResource.setGenOutBufMask(1);
         sourceResource.setSpeechType(0, MP_SPEECH_ACTIVE);
         for (i = 0; i < TEST_DELAY_FRAMES; i++)
         {
            mpFlowGraph->processNextFrame();
            CPPUNIT_ASSERT_EQUAL(0, notfDisp.numMsgs());
         }
         sourceResource.setSpeechType(0, MP_SPEECH_SILENT);
         int delay = delayResource.getDelayFrames();
         for (i = 0; i < delay; i++)
         {
            CPPUNIT_ASSERT_EQUAL(0, notfDisp.numMsgs());
            mpFlowGraph->processNextFrame();
         }
         CPPUNIT_ASSERT_EQUAL(1, notfDisp.numMsgs());
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*&)pMsg, OsTime(10)));
         CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_DELAY_QUIESCENCE, 
            (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());

         // Stop Play
         MprDelay::stopPlay("TestDelay", *mpFlowGraph->getMsgQ());

         // Generate first active frame
         sourceResource.setGenOutBufMask(1);
         sourceResource.setSpeechType(0, MP_SPEECH_ACTIVE);
         mpFlowGraph->processNextFrame();
         CPPUNIT_ASSERT_EQUAL(1, notfDisp.numMsgs());
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, notfDisp.receive((OsMsg*&)pMsg, OsTime(10)));
         CPPUNIT_ASSERT_EQUAL(MpResNotificationMsg::MPRNM_DELAY_SPEECH_STARTED, 
            (MpResNotificationMsg::RNMsgType)((MpResNotificationMsg*)pMsg)->getMsg());

         // Add more frames to cause MprDelay's overflow.
         for (i = 0; i < 2*TEST_DELAY_FRAMES; i++)
         {
            if (i%2 == 1)
               sourceResource.setGenOutBufMask(0);
            else
               sourceResource.setGenOutBufMask(1);

            mpFlowGraph->processNextFrame();
            CPPUNIT_ASSERT_EQUAL(0, notfDisp.numMsgs());
         }
         CPPUNIT_ASSERT_EQUAL(TEST_DELAY_FRAMES, delayResource.getDelayFrames());

         // TESTING END
         mpFlowGraph->processNextFrame();

         // Remove resources from flowgraph. We should remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sinkResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sourceResource));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(delayResource));

         mpFlowGraph->processNextFrame();        
      }
      catch (CppUnit::Exception& e)
      {
         // Remove resources from flowgraph. We should remove them explicitly
         // here, because they are stored on the stack and will be destroyed.
         // If we will not catch this assert we'll have this resources destroyed
         // while still referenced in flowgraph, causing crash.
         if (sinkResource.getFlowGraph() != NULL)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sinkResource));
         }
         if (sourceResource.getFlowGraph() != NULL)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(sourceResource));
         }
         if (delayResource.getFlowGraph() != NULL)
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, mpFlowGraph->removeResource(delayResource));
         }
         mpFlowGraph->processNextFrame();

         // Rethrow exception.
         throw(e);
      }

      RTL_WRITE("testMprDelay.rtl");
      RTL_STOP
   }

private:
   MpFlowGraphBase  *mpFlowGraph; ///< Flowgraph for our test resources
};

CPPUNIT_TEST_SUITE_REGISTRATION(MprDelayTest);
