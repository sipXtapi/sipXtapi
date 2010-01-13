// 
// 
// Copyright (C) 2010 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2010 SIPez LLC  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#if !defined(ANDROID)
#   include <execinfo.h>
#endif

// APPLICATION INCLUDES
#include <SipxPortUnitTestEnvironment.h>
#include <SipxPortUnitTestConstructor.h>
#include <SipxPortUnitTestPointFailure.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
bool SipxPortUnitTestEnvironment::sInitialized = 0;
int SipxPortUnitTestEnvironment::sInitializedSignature = 0;
int SipxPortUnitTestEnvironment::sInitializing = 0;

int SipxPortUnitTestEnvironment::sCurrentTestClassIndex = -1;
int SipxPortUnitTestEnvironment::sCurrentTestMethodIndex = -1;
int SipxPortUnitTestEnvironment::sCurrentTestPointIndex = -1;

int SipxPortUnitTestEnvironment::sTotalTestMethodCount = 0;
int SipxPortUnitTestEnvironment::sTestMethodsRun = 0;
int SipxPortUnitTestEnvironment::sTestPointsPassed = 0;
int SipxPortUnitTestEnvironment::sTestPointsFailed = 0;

int SipxPortUnitTestEnvironment::sTotalTestClassCount = 0;
SipxPortUnitTestConstructor** SipxPortUnitTestEnvironment::sTestClassesToRun = NULL;

int SipxPortUnitTestEnvironment::sNumExceptionsCaught = 0;
int SipxPortUnitTestEnvironment::sLastExceptionsCaught = 0;
int SipxPortUnitTestEnvironment::sLastExceptionClassIndex = -1;
int SipxPortUnitTestEnvironment::sNumExceptionsForSameClass = 0;
int SipxPortUnitTestEnvironment::sLastExceptionMethodIndex = -1;
int SipxPortUnitTestEnvironment::sLastExceptionTestPointIndex = -1;

// Stack to recover to if something bad happens in a test
jmp_buf sLongJumpStack;

//__________________________________________________________________________//
/* ============================= P U B L I C ============================== */

/* ============================ C R E A T O R S =========================== */

// Constructor
SipxPortUnitTestEnvironment::SipxPortUnitTestEnvironment()
{
}

SipxPortUnitTestEnvironment::~SipxPortUnitTestEnvironment()
{
}

/* ======================== M A N I P U L A T O R S ======================= */

void SipxPortUnitTestEnvironment::initializeEnvironment()
{
    if(!sInitialized || 
       sInitializedSignature != SIPX_PORT_UNIT_TEST_SIGNATURE)
    {
        if(sInitializing != SIPX_PORT_UNIT_TEST_SIGNATURE)
        {
            sInitializing = SIPX_PORT_UNIT_TEST_SIGNATURE;

            sCurrentTestClassIndex = -1;
            sCurrentTestMethodIndex = -1;
            sCurrentTestPointIndex = -1;

            sTotalTestMethodCount = 0;
            sTestMethodsRun = 0;
            sTestPointsPassed = 0;
            sTestPointsFailed = 0;

            sTotalTestClassCount = 0;
            sTestClassesToRun = new SipxPortUnitTestConstructor*[SIPX_PORT_UNIT_MAX_TEST_CLASSES];

            sNumExceptionsCaught = -1;
            sLastExceptionsCaught = 0;
            resetExceptionState();

            sInitializing = 0;
            sInitializedSignature = SIPX_PORT_UNIT_TEST_SIGNATURE;
            sInitialized = 1;
        }
    }
}

void SipxPortUnitTestEnvironment::dumpCurrentTestState()
{
    char buffer[SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE];
    sprintf(buffer, "current class index: %d\ncurrent method index: %d\ncurrent test point index: %d\n",
            sCurrentTestClassIndex,
            sCurrentTestMethodIndex,
            sCurrentTestPointIndex);
    printOut(buffer);
}

void SipxPortUnitTestEnvironment::resetExceptionState()
{
    initializeEnvironment();

    //printOut("SipxPortUnitTestEnvironment::resetExceptionState\n");
    sNumExceptionsCaught++;
    if(sLastExceptionClassIndex == sCurrentTestClassIndex)
    {
        sNumExceptionsForSameClass++;
    }
    else
    {
        sNumExceptionsForSameClass = 0;
    }
    sLastExceptionClassIndex = sCurrentTestClassIndex;
    sLastExceptionMethodIndex = sCurrentTestMethodIndex;
    sLastExceptionTestPointIndex = sCurrentTestPointIndex;
}

