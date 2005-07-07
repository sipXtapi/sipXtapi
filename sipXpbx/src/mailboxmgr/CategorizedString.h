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

#ifndef _CategorizedString_h_
#define _CategorizedString_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * CategorizedString is a combination of an integer and a string.
 * It is containable in any of the UtlContainable-based containers.
 * (In particular, it can be sorted by a UtlSortedList.)
 * For purposes of comparison, CategorizedString's sort first on the integer
 * (called the 'priority') and second on the string.
 * The string must not contain zero bytes.
 */
class CategorizedString : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting an integer and a null terminated source string.
     */
    CategorizedString(int priority, const char* szSource);

    /**
     * Destructor
     */
    virtual ~CategorizedString();

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

    /**
     * Return a read-only pointer to the underlying data.  This pointer should
     * not be stored.
     */
    const char* data() const;

    /**
     * Calculate a unique hash code for this object.  If the equals
     * operator returns true for another object, then both of those
     * objects must return the same hashcode.
     */
    virtual unsigned hash() const;

    /**
     * Get the ContainableType for a UtlContainable derived class.
     */
    virtual UtlContainableType getContainableType() const;

/* ============================ INQUIRY =================================== */

    /**
     * Compare this object to another like object.  Results for 
     * designating a non-like object is not equal.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    virtual int compareTo(UtlContainable const *) const;

    /**
     * Test this object to another like object for equality.  This method 
     * returns false if unlike objects are specified.
     */
    virtual UtlBoolean isEqual(UtlContainable const *) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static UtlContainableType TYPE;    /** < Class type used for runtime checking */ 

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    int    mPriority;	//: The priority number.
    char*  mString;     //: The string.
};

/* ============================ INLINE METHODS ============================ */

#endif    // _CategorizedString_h_
