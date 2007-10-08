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
#include "os/OsDefs.h"
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlLongLongInt is a UtlContainable wrapper for a 64-bit long int.
 */
class UtlLongLongInt : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    static const UtlContainableType TYPE ;    /**< Class type used for runtime checking */ 

/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting an optional default value.
     */
    UtlLongLongInt(int64_t initialValue = 0) ;
      
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
    operator int64_t() { return mValue; }

/* ============================ MANIPULATORS ============================== */

    /**
     * Set a new long long int value for this object.
     *
     * @returns the old value
     */
    int64_t setValue(int64_t iValue);

    //! Convert a ascii string rep. to long long int
    static int64_t stringToLongLong(const char* longLongString);

/* ============================ ACCESSORS ================================= */

    /**
     * Get the long long int wrapped by this object.
     */
    int64_t getValue() const ;    

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
    int64_t mValue ;    /** < The long long int wrapped by this object */ 

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlLongLongInt_h_
