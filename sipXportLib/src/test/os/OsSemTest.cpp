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

#include <os/OsBSem.h>
#include <os/OsCSem.h>
#include <sipxunittests.h>

class OsSemTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(OsSemTest);
    CPPUNIT_TEST(testBasicSemaphore);
    CPPUNIT_TEST(testCountingSemaphore);
    CPPUNIT_TEST_SUITE_END();


public:
    void testBasicSemaphore()
    {
        OsBSem* pBSem;

        pBSem = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pBSem->acquire());
        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pBSem->tryAcquire());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pBSem->release());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pBSem->acquire(100));
        CPPUNIT_ASSERT_EQUAL(OS_WAIT_TIMEOUT, pBSem->acquire(100));
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pBSem->release());

        delete pBSem;
    }

    void testCountingSemaphore()
    {
        OsCSem* pCSem;

        // the initial count on the semaphore will be 2
        pCSem = new OsCSem(OsCSem::Q_PRIORITY, 2);
                                                             // take it once
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->acquire(100));
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->acquire());  // take it twice
        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pCSem->tryAcquire());  // try thrice
                                                             // try once more
        CPPUNIT_ASSERT_EQUAL(OS_WAIT_TIMEOUT, pCSem->acquire(100));  
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->release());  // release once
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->release());  // release twice

        // This check work fine under Windows and with sipX implementation
        // of semaphores on Linux. But native pthread's semaphores behave
        // differently. So we could not rely on this property.
//        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pCSem->release());     // release thrice

        delete pCSem;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsSemTest);

