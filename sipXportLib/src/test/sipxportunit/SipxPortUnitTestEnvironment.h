// 
//
// Copyright (C) 2010-2014 SIPez LLC  All rights reserved.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipxPortUnitTestEnvironment_h_
#define _SipxPortUnitTestEnvironment_h_

// SYSTEM INCLUDES
#include <utl/UtlString.h>

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define SIPX_PORT_UNIT_TEST_SIGNATURE 20100108

// For now we hardcode some limits
#define SIPX_PORT_UNIT_MAX_TEST_CLASSES 2048
#define SIPX_PORT_UNIT_MAX_TEST_METHODS 512
#define SIPX_PORT_UNIT_MAX_TEST_ERRORS_PER_CLASS 100
#define SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE 4096
#define SIPX_PORT_UNIT_MAX_TEST_FILENAME_LENGTH 1024
#define SIPX_PORT_UNIT_MAX_STACK_TRACE_SIZE 32
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class SipxPortUnitTestConstructor;
class SipxPortUnitTestClass;
class UtlContainable;

//
//! Top level unit test class and envrionment container
/*! This is the top level class that runs the unt tests, keeps track of 
 * results and state.  All unit tests to be run get registered with this
 * class.  This unit test framework was created as CPPUNIT does not port
 * well to may environments, especially on embeded systems and platforms
 * where exceptions, RTTI, stdlib and templates are not well supported.
 * 
 *
 * \par Concepts
 *
 * The unit tests for a suite are broken down into the the following 
 * primatives:
 * -# Test Class
 *        A test class is a C++ class that defines a set of method to
 *    be run as part of the test suit.  Typically a test class will test a 
 *    specific class or subsystem.
 * -# Test Method
 *        A test method is a C++ method defined in a test class.  It will
 *    typically contain a number of test points.  The test points usually
 *    are a set of related test assertions for the test class.
 * -# Test Point
 *        A test point is a single test check or assertion.  It tests the
 *    value or state of a variable or object.  Test points are implemented
 *    using a set of macros.
 *
 * \par Aborts
 *
 * The test framework attempts to catch signals from SEGV, FPE, etc.
 * and continue the running of the remaining tests.  It also attempts
 * to dump the stack when signals are caught.  If you do not want it
 * to catch signals and instead have it dump core so that you can debug
 * the stack, simply comment out the calls to signal in 
 * SipxPortUnitTestEnvironment.cpp.  To get function names in the
 * stack dump you will want to compile with the -rdynamic option.
 *
 */

typedef void (*SipxUnitStringOutputter)(const char* logMesage);

class SipxPortUnitTestEnvironment 
{
/* ============================= P U B L I C ============================== */
public:

/* ============================ C R E A T O R S =========================== */

/* ======================== M A N I P U L A T O R S ======================= */

    static void initializeEnvironment();

    static void resetExceptionState();

    static void registerTestConstructor(SipxPortUnitTestConstructor* testClassToAdd);

    static void runTests(const char* testClassName);

    static void listTests(const char* testClassName = NULL);

    static void reportResults();

    static void printOut(const char* messageText);

    static void defaultPrintOut(const char* messageText);

    static bool areEqual(long arg1, long arg2);

    static bool areEqual(const UtlString& arg1, const UtlString& arg2);

    static bool areEqual(const UtlContainable& arg1, const UtlContainable& arg2);

    static bool areEqual(const UtlContainable* arg1, const UtlContainable* arg2);

    static bool areEqual(void* arg1, void* arg2);

    static void makeAssertNotEqualMessage(char* messageBuffer, const char* notEqualMessage, const char* arg1String, const char* arg2String,
                    const char* fileName, const char* className, const char* methodName, int testPoint, int lineNum, 
                    int arg1, int arg2);

    static void makeAssertNotEqualMessage(char* messageBuffer, const char* notEqualMessage, const char* arg1String, const char* arg2String,
                    const char* fileName, const char* className, const char* methodName, int testPoint, int lineNum, 
                    const char* arg1, const char* arg2);

    static void makeAssertNotEqualMessage(char* messageBuffer, const char* notEqualMessage, const char* arg1String, const char* arg2String,
                    const char* fileName, const char* className, const char* methodName, int testPoint, int lineNum, 
                    const UtlContainable* arg1, const UtlContainable* arg2);

    static void makeAssertNotEqualMessage(char* messageBuffer, const char* notEqualMessage, const char* arg1String, const char* arg2String,
                    const char* fileName, const char* className, const char* methodName, int testPoint, int lineNum, 
                    const void* arg1, const void* arg2);

/* ========================== A C C E S S O R S =========================== */

    static char* newCopyString(const char* stringToCopy);

    static void incrementMethodsRun();

    static void setCatchSignals(bool enable);

    static void setMethodIndex(int methodIndex);

    static void resetTestPointIndex();

    static void incrementTestPointIndex();

    static void incrementTestPointsPassed();

    static void incrementTestPointsFailed();

    static int getTestPointFailureCount();

    static int getTestAbortCount();

    static void dumpCurrentTestState();

    static SipxPortUnitTestClass* getCurrentTestClass();
    static void setCurrentTestClass(SipxPortUnitTestClass* currentClass);

    static void setStringOutMethod(SipxUnitStringOutputter outputMethod);

    static void setLogHookBegin(void (*logBeginFunc)(const char* testClassName));
    static void setLogHookEnd(void (*logEndFunc)(const char* testClassName));

/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */
protected:

    static void signalHandler(int signalCaught);

//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */
private:


    //! Constructor
    SipxPortUnitTestEnvironment();

    //! Destructor
    virtual
    ~SipxPortUnitTestEnvironment();

    //! Disabled copy constructor
    SipxPortUnitTestEnvironment(const SipxPortUnitTestEnvironment& rSipxPortUnitTestEnvironment);

    //! Disabled assignment operator
    SipxPortUnitTestEnvironment& operator=(const SipxPortUnitTestEnvironment& rhs);

    static bool sInitialized;
    static int sInitializedSignature;  /// simple lock
    static int sInitializing;

    static int sCurrentTestClassIndex;
    static int sCurrentTestMethodIndex;
    static int sCurrentTestPointIndex;
    static int sTestClassesRun;
    static SipxPortUnitTestClass* spCurrentTestClass;

    static int sTotalTestMethodCount;
    static int sTestMethodsRun;
    static int sTestPointsPassed;
    static int sTestPointsFailed;

    static int sTotalTestClassCount;
    static SipxPortUnitTestConstructor** sTestClassesToRun;

    // Note the term exceptions is used loosely here
    // really we are talking about signal events that we are
    // able to recover from
    static bool sCatchSignals;  ///< should try to catch signals and continue or not
    static int sNumExceptionsCaught;
    static int sLastExceptionsCaught;
    static int sLastExceptionClassIndex;
    static int sNumExceptionsForSameClass;
    static int sLastExceptionMethodIndex;
    static int sLastExceptionTestPointIndex;

    static SipxUnitStringOutputter sStringOutputMethod;

    static void (*sLogHookBegin)(const char* testName);
    static void (*sLogHookEnd)(const char* testName);

};


#endif  // _SipxPortUnitTestEnvironment_h_
