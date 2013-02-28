//  
// Copyright (C) 2007-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <os/OsIntTypes.h>

#include <sipxunittests.h>


/**
 * Unittest for MpAudioBuf
 */
class RtcpParserTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(RtcpParserTest);

    // Register methods to be called for testing here
    CPPUNIT_TEST(testBadPaddedPacket);

    CPPUNIT_TEST_SUITE_END();


public:

#if 0
    // This method gets called before running each test method
    // Implement this method if you have common setup for each test method
    void setUp()
    {
    }
#endif

#if 0
    // This method gets called after running each test method
    // Implement this method if you have common teardown/clean to do after each test method
    void tearDown()
    {
    }
#endif

    void testBadPaddedPacket()
    {
        // Do stuff
        int foo = 0;

        // Check expected results
        CPPUNIT_ASSERT(foo==0);
        CPPUNIT_ASSERT_EQUAL(foo, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("This test failed testing the blabla", foo, 0);

        // Do more stuff and check results ...
    }



protected:
    // Add member data or utility methods used by test methods here
};

CPPUNIT_TEST_SUITE_REGISTRATION(RtcpParserTest);
