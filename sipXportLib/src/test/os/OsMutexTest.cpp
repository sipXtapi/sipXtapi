//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsRWMutex.h>
#include <os/OsMutex.h>

class OsMutexTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsMutexTest);
    CPPUNIT_TEST(testMutex);
    CPPUNIT_TEST_SUITE_END();


public:
    void testMutex()
    {
        OsRWMutex* pRWMutex;

        pRWMutex = new OsRWMutex(OsRWMutex::Q_FIFO);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pRWMutex->acquireRead());
        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pRWMutex->tryAcquireWrite());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pRWMutex->releaseRead());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pRWMutex->tryAcquireWrite());
        CPPUNIT_ASSERT_EQUAL(OS_BUSY, pRWMutex->tryAcquireRead());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, pRWMutex->releaseWrite());
        delete pRWMutex;    
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsMutexTest);

