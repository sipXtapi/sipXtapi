// 
// Copyright (C) 2010-2014 SIPez LLCi.  All rights reserved.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipxPortUnitTestConstructor_h_
#define _SipxPortUnitTestConstructor_h_

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
class SipxPortUnitTestClass;
class SipxPortUnitTestPointFailure;

//
//! Factory/helper container for a test class
/*!
 *
 */

class SipxPortUnitTestConstructor 
{
/* ============================= P U B L I C ============================== */
public:

/* ============================ C R E A T O R S =========================== */

    //! Constructor
    SipxPortUnitTestConstructor(const char* testClassName);

    //! Destructor
    virtual
    ~SipxPortUnitTestConstructor();

/* ======================== M A N I P U L A T O R S ======================= */

    /// Run all test metods for the class starting at the given method index
    void runAllMethodsFrom(int methodIndex);
    /**<
     * @param[in] methodIndex - Hense skipping methods 0 through methodIndex-1
     */

    /// Run the named method for the test class
    void runMethod(const char* methodName);
    /**<
     * @param[i] methodName - name of the method to run
     */

    /// Construct an instance of the test class
    virtual void constructTestClass() = 0;

    /// Destro the test class instance
    void releaseTestClass();

    /// Log a failure as we caught a signal while this test class was running
    void addSignalCaughtFailure(int signalNumber);

    /// Log a general class failure
    virtual void addTestClassFailure(const char* className,
                                     const char* message) = 0;

/* ========================== A C C E S S O R S =========================== */

    /// get test class name for SipxPortUnitTestClass
    const char* getClassName() const;

    /// Returns the number of test methods for the test class
    virtual int getTestMethodCount() = 0;

    /// Returns method name for given index
    virtual const char* getTestMethodName(int methodIndex) const = 0;

    /// Return test point success count
    virtual int getPassedTestPointCount() const = 0;

    /// Return test point failure count
    virtual int getFailedTestPointCount() const = 0;

    /// Return test point failure count
    virtual int getFailureCount() const = 0;

    /// Returns a test point failure object
    virtual SipxPortUnitTestPointFailure* getFailure(int failureIndex) = 0;

    /// Get the last known line position for the running test
    int getTestPointLine() const;

    /// Get the last know file name of the source for the running test
    const char* getTestPointFilename() const;

/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */
protected:

    char* mpClassName;
    SipxPortUnitTestClass* mpTestClass;

//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */
private:


    /// Diable default constructor
    SipxPortUnitTestConstructor();

    //! Disabled copy constructor
    SipxPortUnitTestConstructor(const SipxPortUnitTestConstructor& rSipxPortUnitTestConstructor);

    //! Disabled assignment operator
    SipxPortUnitTestConstructor& operator=(const SipxPortUnitTestConstructor& rhs);

};


#endif  // _SipxPortUnitTestConstructor_h_
