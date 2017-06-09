// 
// 
// Copyright (C) 2010-2017 SIPez LLC  All rights reserved.
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
#if !defined(ANDROID) && !defined(_WIN32)
#   include <execinfo.h>
#endif

#if defined(_WIN32)
#define siglongjmp longjmp
#define sigsetjmp(BUF, OPT) setjmp(BUF)
#endif

// APPLICATION INCLUDES
#include <SipxPortUnitTestEnvironment.h>
#include <SipxPortUnitTestConstructor.h>
#include <SipxPortUnitTestPointFailure.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DONT_CATCH_SIGNALS

// STATIC VARIABLE INITIALIZATIONS
bool SipxPortUnitTestEnvironment::sInitialized = 0;
int SipxPortUnitTestEnvironment::sInitializedSignature = 0;
int SipxPortUnitTestEnvironment::sInitializing = 0;

int SipxPortUnitTestEnvironment::sCurrentTestClassIndex = -1;
int SipxPortUnitTestEnvironment::sCurrentTestMethodIndex = -1;
int SipxPortUnitTestEnvironment::sCurrentTestPointIndex = -1;
int SipxPortUnitTestEnvironment::sTestClassesRun = 0;
SipxPortUnitTestClass* SipxPortUnitTestEnvironment::spCurrentTestClass = 0;

int SipxPortUnitTestEnvironment::sTotalTestMethodCount = 0;
int SipxPortUnitTestEnvironment::sTestMethodsRun = 0;
int SipxPortUnitTestEnvironment::sTestPointsPassed = 0;
int SipxPortUnitTestEnvironment::sTestPointsFailed = 0;

int SipxPortUnitTestEnvironment::sTotalTestClassCount = 0;
SipxPortUnitTestConstructor** SipxPortUnitTestEnvironment::sTestClassesToRun = NULL;

bool SipxPortUnitTestEnvironment::sCatchSignals = true;
int SipxPortUnitTestEnvironment::sNumExceptionsCaught = 0;
int SipxPortUnitTestEnvironment::sLastExceptionsCaught = 0;
int SipxPortUnitTestEnvironment::sLastExceptionClassIndex = -1;
int SipxPortUnitTestEnvironment::sNumExceptionsForSameClass = 0;
int SipxPortUnitTestEnvironment::sLastExceptionMethodIndex = -1;
int SipxPortUnitTestEnvironment::sLastExceptionTestPointIndex = -1;

void (*SipxPortUnitTestEnvironment::sLogHookBegin)(const char* testName) = NULL;
void (*SipxPortUnitTestEnvironment::sLogHookEnd)(const char* testName) = NULL;

SipxUnitStringOutputter SipxPortUnitTestEnvironment::sStringOutputMethod = defaultPrintOut;

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
            sTestClassesRun = 0;
            spCurrentTestClass = 0;

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

#if !defined(ANDROID) && !defined(_WIN32)
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

