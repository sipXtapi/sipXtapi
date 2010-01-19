//
// Copyright (C) 2007-2010 SIPez LLC  All rights reserved.
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

class OsTimerTaskTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(OsTimerTaskTest);
    CPPUNIT_TEST(testTimerTask);
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
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsTimerTaskTest);
