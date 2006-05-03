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

#include <os/OsTask.h>
#include <os/OsProcess.h>
#include <os/OsProcessMgr.h>

#include <sipxunit/TestUtilities.h>

class OsProcessMgrTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsProcessMgrTest);
    CPPUNIT_TEST(testManager);
    CPPUNIT_TEST_SUITE_END();


public:

    void testManager()
    {
        OsStatus stat;
        OsProcessMgr processManager;

        UtlString alias = "MyPing1";

        UtlString appName = "ping";
        UtlString params[10];
        params[0] = "127.0.0.1";
#ifdef _WIN32  //need to do this only on win32, linux already does this by default
        params[1] = "-t";
#endif
    
        OsPath inputFile = ""; //this means it will use standard input
        OsPath MyPing1OutputFile = "testManager1.out";
        OsPath MyPing2OutputFile = "testManager2.out";
        OsPath errFile = "testManager.err";
    
        processManager.setIORedirect(inputFile, MyPing1OutputFile, errFile);
        
        UtlString MyPing1("MyPing1");
        UtlString MyPing2("MyPing2");
#ifdef _WIN32
        OsPath startupDir = "";
#else
        OsPath startupDir = ".";
#endif
        stat = processManager.startProcess(MyPing1, appName, params, startupDir);
        CPPUNIT_ASSERT_MESSAGE("Started first proccess", stat == OS_SUCCESS);

        KNOWN_BUG("Unknown failure", "XPL-12");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Alias state", PROCESS_STARTED, 
            processManager.getAliasState(MyPing1));
        
        processManager.setIORedirect(inputFile, MyPing2OutputFile, errFile);
        stat = processManager.startProcess(MyPing2, appName, params, startupDir);
        CPPUNIT_ASSERT_MESSAGE("Started 2nd proccess", stat == OS_SUCCESS);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd alias state", PROCESS_STARTED, 
            processManager.getAliasState(MyPing2));
        
        //std::cout << "Waiting 2 secs before killing process MyPing1...\n";
        OsTask::delay(2000);
      
        stat = processManager.stopProcess(MyPing1);
        CPPUNIT_ASSERT_MESSAGE("Killed 1st process", stat == OS_SUCCESS);

        //std::cout << "Waiting 2 secs before killing process MyPing2...\n";
        OsTask::delay(2000);
        
        stat = processManager.stopProcess(MyPing2);
        CPPUNIT_ASSERT_MESSAGE("Killed 2nd process", stat == OS_SUCCESS);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsProcessMgrTest);