void SipxPortUnitTestEnvironment::runTests(const char* testClassFilterName)
{
    char* testClassName = NULL;
    const char* testMethodName = NULL;
    if(testClassFilterName)
    {
        const char* classNameEnd = strchr(testClassFilterName, ':');
        if(classNameEnd)
        {
            // Get just the class name (text prior to ':')
            int classNameLength = classNameEnd - testClassFilterName;
            testClassName = new char[classNameLength + 1];
            memcpy(testClassName, testClassFilterName, classNameLength);
            testClassName[classNameLength] = '\0';

            // If there are 2 colins ("::"), get the method name
            if(testClassFilterName[classNameLength + 1] == ':')
            {
                testMethodName = &(testClassFilterName[classNameLength + 2]);
            }
        }
        else
        {
            int classNameLength = strlen(testClassFilterName);
            testClassName = new char[classNameLength + 1];
            memcpy(testClassName, testClassFilterName, classNameLength);
            testClassName[classNameLength] = '\0';
        }
       
        printf("Only running test class: %s method%s%s\n",
               testClassName,
               testMethodName ? ": " : "(s)",
               testMethodName ? testMethodName : "");
    }

    initializeEnvironment();


    if(SipxPortUnitTestEnvironment::sCatchSignals)
    {
        // Prepare to run tests and catch signals if something bad happends
        signal(SIGABRT, SipxPortUnitTestEnvironment::signalHandler);
        signal(SIGFPE, SipxPortUnitTestEnvironment::signalHandler);
        signal(SIGSEGV, SipxPortUnitTestEnvironment::signalHandler);
        signal(SIGILL, SipxPortUnitTestEnvironment::signalHandler);
    }

    // Render test inforation for each test class first time this gets run
    // This will be run more than once if we catch a signal and are able
    // to recover.
    if(sigsetjmp(sLongJumpStack, 1)  == 0 /*sCurrentTestClassIndex == -1i*/)
    {
        sCurrentTestClassIndex = 0;
        sTestClassesRun = 0;
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
        if(testClassName == NULL || 
           strcmp(testClassName, "") == 0 ||
           strcmp(testClassName, sTestClassesToRun[sCurrentTestClassIndex]->getClassName()) == 0)
        {
            assert(sTestClassesToRun[sCurrentTestClassIndex]);

            if(sLogHookBegin)
            {
                sLogHookBegin(sTestClassesToRun[sCurrentTestClassIndex]->getClassName());
            }

            if(testMethodName)
            {
                sTestClassesToRun[sCurrentTestClassIndex]->runMethod(testMethodName);
            }
            else
            {
                sTestClassesToRun[sCurrentTestClassIndex]->runAllMethodsFrom(sCurrentTestMethodIndex);
            }

            // Can now free up the test class, but we keep the test class 
            //constructor around so we can get at the stats
            sTestClassesToRun[sCurrentTestClassIndex]->releaseTestClass();

            if(sLogHookEnd)
            {
                sLogHookEnd(sTestClassesToRun[sCurrentTestClassIndex]->getClassName());
            }

            sCurrentTestMethodIndex = 0;
            sCurrentTestPointIndex = 0;
            sTestClassesRun++;
        }
    }

    if(SipxPortUnitTestEnvironment::sCatchSignals)
    {
        // Now that we are done with the tests, we do not want the signal
        // handler to catch stuff any more
        signal(SIGFPE, SIG_DFL);
        signal(SIGSEGV, SIG_DFL);
        signal(SIGILL, SIG_DFL);
    }

    delete testClassName;
}

void SipxPortUnitTestEnvironment::listTests(const char* testClassName)
{
    initializeEnvironment();

    for(int classIndex = 0; classIndex < sTotalTestClassCount; classIndex++)
    {
        SipxPortUnitTestConstructor* classConstr = sTestClassesToRun[classIndex];
        assert(classConstr);

        const char* constructorTestClassName = classConstr->getClassName();

        //printf("testClassName: %s constructorTestClassName: %s\n",
        //        testClassName, constructorTestClassName);
        if(testClassName == NULL ||
           strcmp(testClassName, "") == 0 ||
           strcmp(testClassName, constructorTestClassName) == 0)
        {
            int methodCount = classConstr->getTestMethodCount();
            //printf("Unit test class: %s with %d methods\n",
            //       constructorTestClassName, methodCount);


            for(int methodIndex = 0; methodIndex < methodCount; methodIndex++)
            {
                printf("%s::%s\n",
                       constructorTestClassName,
                       classConstr->getTestMethodName(methodIndex));
            }
        }
    }
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
            sTestClassesRun, sTestMethodsRun, 
            sTestPointsPassed + sTestPointsFailed);
    printOut(buffer);

    sprintf(buffer, "%d test points passed, %d test points failed, %d tests aborted\n",
            sTestPointsPassed, sTestPointsFailed, sNumExceptionsCaught);
    printOut(buffer);

    // Detail successes
    printOut("\n=============================================\nSuccess tallies:\n");

    for(int classIndex = 0; classIndex < sTotalTestClassCount; classIndex++)
    {
        SipxPortUnitTestConstructor* classConstr = sTestClassesToRun[classIndex];
        assert(classConstr);
        int passed = classConstr->getPassedTestPointCount();
        sprintf(buffer,
                "%s: %d test methods, %d/%d test points succeeded\n",
                classConstr->getClassName(),
                classConstr->getTestMethodCount(),
                passed,
                (passed + classConstr->getFailedTestPointCount()));
        printOut(buffer);
    }

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
                 sprintf(buffer, "\n%s %d failure(s):\n", classConstr->getClassName(), classConstr->getFailureCount());
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

void SipxPortUnitTestEnvironment::defaultPrintOut(const char* messageText)
{
    printf("%s", messageText);
}

void SipxPortUnitTestEnvironment::printOut(const char* messageText)
{
    if(sStringOutputMethod)
    {
        (sStringOutputMethod)(messageText);
    }
}

bool SipxPortUnitTestEnvironment::areEqual(long arg1, long arg2)
{
   return(arg1 == arg2);
}

bool SipxPortUnitTestEnvironment::areEqual(const UtlString& arg1, const UtlString& arg2)
{
   return(arg1.compareTo(arg2) == 0);
}

