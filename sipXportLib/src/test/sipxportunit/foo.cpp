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

class FooTest : public SipxPortUnitTestClass
{

    CPPUNIT_TEST_SUITE(FooTest);
    CPPUNIT_TEST(fooMethod1); 
    CPPUNIT_TEST(fooMethod2); 
    CPPUNIT_TEST(fooMethod3); 
    CPPUNIT_TEST_SUITE_END();

    private:

    void fooMethod1()
    {
        //ASSERT_STR_EQUAL(EXPECTED, ACTUAL)

        CPPUNIT_ASSERT_MESSAGE("test point 1, should pass", true);

        //ASSERT_STR_EQUAL_MESSAGE
        //CPPUNIT_ASSERT_EQUAL
        //CPPUNIT_ASSERT_EQUAL_MESSAGE

        CPPUNIT_ASSERT_MESSAGE("test point 2 should fail", false) ;

        CPPUNIT_ASSERT_MESSAGE("test point 3 should pass", 2 + 2 == 4);

        int* myPtr = 0;

        CPPUNIT_ASSERT_MESSAGE("test point 4 should fail (null)", myPtr);
    }

    void fooMethod2()
    {
        void* myPtr = this;
        CPPUNIT_ASSERT_MESSAGE("should pass", 2 == 2);
        CPPUNIT_ASSERT_MESSAGE("should fail", 0);
        CPPUNIT_ASSERT_MESSAGE("should pass", myPtr == myPtr);
        CPPUNIT_ASSERT_MESSAGE("should pass", myPtr == this);
        int* g = 0;
        int h = *g;

        CPPUNIT_ASSERT_MESSAGE("sshould fail", this == 0);
        CPPUNIT_ASSERT_MESSAGE("should fail", myPtr == 0);
        CPPUNIT_ASSERT_MESSAGE("should fail", 0);
    }

    void fooMethod3()
    {
        CPPUNIT_ASSERT_MESSAGE("should pass", 2 == 2);
        int g = 0;
        int h = 1 / g;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FooTest);

