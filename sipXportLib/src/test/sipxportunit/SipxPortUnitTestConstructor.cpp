// 
// 
// Copyright (C) 2010-2014 SIPez LLC.  All rights reserved.
// 
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include <SipxPortUnitTestConstructor.h>
#include <SipxPortUnitTestEnvironment.h>
#include <SipxPortUnitTestClass.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

//__________________________________________________________________________//
/* ============================= P U B L I C ============================== */

/* ============================ C R E A T O R S =========================== */

// Constructor
SipxPortUnitTestConstructor::SipxPortUnitTestConstructor(const char* testClassName) :
    mpClassName(SipxPortUnitTestEnvironment::newCopyString(testClassName)),
    mpTestClass(0)
{
    // Register this test to be run
    SipxPortUnitTestEnvironment::registerTestConstructor(this);
}

SipxPortUnitTestConstructor::~SipxPortUnitTestConstructor()
{
    // SipxPortUnitTestConstructor should never get destroyed
    assert(0); 
    if(mpClassName != 0)
    {
        delete[] mpClassName;
        mpClassName = 0;
    }
}

/* ======================== M A N I P U L A T O R S ======================= */

void SipxPortUnitTestConstructor::runAllMethodsFrom(int methodIndex)
{
    if(mpTestClass == 0)
    {
        constructTestClass();
        assert(mpTestClass);
    }

    SipxPortUnitTestEnvironment::setCurrentTestClass(mpTestClass);
    mpTestClass->runAllMethodsFrom(methodIndex);
}

void SipxPortUnitTestConstructor::runMethod(const char* methodName)
{
    if(mpTestClass == 0)
    {
        constructTestClass();
        assert(mpTestClass);
    }

    SipxPortUnitTestEnvironment::setCurrentTestClass(mpTestClass);
    mpTestClass->runMethod(methodName);
}

void SipxPortUnitTestConstructor::releaseTestClass()
{
    /*
    char buffer[256];
    sprintf(buffer, "SipxPortUnitTestConstructor::releaseTestClass: %s\n",
            mpTestClass->getClassName());
    SipxPortUnitTestEnvironment::printOut(buffer);
    */
    assert(mpTestClass);
    delete mpTestClass;
    mpTestClass = 0;
}

void SipxPortUnitTestConstructor::addSignalCaughtFailure(int signalNumber)
{
    char message[SIPX_PORT_UNIT_MAX_ERROR_MESSAGE_SIZE];
    if(mpTestClass)
    {
        const char* fileName = mpTestClass->getTestPointFilename();
        const char* methodName = mpTestClass->getCurrentMethodName();
        int testPoint = mpTestClass->getTestPointIndex();
        int lineNumber = mpTestClass->getTestPointLine();

        sprintf(message, 
                "ABORT: due to signal: %d caught while in file: %s %s::%s test point: %d between line: %d and next test point\n",
                signalNumber,
                fileName, 
                mpClassName,
                methodName,
                testPoint,
                lineNumber);

        mpTestClass->addFailedTestPoint(fileName,
                                        mpClassName,
                                        methodName,
                                        testPoint,
                                        lineNumber,
                                        message);
    }
    // no class, something bad must have happened during construction or
    // initialization.  Log a general class test failure
    else
    {
        sprintf(message, "ABORT: due to signal %d caught while constructing or initializing test class: %s.  ALL test methods skipped.\n", signalNumber, mpClassName);
        addTestClassFailure(mpClassName, message);
    }
}

/* ========================== A C C E S S O R S =========================== */

const char* SipxPortUnitTestConstructor::getClassName() const
{
    return(mpClassName);
}

int SipxPortUnitTestConstructor::getTestPointLine() const
{
    return(mpTestClass ? mpTestClass->getTestPointLine() : -1);
}

const char* SipxPortUnitTestConstructor::getTestPointFilename() const
{
    return(mpTestClass ? mpTestClass->getTestPointFilename() : 0);
}

/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */

//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */

/* ========================== F U N C T I O N S =========================== */

//__________________________________________________________________________//
