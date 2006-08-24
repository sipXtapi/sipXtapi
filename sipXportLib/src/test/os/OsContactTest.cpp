//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <string.h>
#include <stdlib.h>
#include <cstdarg>

#include <os/OsContact.h>
#include <sipxunit/TestUtilities.h>

using namespace std ; 

/**  This class is used to test the OsContact class. 
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE SAME DIRECTORY AS
*    THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend. 
*/
class OsContactTests : public CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(OsContactTests);
    CPPUNIT_TEST(testConstructor) ; 
    CPPUNIT_TEST(testCompareTo) ; 
    CPPUNIT_TEST(testCompareTo_NonContact) ; 
    CPPUNIT_TEST(testEquals) ; 
    CPPUNIT_TEST(testEquals_NonContact) ; 
    CPPUNIT_TEST(testGetContainableType) ; 
    CPPUNIT_TEST_SUITE_END();

private:

    
public:
    OsContactTests()
    {
    }

    void setUp()
    {
    }
    
    void tearDown()
    {
    }

    ~OsContactTests()
    {
    }

    /** Sandbox method for experimenting with the API Under Test. 
    *   This method MUST be empty when the test drivers are being
    *   checked in (final checkin) to the repository.
    */
    void DynaTest()
    {
    }

    /** Test the constructor
    * Checks that the object's values are set.
    */
    void testConstructor()
    {
        // First test the default constructor
        OsContact testContact("192.168.0.1", 4242, OsSocket::UDP, IP6) ; 
        const char* msg0 = "Test the constructor" ;
        UtlString address;
        testContact.getAddress(address);
        CPPUNIT_ASSERT(address.compareTo("192.168.0.1") == 0);
        CPPUNIT_ASSERT(testContact.getPort() == 4242);
        CPPUNIT_ASSERT(testContact.getProtocol() == OsSocket::UDP);
        CPPUNIT_ASSERT(testContact.getAddressType() == IP6);
    }

    /** Test the compareTo method
    *   Data members are varied and compared to the match contact.
    *   Then the match contact is compared to itself.
    */
    void testCompareTo()
    {
        OsContact matchContact("192.168.0.1", 4242, OsSocket::UDP, IP6) ;  
        
        OsContact testContact1("192.168.0.2", 4242, OsSocket::UDP, IP6) ;  
        CPPUNIT_ASSERT(testContact1.compareTo(&matchContact) != 0);

        OsContact testContact2("192.168.0.1", 4243, OsSocket::UDP, IP6) ;  
        CPPUNIT_ASSERT(testContact2.compareTo(&matchContact) != 0);

        OsContact testContact3("192.168.0.1", 4242, OsSocket::TCP, IP6) ;  
        CPPUNIT_ASSERT(testContact3.compareTo(&matchContact) != 0);

        OsContact testContact4("192.168.0.1", 4242, OsSocket::UDP, IP4) ;  
        CPPUNIT_ASSERT(testContact4.compareTo(&matchContact) != 0);
        
        OsContact testContact5("192.168.0.1", 4242, OsSocket::UDP, IP6) ;  
        CPPUNIT_ASSERT(testContact5.compareTo(&matchContact) == 0);
        
        CPPUNIT_ASSERT(matchContact.compareTo(&matchContact) == 0);
    }

    /** Test the isEqual(UtlContainable*) method. 
    *   
    *   The test data for this test is the same as the compareTo method. 
    */ 
    void testEquals()
    {
        OsContact matchContact("192.168.0.1", 4242, OsSocket::UDP, IP6) ;  
        
        OsContact testContact1("192.168.0.2", 4242, OsSocket::UDP, IP6) ;  
        CPPUNIT_ASSERT(testContact1.isEqual(&matchContact) == false);

        OsContact testContact2("192.168.0.1", 4243, OsSocket::UDP, IP6) ;  
        CPPUNIT_ASSERT(testContact2.isEqual(&matchContact) == false);

        OsContact testContact3("192.168.0.1", 4242, OsSocket::TCP, IP6) ;  
        CPPUNIT_ASSERT(testContact3.isEqual(&matchContact) == false);

        OsContact testContact4("192.168.0.1", 4242, OsSocket::UDP, IP4) ;  
        CPPUNIT_ASSERT(testContact4.isEqual(&matchContact) == false);
        
        OsContact testContact5("192.168.0.1", 4242, OsSocket::UDP, IP6) ;  
        CPPUNIT_ASSERT(testContact5.isEqual(&matchContact) == true);
        
        CPPUNIT_ASSERT(matchContact.isEqual(&matchContact) == true);
    }
    
    
    /*!a Test the compareTo method when a non-UtlInt is passed. 
    *    
    */
    void testCompareTo_NonContact()
    {
        OsContact matchContact("192.168.0.1", 4242, OsSocket::UDP, IP6) ;  
        UtlString foo("boo");
        
        CPPUNIT_ASSERT(matchContact.compareTo(&foo) != 0);
    }

    /*!a Test the Equals Method when the argument passed is not a UtlInt
    *
    */
    void testEquals_NonContact()
    {
        OsContact matchContact("192.168.0.1", 4242, OsSocket::UDP, IP6) ;  
        UtlString foo("boo");
        
        CPPUNIT_ASSERT(matchContact.isEqual(&foo) == false);

    } //testEquals_NonInteger
    
    /*!a Test the getContainableType method
    *
    *    Test data = common test data set
    */
    void testGetContainableType()
    {
        OsContact matchContact("192.168.0.1", 4242, OsSocket::UDP, IP6) ;  
        CPPUNIT_ASSERT(matchContact.isInstanceOf(OsContact::TYPE));
    } //testGetContainableType
};


// ------------------- Static constant initializers -------------------------


CPPUNIT_TEST_SUITE_REGISTRATION(OsContactTests);
