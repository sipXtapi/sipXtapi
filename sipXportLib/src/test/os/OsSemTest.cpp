//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsBSem.h>
#include <os/OsCSem.h>

class OsSemTest : public CppUnit::TestCase
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
        delete pBSem;
    }

    void testCountingSemaphore()
    {
        OsCSem* pCSem;

        // the initial count on the semaphore will be 2
        pCSem = new OsCSem(OsCSem::Q_PRIORITY, 2);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->acquire());  // take it once
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->acquire());  // take it twice
        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pCSem->tryAcquire());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->release());  // release once
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pCSem->release());  // release twice
        delete pCSem;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsSemTest);

