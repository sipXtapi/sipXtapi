//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsTask.h>
#include <os/OsProcess.h>

class OsProcessTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsProcessTest);
#ifndef _WIN32
    CPPUNIT_TEST(testLaunch);
#endif
    CPPUNIT_TEST_SUITE_END();

public:

    void testLaunch()
    {
        OsStatus stat;

        UtlString appName = "ping";
        UtlString params[10];
        params[0] = "127.0.0.1";

#ifdef _WIN32  //need to do this only on win32, linux already does this by default
        params[1] = "-t";
#endif
        OsPath inputFile = ""; //this means it will use standard input
        OsPath outputFile = "testLaunch.out";
        OsPath errFile = "testLaunch.err";

        OsProcess process;
        process.setIORedirect(inputFile, outputFile, errFile);

        UtlString envKey =   "TESTKEY1";
        UtlString envValue = "TESTVALUE1";
        process.setEnv(envKey,envValue);
    
        envKey =  "TESTKEY2";
        envValue ="TESTVALUE2";
        process.setEnv(envKey,envValue);

        envKey = "TESTKEY3";
        envValue = "TESTVALUE3";
        process.setEnv(envKey,envValue);

        OsPath startupDir = ".";

        //std::cout << "Launching process: " << appName.data() << std::endl;
        stat = process.launch(appName,params,startupDir);
        CPPUNIT_ASSERT_MESSAGE("Launched application", stat == OS_SUCCESS);
        KNOWN_BUG("Unknown failure", "XPL-12");
        CPPUNIT_ASSERT_MESSAGE("Application running", process.isRunning());
        
        int priority;
        process.setPriority(1);
        process.getPriority(priority);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Set priority ok", 1, priority);

        OsProcess newProcess;
        stat = OsProcess::getByPID(process.getPID(), newProcess);
        CPPUNIT_ASSERT_MESSAGE("Got process pid ok", stat == OS_SUCCESS);

        //std::cout << "Waiting 5 secs before killing process..." << std::endl;
        OsTask::delay(5000);
        stat = newProcess.kill();
        CPPUNIT_ASSERT_MESSAGE("Able to kill process", stat == OS_SUCCESS);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsProcessTest);

