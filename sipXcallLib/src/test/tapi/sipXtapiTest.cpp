//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include "sipXtapiTest.h"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "EventRecorder.h"
#include "EventValidator.h"
#include "os/OsBSem.h"
#include "os/OsDatagramSocket.h"


#ifdef PURIFY
    #include "pure.h"
#endif
SIPX_INST g_hInst = NULL ;
EventRecorder g_recorder ;
EventRecorder g_lineRecorder;

SIPX_INST g_hInst2 = NULL ;
EventRecorder g_recorder2 ;

SIPX_INST g_hInst3 = NULL ;
EventRecorder g_recorder3 ;

SIPX_INST g_hInst4 = NULL ;
EventRecorder g_recorder4 ;

SIPX_INST g_hInst5 = NULL ;

SIPX_CALL ghCallHangup = 0;

TestStunServerTask* g_pPrimaryStunServer ;
TestStunServerTask* g_pPrimaryStunServer2 ;
TestStunServerTask* g_pSecondaryStunServer ;
TestStunServerTask* g_pSecondaryStunServer2 ;

OsDatagramSocket* g_pPrimaryStunSeverSocket ;
OsDatagramSocket* g_pPrimaryStunSeverSocket2 ;
OsDatagramSocket* g_pSecondaryStunServerSocket ;
OsDatagramSocket* g_pSecondaryStunServerSocket2 ;


int main(int argc, char* argv[])
{
    enableConsoleOutput(FALSE) ;

    // Get the top level suite from the registry
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    // Adds the test to the list of tests to run
    CppUnit::TextUi::TestRunner runner ;
    runner.addTest(suite) ;

    // Change the default outputter to a compiler error format outputter
    runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr)) ;

    // Run the tests.
    bool wasSuccessful = runner.run() ;

    // Return error code 1 if one of the tests failed.
    return wasSuccessful ? 0 : 1 ;
}


CPPUNIT_TEST_SUITE_REGISTRATION( sipXtapiTestSuite );

void sipXtapiTestSuite::testNothing() 
{
    // Effectively does nothing; however, prime memory leak
    // detection.
    checkForLeaks() ;
}


sipXtapiTestSuite::sipXtapiTestSuite()    
{

}

OsBSem suiteLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
void sipXtapiTestSuite::setUp()
{
#ifdef _WIN32
#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT);    

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF) ;
    _CrtMemCheckpoint( &msBeforeTest );
#endif
#endif

    suiteLock.acquire();
    sipxConfigSetLogLevel(LOG_LEVEL_DEBUG);
    sipxConfigSetLogFile("sipXtapiTests.txt") ;

    if (g_hInst == NULL)
    {
	    sipxInitialize(&g_hInst, 8000, 8000, 8001, 8050, 32, HINST_ADDRESS, "127.0.0.1") ;
    }

    if (g_hInst2 == NULL)
    {
	    sipxInitialize(&g_hInst2, 9100, 9100, 9101, 9050, 32, HINST2_ADDRESS, "127.0.0.1") ;
    }

    if (g_hInst3 == NULL)
    {
	    sipxInitialize(&g_hInst3, 10000, 10000, 10001, 10050, 32, HINST3_ADDRESS, "127.0.0.1") ;
    }

    if (g_hInst4 == NULL)
    {
	    sipxInitialize(&g_hInst4, 12070, 12070, 12071, 12050, 32, HINST4_ADDRESS, "127.0.0.1") ;
    }


/*
    if (g_hInst5 == NULL)
    {
        sipxInitialize(&g_hInst5, 5060, 5060, 5061, 13000, 32) ;
    }
*/

#ifdef PURIFY
    PurifyClearLeaks();
#endif
    suiteLock.release();
} 

