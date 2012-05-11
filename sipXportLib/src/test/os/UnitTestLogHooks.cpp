//
// Copyright (C) 2007-2012 SIPez LLC  All rights reserved.
//
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include <os/OsIntTypes.h>

#include <os/OsSysLog.h>
#include <os/OsFileSystem.h>

#include <SipxPortUnitTestEnvironment.h>

#define LOG_DIR "./"

void beginLogHook(const char* testClassName)
{
    UtlString testLogFilename(LOG_DIR);
    testLogFilename.append(testClassName);
    testLogFilename.append(".log");

    OsSysLog::initialize(0, "UnitTest");
    OsSysLog::setLoggingPriority(PRI_DEBUG);

    // Remove old log before starting test
    OsFileSystem::remove(testLogFilename, FALSE, TRUE);

    OsSysLog::setOutputFile(0, testLogFilename);
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

