//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include "os/OsServerTask.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

/// How many times to start/stop threads
#define TEST_START_STOP_ITERATIONS 5

/// Stub class for testing OsServerTask.
/**
*  Only purpose of this class is to implement abstract members of OsServerTask.
*  It is possible to do other fancy things in it, though.
*/
class TestServerTask : public OsServerTask
{
public:

   TestServerTask(const UtlString& name)
   : OsServerTask(name)
   {}

   virtual
   ~TestServerTask()
   {}

protected:

   virtual UtlBoolean handleMessage(OsMsg& rMsg)
   {
      return FALSE;
   }

};


class OsServerTaskTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(OsServerTaskTest);
   CPPUNIT_TEST(testFastStartStop);
   CPPUNIT_TEST_SUITE_END();

public:

   void testFastStartStop()
   {
      UtlBoolean boolRes;

      for (int i=0; i<TEST_START_STOP_ITERATIONS; i++)
      {
         // Create test task
         TestServerTask testTask("testTask");

         // Thread should not be initialized
         CPPUNIT_ASSERT(testTask.isUnInitialized());

         // Start task
         boolRes = testTask.start();
         CPPUNIT_ASSERT(boolRes);

         // Thread should go to STARTED state
         CPPUNIT_ASSERT(testTask.isStarted());

         // Thread will be stopped in destructor, leaving this scope.
      }
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsServerTaskTest);