void SipxPortUnitTestEnvironment::registerTestConstructor(SipxPortUnitTestConstructor* testClassToAdd)
{
    initializeEnvironment();

    assert(testClassToAdd);
    sTestClassesToRun[sTotalTestClassCount] = testClassToAdd;
    sTotalTestClassCount++;
}

void SipxPortUnitTestEnvironment::signalHandler(int signalCaught)
{
    char buffer[256];
    sprintf(buffer, "ABORT: due to caught signal: %d", signalCaught);
    printOut(buffer);

    assert(sCurrentTestClassIndex >= 0);
    assert(sCurrentTestClassIndex < sTotalTestClassCount);
    assert(sCurrentTestClassIndex < SIPX_PORT_UNIT_MAX_TEST_CLASSES);

    sprintf(buffer, " while in %s test class: %s between line: %d and next test point\n", 
            sTestClassesToRun[sCurrentTestClassIndex]->getTestPointFilename(),
            sTestClassesToRun[sCurrentTestClassIndex]->getClassName(),
            sTestClassesToRun[sCurrentTestClassIndex]->getTestPointLine());
    printOut(buffer);

    sTestClassesToRun[sCurrentTestClassIndex]->addSignalCaughtFailure(signalCaught);

#if !defined(ANDROID)
    // Try to get a back trace, but we loose the top function call due to
    // being in the signal handler
    void* traceStack[SIPX_PORT_UNIT_MAX_STACK_TRACE_SIZE];
    int stackSize = backtrace(traceStack, SIPX_PORT_UNIT_MAX_STACK_TRACE_SIZE);
    char** entryPointArray = backtrace_symbols(traceStack, stackSize);
    printOut("stack: =========\n");

    for(int stackIndex = 0; stackIndex < stackSize; stackIndex++)
    {
        sprintf(buffer, "  %d] %s\n", stackIndex + 1, entryPointArray[stackIndex]);
        printOut(buffer);
    }
#endif

    resetExceptionState();
    sLastExceptionsCaught = signalCaught;

    siglongjmp(sLongJumpStack, 1);
}

void SipxPortUnitTestEnvironment::runTests()
{
    initializeEnvironment();


    // Prepare to run tests and catch signals if something bad happends
    signal(SIGFPE, SipxPortUnitTestEnvironment::signalHandler);
    signal(SIGSEGV, SipxPortUnitTestEnvironment::signalHandler);

    // Render test inforation for each test class first time this gets run
    // This will be run more than once if we catch a signal and are able
    // to recover.
    if(sigsetjmp(sLongJumpStack, 1)  == 0 /*sCurrentTestClassIndex == -1i*/)
    {
        sCurrentTestClassIndex = 0;
        sTotalTestMethodCount = 0;
        for(int testIndex = 0; testIndex < sTotalTestClassCount; testIndex++)
        {
            assert(sTestClassesToRun[testIndex]);
            sTotalTestMethodCount +=
                (sTestClassesToRun[testIndex])->getTestMethodCount();
        }
    }

    // We are recovering from an exception/signal that was caught
    else
    {
        if(sCurrentTestMethodIndex == -1)
        {
            // caught signal before running first method of test class
            // something must be wrong with the initialization or
            // constuction of the test class.  So skip the whole test
            // class
            sCurrentTestClassIndex++;
            char buffer[256];
            sprintf(buffer, "initialization or construction of: %s failed skipping to class: %s\n",
                    sTestClassesToRun[sCurrentTestClassIndex-1]->getClassName(),
                    sTestClassesToRun[sCurrentTestClassIndex]->getClassName());
            printOut(buffer);

        }
        else
        {
            printOut("recovered from failure, starting next test method\n");
            // Move to the next test method as something went bad
            sCurrentTestMethodIndex++;
        }
        dumpCurrentTestState();
    }


    for(;sCurrentTestClassIndex < sTotalTestClassCount; sCurrentTestClassIndex++)
    {
        assert(sTestClassesToRun[sCurrentTestClassIndex]);

        sTestClassesToRun[sCurrentTestClassIndex]->runAllMethodsFrom(sCurrentTestMethodIndex);

        // Can now free up the test class, but we keep the test class 
        //constructor around so we can get at the stats
        sTestClassesToRun[sCurrentTestClassIndex]->releaseTestClass();

        sCurrentTestMethodIndex = 0;
        sCurrentTestPointIndex = 0;
    }

    // Now that we are done with the tests, we do not want the signal
    // handler to catch stuff any more
    signal(SIGFPE, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);

}

