// 
//
// Copyright (C) 2010 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2010 SIPez LLC All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
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
    CPPUNIT_ASSERT_MESSAGE(STR_NOT_EQ_MSG, (strcmp(STR1, STR2) == 0))

#define CPPUNIT_ASSERT_EQUAL(EQUAL_ARG1, EQUAL_ARG2) \
    CPPUNIT_ASSERT_EQUAL_MESSAGE("", EQUAL_ARG1, EQUAL_ARG2)

#define CPPUNIT_ASSERT_EQUAL_MESSAGE(NOT_EQ_MSG, EQ_ARG1, EQ_ARG2) \
    CPPUNIT_ASSERT_MESSAGE(NOT_EQ_MSG, (EQ_ARG1 == EQ_ARG2)) 

#define CPPUNIT_ASSERT(ASSERT_VAL) \
    CPPUNIT_ASSERT_MESSAGE("",  ASSERT_VAL)

#define CPPUNIT_FAIL(FAIL_MSG) \
    CPPUNIT_ASSERT_MESSAGE(FAIL_MSG, 0)

#define CPPUNIT_ASSERT_MESSAGE(ERROR_MESSAGE, TRUE_VALUE) \
        incrementTestPointIndex(); \
        setTestPointLine(__LINE__); \
        setTestPointFilename(__FILE__); \
        if(TRUE_VALUE) \
        { \
            incrementTestPointsPassed(); \
        } \
        else \
        { \
            incrementTestPointsFailed(); \
            char message[SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE]; \
            const char* className = getClassName(); \
            const char* methodName = spMethodNames[getCurrentMethodIndex()]; \
            int testPoint = getTestPointIndex(); \
            snprintf(message, SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE - 1, \
                    "%s, \"%s\" is not true, file: %s %s::%s test point: %d line: %d\n", \
                    ERROR_MESSAGE, #TRUE_VALUE, \
                    __FILE__, className, methodName, testPoint, __LINE__); \
            SipxPortUnitTestEnvironment::printOut(message); \
            addFailedTestPoint( __FILE__, \
                               className, \
                               methodName, \
                               testPoint, \
                               __LINE__, \
                               message); \
        }



// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#endif  // _SipxPortUnitTestAsserts_h_
