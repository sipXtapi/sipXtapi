//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <string.h>
#include <stdlib.h>
#include <cstdarg>

#include <utl/UtlCrc32.h>
#include <sipxunit/TestUtilities.h>

using namespace std ; 


/**  This class is used to test the UtlCrc32 utility class. 
 *
 *    PLEASE READ THE README FILE THAT CAN FOUND IN THE SAME DIRECTORY AS
 *    THIS FILE. The Readme describes the organization / flow of tests and
 *    without reading this file, the following class (and all unit tests)
 *    may not make a lot of sense and might be difficult to comprehend. 
 */
class UtlCrc32Tests : public CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(UtlCrc32Tests);
    CPPUNIT_TEST(testAll) ; 
    CPPUNIT_TEST_SUITE_END();

private:

    
public:
    UtlCrc32Tests()
    {
    }

    void setUp()
    {
    }
    
    void tearDown()
    {
    }

    ~UtlCrc32Tests()
    {
    }

    /** Sandbox method for experimenting with the API Under Test. 
     *   This method MUST be empty when the test drivers are being
     *   checked in (final checkin) to the repository.
     */
    void DynaTest()
    {
    }

    /** 
     * Some simple tests to verify that Crc32s are generated correctly.
     *  - validate checksums
     *  - validate all APIs / methods of adding data
     */
    void testAll()
    {
        UtlCrc32 mGenerator ;

        // By default, Crc32 is zero
        CPPUNIT_ASSERT(mGenerator.getValue() == 0) ;

        // Add single byte of data
        mGenerator.calc(0x80) ;
        CPPUNIT_ASSERT(mGenerator.getValue() == 0x3FBA6CAD) ;

        // Make sure reset, does reset.
        mGenerator.reset() ;
        CPPUNIT_ASSERT(mGenerator.getValue() == 0) ;

        // Try adding a sequences of bytes
        mGenerator.calc((unsigned char*) "bandreasen", 10) ;
        CPPUNIT_ASSERT(mGenerator.getValue() == 0xB78D3286) ;

        // Try appending more bytes
        mGenerator.calc((unsigned char*) "bandreasen", 10) ;
        CPPUNIT_ASSERT(mGenerator.getValue() == 0x71BD42B1) ;

        // Trying using a UtlString as a container
        mGenerator.reset() ;
        UtlString str("eatatjoes") ;
        mGenerator.calc(str) ;
        CPPUNIT_ASSERT(mGenerator.getValue() == 0xFDF73AB6) ;
    }
};


CPPUNIT_TEST_SUITE_REGISTRATION(UtlCrc32Tests);
