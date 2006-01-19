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
#include "sipdb/RegistrationDB.h"
#include "SipRegistrar.h"

using namespace std;

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SyncRpcTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SyncRpcTest);
   CPPUNIT_TEST(testPullUpdates);
   CPPUNIT_TEST_SUITE_END();

public:
   SyncRpcTest() :
      mTestDir(TEST_DATA_DIR),
      mHttpPort(SipRegistrar::REGISTRAR_DEFAULT_RPC_PORT + 1),
      mHttpServer(NULL),
      mXmlRpcDispatch(NULL)
      {}

   void setUp()
      {
         // :HA: Turn this into a common utility for relocating the registration DB

         // Locate the registration DB in a test directory so that
         // we don't collide with the production DB.
         int status = setenv("SIPX_DB_CFG_PATH", mTestDir, 1);
         CPPUNIT_ASSERT_EQUAL(0, status);
         status = setenv("SIPX_DB_VAR_PATH", mTestDir, 1);
         CPPUNIT_ASSERT_EQUAL(0, status);
         //cout << "Loading registration DB from " << mTestDir << endl;

         // Delete previous imdb.odb file if it exists.
         // (Previously we were deleting registration.xml as well, but then had trouble
         // recreating/opening it.)
         removeTestFile("imdb.odb");

         // Copy registration.xml.in to registration.xml, updating timestamps
         createRegistrationDbFromTemplate();
      }

   void testPullUpdates()
      {
#ifdef COMMENT_OUT_TEMPORARILY
         // Load the registration DB
         RegistrationDB* regDb = RegistrationDB::getInstance();

         // Set up a sync RPC server
         mXmlRpcDispatch = new XmlRpcDispatch(mHttpPort, true /* use https */);
         mHttpServer = mXmlRpcDispatch->getHttpServer();

         // Pull all updates with primary = R1 and updateNumber > 1
         // Verify that the right updates got pulled






         regDb->releaseInstance();
#endif
      }

private:
   static const char* REGDB_FILENAME; 
   static const char* REGDBTEMPLATE_FILENAME;

   UtlString          mTestDir;
   int                mHttpPort;
   HttpServer*        mHttpServer;
   XmlRpcDispatch*    mXmlRpcDispatch;

   void removeTestFile(const char* testFilename)
      {
         auto_ptr<OsFile> testFile(testFileFromFilename(testFilename));
         if (testFile->exists())
         {
            CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, testFile->remove());
         }
      }

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

   OsFile* testFileFromFilename(const char* testFilename)
      {
         UtlString testFilePath(mTestDir);
         OsPath path(testFilePath.append(testFilename));
         OsFile* testFile = new OsFile(path);
         return testFile;
      }
};

const char* SyncRpcTest::REGDBTEMPLATE_FILENAME="registration.xml.in";
const char* SyncRpcTest::REGDB_FILENAME="registration.xml";

CPPUNIT_TEST_SUITE_REGISTRATION(SyncRpcTest);
