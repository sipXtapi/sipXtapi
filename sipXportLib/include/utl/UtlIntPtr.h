//
// Copyright (C) 2006-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlIntPtr_h_
#define _UtlIntPtr_h_

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
 * UtlIntPtr is a UtlContainable wrapper for an intptr_t.
 */
class UtlIntPtr : public UtlCopyableContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    static const UtlContainableType TYPE ;    /** < Class type used for runtime checking */ 

/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting an optional default value.
     */
    UtlIntPtr(intptr_t initialValue = 0) ;
    UtlIntPtr(const UtlIntPtr& rhs) ;
      
    /**
     * Destructor
     */
    virtual ~UtlIntPtr();

    UtlCopyableContainable* clone() const;

/* ============================ OPERATORS ============================== */

    // Declare prefix and postfix increment operators.
    UtlIntPtr& operator++();       // Prefix increment operator
    UtlIntPtr operator++(int);     // Postfix increment operator

    // Declare prefix and postfix decrement operators.
    UtlIntPtr& operator--();       // Prefix decrement operator
    UtlIntPtr operator--(int);     // Postfix decrement operator

    UtlIntPtr& operator=(const UtlIntPtr& rhs); // assigment operator

    // Conversion to int
    operator intptr_t() { return mValue; }

/* ============================ MANIPULATORS ============================== */

    /**
     * Set a new int value for this object.
     *
     * @returns the old value
     */
    intptr_t setValue(intptr_t iValue) ;

/* ============================ ACCESSORS ================================= */

    /**
     * Get the int wrapped by this object.
     */
    intptr_t getValue() const ;    

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

    /// @copydoc UtlContainable::isInstanceOf
    virtual UtlBoolean isInstanceOf(const UtlContainableType type) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    intptr_t mValue ;    ///< The int wrapped by this object 

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlIntPtr_h_
