//
// Copyright (C) 2007-2026 SIPez LLC  All rights reserved.
//
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include <os/OsIntTypes.h>

#include <os/OsSysLog.h>
#include <os/OsFileSystem.h>

#include <SipxPortUnitTestEnvironment.h>

#define LOG_DIR "./logs/"

// Path of the log file that this process already cleared.  Prevents a
// second clearing when the test framework recovers from a caught signal
// and re-enters the test class loop for the same class.
static UtlString sClearedLogFilename;

void beginLogHook(const char* testClassName)
{
    const char* testMethodName =
        SipxPortUnitTestEnvironment::getTestMethodFilter();

    // One directory per test class
    UtlString testLogDirname(LOG_DIR);
    OsFileSystem::createDir(testLogDirname);
    testLogDirname.append(testClassName);
    OsFileSystem::createDir(testLogDirname);

    // One log file per test method, as the test runner invokes a separate
    // process per method.  Each process clears only the file it owns, so
    // the logs for a class are retained across its methods without the
    // logs growing without bound from one run to the next.
    UtlString testLogFilename(testLogDirname);
    testLogFilename.append("/");
    testLogFilename.append(testMethodName ? testMethodName : testClassName);
    testLogFilename.append(".log");

    OsSysLog::initialize(0, "UnitTest");
    OsSysLog::setLoggingPriority(PRI_DEBUG);

    // Remove old log before starting test, but only once per process
    if(sClearedLogFilename.compareTo(testLogFilename) != 0)
    {
        OsFileSystem::remove(testLogFilename, FALSE, TRUE);
        sClearedLogFilename = testLogFilename;
    }

    OsSysLog::setOutputFile(0, testLogFilename);

    // setOutputFile only queues the request.  Flush so that the log file is
    // known to be open before any test code runs, otherwise early entries
    // are silently dropped.
    OsSysLog::flush(OsTime(1, 0));
}

void endLogHook(const char* testClassName)
{
    OsSysLog::flush();
    OsSysLog::shutdown();
}


static int initLogHooks()
{
    SipxPortUnitTestEnvironment::setLogHookBegin(beginLogHook);
    SipxPortUnitTestEnvironment::setLogHookEnd(endLogHook);

    return(0);
}

int sLogHookFoo = initLogHooks();

