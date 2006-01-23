// 
// Copyright (C) 2006 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include "sipdb/RegistrationDB.h"
#include "testlib/RegistrationDbTestContext.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class RegistryDbTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(RegistryDbTest);
   CPPUNIT_TEST(testGetMaxUpdateNumberForRegistrar);
   CPPUNIT_TEST_SUITE_END();

public:

   void testGetMaxUpdateNumberForRegistrar()
      {
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );

         testDbContext.inputFile("getMaxUpdate.xml");

         RegistrationDB* regDb = RegistrationDB::getInstance();

         intll seqOneMax = regDb->getMaxUpdateNumberForRegistrar("seqOne");
         CPPUNIT_ASSERT_EQUAL(5LL, seqOneMax);

         regDb->releaseInstance();
      };
};

CPPUNIT_TEST_SUITE_REGISTRATION(RegistryDbTest);

