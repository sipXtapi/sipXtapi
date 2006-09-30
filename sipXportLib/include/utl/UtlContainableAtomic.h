//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _UtlContainableAtomic_h_
#define _UtlContainableAtomic_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
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
 * UtlContainableAtomic is a virtual subclass of UtlContainable that
 * is suitable as a base class for objects that need to be
 * containable, but are not otherwise "data items" that can be
 * compared.
 * A notable feature is that any two UtlContainableAtomic objects are
 * considered equal only if they are the same (their pointers are equal).
 * Comparison between objects in UtlContainableAtomic and its subclasses
 * provides a consistent (albeit arbitrary) linear ordering.
 *
 * A subclass of UtlContainableAtomic only needs to define
 * subclass::getContainableType() and subclass::TYPE, the remaining
 * necessary methods can be inherited from UtlContainableAtomic.
 */
class UtlContainableAtomic : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /// Calculate a hash code for this object.
    virtual unsigned hash() const;
    /**<
     * Returns a hash of the pointer to the object.
     */

    /// Compare this object to another object. 
    virtual int compareTo(UtlContainable const *) const;
    /**<
     * For all members of subclasses of UtlContainableAtomic,
     * compareTo provides a consistent linear ordering.
     * A copy of an object is never equal to the original.
     */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        
} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlContainableAtomic_h_
