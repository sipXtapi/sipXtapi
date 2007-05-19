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

#include <os/OsContactList.h>
#include <sipxunit/TestUtilities.h>

using namespace std ; 

/**  This class is used to test the OsContact class. 
*
*    PLEASE READ THE README FILE THAT CAN FOUND IN THE SAME DIRECTORY AS
*    THIS FILE. The Readme describes the organization / flow of tests and
*    without reading this file, the following class (and all unit tests)
*    may not make a lot of sense and might be difficult to comprehend. 
*/
class OsContactListTests : public CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(OsContactListTests);
    CPPUNIT_TEST(test) ; 
    CPPUNIT_TEST_SUITE_END();

private:

    
public:
    OsContactListTests()
    {
    }

    void setUp()
    {
    }
    
    void tearDown()
    {
    }

    ~OsContactListTests()
    {
    }

    /** Sandbox method for experimenting with the API Under Test. 
    *   This method MUST be empty when the test drivers are being
    *   checked in (final checkin) to the repository.
    */
    void DynaTest()
    {
    }

    /** Test an OsContactList class instance.
    */
    void test()
    {
        OsContact contact1("192.168.0.1", 4141, OsSocket::TCP, IP4);
        OsContact contact2("192.168.0.1", 4242, OsSocket::UDP, IP6) ;  
        OsContact contact3("192.168.0.2", 4242, OsSocket::UDP, IP6) ;  
        OsContactList contactList;
        
        // insert the first one
        contactList.insert(&contact2);        
        const OsContact* pContact = NULL;
        
        // no primary is set, so the first in the list is returned.
        pContact = contactList.getPrimary();
        CPPUNIT_ASSERT(pContact != NULL);
        
        // verify equality
        CPPUNIT_ASSERT(pContact->isEqual(&contact2));

        // verify number of records in list
        CPPUNIT_ASSERT(contactList.entries() == 1);
        
        // insert the second one at the end of the list
        contactList.insert(&contact1);        
        
        // no primary is set, so the first in the list is returned.
        pContact = contactList.getPrimary();
        CPPUNIT_ASSERT(pContact != NULL);
        
        // verify equality
        CPPUNIT_ASSERT(pContact->isEqual(&contact2));
        
        // call setPrimary
        contactList.setPrimary(contact1);
        
        // get the primary contact
        pContact = contactList.getPrimary();
        CPPUNIT_ASSERT(pContact != NULL);
        // verify equality
        CPPUNIT_ASSERT(pContact->isEqual(&contact1));
        
        // verify number of records in list
        CPPUNIT_ASSERT(contactList.entries() == 2);
        
        // call setPrimary on a contact that is not already in the list
        contactList.setPrimary(contact3);
                
        // get the primary contact
        pContact = contactList.getPrimary();
        CPPUNIT_ASSERT(pContact != NULL);
        // verify equality
        CPPUNIT_ASSERT(pContact->isEqual(&contact3));
        
        // verify number of records in list
        CPPUNIT_ASSERT(contactList.entries() == 3);
                
    }
    
};


// ------------------- Static constant initializers -------------------------


CPPUNIT_TEST_SUITE_REGISTRATION(OsContactListTests);
