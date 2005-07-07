// $Id$
//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _UtlContainable_h_
#define _UtlContainable_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * An UtlContainable object is an abstract objected that serves as base 
 * class for anything that can be contained in one of the UtlContainer 
 * derived classes.  One of the largest values of a UtlContainable derived 
 * object is ability for any of UtlContainers to destroy objects, sort 
 * objects, etc.
 */
class UtlContainable 
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Destructor
     */
    virtual ~UtlContainable();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

    /**
     * Calculate a unique hash code for this object.  If the equals
     * operator returns true for another object, then both of those
     * objects must return the same hashcode.
     */
    virtual unsigned hash() const = 0 ;

    /**
     * Get the ContainableType for a UtlContainable derived class.
     */
    virtual UtlContainableType getContainableType() const = 0 ;

/* ============================ INQUIRY =================================== */

    /**
     * Compare the this object to another like-objects.  Results for 
     * designating a non-like object are undefined.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    virtual int compareTo(UtlContainable const *) const = 0  ;    

    /**
     * Test this object to another like-object for equality.  Results for
     * non-like objects are undefined.
     */
    virtual UtlBoolean isEqual(UtlContainable const *) const;    

    /**
     * Determine if this object is an instance of the designated runtime
     * class identifer.  For example:
     * <pre>
     * if (pMyObject->isInstanceOf(UtlInt::TYPE))
     * {
     *     ...
     * }
     * </pre>
     */
    virtual UtlBoolean isInstanceOf(const UtlContainableType type) const ; 

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static const UtlContainableType TYPE ;    /** < Class type used for runtime checking */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        
} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlContainable_h_

