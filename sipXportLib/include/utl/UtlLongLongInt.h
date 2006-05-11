// 
// 
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004-2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////


#ifndef _UtlLongLongInt_h_
#define _UtlLongLongInt_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"

// DEFINES
// MACROS

// Borrow this from the gcc include file "limits.h".
// LLONG_MIN and LLONG_MAX are defined by the ISO C99 standard but not by C++
// LONG_LONG_MIN and LONG_LONG_MAX are defined by gcc.
#ifndef LLONG_MIN
# define LLONG_MIN	LONG_LONG_MIN
#endif
#ifndef LLONG_MAX
# define LLONG_MAX	LONG_LONG_MAX
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlLongLongInt is a UtlContainable wrapper for a "long long int".
 * This data type is officially part of C99, see http://www.open-std.org/jtc1/sc22/wg14/ .
 * C++ has de facto support as well.  The size of a "long long int" is guaranteed to be at least 64 bits.
 */
class UtlLongLongInt : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting an optional default value.
     */
    UtlLongLongInt(INT64 initialValue = 0) ;
      
    /**
     * Destructor
     */
    virtual ~UtlLongLongInt();

/* ============================ OPERATORS ============================== */

    // Declare prefix and postfix increment operators.
    UtlLongLongInt& operator++();       // Prefix increment operator
    UtlLongLongInt operator++(int);     // Postfix increment operator

    // Declare prefix and postfix decrement operators.
    UtlLongLongInt& operator--();       // Prefix decrement operator
    UtlLongLongInt operator--(int);     // Postfix decrement operator

    // Conversion to long long int
    operator INT64() { return mValue; }

/* ============================ MANIPULATORS ============================== */

    /**
     * Set a new long long int value for this object.
     *
     * @returns the old value
     */
    INT64 setValue(INT64 iValue);

    //! Convert a ascii string rep. to long long int
    static INT64 stringToLongLong(const char* longLongString);

/* ============================ ACCESSORS ================================= */

    /**
     * Get the long long int wrapped by this object.
     */
    INT64 getValue() const ;    

    /**
     * Calculate a unique hash code for this object.  If the equals
     * operator returns true for another object, then both of those
     * objects must return the same hashcode.
     */
    virtual unsigned hash() const ;

    /**
     * Get the ContainableType for a UtlContainable derived class.
     */
    virtual UtlContainableType getContainableType() const;

    static UtlContainableType TYPE ;    /**< Class type used for runtime checking */ 

/* ============================ INQUIRY =================================== */

    /**
     * Compare the this object to another like-object.  Results for 
     * designating a non-like object are undefined.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    virtual int compareTo(UtlContainable const *) const ;    

    /**
     * Test this object to another like-object for equality.  This method 
     * returns false if unlike-objects are specified.
     */
    virtual UtlBoolean isEqual(UtlContainable const *) const ; 

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    INT64 mValue ;    /** < The long long int wrapped by this object */ 

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlLongLongInt_h_
