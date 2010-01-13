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

#ifndef _SipxPortUnitTestPointFailure_h_
#define _SipxPortUnitTestPointFailure_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
//
//! Container for information about a failed test point
/*! 
 *
 */

class SipxPortUnitTestPointFailure 
{
/* ============================= P U B L I C ============================== */
public:


/* ============================ C R E A T O R S =========================== */

    //! Constructor
    SipxPortUnitTestPointFailure(const char* fileName,
                                 const char* className,
                                 const char* methodName,
                                 int testPointIndex,
                                 int lineNumber,
                                 const char* message);

    //! Destructor
    virtual
    ~SipxPortUnitTestPointFailure();

/* ======================== M A N I P U L A T O R S ======================= */


/* ========================== A C C E S S O R S =========================== */

    const char* getClassName() const;
    const char* getMessage() const;

/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */
protected:
    char* mSourceFilename;
    char* mClassName;
    char* mMethodName;
    int mTestPointIndex;
    int mLineNumber;
    char* mTestPointMessage;

//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */
private:

    /// Disable default constructor
    SipxPortUnitTestPointFailure();

    //! Disabled copy constructor
    SipxPortUnitTestPointFailure(const SipxPortUnitTestPointFailure& rSipxPortUnitTestPointFailure);

    //! Disabled assignment operator
    SipxPortUnitTestPointFailure& operator=(const SipxPortUnitTestPointFailure& rhs);

};


#endif  // _SipxPortUnitTestPointFailure_h_
