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
#include "RegistrarTestCase.h"
#include "SipRegistrar.h"
#include "SipRegistrarServer.h"

using namespace std;

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipRegistrarServerTest : public RegistrarTestCase
{
   CPPUNIT_TEST_SUITE(SipRegistrarServerTest);
   CPPUNIT_TEST(testPullUpdates);
   CPPUNIT_TEST_SUITE_END();

public:
   SipRegistrarServerTest() :
      mRegistrar(NULL)
      {}

   void setUp()
      {
         RegistrarTestCase::setUp();

         // Copy registration.xml.in to registration.xml, updating timestamps
         createRegistrationDbFromTemplate();

         // Create the SipRegistrar for testing, without starting the associated thread
         initSipRegistrar();
      }

   void testPullUpdates()
      {
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
   static const char* REGDB_FILENAME; 
   static const char* REGDBTEMPLATE_FILENAME;

   SipRegistrar*      mRegistrar;

   void createRegistrationDbFromTemplate()
      {
         // Open the template file
         auto_ptr<OsFile> regTemplate(testFileFromFilename(REGDBTEMPLATE_FILENAME));
         CPPUNIT_ASSERT(regTemplate->exists());
         
         // Create/open the registration DB file.
         // Touch it to make sure it exists.  Set its length to 1 to wipe out (almost) all
         // existing content.  Because of bug XPL-141 we can't set it to zero length.
         auto_ptr<OsFile> regFile(testFileFromFilename(REGDB_FILENAME));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, regFile->touch());
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, regFile->setLength(1));

         // Go through line by line, treating each expiration value as an offset from now
         // and writing out the registration DB with time-shifted expiration values.
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, regTemplate->open(OsFile::READ_ONLY));
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, regFile->open(OsFile::READ_WRITE));
         UtlString line;
         long timeNow = OsDateTime::getSecsSinceEpoch();
         while (regTemplate->readLine(line) == OS_SUCCESS)
         {
            timeShiftExpiresLine(line, timeNow);
            line.append("\n");
            unsigned long bytesWritten = 0;
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, regFile->write(line.data(), line.length(), bytesWritten));
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned long>(line.length()), bytesWritten);
         }
         CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, regFile->flush());

         CPPUNIT_ASSERT(regTemplate->close());
         CPPUNIT_ASSERT(regFile->close());
      }

   // If this is an expires line, then modify it by adding timeNow to the expiration time value.
   // Otherwise don't mess with it.
   void timeShiftExpiresLine(UtlString& line, long timeNow)
      {
         const char* EXPIRES_BEGIN = "<expires>";
         const int EXPIRES_TAG_LENGTH = 9; 
         const char* EXPIRES_END = "</expires>";
         int pos1, pos2;
         // If the line has an expiration value, then time-shift it
         if ((pos1 = line.index(EXPIRES_BEGIN)) != static_cast<int>(UTL_NOT_FOUND) &&
             (pos2 = line.index(EXPIRES_END)) != static_cast<int>(UTL_NOT_FOUND))
         {
            pos1 += EXPIRES_TAG_LENGTH;    // skip past the tag, to the expires value
            CPPUNIT_ASSERT(pos2 > pos1);   // expires value cannot be empty
            UtlString timeText(line(pos1, pos2 - pos1));
            char* endptr = NULL;
            long timeNumber = strtol(timeText, &endptr, 10);
            CPPUNIT_ASSERT_EQUAL(*endptr, '\0');
            char newTime[20];          // room for up to a 64-bit number, may have minus sign
            int newTimeLen = sprintf(newTime, "%ld", timeNow + timeNumber);
            CPPUNIT_ASSERT(newTimeLen > 0);

            // Replace the old expiration value with the new shifted value
            UtlString lineEnd(line(pos2, line.length() - pos2));
            line.replace(pos1, newTimeLen, newTime);
            line.replace(pos1 + newTimeLen, lineEnd.length(), lineEnd);
         }
      }

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

const char* SipRegistrarServerTest::REGDBTEMPLATE_FILENAME = "registration.xml.in";
const char* SipRegistrarServerTest::REGDB_FILENAME = "registration.xml";

CPPUNIT_TEST_SUITE_REGISTRATION(SipRegistrarServerTest);