void SipxPortUnitTestEnvironment::reportResults()
{
    initializeEnvironment();

    printOut("\nTEST SUMMARY:\n");

    char buffer[SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE];
    sprintf(buffer, "Total: %d test class(es), %d test method(s), ?? test points\n",
            sTotalTestClassCount, sTotalTestMethodCount);
    printOut(buffer);

    sprintf(buffer, "Ran: %d test class(es), %d test method(s), %d test points\n",
            sCurrentTestClassIndex, sTestMethodsRun, 
            sTestPointsPassed + sTestPointsFailed);
    printOut(buffer);

    sprintf(buffer, "%d test points passed, %d test points failed, %d tests aborted\n",
            sTestPointsPassed, sTestPointsFailed, sNumExceptionsCaught);
    printOut(buffer);

    // Dig out all the failure messages for all of the classes
    if(sTestPointsFailed || sNumExceptionsCaught)
    {
        sprintf(buffer,
                "\n=============================================\n%d FAILURE(S), %d ABORT(S):\n",
                sTestPointsFailed, sNumExceptionsCaught);
        printOut(buffer);
    }

    int failureIndex = 0;
    SipxPortUnitTestPointFailure* failure = 0;
    for(int classIndex = 0; classIndex < sTotalTestClassCount; classIndex++)
    {
        SipxPortUnitTestConstructor* classConstr = sTestClassesToRun[classIndex];
        assert(classConstr);
        failureIndex = 0;
        while(failureIndex < SIPX_PORT_UNIT_MAX_TEST_ERRORS_PER_CLASS &&
              (failure = classConstr->getFailure(failureIndex)))
        {
             // If first failure for this test class label it
             if(failureIndex == 0)
             {
                 sprintf(buffer, "\n%s failure(s):\n", classConstr->getClassName());
                 printOut(buffer);
             }

             sprintf(buffer, "  %d) ", failureIndex + 1);
             printOut(buffer);
             printOut(failure->getMessage());

             failureIndex++;
        }

        if(failureIndex == SIPX_PORT_UNIT_MAX_TEST_ERRORS_PER_CLASS)
        {
            sprintf(buffer, "WARNING: maximum failures (%d) reached for test class: %s\n",
                    SIPX_PORT_UNIT_MAX_TEST_ERRORS_PER_CLASS, 
                    classConstr->getClassName());
            printOut(buffer);
        }
    }
}

void SipxPortUnitTestEnvironment::printOut(const char* messageText)
{
    printf("%s", messageText);
}

/* ========================== A C C E S S O R S =========================== */

char* SipxPortUnitTestEnvironment::newCopyString(const char* stringToCopy)
{
    char* stringCopy = 0;
    if(stringToCopy)
    {
        int nameLength = strlen(stringToCopy);
        stringCopy = new char[nameLength + 1];
        strcpy(stringCopy, stringToCopy);
    }
    return(stringCopy);
}

void SipxPortUnitTestEnvironment::incrementMethodsRun()
{
    sTestMethodsRun++;
}

void SipxPortUnitTestEnvironment::setMethodIndex(int methodIndex)
{
    sCurrentTestMethodIndex = methodIndex;
}

void SipxPortUnitTestEnvironment::resetTestPointIndex()
{
    sCurrentTestPointIndex = -1;
}

void SipxPortUnitTestEnvironment::incrementTestPointIndex()
{
    sCurrentTestPointIndex++;
}

void SipxPortUnitTestEnvironment::incrementTestPointsPassed()
{
    sTestPointsPassed++;
}

void SipxPortUnitTestEnvironment::incrementTestPointsFailed()
{
    sTestPointsFailed++;
}

int SipxPortUnitTestEnvironment::getTestPointFailureCount()
{
    return(sTestPointsFailed);
}

int SipxPortUnitTestEnvironment::getTestAbortCount()
{
    return(sNumExceptionsCaught);
}

/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */

//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */

/* ========================== F U N C T I O N S =========================== */

//__________________________________________________________________________//