void sipXtapiTestSuite::tearDown()
{
#ifdef _WIN32
#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
    static bool bFirstRun = true ;
#endif
#endif
    SIPX_RESULT rc ;

    suiteLock.acquire();
    
#ifdef PURIFY
    Sleep(250000);
    PurifyNewLeaks();
#endif
        
    if (g_hInst != NULL)
    {
        rc = sipxUnInitialize(g_hInst);
        if (rc != SIPX_RESULT_SUCCESS)
        {
            printf("\nERROR: sipxTapiTest -- Forcing shutdown of g_hInst (0x%08X)\n", g_hInst) ;
            rc = sipxUnInitialize(g_hInst, true);
        }        
        g_hInst = NULL;
    }

    if (g_hInst2 != NULL)
    {
        rc = sipxUnInitialize(g_hInst2);
        if (rc != SIPX_RESULT_SUCCESS)
        {
            printf("\nERROR: sipxTapiTest -- Forcing shutdown of g_hInst2 (0x%08X)\n", g_hInst2) ;
            rc = sipxUnInitialize(g_hInst2, true);
        }        
        g_hInst2 = NULL;
    }

    if (g_hInst3 != NULL)
    {
        rc = sipxUnInitialize(g_hInst3);
        if (rc != SIPX_RESULT_SUCCESS)
        {
            printf("\nERROR: sipxTapiTest -- Forcing shutdown of g_hInst3 (0x%08X)\n", g_hInst3) ;
            rc = sipxUnInitialize(g_hInst3, true);
        }        
        g_hInst3 = NULL;
    }

    if (g_hInst4 != NULL)
    {
        rc = sipxUnInitialize(g_hInst4);
        if (rc != SIPX_RESULT_SUCCESS)
        {
            printf("\nERROR: sipxTapiTest -- Forcing shutdown of g_hInst4 (0x%08X)\n", g_hInst4) ;
            rc = sipxUnInitialize(g_hInst4, true);
        }                
        g_hInst4 = NULL;
    }

    if (g_hInst5 != NULL)
    {
        rc = sipxUnInitialize(g_hInst5);
        if (rc != SIPX_RESULT_SUCCESS)
        {
            printf("\nERROR: sipxTapiTest -- Forcing shutdown of g_hInst5 (0x%08X)\n", g_hInst5) ;
            rc = sipxUnInitialize(g_hInst5, true);
        }                        
        g_hInst5 = NULL;
    }

    sipxFlushHandles() ;

    suiteLock.release();

#ifdef _WIN32
#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
    if (bFirstRun == false)
    {   
        _CrtMemCheckpoint( &msAfterTest );
        _CrtMemState diff ;

        if (_CrtMemDifference( &diff, &msBeforeTest, &msAfterTest))
        {
            _CrtMemDumpStatistics( &diff );
            _CrtMemDumpAllObjectsSince(&msBeforeTest) ;
        }        
    }
    else
    {
        bFirstRun = false ;
    }
#endif
#endif
}

void sipXtapiTestSuite::setupStunServer() 
{
    // Cheating: We should use multiple interface; assuming machine has only 
    // one network interface and simply choosing different ports -- this will
    // result in incorrect determination of NAT type.
#define PRIMARY_HOST        "127.0.0.1"
#define SECONDARY_HOST      "127.0.0.1"


    g_pPrimaryStunSeverSocket     = new OsDatagramSocket(0, NULL, STUN_SERVER_PORT_1,  PRIMARY_HOST) ;
    g_pPrimaryStunSeverSocket2    = new OsDatagramSocket(0, NULL, STUN_SERVER_PORT_2,  PRIMARY_HOST) ;
    g_pSecondaryStunServerSocket  = new OsDatagramSocket(0, NULL, STUN_SERVER_PORT_3, SECONDARY_HOST) ;
    g_pSecondaryStunServerSocket2 = new OsDatagramSocket(0, NULL, STUN_SERVER_PORT_4, SECONDARY_HOST) ;

    if (    !g_pPrimaryStunSeverSocket->isOk() || 
            !g_pPrimaryStunSeverSocket2->isOk() || 
            !g_pSecondaryStunServerSocket->isOk() || 
            !g_pSecondaryStunServerSocket2->isOk())
    {
        printf("\nTestStunServer Binding: \n    STUN:%s:%d [%s]\n    STUN:%s:%d [%s]\n    STUN:%s:%d [%s]\n    STUN:%s:%d [%s]\n",
                PRIMARY_HOST, STUN_SERVER_PORT_1,
                (const char*) (g_pPrimaryStunSeverSocket->isOk() ? "OK" : "ERROR"),
                PRIMARY_HOST, STUN_SERVER_PORT_2,
                (const char*) (g_pPrimaryStunSeverSocket2->isOk() ? "OK" : "ERROR"),
                SECONDARY_HOST, STUN_SERVER_PORT_3,
                (const char*) (g_pSecondaryStunServerSocket->isOk() ? "OK" : "ERROR"),
                SECONDARY_HOST, STUN_SERVER_PORT_4,
                (const char*) (g_pSecondaryStunServerSocket2->isOk() ? "OK" : "ERROR")) ;
    }

    g_pPrimaryStunServer = new TestStunServerTask(g_pPrimaryStunSeverSocket, 
                                                  g_pPrimaryStunSeverSocket2, 
                                                  g_pSecondaryStunServerSocket, 
                                                  g_pSecondaryStunServerSocket2) ;
    g_pPrimaryStunServer->start() ;
    g_pPrimaryStunServer2 = new TestStunServerTask(g_pPrimaryStunSeverSocket2, 
                                                   g_pPrimaryStunSeverSocket, 
                                                   g_pSecondaryStunServerSocket2, 
                                                   g_pSecondaryStunServerSocket) ;
    g_pPrimaryStunServer2->start() ;
    g_pSecondaryStunServer = new TestStunServerTask(g_pSecondaryStunServerSocket, 
                                                    g_pSecondaryStunServerSocket2, 
                                                    g_pPrimaryStunSeverSocket, 
                                                    g_pPrimaryStunSeverSocket2) ;
    g_pSecondaryStunServer->start() ;
    g_pSecondaryStunServer2 = new TestStunServerTask(g_pSecondaryStunServerSocket2, 
                                                     g_pSecondaryStunServerSocket, 
                                                     g_pPrimaryStunSeverSocket2, 
                                                     g_pPrimaryStunSeverSocket) ;
    g_pSecondaryStunServer2->start() ;
}

