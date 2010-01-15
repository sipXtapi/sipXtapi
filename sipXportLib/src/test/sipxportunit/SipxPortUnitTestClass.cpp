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

// APPLICATION INCLUDES
#include <SipxPortUnitTestClass.h>
#include <SipxPortUnitTestEnvironment.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

//__________________________________________________________________________//
/* ============================= P U B L I C ============================== */

/* ============================ C R E A T O R S =========================== */

// Constructor
SipxPortUnitTestClass::SipxPortUnitTestClass(const char* name) :
    mpClassName(SipxPortUnitTestEnvironment::newCopyString(name)),
    mCurrentMethodIndex(-1),
    mPriorTestPointIndex(-1),
    mPriorTestPointLine(-1)
{
    mPriorTestPointFileName[0] = '\0';
}

SipxPortUnitTestClass::~SipxPortUnitTestClass()
{
    /*
    char buffer[SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE];
    sprintf(buffer, "~SipxPortUnitTestClass destroying: %s\n", mpClassName);
    SipxPortUnitTestEnvironment::printOut(buffer);
    */
}

/* ======================== M A N I P U L A T O R S ======================= */

void SipxPortUnitTestClass::setUp()
{
    //SipxPortUnitTestEnvironment::printOut("Default setUp\n");
}

void SipxPortUnitTestClass::tearDown()
{
    //SipxPortUnitTestEnvironment::printOut("Default tearDown\n");
}

/* ========================== A C C E S S O R S =========================== */

void SipxPortUnitTestClass::setClassName(const char* className)
{
    assert(mpClassName == 0);
    mpClassName = SipxPortUnitTestEnvironment::newCopyString(className);
}

const char* SipxPortUnitTestClass::getClassName()
{
    return(mpClassName);
}

void SipxPortUnitTestClass::setCurrentMethodIndex(int methodIndex)
{
    assert(methodIndex >= 0);
    mCurrentMethodIndex = methodIndex;
    SipxPortUnitTestEnvironment::setMethodIndex(methodIndex);
    SipxPortUnitTestEnvironment::incrementMethodsRun();
}

int SipxPortUnitTestClass::getCurrentMethodIndex() const
{
    return(mCurrentMethodIndex);
}

void SipxPortUnitTestClass::resetTestPointIndex()
{
    mPriorTestPointIndex = -1;
    SipxPortUnitTestEnvironment::resetTestPointIndex();
}

void SipxPortUnitTestClass::incrementTestPointIndex()
{
    mPriorTestPointIndex++;
    SipxPortUnitTestEnvironment::incrementTestPointIndex();
}

int SipxPortUnitTestClass::getTestPointIndex() const
{
    return(mPriorTestPointIndex);
}

void SipxPortUnitTestClass::setTestPointLine(int lineNumber)
{
    mPriorTestPointLine = lineNumber;
}

int SipxPortUnitTestClass::getTestPointLine() const
{
    return(mPriorTestPointLine);
}

void SipxPortUnitTestClass::setTestPointFilename(const char* filename)
{
    strncpy(mPriorTestPointFileName, filename, 
            SIPX_PORT_UNIT_MAX_TEST_FILENAME_LENGTH - 1);
    // Make sure string is always null terminated
    mPriorTestPointFileName[SIPX_PORT_UNIT_MAX_TEST_FILENAME_LENGTH - 1] = '\0';
}

const char* SipxPortUnitTestClass::getTestPointFilename() const
{
    return(mPriorTestPointFileName);
}

/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */

//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */

/* ========================== F U N C T I O N S =========================== */

//__________________________________________________________________________//
