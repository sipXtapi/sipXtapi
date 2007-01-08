//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlInt_h_
#define _UtlInt_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlCopyableContainable.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlInt is a UtlContainable wrapper for an int.
 */
class UtlInt : public UtlCopyableContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    static const UtlContainableType TYPE ;    /** < Class type used for runtime checking */ 

/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting an optional default value.
     */
    UtlInt(int initialValue = 0) ;
    UtlInt(const UtlInt& rhs) ;
      
    /**
     * Destructor
     */
    virtual ~UtlInt();

    UtlCopyableContainable* clone() const;

/* ============================ OPERATORS ============================== */

    // Declare prefix and postfix increment operators.
    UtlInt& operator++();       // Prefix increment operator
    UtlInt operator++(int);     // Postfix increment operator

    // Declare prefix and postfix decrement operators.
    UtlInt& operator--();       // Prefix decrement operator
    UtlInt operator--(int);     // Postfix decrement operator

    UtlInt& operator=(const UtlInt& rhs); // assigment operator

    // Conversion to int
    operator int() { return mValue; }

/* ============================ MANIPULATORS ============================== */

    /**
     * Set a new int value for this object.
     *
     * @returns the old value
     */
    int setValue(int iValue) ;

/* ============================ ACCESSORS ================================= */

    /**
     * Get the int wrapped by this object.
     */
    int getValue() const ;    

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
     * Compare this object to another like-object.  Results for 
     * comparing to a non-like object are undefined.
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
    int mValue ;    /** < The int wrapped by this object */ 

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlInt_h_
