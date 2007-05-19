//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _UTLDATETIME_H_
#define _UTLDATETIME_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"
#include "os/OsDateTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlDateTime is a UtlContainable wrapper for a OsDateTime object.
 */
class UtlDateTime : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Constructor
     */
    UtlDateTime(OsDateTime time) ;
      
    /**
     * Destructor
     */
    virtual ~UtlDateTime();

/* ============================ MANIPULATORS ============================== */

    /**
     * Set a new time value for this object.
     *
     */
    void setTime(const OsDateTime& time) ;

/* ============================ ACCESSORS ================================= */

    /**
     * Get the time wrapped by this object.
     */
    void getTime(OsDateTime& time) const ;    

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
     * Compare the this object to another like-objects.  Results for 
     * designating a non-like object are undefined.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    virtual int compareTo(UtlContainable const *) const ;    


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static UtlContainableType TYPE ;    /** < Class type used for runtime checking */ 

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    OsDateTime mTime ;    /** < The OsDateTime wrapped by this object */ 

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UTLDATETIME_H_
