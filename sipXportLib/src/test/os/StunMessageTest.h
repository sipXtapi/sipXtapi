#include <cppunit/extensions/HelperMacros.h>

class StunMessageTestSuite : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(StunMessageTestSuite) ;

    CPPUNIT_TEST(testFullEncodeParse) ;    

    CPPUNIT_TEST_SUITE_END() ;

public:

    void testFullEncodeParse() ;
} ;