//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlVoidPtr_h_
#define _UtlVoidPtr_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"

// APPLICATION INCLUDES
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
 * UtlVoidPtr is a UtlContainable wrapper for a void ptr.  Ideally, 
 * derived objects should be defined instead of "cheating" by using a  
 * UtlVoidPtr wrapper. 
 */
class UtlVoidPtr : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting an optional default value.
     */
    UtlVoidPtr(void * pPtr = NULL) ;
     
    /**
     * Destructor
     */
    virtual ~UtlVoidPtr() ;

/* ============================ MANIPULATORS ============================== */

    /**
     * Set a new void ptr value for this object.
     *
     * @returns the old value
     */
    void* setValue(void *) ;

/* ============================ ACCESSORS ================================= */

    /**
     * Get the void ptr wrapped by this object.
     */
    void* getValue() const ;    

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

    static UtlContainableType TYPE ;   /**< Class type used for runtime checking */ 

/* ============================ INQUIRY =================================== */

    /**
     * Compare the this object to another like-objects.  Results for 
     * designating a non-like object are undefined.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    virtual int compareTo(UtlContainable const *) const ;

  
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    void* mpValue ;  /** < The void ptr wrapped by this object */
} ;

/* ============================ INLINE METHODS ============================ */

#endif  // _UtlVoidPtr_h_