void sipXtapiTestSuite::teardownStunServer() 
{
    g_pPrimaryStunServer->requestShutdown() ;
    g_pPrimaryStunServer2->requestShutdown() ;
    g_pSecondaryStunServer->requestShutdown() ;
    g_pSecondaryStunServer2->requestShutdown() ;

    g_pPrimaryStunSeverSocket->close() ;
    g_pPrimaryStunSeverSocket2->close() ;
    g_pSecondaryStunServerSocket->close() ;
    g_pSecondaryStunServerSocket2->close() ;

    delete g_pPrimaryStunServer ;
    g_pPrimaryStunServer = NULL ;
    delete g_pPrimaryStunServer2 ;
    g_pPrimaryStunServer2 = NULL ;
    delete g_pSecondaryStunServer ;
    g_pSecondaryStunServer = NULL ;
    delete g_pSecondaryStunServer2 ;
    g_pSecondaryStunServer2 = NULL ;
    
    delete g_pPrimaryStunSeverSocket ;
    g_pPrimaryStunSeverSocket = NULL ;
    delete g_pPrimaryStunSeverSocket2 ;
    g_pPrimaryStunSeverSocket2 = NULL ;
    delete g_pSecondaryStunServerSocket ;
    g_pSecondaryStunServerSocket = NULL ;
    delete g_pSecondaryStunServerSocket2 ;
    g_pSecondaryStunServerSocket2 = NULL ;   
}

void sipXtapiTestSuite::setStunServerMode(STUN_TEST_MODE mode) 
{
    g_pPrimaryStunServer->setTestMode(mode) ;
    g_pPrimaryStunServer2->setTestMode(mode) ;
    g_pSecondaryStunServer->setTestMode(mode) ;
    g_pSecondaryStunServer->setTestMode(mode) ;
}

void sipXtapiTestSuite::setStunServerValidator(EventValidator* pValidator) 
{
    g_pPrimaryStunServer->setEventValidator(pValidator) ;
    g_pPrimaryStunServer2->setEventValidator(pValidator) ;
    g_pSecondaryStunServer->setEventValidator(pValidator) ;
    g_pSecondaryStunServer->setEventValidator(pValidator) ;
}


void sipXtapiTestSuite::checkForLeaks()
{
    if (g_hInst)
    {
        checkForCallLeaks(g_hInst) ;
    }
    if (g_hInst2)
    {
        checkForCallLeaks(g_hInst2) ;
    }
    if (g_hInst3)
    {
        checkForCallLeaks(g_hInst3) ;
    }
    if (g_hInst4)
    {
        checkForCallLeaks(g_hInst4) ;
    }
    
    if (g_hInst5)
    {
        checkForCallLeaks(g_hInst5) ;
    }

    sipxCheckForHandleLeaks() ;
}

#define MAX_CALL_CHECK  16
void sipXtapiTestSuite::checkForCallLeaks(SIPX_INST hInst) 
{
    SIPX_RESULT rc ;
    UtlString callIds[MAX_CALL_CHECK] ;
    int numCalls ;
    
    rc = sipxGetActiveCallIds(hInst, MAX_CALL_CHECK, numCalls, callIds) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    if (rc == SIPX_RESULT_SUCCESS)
    {
        if (numCalls != 0)
        {
            printf("Call leak(s) detected (%d):\n", numCalls) ;
            for (int i=0; i<numCalls; i++)
            {
                printf("\tCallId=%s\n", callIds[i].data()) ;
            }

            CPPUNIT_ASSERT_EQUAL(numCalls, 0) ;
        }
    }     
}
