// 
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004-2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsFS.h>
#include <os/OsTestUtilities.h>
#include <os/OsTask.h>

#include <stdlib.h>

#ifndef WINCE
#include <errno.h>
#endif

/**
 * Test Description
 */
class OsFileTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsFileTest);
    CPPUNIT_TEST(testCreateFile);
    CPPUNIT_TEST(testDeleteFile);
    CPPUNIT_TEST(testReadWriteBuffer);
    CPPUNIT_TEST(testCopyFile);
    CPPUNIT_TEST(testReadOnly);
    CPPUNIT_TEST(testTouch);
#ifndef _WIN32
    CPPUNIT_TEST(testFileLocking);
#endif
    CPPUNIT_TEST_SUITE_END();

    /** where all tests should r/w data */
    OsPath mRootPath;

public:

    void setUp()
    {
        OsTestUtilities::createTestDir(mRootPath);
    }


    void tearDown()
    {
        OsTestUtilities::removeTestDir(mRootPath);
    }

    /**
     * Create empty file
     */
    void testCreateFile()
    {
        OsPath testFile = mRootPath + OsPath::separator + "testCreateFile";

        OsFile file(testFile);
        file.touch();
        CPPUNIT_ASSERT_MESSAGE("Touched file exists", file.exists());
    }

    /**
     * Create and delete empty file
     */
    void testDeleteFile()
    {
        OsStatus stat;
        OsPath testFile = mRootPath + OsPath::separator + "testDeleteFile";

        OsFile file(testFile);
        file.touch();
        CPPUNIT_ASSERT(file.exists());
        stat = file.remove();
        CPPUNIT_ASSERT_MESSAGE("File deleted", stat == OS_SUCCESS);
        CPPUNIT_ASSERT(!file.exists());
    }

    /**
     * Creates a new dummy file then reads it back in to verify it's
     * contents, buffer by buffer.
     */
    void testReadWriteBuffer()
    {
        ///////////////////////
        //       W R I T E
        ///////////////////////
        OsStatus stat;
        OsPath testFile = mRootPath + OsPath::separator + "testWriteBuffer";
        char wbuff[10000];
        unsigned long wbuffsize = (unsigned long)sizeof(wbuff);

        OsTestUtilities::initDummyBuffer(wbuff, sizeof(wbuff));

        OsFile wfile(testFile);
        stat = wfile.open(OsFile::CREATE);
        CPPUNIT_ASSERT(stat == OS_SUCCESS);

        unsigned long wposition = 0;
        int i;
        for (i = 0; wposition < wbuffsize; i++)
        {
            unsigned long remaining = wbuffsize - wposition;
            unsigned long byteswritten = 0;
            stat = wfile.write(wbuff + wposition, remaining, byteswritten);
            CPPUNIT_ASSERT(stat == OS_SUCCESS);
            wposition += byteswritten;
        }

        //close the file after working with it.
        wfile.close();

        ///////////////////////
        //       R E A D
        ///////////////////////
        char rbuff[256];
        unsigned long rbuffsize = (unsigned long)sizeof(rbuff);
        OsFile rfile(testFile);
        stat = rfile.open();
        CPPUNIT_ASSERT(stat == OS_SUCCESS);
        
        unsigned long rposition = 0;
        for (i = 0; rposition < wbuffsize; i++)
        {
            unsigned long remaining = (wbuffsize - rposition);
            unsigned long readsize = remaining < rbuffsize ? remaining : rbuffsize;
            unsigned long bytesread = 0;
            stat = rfile.read(rbuff, readsize, bytesread);
            CPPUNIT_ASSERT_MESSAGE("Read buffer", stat == OS_SUCCESS);
            UtlBoolean ok = OsTestUtilities::testDummyBuffer(rbuff, bytesread, rposition);
            CPPUNIT_ASSERT_MESSAGE("Test buffer data", ok);
            rposition += bytesread;
        }

        // proper EOF
        unsigned long zeroread = 0;
        stat = rfile.read(rbuff, 1, zeroread);
        CPPUNIT_ASSERT_MESSAGE("End of file", stat == OS_FILE_EOF);
        CPPUNIT_ASSERT_MESSAGE("No bytes read", zeroread == 0);
    }

    /**
     * Creates a dummy file, copies it into a new file then verifies
     * it's contents
     */
    void testCopyFile()
    {
        OsStatus stat;
        OsPath copyFrom = mRootPath + OsPath::separator + "testCopyFileFrom";
        OsPath copyTo = mRootPath + OsPath::separator + "testCopyFileTo";

        stat = OsTestUtilities::createDummyFile(copyFrom, 1000);
        CPPUNIT_ASSERT_MESSAGE("Create test file", stat == OS_SUCCESS);

        OsFile copyFromFile(copyFrom);
        copyFromFile.copy(copyTo);
        
        CPPUNIT_ASSERT_MESSAGE("Copies file exists", OsFileSystem::exists(copyTo));
        UtlBoolean ok = OsTestUtilities::verifyDummyFile(copyTo, 1000);
        CPPUNIT_ASSERT_MESSAGE("Test file verified", ok);
    }

    void testReadOnly()
    {
        OsStatus stat;
        OsPath testPath = mRootPath + OsPath::separator + "testReadOnly";
        OsFile testFile(testPath);

        testFile.touch();

        stat = testFile.setReadOnly(TRUE);
        CPPUNIT_ASSERT_MESSAGE("No error setting read only", stat == OS_SUCCESS);
        CPPUNIT_ASSERT_MESSAGE("Read only", testFile.isReadonly());

        stat = testFile.setReadOnly(FALSE);
        CPPUNIT_ASSERT_MESSAGE("No error setting read only", stat == OS_SUCCESS);
        CPPUNIT_ASSERT_MESSAGE("Not Read only", !testFile.isReadonly());
    }

    void testTouch()
    {
        OsFileInfoBase info;
        OsTime time1, time2;
        OsPath testFile = mRootPath + OsPath::separator + "testTouch";
        OsFile file(testFile);

        // Create the file
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, file.touch());

        // Get the modification time and sleep for a few seconds
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, file.getFileInfo(info));
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, info.getModifiedTime(time1));
        OsTask::delay(3000);

        // Touch the file and get the modification time again
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, file.touch());
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, file.getFileInfo(info));
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, info.getModifiedTime(time2));

        // The modification times must differ by the sleep amount
        CPPUNIT_ASSERT_MESSAGE("The time is the same", time1.seconds() != time2.seconds());
    }

    /**
     * Created dummy files and attempts to gain and deny access to locks
     */
    void testFileLocking()
    {
        OsStatus stat;
        OsPath testPath = mRootPath + OsPath::separator + "testFileLocking";
        OsFile testFile1(testPath);
        OsFile testFile2(testPath);
        OsFile testFile3(testPath);
        OsFile testFile4(testPath);
        testFile1.touch();

        stat = testFile1.open(OsFile::READ_ONLY | OsFile::FSLOCK);
        CPPUNIT_ASSERT_MESSAGE("Attempt to open a file read-only with a lock", stat != OS_SUCCESS);

        stat = testFile1.open(OsFile::READ_ONLY);
        CPPUNIT_ASSERT_MESSAGE("Open a file", stat == OS_SUCCESS);

        stat = testFile2.open(OsFile::READ_WRITE | OsFile::FSLOCK_WAIT);
        CPPUNIT_ASSERT_MESSAGE("Wait to open a file with a lock", stat == OS_SUCCESS);

        stat = testFile3.open(OsFile::READ_WRITE);
        CPPUNIT_ASSERT_MESSAGE("Open a locked file ignoring the lock", stat == OS_SUCCESS);

        stat = testFile4.open(OsFile::READ_WRITE | OsFile::FSLOCK);
        CPPUNIT_ASSERT_MESSAGE("Attempt to open a locked file with a lock", stat != OS_SUCCESS);

        testFile2.close();

        stat = testFile4.open(OsFile::READ_WRITE | OsFile::FSLOCK);
        CPPUNIT_ASSERT_MESSAGE("Open a file with a lock", stat == OS_SUCCESS);

        testFile4.close();
        testFile3.close();
        testFile1.close();

        testFile1.remove();

        // TODO: Properly test FSLOCK_WAIT and cross-process locking
    }
    
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsFileTest);
