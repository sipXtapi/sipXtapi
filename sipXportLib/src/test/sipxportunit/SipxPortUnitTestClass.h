// 
//
// Copyright (C) 2010-2014 SIPez LLC.  All rights reserved.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipxPortUnitTestClass_h_
#define _SipxPortUnitTestClass_h_

// SYSTEM INCLUDES
#include <stdio.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include <SipxPortUnitTestEnvironment.h>

// DEFINES

// MACROS
#define CPPUNIT_TEST_SUB_SUITE(CHILD_CLASS_NAME, PARENT_CLASS_NAME) \
    CPPUNIT_TEST_SUITE(CHILD_CLASS_NAME)

#define CPPUNIT_TEST_SUITE(CLASS_NAME) \
    public: \
    /* Not implemented here so actual tests can code constructor 
       and destructors
    CLASS_NAME() : \
        SipxPortUnitTestClass(#CLASS_NAME) \
    { \
    } \
 \
    ~  CLASS_NAME() \
    { \
        char buffer[256]; \
        sprintf(buffer, "Entering ~%s\n", \
                #CLASS_NAME); \
        SipxPortUnitTestEnvironment::printOut(buffer); \
         \
    } */\
 \
    static int getMethodCount() \
    { \
        initializeStatics(); \
        return(sMethodCount); \
    } \
    virtual const char* getCurrentMethodName() const\
    { \
        assert(spMethodNames); \
        return(mCurrentMethodIndex >= 0 && \
               mCurrentMethodIndex < sMethodCount ? \
               spMethodNames[mCurrentMethodIndex] : \
               0); \
    } \
 \
    static const char* getMethodName(int methodIndex) \
    { \
        assert(spMethodNames); \
        return(methodIndex >= 0 && \
               methodIndex < sMethodCount ? \
               spMethodNames[methodIndex] : \
               0); \
    } \
 \
    virtual void incrementTestPointsPassed() \
    { \
        sTestPointsPassed++; \
        SipxPortUnitTestEnvironment::incrementTestPointsPassed(); \
    } \
 \
    static int getPassedTestPointCount() \
    { \
        return(sTestPointsPassed); \
    } \
 \
    static int getFailedTestPointCount() \
    { \
        return(sTestPointsFailed); \
    } \
 \
    virtual void incrementTestPointsFailed() \
    { \
        sTestPointsFailed++; \
        SipxPortUnitTestEnvironment::incrementTestPointsFailed(); \
    } \
 \
    static int getFailureCount() \
    { \
        return(sTestPointFailureCount); \
    } \
 \
    static SipxPortUnitTestPointFailure* getFailure(int failureIndex) \
    { \
        assert(failureIndex >= 0); \
        assert(failureIndex < SIPX_PORT_UNIT_MAX_TEST_ERRORS_PER_CLASS); \
        return(sTestPointFailures[failureIndex]); \
    } \
 \
    static bool addFailure(SipxPortUnitTestPointFailure* failure) \
    { \
        bool wasAdded = false; \
        if(sTestPointFailureCount < SIPX_PORT_UNIT_MAX_TEST_ERRORS_PER_CLASS) \
        { \
            sTestPointFailures[sTestPointFailureCount] = failure; \
            sTestPointFailureCount++; \
            wasAdded = true; \
        } \
        else \
        { \
            char message[SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE]; \
            sprintf(message, "WARNING: maximum failures of %d excedded for class: %s, failure not added: %s\n", \
                    SIPX_PORT_UNIT_MAX_TEST_ERRORS_PER_CLASS, \
                    failure->getClassName(), failure->getMessage()); \
            SipxPortUnitTestEnvironment::printOut(message); \
        } \
        return(wasAdded); \
    } \
 \
    private: \
    static int sInitialized; \
    static int sInitializedSignature; \
    static int sMethodCount; \
    static char** spMethodNames; \
    static SipxPortTestMethodState* spMethodStates; \
    static int sTestPointsPassed; \
    static int sTestPointsFailed; \
    static int sTestPointFailureCount; \
    static SipxPortUnitTestPointFailure* sTestPointFailures[SIPX_PORT_UNIT_MAX_TEST_ERRORS_PER_CLASS]; \
 \
    void runAllMethodsFrom(int startMethodIndex) \
    { \
        forEachTestMethod(RUN_ALL_FROM, this, startMethodIndex); \
    } \
 \
    void runMethod(const char* methodName) \
    { \
        int methodCount = getMethodCount(); \
        for(int methodIndex = 0; methodIndex < methodCount; methodIndex++) \
        { \
            if(strcmp(getMethodName(methodIndex), methodName) == 0) \
            { \
                printf("runMethod(%s) index: %d running\n", \
                       methodName, methodIndex); \
                forEachTestMethod(RUN_ONE, this, methodIndex); \
                break ; \
            } \
        } \
    } \