bool SipxPortUnitTestEnvironment::areEqual(const UtlContainable& arg1, const UtlContainable& arg2)
{
   return(arg1.isEqual(&arg2));
}

bool SipxPortUnitTestEnvironment::areEqual(const UtlContainable* arg1, const UtlContainable* arg2)
{
   return(arg1 == arg2 || (arg1 && arg2 && arg1->isEqual(arg2)));
}

bool SipxPortUnitTestEnvironment::areEqual(void* arg1, void* arg2)
{
   return(arg1 == arg2);
}

void SipxPortUnitTestEnvironment::makeAssertNotEqualMessage(char* messageBuffer, const char* notEqualMessage, 
                                                            const char* arg1String, const char* arg2String,
                                                            const char* fileName, const char* className, 
                                                            const char* methodName, int testPoint, int lineNum, 
                                                            int arg1, int arg2)
{
    snprintf(messageBuffer, SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE - 1,
             "%s, \"%s\" is not equal to \"%s\", file: %s %s::%s test point: %d line: %d\n\t%s=%d\n\t%s=%d\n",
             notEqualMessage, arg1String, arg2String,
             fileName, className, methodName, testPoint, lineNum, arg1String, arg1, arg2String, arg2);
}

void SipxPortUnitTestEnvironment::makeAssertNotEqualMessage(char* messageBuffer, const char* notEqualMessage, 
                                                            const char* arg1String, const char* arg2String,
                                                            const char* fileName, const char* className, 
                                                            const char* methodName, int testPoint, int lineNum, 
                                                            const char* arg1, const char* arg2)
{
    snprintf(messageBuffer, SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE - 1,
             "%s, \"%s\" is not equal to \"%s\", file: %s %s::%s test point: %d line: %d\n\t%s=\"%s\"\n\t%s=\"%s\"\n",
             notEqualMessage, arg1String, arg2String,
             fileName, className, methodName, testPoint, lineNum, arg1String, arg1, arg2String, arg2);
}

void SipxPortUnitTestEnvironment::makeAssertNotEqualMessage(char* messageBuffer, const char* notEqualMessage, 
                                                            const char* arg1String, const char* arg2String,
                                                            const char* fileName, const char* className, 
                                                            const char* methodName, int testPoint, int lineNum, 
                                                            const void* arg1, const void* arg2)
{
    snprintf(messageBuffer, SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE - 1,
             "%s, \"%s\" is not equal to \"%s\", file: %s %s::%s test point: %d line: %d\n\t%s=%p\n\t%s=%p\n",
             notEqualMessage, arg1String, arg2String,
             fileName, className, methodName, testPoint, lineNum, arg1String, arg1, arg2String, arg2);
}

void SipxPortUnitTestEnvironment::makeAssertNotEqualMessage(char* messageBuffer, const char* notEqualMessage, 
                                                            const char* arg1String, const char* arg2String,
                                                            const char* fileName, const char* className, 
                                                            const char* methodName, int testPoint, int lineNum, 
                                                            const UtlContainable* arg1, const UtlContainable* arg2)
{
    // No generic way to get value of UtlContainable so we do not print it
    snprintf(messageBuffer, SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE - 1,
             "%s, \"%s\" is not equal to \"%s\", file: %s %s::%s test point: %d line: %d\n",
             notEqualMessage, arg1String, arg2String,
             fileName, className, methodName, testPoint, lineNum); //, arg1String, arg1, arg2String, arg2);
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

void SipxPortUnitTestEnvironment::setCatchSignals(bool enable)
{
    SipxPortUnitTestEnvironment::sCatchSignals = enable;
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

SipxPortUnitTestClass* SipxPortUnitTestEnvironment::getCurrentTestClass()
{
    return(spCurrentTestClass);
}

void SipxPortUnitTestEnvironment::setCurrentTestClass(SipxPortUnitTestClass* currentClass)
{
    spCurrentTestClass = currentClass;
}

void SipxPortUnitTestEnvironment::setStringOutMethod(SipxUnitStringOutputter newOutputMethod)
{
    sStringOutputMethod = newOutputMethod;
}

void SipxPortUnitTestEnvironment::setLogHookBegin(void (*logBeginFunc)(const char* testClassName))
{
    sLogHookBegin = logBeginFunc;
}

void SipxPortUnitTestEnvironment::setLogHookEnd(void (*logEndFunc)(const char* testClassName))
{
    sLogHookEnd = logEndFunc;
}


/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */

//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */

/* ========================== F U N C T I O N S =========================== */

//__________________________________________________________________________//
