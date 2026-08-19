//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <sys/stat.h>

#include <SipxPortUnitTest.h>
#include <os/OsSysLog.h>
#include <os/OsTime.h>
#include <utl/UtlString.h>

#define TEST_CLASS_NAME "UnitTestLogHooksTest"
#define TEST_LOG_DIR "logs/" TEST_CLASS_NAME

// Size of this process's log file as of the end of testLogFileWritten.
// Only meaningful when the whole class runs in a single process.
static long sLogSizeAfterWrite = -1;

class UnitTestLogHooksTest : public SipxPortUnitTestClass
{
    CPPUNIT_TEST_SUITE(UnitTestLogHooksTest);
    CPPUNIT_TEST(testLogDirectoryCreated);
    CPPUNIT_TEST(testLogFileWritten);
    CPPUNIT_TEST(testLogRetainedAcrossMethods);
    CPPUNIT_TEST_SUITE_END();

    private:

    // Build the log path the same way UnitTestLogHooks::beginLogHook does:
    // one file per method when a method filter was given on the command
    // line, otherwise one file for the whole class.
    void getExpectedLogPath(UtlString& path)
    {
        const char* methodName =
            SipxPortUnitTestEnvironment::getTestMethodFilter();

        path = TEST_LOG_DIR;
        path.append("/");
        path.append(methodName ? methodName : TEST_CLASS_NAME);
        path.append(".log");
    }

    // stat() rather than opening the file, as the sys log task holds it
    // open for append while the tests run.
    long getFileSize(const char* path)
    {
        struct stat statInfo;
        if(stat(path, &statInfo) != 0)
        {
            return(-1);
        }
        return((long) statInfo.st_size);
    }

    void testLogDirectoryCreated()
    {
        struct stat statInfo;
        CPPUNIT_ASSERT_MESSAGE("log directory not created",
                               stat(TEST_LOG_DIR, &statInfo) == 0);
        CPPUNIT_ASSERT_MESSAGE("log directory is not a directory",
                               (statInfo.st_mode & S_IFDIR) != 0);
    }

    void testLogFileWritten()
    {
        OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                      "%s marker testLogFileWritten", TEST_CLASS_NAME);
        OsSysLog::flush(OsTime(1, 0));

        UtlString logPath;
        getExpectedLogPath(logPath);

        long size = getFileSize(logPath.data());
        CPPUNIT_ASSERT_MESSAGE("log file not created", size >= 0);
        CPPUNIT_ASSERT_MESSAGE("log file is empty", size > 0);

        sLogSizeAfterWrite = size;
    }

    void testLogRetainedAcrossMethods()
    {
        OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                      "%s marker testLogRetainedAcrossMethods",
                      TEST_CLASS_NAME);
        OsSysLog::flush(OsTime(1, 0));

        UtlString logPath;
        getExpectedLogPath(logPath);

        long size = getFileSize(logPath.data());
        CPPUNIT_ASSERT_MESSAGE("log file not created", size > 0);

        // Accumulation across methods is only observable when the whole
        // class runs in one process.  The test runner invokes a process
        // per method, and then each method owns a separate file.
        if(SipxPortUnitTestEnvironment::getTestMethodFilter() == NULL)
        {
            CPPUNIT_ASSERT_MESSAGE("testLogFileWritten did not run first",
                                   sLogSizeAfterWrite > 0);
            CPPUNIT_ASSERT_MESSAGE("log truncated between methods",
                                   size >= sLogSizeAfterWrite);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(UnitTestLogHooksTest);

