// 
//
// Copyright (C) 2010-2017 SIPez LLC.  All rights reserved.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipxPortUnitTestAsserts_h_
#define _SipxPortUnitTestAsserts_h_

// SYSTEM INCLUDES
#include <string.h>

// APPLICATION INCLUDES
// DEFINES
// MACROS

// As some point these ASSERT macros should all be more specific so they 
// provide more information on the test operands.  However for now until 
// things are refined, they are all based upon the general 
// CPPUNIT_ASSERT_MESSAGE macro

#define KNOWN_BUG(BUG_MSG, BUG_NUMBER) \
     CPPUNIT_ASSERT_MESSAGE("KNOWN_BUG: " BUG_MSG " " BUG_NUMBER, 0)

#define KNOWN_EFENCE_BUG(EFENCE_MSG, BUG_NUM) \
     CPPUNIT_ASSERT_MESSAGE("EFENCE_BUG: " EFENCE_MSG " " BUG_NUM, 0)

#define ASSERT_STR_EQUAL(STRING1, STRING2) \
    ASSERT_STR_EQUAL_MESSAGE("", STRING1, STRING2)

#define ASSERT_STR_EQUAL_MESSAGE(STR_NOT_EQ_MSG, STR1, STR2) \
    CPPUNIT_ASSERT_EQUAL_MESSAGE(STR_NOT_EQ_MSG, STR1, STR2)

#define CPPUNIT_ASSERT_EQUAL(EQUAL_ARG1, EQUAL_ARG2) \
    CPPUNIT_ASSERT_EQUAL_MESSAGE("", EQUAL_ARG1, EQUAL_ARG2)

#define CPPUNIT_ASSERT_EQUAL_MESSAGE(NOT_EQ_MSG, EQ_ARG1, EQ_ARG2) \
        { \
            SipxPortUnitTestClass* currentTestClass = 0; \
            if((currentTestClass = SipxPortUnitTestEnvironment::getCurrentTestClass())) \
            { \
                currentTestClass->incrementTestPointIndex(); \
                currentTestClass->setTestPointLine(__LINE__); \
                currentTestClass->setTestPointFilename(__FILE__); \
                if(SipxPortUnitTestEnvironment::areEqual(EQ_ARG1,EQ_ARG2)) \
                { \
                    currentTestClass->incrementTestPointsPassed(); \
                } \
                else \
                { \
                    currentTestClass->incrementTestPointsFailed(); \
                    char _sipxportunit_message[SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE]; \
                    const char* className = currentTestClass->getClassName(); \
                    const char* methodName = currentTestClass->getCurrentMethodName(); /* spMethodNames[getCurrentMethodIndex()]; */ \
                    int testPoint = currentTestClass->getTestPointIndex(); \
                    SipxPortUnitTestEnvironment::makeAssertNotEqualMessage(_sipxportunit_message, ((const char*)NOT_EQ_MSG), #EQ_ARG1, #EQ_ARG2, \
                        __FILE__, className, methodName, testPoint, __LINE__, EQ_ARG1, EQ_ARG2); \
                    SipxPortUnitTestEnvironment::printOut(_sipxportunit_message); \
                    currentTestClass->addFailedTestPoint(__FILE__, \
                                                         className, \
                                                         methodName, \
                                                         testPoint, \
                                                         __LINE__, \
                                                         _sipxportunit_message); \
                } \
            } \
            assert(currentTestClass); \
        }

#define CPPUNIT_ASSERT(ASSERT_VAL) \
    CPPUNIT_ASSERT_MESSAGE("",  ASSERT_VAL)

#define CPPUNIT_FAIL(FAIL_MSG) \
    CPPUNIT_ASSERT_MESSAGE(FAIL_MSG, 0)

#define CPPUNIT_ASSERT_MESSAGE(ERROR_MESSAGE, TRUE_VALUE) \
        { \
            SipxPortUnitTestClass* currentTestClass = 0; \
            if((currentTestClass = SipxPortUnitTestEnvironment::getCurrentTestClass())) \
            { \
                currentTestClass->incrementTestPointIndex(); \
                currentTestClass->setTestPointLine(__LINE__); \
                currentTestClass->setTestPointFilename(__FILE__); \
                if(TRUE_VALUE) \
                { \
                    currentTestClass->incrementTestPointsPassed(); \
                } \
                else \
                { \
                    currentTestClass->incrementTestPointsFailed(); \
                    char _sipxportunit_message[SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE]; \
                    const char* className = currentTestClass->getClassName(); \
                    const char* methodName = currentTestClass->getCurrentMethodName(); /* spMethodNames[getCurrentMethodIndex()]; */ \
                    int testPoint = currentTestClass->getTestPointIndex(); \
                    snprintf(_sipxportunit_message, SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE - 1, \
                            "%s, \"%s\" is not true, file: %s %s::%s test point: %d line: %d\n", \
                            ((const char*)ERROR_MESSAGE), #TRUE_VALUE, \
                            __FILE__, className, methodName, testPoint, __LINE__); \
                    SipxPortUnitTestEnvironment::printOut(_sipxportunit_message); \
                    currentTestClass->addFailedTestPoint(__FILE__, \
                                                         className, \
                                                         methodName, \
                                                         testPoint, \
                                                         __LINE__, \
                                                         _sipxportunit_message); \
                } \
            } \
            assert(currentTestClass); \
        }



// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#endif  // _SipxPortUnitTestAsserts_h_
