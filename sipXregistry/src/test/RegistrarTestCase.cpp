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
#include <memory>
#include <stdlib.h>
using std::auto_ptr;

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "RegistrarTestCase.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


void RegistrarTestCase::setUp()
{
   // Locate the registration DB in a test directory so that
   // we don't collide with the production DB.
   relocateRegistrationDb();

   // Delete the previous imdb.odb file if it exists.
   // (Previously we were deleting registration.xml as well, but then got ACCESS_DENIED
   // when trying to recreate/open it.)
   removeTestFile("imdb.odb");
}

// Locate the registration DB in a test directory so that
// we don't collide with the production DB.
// :HA: Turn this into a common test utility
void RegistrarTestCase::relocateRegistrationDb()
{
   int status = setenv("SIPX_DB_CFG_PATH", mTestDir, 1);
   CPPUNIT_ASSERT_EQUAL(0, status);
   status = setenv("SIPX_DB_VAR_PATH", mTestDir, 1);
   CPPUNIT_ASSERT_EQUAL(0, status);
}

void RegistrarTestCase::removeTestFile(const char* testFilename)
{
   auto_ptr<OsFile> testFile(testFileFromFilename(testFilename));
   if (testFile->exists())
   {
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, testFile->remove());
   }
}

OsFile* RegistrarTestCase::testFileFromFilename(const char* testFilename)
{
   UtlString testFilePath(mTestDir);
   OsPath path(testFilePath.append(testFilename));
   OsFile* testFile = new OsFile(path);
   return testFile;
}

