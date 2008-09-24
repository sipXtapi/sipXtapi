#include <cppunit/extensions/HelperMacros.h>

class TurnMessageTestSuite : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TurnMessageTestSuite) ;

    CPPUNIT_TEST(testStunEncodeParse) ;    
    CPPUNIT_TEST(testTurnEncodeParseBasic) ;    
    CPPUNIT_TEST(testTransportPrefs) ;    

    CPPUNIT_TEST_SUITE_END() ;

public:
    void testStunEncodeParse() ;
    void testTurnEncodeParseBasic() ;
    void testTransportPrefs() ;
    
} ;