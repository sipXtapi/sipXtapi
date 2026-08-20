//
// Copyright (C) 2007-2026 SIPez LLC  All rights reserved.
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

#include <sipxunittests.h>
#include <os/OsTimerTask.h>
#include <os/OsTimer.h>
#include <os/OsCallback.h>
#include <os/OsTime.h>
#include <string.h>

// Census timers are given long periods so that they cannot fire before
// the timer task is destroyed.  This callback should never be invoked.
static void censusTimerCallback(const intptr_t userData, const intptr_t eventData)
{
}

class OsTimerTaskTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(OsTimerTaskTest);
    CPPUNIT_TEST(testTimerTask);
    CPPUNIT_TEST(testTimerCensus);
    CPPUNIT_TEST_SUITE_END();

public:
    void testTimerTask()
    {
        OsTimerTask* pTimerTask;
        pTimerTask = OsTimerTask::getTimerTask();
        CPPUNIT_ASSERT_MESSAGE("Timer task created 1", pTimerTask != NULL);
        OsTask::delay(500);    // wait 1/2 second

        pTimerTask->destroyTimerTask();

        OsTask::delay(500);    // wait 1/2 second

        pTimerTask = OsTimerTask::getTimerTask();
        CPPUNIT_ASSERT_MESSAGE("Timer task created 2", pTimerTask != NULL);
        OsTask::delay(500);    // wait 1/2 second

        pTimerTask->destroyTimerTask();
    }

void testTimerCensus()
    {
        // Destroying the timer task force stops every timer still in the
        // queue.  The census records which ones, so that a caller can tell
        // whether a teardown took timers that were still in use.
        OsTimerTask::clearCensus();

        // Timers constructed from here on have ids at or above this value.
        // Other parts of the process may have started timers of their own;
        // the watermark is how this test tells its timers from theirs.
        unsigned long watermark = OsTimer::getNextId();

        OsCallback notifier((intptr_t)0, censusTimerCallback);
        OsTimer* pPeriodic = new OsTimer(notifier);
        OsTimer* pOneShot = new OsTimer(notifier);
        OsTimer* pNeverStarted = new OsTimer(notifier);

        pPeriodic->periodicEvery(OsTime(60, 0), OsTime(60, 0));
        pOneShot->oneshotAfter(OsTime(60, 0));

        // Let the timer task process both start messages.
        OsTask::delay(200);

        OsTimerTask::destroyTimerTask();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("census did not record the shutdown",
                                     1, OsTimerTask::getCensusShutdownCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("census overflowed",
                                     0, OsTimerTask::getCensusOverflowCount());

        int ours = 0;
        int periodicFound = 0;
        int oneShotFound = 0;
        int index;
        for (index = 0; index < OsTimerTask::getCensusCount(); index++)
        {
            const OsTimerTask::OsTimerCensusEntry* entry =
                OsTimerTask::getCensusEntry(index);
            CPPUNIT_ASSERT(entry != NULL);

            if (entry->mId < watermark)
            {
                // Belongs to some other part of the process.
                continue;
            }
            ours++;

            // The caller signature is captured at the construction site.
            // Empty values mean the compiler builtins did not resolve.
            CPPUNIT_ASSERT_MESSAGE("no caller function recorded",
                                   entry->mCallerFunction != NULL &&
                                   entry->mCallerFunction[0] != '\0');
            CPPUNIT_ASSERT_MESSAGE("caller file is not this test",
                                   entry->mCallerFile != NULL &&
                                   strstr(entry->mCallerFile,
                                          "OsTimerTaskTest.cpp") != NULL);
            CPPUNIT_ASSERT_MESSAGE("no caller line recorded",
                                   entry->mCallerLine > 0);

            if (entry->mPeriodic)
            {
                periodicFound++;
                CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong period recorded",
                                             60000, entry->mPeriodMsec);
            }
            else
            {
                oneShotFound++;
            }
        }

        // The timer that was never started must not be in the census.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong number of started timers recorded",
                                     2, ours);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("periodic timer not recorded",
                                     1, periodicFound);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("one shot timer not recorded",
                                     1, oneShotFound);

        // The shutdown handler already stopped these, so the destructors
        // have no message to send and do not need the timer task.
        delete pPeriodic;
        delete pOneShot;
        delete pNeverStarted;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsTimerTaskTest);
