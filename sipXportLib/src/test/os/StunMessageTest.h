#include <sipxunittests.h>

class StunMessageTestSuite : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(StunMessageTestSuite) ;

    CPPUNIT_TEST(testFullEncodeParse) ;
    CPPUNIT_TEST(testLegacyMode) ;
    CPPUNIT_TEST(testFingerPrint) ;
    CPPUNIT_TEST(testMalformedAttributePadding) ;

    CPPUNIT_TEST_SUITE_END() ;

public:

    void testFullEncodeParse() ;
    void testLegacyMode() ;
    void testFingerPrint() ;
    void testMalformedAttributePadding() ;
} ;