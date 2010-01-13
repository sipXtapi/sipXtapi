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
// APPLICATION INCLUDES
#include <SipxPortUnitTestPointFailure.h>
#include <SipxPortUnitTestEnvironment.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

//__________________________________________________________________________//
/* ============================= P U B L I C ============================== */

/* ============================ C R E A T O R S =========================== */

// Constructor
SipxPortUnitTestPointFailure::SipxPortUnitTestPointFailure(const char* fileName,
                                                           const char* className,
                                                           const char* methodName,
                                                           int testPointIndex,
                                                           int lineNumber,
                                                           const char* message) :
    mSourceFilename(SipxPortUnitTestEnvironment::newCopyString(fileName)),
    mClassName(SipxPortUnitTestEnvironment::newCopyString(className)),
    mMethodName(SipxPortUnitTestEnvironment::newCopyString(methodName)),
    mTestPointIndex(testPointIndex),
    mLineNumber(lineNumber),
    mTestPointMessage(SipxPortUnitTestEnvironment::newCopyString(message))


{
}

// Destructor
SipxPortUnitTestPointFailure::~SipxPortUnitTestPointFailure()
{
}

/* ======================== M A N I P U L A T O R S ======================= */

/* ========================== A C C E S S O R S =========================== */

const char* SipxPortUnitTestPointFailure::getClassName() const
{
    return(mClassName);
}

const char* SipxPortUnitTestPointFailure::getMessage() const
{
    return(mTestPointMessage);
}

/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */

//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */

/* ========================== F U N C T I O N S =========================== */

//__________________________________________________________________________//
