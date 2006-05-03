#include "StunMessageTest.h"
#include "os/StunMessage.h"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>


int main(int argc, char* argv[])
{
    // Get the top level suite from the registry
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    // Adds the test to the list of tests to run
    CppUnit::TextUi::TestRunner runner ;
    runner.addTest(suite) ;

    // Change the default outputter to a compiler error format outputter
    runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr)) ;

    // Run the tests.
    bool wasSuccessful = runner.run() ;

    // Return error code 1 if one of the tests failed.
    return wasSuccessful ? 0 : 1 ;
}


CPPUNIT_TEST_SUITE_REGISTRATION( StunMessageTestSuite );

void StunMessageTestSuite::testFullEncodeParse() 
{
    StunMessage msg ;
    StunMessage check ;
    bool bRC ;
    char szString[1024] ;
    unsigned short usValue ;
    bool bValue ;
    unsigned short values[16] ;
    size_t size ;

    // Valid empty state
    CPPUNIT_ASSERT(!msg.getMappedAddress(szString, usValue)) ;
    CPPUNIT_ASSERT(!msg.getResponseAddress(szString, usValue)) ;
    CPPUNIT_ASSERT(!msg.getChangePort()) ;
    CPPUNIT_ASSERT(!msg.getChangeIp()) ;
    CPPUNIT_ASSERT(!msg.getSourceAddress(szString, usValue)) ;
    CPPUNIT_ASSERT(!msg.getChangedAddress(szString, usValue)) ;
    CPPUNIT_ASSERT(!msg.getUsername(szString)) ;
    CPPUNIT_ASSERT(!msg.getPassword(szString)) ;
    CPPUNIT_ASSERT(!msg.getMessageIntegrity(szString)) ;
    CPPUNIT_ASSERT(!msg.getError(usValue, szString)) ;
    CPPUNIT_ASSERT(!msg.getUnknownAttributes(values, 16, size)) ;
    CPPUNIT_ASSERT(!msg.getReflectedFrom(szString, usValue)) ;
    CPPUNIT_ASSERT(!msg.getServer(szString)) ;

    // Set Values
    msg.setType(MSG_STUN_BIND_REQUEST) ;
    msg.allocTransactionId() ;
    msg.setMappedAddress("10.1.1.1", 1) ;
    msg.setResponseAddress("10.1.1.2", 2) ;
    msg.setChangePort(true) ;
    msg.setChangeIp(true) ;
    msg.setSourceAddress("10.1.1.3", 3) ;
    msg.setChangedAddress("10.1.1.4", 4) ;
    msg.setUsername("Username") ;
    msg.setPassword("Password") ;
    msg.setMessageIntegrity("12345678901234567890") ;
    msg.setError(302, "Reason") ;
    msg.addUnknownAttribute(0x1234) ;
    msg.setReflectedFrom("10.1.1.5", 5) ;
    msg.setServer("Server") ;

    // Verify Values
    CPPUNIT_ASSERT(msg.getType() == MSG_STUN_BIND_REQUEST) ;

    bRC = msg.getMappedAddress(szString, usValue) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(strcmp(szString, "10.1.1.1") == 0) ;
    CPPUNIT_ASSERT(usValue == 1) ;

    bRC = msg.getResponseAddress(szString, usValue) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(strcmp(szString, "10.1.1.2") == 0) ;
    CPPUNIT_ASSERT(usValue == 2) ;

    bValue = msg.getChangePort() ;
    CPPUNIT_ASSERT(bValue == true) ;

    bValue = msg.getChangeIp() ;
    CPPUNIT_ASSERT(bValue == true) ;

    bRC = msg.getSourceAddress(szString, usValue) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(strcmp(szString, "10.1.1.3") == 0) ;
    CPPUNIT_ASSERT(usValue == 3) ;

    bRC = msg.getChangedAddress(szString, usValue) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(strcmp(szString, "10.1.1.4") == 0) ;
    CPPUNIT_ASSERT(usValue == 4) ;

    bRC = msg.getUsername(szString) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(strcmp(szString, "Username") == 0) ;

    bRC = msg.getPassword(szString) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(strcmp(szString, "Password") == 0) ;

    bRC = msg.getMessageIntegrity(szString) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(strcmp(szString, "12345678901234567890") == 0) ;

    bRC = msg.getError(usValue, szString) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(usValue == 302) ;
    CPPUNIT_ASSERT(strcmp(szString, "Reason") == 0) ;

    bRC = msg.getUnknownAttributes(values, 16, size) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(size == 1) ;
    CPPUNIT_ASSERT(values[0] == 0x1234) ;
    
    bRC = msg.getReflectedFrom(szString, usValue) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(strcmp(szString, "10.1.1.5") == 0) ;
    CPPUNIT_ASSERT(usValue == 5) ;

    bRC = msg.getServer(szString) ;
    CPPUNIT_ASSERT(bRC) ;
    CPPUNIT_ASSERT(strcmp(szString, "Server") == 0) ;

    msg.setRequestXorOnly() ;

    char encoded[4096] ;
    size_t nLength ;
    if (msg.encode(encoded, 4096, nLength))
    {
        if (check.parse(encoded, nLength))
        {
            // Verify Values (again -- copy from above)
            CPPUNIT_ASSERT(msg.getType() == MSG_STUN_BIND_REQUEST) ;

            bRC = msg.getMappedAddress(szString, usValue) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(strcmp(szString, "10.1.1.1") == 0) ;
            CPPUNIT_ASSERT(usValue == 1) ;

            bRC = msg.getResponseAddress(szString, usValue) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(strcmp(szString, "10.1.1.2") == 0) ;
            CPPUNIT_ASSERT(usValue == 2) ;

            bValue = msg.getChangePort() ;
            CPPUNIT_ASSERT(bValue == true) ;

            bValue = msg.getChangeIp() ;
            CPPUNIT_ASSERT(bValue == true) ;

            bRC = msg.getSourceAddress(szString, usValue) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(strcmp(szString, "10.1.1.3") == 0) ;
            CPPUNIT_ASSERT(usValue == 3) ;

            bRC = msg.getChangedAddress(szString, usValue) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(strcmp(szString, "10.1.1.4") == 0) ;
            CPPUNIT_ASSERT(usValue == 4) ;

            bRC = msg.getUsername(szString) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(strcmp(szString, "Username") == 0) ;

            bRC = msg.getPassword(szString) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(strcmp(szString, "Password") == 0) ;

            bRC = msg.getMessageIntegrity(szString) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(strcmp(szString, "12345678901234567890") == 0) ;

            bRC = msg.getError(usValue, szString) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(usValue == 302) ;
            CPPUNIT_ASSERT(strcmp(szString, "Reason") == 0) ;

            bRC = msg.getUnknownAttributes(values, 16, size) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(size == 1) ;
            CPPUNIT_ASSERT(values[0] == 0x1234) ;
            
            bRC = msg.getReflectedFrom(szString, usValue) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(strcmp(szString, "10.1.1.5") == 0) ;
            CPPUNIT_ASSERT(usValue == 5) ;

            bRC = msg.getServer(szString) ;
            CPPUNIT_ASSERT(bRC) ;
            CPPUNIT_ASSERT(strcmp(szString, "Server") == 0) ;
        }
        else
        {
            CPPUNIT_ASSERT(false) ;
        }
    }
    else
    {
        CPPUNIT_ASSERT(false) ;
    }
}