//
//
// Copyright (C) 2010 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2010 SIPez LLC All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////

#include <SipxPortUnitTest.h>

class BarTest : public SipxPortUnitTestClass
{
    CPPUNIT_TEST_SUITE(BarTest);
    CPPUNIT_TEST(barTest1);
    CPPUNIT_TEST(barTest2);
    CPPUNIT_TEST(barTest3);
    CPPUNIT_TEST_SUITE_END();

    public:

    BarTest()
    {
        int f = 1/0;
        //SipxPortUnitTestEnvironment::printOut("My special BarTest init\n");
    }

    void setUp()
    {
        //SipxPortUnitTestEnvironment::printOut("My special BarTest setUp\n");
    }

    void tearDown()
    {
        //SipxPortUnitTestEnvironment::printOut("My special BarTest tearDown\n");
    }

    ~BarTest()
    {
        //SipxPortUnitTestEnvironment::printOut("My special BarTest cleanup\n");
    }

    private:
    void barTest1()
    {
        CPPUNIT_ASSERT_MESSAGE("pass 1", 1 == 1);
        CPPUNIT_ASSERT_MESSAGE("pass 2", 2 == 2);
        CPPUNIT_ASSERT_MESSAGE("pass 3", 3 == 3);

        int* intPtr = 0;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should segv", 4, *intPtr);
    }

    void barTest2()
    {
        CPPUNIT_ASSERT_MESSAGE("fail 2-1", 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be equal", 2, 1 + 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should fail", 2, 1);
        CPPUNIT_ASSERT_EQUAL(1, 2-1);
        CPPUNIT_ASSERT_EQUAL(1.3, 2);
        CPPUNIT_ASSERT_EQUAL(2-1, 4 + 9);
    }

    void barTest3()
    {
        CPPUNIT_ASSERT_MESSAGE("should pass", 23 * 1.23456666);
        ASSERT_STR_EQUAL_MESSAGE("should pass", "abc", "abc");
        ASSERT_STR_EQUAL_MESSAGE("should fail", "abc", "ABC");
        char* str1 = "abc";
        ASSERT_STR_EQUAL_MESSAGE("should pass", "abc", str1);
        ASSERT_STR_EQUAL_MESSAGE("should fail", str1, "def");
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(BarTest);

