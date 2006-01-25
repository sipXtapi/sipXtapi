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
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <iostream>
#include <memory>
#include <sipxunit/TestUtilities.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "net/Url.h"
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "os/OsProcess.h"
#include "testlib/RegistrationDbTestContext.h"
#include "SipRegistrar.h"
#include "SipRegistrarServer.h"

using namespace std;

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipRegistrarServerTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipRegistrarServerTest);
   CPPUNIT_TEST(testPullUpdates);
   CPPUNIT_TEST_SUITE_END();

public:
   SipRegistrarServerTest() :
      mRegistrar(NULL)
      {}

   void testPullUpdates()
      {
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );
         testDbContext.inputFile("updatesToPull.xml");

         // Create the SipRegistrar for testing, without starting the associated thread
         initSipRegistrar();

         // Pull all updates with primary = R1 and updateNumber > 1.
         UtlSList updates;
         int numUpdates = getRegistrarServer().pullUpdates(
            "R1",        // registrarName
            1,           // updateNumber -- pull only the updates with larger numbers
            updates);    // updates are returned in this list

         // Verify that the right updates got pulled.
         CPPUNIT_ASSERT_EQUAL(1, numUpdates);
      }

private:
   SipRegistrar*      mRegistrar;

   // Create the registrar for testing, without starting the associated thread
   void initSipRegistrar()
      {
         // Create and initialize the registrar, but don't start it.
         // For unit testing, we just need the registrar object, not the thread.
         // This arrangement is wacky and we'll try to improve it in the future.
         OsConfigDb configDb;        // empty configuration DB is OK
         mRegistrar = SipRegistrar::getInstance(&configDb);

         // The config was empty so replication is not configured
         CPPUNIT_ASSERT(!mRegistrar->isReplicationConfigured());
      }

   SipRegistrarServer& getRegistrarServer()
      {
         return mRegistrar->getRegistrarServer();
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipRegistrarServerTest);
