#include "StunMessageTest.h"
#include "os/StunMessage.h"
#include <sipxunittests.h>
#include <string.h>

/*
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
*/


CPPUNIT_TEST_SUITE_REGISTRATION( StunMessageTestSuite );

void StunMessageTestSuite::testFullEncodeParse() 
{
    StunMessage msg ;
    StunMessage check ;
    bool bRC ;
    char szString[1024] ;
    uint16_t usValue ;
    bool bValue ;
    uint16_t values[16] ;
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

void StunMessageTestSuite::testLegacyMode() 
{
    StunMessage legacy(NULL, true) ;
    StunMessage nonLegacy(NULL, false) ;
    StunMessage responseWithLegacy(&legacy, false) ;
    StunMessage responseWithNonLegacy(&nonLegacy, true) ;

    STUN_MAGIC_ID magicId ;

    legacy.getMagicId(&magicId) ;
    CPPUNIT_ASSERT(magicId.id != STUN_MAGIC_COOKIE) ;

    nonLegacy.getMagicId(&magicId) ;
    CPPUNIT_ASSERT(magicId.id == STUN_MAGIC_COOKIE) ;

    responseWithLegacy.getMagicId(&magicId) ;
    CPPUNIT_ASSERT(magicId.id != STUN_MAGIC_COOKIE) ;

    responseWithNonLegacy.getMagicId(&magicId) ;
    CPPUNIT_ASSERT(magicId.id == STUN_MAGIC_COOKIE) ;
}


void StunMessageTestSuite::testFingerPrint() 
{
    char   cBuf[4096] ;
    size_t length ;
    StunMessage check(NULL, false) ;

    // Disabled FingerPrint
    check.reset() ;
    check.setIncludeFingerPrint(false) ;
    check.setType(MSG_STUN_BIND_REQUEST) ;
    check.encode(cBuf, sizeof(cBuf), length) ;
    CPPUNIT_ASSERT(check.isStunMessage(cBuf, (unsigned short) length) == true) ;
    CPPUNIT_ASSERT(check.isFingerPrintValid() == false) ;

    // Enable FingerPrint
    check.reset() ;
    check.allocTransactionId() ;
    check.setIncludeFingerPrint(true) ;
    check.setType(MSG_STUN_BIND_REQUEST) ;
    check.encode(cBuf, sizeof(cBuf), length) ;
    CPPUNIT_ASSERT(check.isStunMessage(cBuf, (unsigned short) length) == true) ;
    CPPUNIT_ASSERT(check.isFingerPrintValid() == true) ;

    // Bogus FingerPrint
    check.reset() ;
    check.allocTransactionId() ;
    check.setIncludeFingerPrint(true) ;
    check.setType(MSG_STUN_BIND_REQUEST) ;
    check.encode(cBuf, sizeof(cBuf), length) ;
    cBuf[length-3] = cBuf[length-3] + 1;
    CPPUNIT_ASSERT(check.isStunMessage(cBuf, (unsigned short) length) == false) ;
    check.reset() ;
    check.parse(cBuf, length) ;
    CPPUNIT_ASSERT(check.isFingerPrintValid() == false) ;
}


// Exercise the attribute-length handling in StunMessage::parse().
//
// Each STUN attribute has a 16-bit length; in non-legacy mode the parser
// advances over the attribute rounded up ("padded") to a 4-byte boundary.
// The bounds check must validate the PADDED length against the bytes
// remaining -- validating only the unpadded length let an attribute whose
// length is not a multiple of 4, placed near the end of the message, pass
// the check while the padded advance exceeded the remaining bytes.  That
// wrapped the size_t remaining-bytes counter and drove the loop past the
// end of the receive buffer (heap over-read).
void StunMessageTestSuite::testMalformedAttributePadding()
{
    // Positive control: a well-formed non-legacy message whose attribute
    // length (3) is not a multiple of 4 must still round-trip.  Non-legacy
    // mode (bLegacyMode = false) is what applies the 4-byte padding, so this
    // confirms the bounds check does not reject legitimate padded attributes.
    {
        StunMessage src(NULL, false) ;
        src.reset() ;
        src.allocTransactionId() ;
        src.setType(MSG_STUN_BIND_REQUEST) ;
        src.setUsername("abc") ;

        char   cBuf[4096] ;
        size_t length ;
        CPPUNIT_ASSERT(src.encode(cBuf, sizeof(cBuf), length)) ;

        StunMessage parsed(NULL, false) ;
        CPPUNIT_ASSERT(parsed.parse(cBuf, length) == true) ;
        char szUser[STUN_MAX_STRING_LENGTH + 1] ;
        CPPUNIT_ASSERT(parsed.getUsername(szUser)) ;
        CPPUNIT_ASSERT(strcmp(szUser, "abc") == 0) ;
    }

    // Malformed: the padded attribute length overruns the remaining bytes.
    // Message length = 9 (a 4-byte attribute header + a 5-byte body).  The
    // unpadded length (5) equals the 5 bytes left after the attribute header,
    // so it passes the old check, but the padded length (8) exceeds them.
    // The magic cookie forces non-legacy mode so the 4-byte padding applies.
    {
        unsigned char pkt[] = {
            0x00, 0x01,             // type   = Binding Request
            0x00, 0x09,             // length = 9 (attribute section)
            0x21, 0x12, 0xA4, 0x42, // magic cookie -> non-legacy mode
            0x00, 0x00, 0x00, 0x00, // transaction id (12 bytes)
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x06,             // attribute type
            0x00, 0x05,             // attribute length = 5 (padded to 8)
            0x41, 0x42, 0x43, 0x44, 0x45 // 5 bytes of body
        };
        // 20-byte header + 9 attribute bytes = 29.
        CPPUNIT_ASSERT_EQUAL((size_t) 29, sizeof(pkt)) ;

        StunMessage msg ;
        CPPUNIT_ASSERT(msg.parse((char*) pkt, sizeof(pkt)) == false) ;
    }

    // Malformed: the attribute's declared length exceeds the bytes remaining
    // outright.  Message length = 8 (4-byte header + 4-byte body), but the
    // attribute claims a length of 10.
    {
        unsigned char pkt[] = {
            0x00, 0x01,             // type   = Binding Request
            0x00, 0x08,             // length = 8 (attribute section)
            0x21, 0x12, 0xA4, 0x42, // magic cookie -> non-legacy mode
            0x00, 0x00, 0x00, 0x00, // transaction id (12 bytes)
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x06,             // attribute type
            0x00, 0x0A,             // attribute length = 10 (only 4 remain)
            0x41, 0x42, 0x43, 0x44  // 4 bytes of body
        };
        // 20-byte header + 8 attribute bytes = 28.
        CPPUNIT_ASSERT_EQUAL((size_t) 28, sizeof(pkt)) ;

        StunMessage msg ;
        CPPUNIT_ASSERT(msg.parse((char*) pkt, sizeof(pkt)) == false) ;
    }
}