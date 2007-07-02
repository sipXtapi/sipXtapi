//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>

#include <mp/MpMisc.h>

/**
 * Unittest for MpMediaTask
 */
class MpMediaTaskTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(MpMediaTaskTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManagedAndUnmanagedFlowGraph);
    CPPUNIT_TEST(testDebugMode);
    CPPUNIT_TEST(testFocus);
    CPPUNIT_TEST(testStartAndStopFlowGraph);
    CPPUNIT_TEST(testTimeLimitAndTimeout);
    CPPUNIT_TEST(testMultipleManagedAndUnmanagedFlowgraph);
    CPPUNIT_TEST_SUITE_END();

/// Number of frames in one frame
#define TEST_SAMPLES_PER_FRAME 80
/// Number of frames in one second
#define TEST_SAMPLES_PER_SEC 8000

public:

   // Initialize test framework
   void setUp()
   {
      OsStatus          res;

      // Setup media task
      res = mpStartUp(TEST_SAMPLES_PER_SEC, TEST_SAMPLES_PER_FRAME, 6*10, 0);
      CPPUNIT_ASSERT(res == OS_SUCCESS);

      // Call getMediaTask() which causes the task to get instantiated
      mpMediaTask = MpMediaTask::getMediaTask(10);
      CPPUNIT_ASSERT(mpMediaTask != NULL);
   }

   // Clean up after test is done.
   void tearDown()
   {
      OsStatus          res;

      // Clear all Media Tasks data
      res = mpShutdown();
      CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   void testCreators()
    {
        OsStatus     res;

        // Check the initial state of the MpMediaTask object
        CPPUNIT_ASSERT(mpMediaTask->getDebugMode() == FALSE);
        CPPUNIT_ASSERT(mpMediaTask->getFocus() == NULL);
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->getLimitExceededCnt());
        CPPUNIT_ASSERT(mpMediaTask->getTimeLimit() == MpMediaTask::DEF_TIME_LIMIT_USECS);
        CPPUNIT_ASSERT(mpMediaTask->getWaitTimeout() == MpMediaTask::DEF_SEM_WAIT_MSECS);
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->getWaitTimeoutCnt());
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numManagedFlowGraphs());
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numProcessedFrames());
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numStartedFlowGraphs());

        // Verify that the task is actually running by:
        //   enabling debug mode
        //   calling signalFrameStart()
        //   checking the processed frame count
        res = mpMediaTask->setDebug(TRUE);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numProcessedFrames());
    }

    void testManagedAndUnmanagedFlowGraph()
    {
        MpFlowGraphBase* pFlowGraph = 0;
        OsStatus         res;

        // Test 1: Create an empty flow graph and manage it
        pFlowGraph = new MpFlowGraphBase(30, 30);
        res = mpMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run

        // NOTE: original delay of 20 was tempermental, I increased
        // this to 100 to reduce the chance of this happening to
        // hopefully 0% - DLH
        OsTask::delay(100);

        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numManagedFlowGraphs());

        // Test 2: Invoke manageFlowGraph() with the same flow graph
        //         (will increment the numHandledMsgErrs() count for that
        //         frame processing interval but should otherwise have no
        //         effect)
        res = mpMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numManagedFlowGraphs());
        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numHandledMsgErrs());

        // Test 3: Unmanage the flow graph
        res = mpMediaTask->unmanageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numManagedFlowGraphs());
        // Test 4: Unmanage a flow graph which is not currently managed
        //         (will increment the numHandledMsgErrs() count for that
        //         frame processing interval but should otherwise have no
        //         effect)
        res = mpMediaTask->unmanageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numManagedFlowGraphs());
        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numHandledMsgErrs());

        // Test 5: Attempt to manage a flow graph that is not in the
        //         MpFlowGraphBase::STOPPED state
        res = pFlowGraph->start();              // send the flow graph a start
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // command and a signal to
        res = pFlowGraph->processNextFrame();   // process its messages
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = mpMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_INVALID_ARGUMENT);

        res = pFlowGraph->stop();               // send the flow graph a stop
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // command and a signal to
        res = pFlowGraph->processNextFrame();   // process its messages
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        // Test 6: Unmanage a flow graph that is "started"
        res = mpMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = mpMediaTask->startFlowGraph(*pFlowGraph); // start the flow graph
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        res = mpMediaTask->unmanageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        // verify that the flow graph has been stopped and is unmanaged
        CPPUNIT_ASSERT(pFlowGraph->getState() == MpFlowGraphBase::STOPPED);
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numManagedFlowGraphs());
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numStartedFlowGraphs());

        delete pFlowGraph;

        // Clear all Media Tasks data
        res = mpShutdown();
        CPPUNIT_ASSERT(res == OS_SUCCESS);
    }

    void testDebugMode()
    {
        OsStatus         res;
        int              waitTimeoutCnt;

        // Test 1: Verify that wait for "frame start" timeouts are noticed
        //         only when the media task is not in debug mode
        res = mpMediaTask->setDebug(FALSE);      // turn debug mode off
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT(mpMediaTask->getDebugMode() == FALSE);

        waitTimeoutCnt = mpMediaTask->getWaitTimeoutCnt();
        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        OsTask::delay(1000);                    // and wait 1 second

        // $$$ Need to understand why the following test fails
        // WHAT THE #(*$&#(*&???  CPPUNIT_ASSERT(mpMediaTask->getWaitTimeoutCnt() > waitTimeoutCnt);

        res = mpMediaTask->setDebug(TRUE);       // turn debug mode on
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT(mpMediaTask->getDebugMode() == TRUE);

        res = MpMediaTask::signalFrameStart();  // send a signal to the task
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // and give it a chance to run
        OsTask::delay(20);

        waitTimeoutCnt = mpMediaTask->getWaitTimeoutCnt();
        OsTask::delay(1000);                     // wait 1 second
        CPPUNIT_ASSERT_EQUAL(waitTimeoutCnt, mpMediaTask->getWaitTimeoutCnt());

        // Clear all Media Tasks data
        res = mpShutdown();
        CPPUNIT_ASSERT(res == OS_SUCCESS);
    }


    void testFocus()
    {
        MpFlowGraphBase* pFlowGraph = 0;
        OsStatus         res;

        // Test 1: Attempt to setFocus to a flow graph that the media task
        //         is not managing
        pFlowGraph = new MpFlowGraphBase(30, 30);
        res = mpMediaTask->setFocus(pFlowGraph); // send the media task a
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // set_focus command and
        res = MpMediaTask::signalFrameStart();  // give it a chance to run
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        OsTask::delay(20);

        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numHandledMsgErrs());

        // Test 2: Set the focus to a flow graph that has not been started
        res = mpMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // manage the flow graph and
        res = mpMediaTask->setFocus(pFlowGraph); // send the media task a
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // set_focus command and
        res = MpMediaTask::signalFrameStart();  // give it a chance to run
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        OsTask::delay(20);
        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numHandledMsgErrs());

        // Test 3: Set the focus to a flow graph that has been started
        res = mpMediaTask->startFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // start the flow graph and
        res = mpMediaTask->setFocus(pFlowGraph); // send the media task a
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // set_focus command and
        res = MpMediaTask::signalFrameStart();  // give it a chance to run
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        OsTask::delay(20);

        // Test 4: Set the focus to NULL
        res = mpMediaTask->unmanageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // unmanage the flow graph
        res = mpMediaTask->setFocus(NULL);      // and send the media task a
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // set_focus command and
        res = MpMediaTask::signalFrameStart();  // give it a chance to run
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT(mpMediaTask->getFocus() == NULL);

        delete pFlowGraph;

        // Clear all Media Tasks data
        res = mpShutdown();
        CPPUNIT_ASSERT(res == OS_SUCCESS);
    }

    void testTimeLimitAndTimeout()
    {
        OsStatus         res;
        int              oldValue;

        // Test 1: Set the time limit to twice its original value
        oldValue = mpMediaTask->getTimeLimit();
        res = mpMediaTask->setTimeLimit(oldValue * 2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(oldValue * 2, mpMediaTask->getTimeLimit());

        // Test 2: Set the time limit back to its original value
        res = mpMediaTask->setTimeLimit(oldValue);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(oldValue, mpMediaTask->getTimeLimit());

        // Test 3: Set the wait timeout to twice its original value
        oldValue = mpMediaTask->getWaitTimeout();
        res = mpMediaTask->setWaitTimeout(oldValue * 2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(oldValue * 2, mpMediaTask->getWaitTimeout());

        // Test 4: Set the wait timeout to -1 (infinity)
        res = mpMediaTask->setWaitTimeout(-1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(-1, mpMediaTask->getWaitTimeout());

        // Test 5: Set the wait timeout back to its original value
        res = mpMediaTask->setWaitTimeout(oldValue);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(oldValue, mpMediaTask->getWaitTimeout());
    }

    void testStartAndStopFlowGraph()
    {
        MpFlowGraphBase* pFlowGraph = 0;
        OsStatus         res;

        // Test 1: Set the time limit to twice its original value
        pFlowGraph = new MpFlowGraphBase(30, 30);

        mpMediaTask->numHandledMsgErrs(); // clear count
        // Test 1: Attempt to start a flow graph that is not being managed
        res = mpMediaTask->startFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run
        OsTask::delay(20);
        //CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numStartedFlowGraphs());
        // NOTE: Original test code had "1", not sure what's correct
        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numStartedFlowGraphs());

        // Test 2: Start a flow graph that is managed
        mpMediaTask->numHandledMsgErrs(); // clear the count

        res = mpMediaTask->manageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = mpMediaTask->startFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run
        OsTask::delay(20);
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numStartedFlowGraphs());
        CPPUNIT_ASSERT(pFlowGraph->isStarted());

        // Test 3: Attempt to start the same flow graph again
        mpMediaTask->numHandledMsgErrs(); // clear the count

        res = mpMediaTask->startFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run
        OsTask::delay(20);
        //CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numStartedFlowGraphs());


        // Test 4: Stop the flow graph
        mpMediaTask->numHandledMsgErrs(); // clear the count

        res = mpMediaTask->stopFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run
        OsTask::delay(20);
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numStartedFlowGraphs());
        CPPUNIT_ASSERT(!pFlowGraph->isStarted());

        // Test 5: Attempt to stop the same flow graph again
        mpMediaTask->numHandledMsgErrs(); // clear the count

        res = mpMediaTask->stopFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run

        OsTask::delay(20);
        //CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numStartedFlowGraphs());
        CPPUNIT_ASSERT(!pFlowGraph->isStarted());

        // Test 6: Attempt to stop a flow graph that is not being managed
        mpMediaTask->numHandledMsgErrs(); // clear the count

        res = mpMediaTask->unmanageFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = mpMediaTask->stopFlowGraph(*pFlowGraph);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);      // give it a chance to run
        OsTask::delay(20);
        CPPUNIT_ASSERT_EQUAL(1, mpMediaTask->numHandledMsgErrs());
        CPPUNIT_ASSERT_EQUAL(0, mpMediaTask->numStartedFlowGraphs());
        CPPUNIT_ASSERT(!pFlowGraph->isStarted());

        delete pFlowGraph;
    }

    void testMultipleManagedAndUnmanagedFlowgraph()
    {
        MpFlowGraphBase* pFlowGraph1 = 0;
        MpFlowGraphBase* pFlowGraph2 = 0;
        MpFlowGraphBase* flowGraphs[2];
        int              itemCnt;
        OsStatus         res;

        // Setup media task
        pFlowGraph1 = new MpFlowGraphBase(30, 30);
        pFlowGraph2 = new MpFlowGraphBase(30, 30);

        // Test 1: Add one managed flow graph
        res = mpMediaTask->manageFlowGraph(*pFlowGraph1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);              // give it a chance to run

        // NOTE: original delay of 20 was tempermental, I increased
        // this to 100 to reduce the chance of this happening to
        // hopefully 0% - DLH
        OsTask::delay(100);
        
        flowGraphs[0] = flowGraphs[1] = NULL;
        res = mpMediaTask->getManagedFlowGraphs(flowGraphs, 2, itemCnt);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(1, itemCnt);
        CPPUNIT_ASSERT(flowGraphs[0] == pFlowGraph1);

        // Test 2: Add a second managed flow graph
        res = mpMediaTask->manageFlowGraph(*pFlowGraph2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);              // give it a chance to run
        OsTask::delay(20);

        flowGraphs[0] = flowGraphs[1] = NULL;
        res = mpMediaTask->getManagedFlowGraphs(flowGraphs, 2, itemCnt);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(2, itemCnt);
        CPPUNIT_ASSERT(flowGraphs[0] == pFlowGraph1 ||
                       flowGraphs[0] == pFlowGraph2);

        CPPUNIT_ASSERT(flowGraphs[1] == pFlowGraph1 ||
                       flowGraphs[1] == pFlowGraph2);

        CPPUNIT_ASSERT(flowGraphs[0] != flowGraphs[1]);

        res = mpMediaTask->unmanageFlowGraph(*pFlowGraph1);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = mpMediaTask->unmanageFlowGraph(*pFlowGraph2);
        CPPUNIT_ASSERT(res == OS_SUCCESS);
        res = MpMediaTask::signalFrameStart();  // signal the media task and
        CPPUNIT_ASSERT(res == OS_SUCCESS);              // give it a chance to run
        OsTask::delay(20);

        delete pFlowGraph1;
        delete pFlowGraph2;
    }

protected:

   MpMediaTask *mpMediaTask;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpMediaTaskTest);