\
    void addFailedTestPoint(const char* fileName, \
                            const char* className, \
                            const char* methodName, \
                            int testPoint, \
                            int lineNumber, \
                            const char* message) \
    { \
        SipxPortUnitTestPointFailure* failure = \
            new SipxPortUnitTestPointFailure(fileName, \
                                             className, \
                                             methodName, \
                                             testPoint, \
                                             lineNumber, \
                                             message); \
        if(!addFailure(failure)) \
        { \
            /* not added. array is full, so clean up */ \
            delete failure; \
            failure = 0; \
        } \
    } \
 \
    static void initializeStatics() \
    { \
        if(!sInitialized || \
           sInitializedSignature != SIPX_PORT_UNIT_TEST_SIGNATURE) \
        { \
            sInitialized = false; \
            sMethodCount = -1; \
            spMethodNames = 0; \
            spMethodStates = 0; \
            sTestPointsPassed = 0; \
            sTestPointsFailed = 0; \
            sTestPointFailureCount = 0; \
            for(int failureIndex = 0; failureIndex < SIPX_PORT_UNIT_MAX_TEST_ERRORS_PER_CLASS; failureIndex++)  \
            { \
                sTestPointFailures[failureIndex] = 0; \
            } \
 \
            forEachTestMethod(INIT_TEST_METHOD_INFO, 0, 0); \
 \
            sInitialized = true; \
            sInitializedSignature = SIPX_PORT_UNIT_TEST_SIGNATURE; \
        } \
    } \
 \
    static void forEachTestMethod(SipxPortTestMethodOperation operation, \
                                  CLASS_NAME* testInstance, \
                                  int methodIndexStart) \
    { \
        if(operation == INIT_TEST_METHOD_INFO && sMethodCount == -1) \
        { \
            forEachTestMethod(SET_METHOD_COUNT, 0, 0); \
        } \
 \
        int methodIndex = 0; \
        char buffer[SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE]; \



#define CPPUNIT_TEST(METHOD_NAME) \
        switch(operation) \
        { \
            case SET_METHOD_COUNT: \
                break; \
 \
            case INIT_TEST_METHOD_INFO: \
                { \
                    assert(methodIndex >= 0 && \
                           methodIndex < sMethodCount); \
                    assert(spMethodNames[methodIndex] == 0); \
                    spMethodNames[methodIndex] = \
                        SipxPortUnitTestEnvironment::newCopyString(#METHOD_NAME) ; \
                } \
                break; \
 \
            case RUN_ALL_FROM: \
            case RUN_ONE: \
                assert(testInstance); \
                if(methodIndex == methodIndexStart || \
                   (operation == RUN_ALL_FROM && methodIndex > methodIndexStart)) \
                { \
                    testInstance->setCurrentMethodIndex(methodIndex); \
                    testInstance->resetTestPointIndex(); \
                    spMethodStates[methodIndex] = RUNNING; \
                    snprintf(buffer, SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE - 1, \
                            "[TEST]: %s::%s\n", testInstance->getClassName(), \
                            #METHOD_NAME); \
                    buffer[SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE - 1] = '\0'; \
                    SipxPortUnitTestEnvironment::printOut(buffer); \
                    testInstance->setUp(); \
                    testInstance->METHOD_NAME(); \
                    testInstance->tearDown(); \
                    spMethodStates[methodIndex] = COMPLETED; \
                } \
                break; \
 \
            /* Invalid operation id */ \
            default: \
                assert(0); \
                break; \
        } \
        methodIndex++; \



#define CPPUNIT_TEST_SUITE_END() \
        if(operation == SET_METHOD_COUNT) \
        { \
            sMethodCount = methodIndex; \
            spMethodNames = new char*[sMethodCount]; \
            spMethodStates = new SipxPortTestMethodState[sMethodCount]; \
            for(int mIndex = 0; mIndex < sMethodCount; mIndex++) \
            { \
                spMethodNames[mIndex] = 0; \
                spMethodStates[mIndex] = NOT_RUN; \
            } \
        } \
        \
    } \




#define CPPUNIT_TEST_SUITE_REGISTRATION(CLASS_NAME) \
int CLASS_NAME::sInitialized = false; \
int CLASS_NAME::sInitializedSignature = 0; \
int CLASS_NAME::sMethodCount = -1; \
char** CLASS_NAME::spMethodNames = 0; \
SipxPortUnitTestClass::SipxPortTestMethodState* CLASS_NAME::spMethodStates = 0; \
int CLASS_NAME::sTestPointsPassed = 0; \
int CLASS_NAME::sTestPointsFailed = 0; \
int CLASS_NAME::sTestPointFailureCount = 0; /* num pointers in following array */ \
SipxPortUnitTestPointFailure* CLASS_NAME::sTestPointFailures[SIPX_PORT_UNIT_MAX_TEST_ERRORS_PER_CLASS]; \
 \
class CLASS_NAME##Constructor : public SipxPortUnitTestConstructor \
{ \
    public: \
    CLASS_NAME##Constructor() : \
        SipxPortUnitTestConstructor(#CLASS_NAME) \
    { \
    } \
 \
    /* virtual ~ CLASS_NAME##Constructor() \
    { \
         Should never get destroyed  \
        assert(0); \
    } */ \
 \
    void constructTestClass() \
    { \
        if(mpTestClass == 0) \
        { \
            mpTestClass = new CLASS_NAME(); \
            mpTestClass->setClassName(#CLASS_NAME); \
        } \
    } \
 \
    int getTestMethodCount() \
    { \
        return(CLASS_NAME::getMethodCount()); \
    } \
 \
    const char* getTestMethodName(int methodIndex) const\
    { \
        return(CLASS_NAME::getMethodName(methodIndex)); \
    } \
 \
    void addTestClassFailure(const char* className, \
                             const char* message) \
    { \
        SipxPortUnitTestPointFailure* failure = \
            new SipxPortUnitTestPointFailure("", \
                                             className, \
                                             "", \
                                             -1, \
                                             -1, \
                                             message); \
        if(!CLASS_NAME::addFailure(failure)) \
        { \
            /* no room to add, clean up */ \
            delete failure; \
            failure = 0; \
        } \
    } \
 \
    int getPassedTestPointCount() const \
    { \
        return(CLASS_NAME::getPassedTestPointCount()); \
    } \
 \
    int getFailedTestPointCount() const \
    { \
        return(CLASS_NAME::getFailedTestPointCount()); \
    } \
 \
    int getFailureCount() const\
    { \
       return(CLASS_NAME::getFailureCount()); \
    } \
 \
    SipxPortUnitTestPointFailure* getFailure(int failureIndex) \
    { \
        return(CLASS_NAME::getFailure(failureIndex)); \
    } \
}; \
 \
static CLASS_NAME##Constructor* instance##CLASS_NAME##Constructor = new CLASS_NAME##Constructor(); \



// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
//
//! Abstract class that all unit test classes should derive from
/*! 
 *
 */

class SipxPortUnitTestClass
{
/* ============================= P U B L I C ============================== */
public:

    typedef enum 
    {
        SET_METHOD_COUNT = 0,
        INIT_TEST_METHOD_INFO,
        RUN_ALL_FROM,
        RUN_ONE
    } SipxPortTestMethodOperation;

    typedef enum
    {
        NOT_RUN = 0,
        RUNNING,
        ABORTED,
        COMPLETED
    } SipxPortTestMethodState;


/* ============================ C R E A T O R S =========================== */

    //! Constructor
    SipxPortUnitTestClass(const char* testClassName = 0);

    //! Destructor
    virtual
    ~SipxPortUnitTestClass();

/* ======================== M A N I P U L A T O R S ======================= */

    virtual void runAllMethodsFrom(int methodIndex) = 0;

    virtual void runMethod(const char* methodName) = 0;

    /// User overridable initialization to be done before each test method is invoked
    virtual void setUp();

    /// User overridable clean up to be done after each test method is invoked
    virtual void tearDown();

    /// Helper method to create a failure entry for the class
    virtual void addFailedTestPoint(const char* fileName, 
                                    const char* className, 
                                    const char* methodName, 
                                    int testPoint, 
                                    int lineNumber, 
                                    const char* message) = 0;

/* ========================== A C C E S S O R S =========================== */

    void setClassName(const char* className);
    const char* getClassName();

    void setCurrentMethodIndex(int methodIndex);
    int getCurrentMethodIndex() const;
    virtual const char* getCurrentMethodName() const = 0;

    void resetTestPointIndex();
    void incrementTestPointIndex();
    int getTestPointIndex() const;

    virtual void incrementTestPointsPassed() = 0;
    virtual void incrementTestPointsFailed() = 0;

    void setTestPointLine(int lineNumber);
    int getTestPointLine() const;

    void setTestPointFilename(const char* testFilename);
    const char* getTestPointFilename() const;

/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */
protected:

    char* mpClassName;
    int mCurrentMethodIndex;
    int mPriorTestPointIndex;
    int mPriorTestPointLine;
    char mPriorTestPointFileName[SIPX_PORT_UNIT_MAX_TEST_FILENAME_LENGTH];

//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */
private:

    //! Disabled copy constructor
    SipxPortUnitTestClass(const SipxPortUnitTestClass& rSipxPortUnitTestClass);

    //! Disabled assignment operator
    SipxPortUnitTestClass& operator=(const SipxPortUnitTestClass& rhs);

};


#endif  // _SipxPortUnitTestClass_h_
