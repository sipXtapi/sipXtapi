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
#include <cppunit/TestCase.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "utl/UtlString.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Base class for functionality common to registrar test cases
class RegistrarTestCase : public CppUnit::TestCase
{
protected:
   UtlString          mTestDir;

   /// Protocted constructor, this class cannot be instantiated directly
   RegistrarTestCase() : mTestDir(TEST_DATA_DIR) {}

   virtual void setUp();

   // Locate the registration DB in a test directory so that
   // we don't collide with the production DB.
   // :HA: Turn this into a common test utility
   void relocateRegistrationDb();

   void removeTestFile(const char* testFilename);

   OsFile* testFileFromFilename(const char* testFilename);
};
