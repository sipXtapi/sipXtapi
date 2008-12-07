//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlContainablePair_h_
#define _UtlContainablePair_h_

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

/// A UtlContainable wrapper for a pair of UtlContainable objects.
class UtlContainablePair : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    static const UtlContainableType TYPE; ///< Class type used for runtime checking.

/* ============================ CREATORS ================================== */

      /// Constructor accepting an optional default value.
    UtlContainablePair(UtlContainable *pFirst, UtlContainable *pSecond);
     
      /// Destructor
    virtual ~UtlContainablePair();

/* ============================ MANIPULATORS ============================== */

      /// Set a new first value for this object.
    UtlContainable* setFirst(UtlContainable *val);
      /**<
      *  @returns The old first value.
      */

      /// Set a new second value for this object.
    UtlContainable* setSecond(UtlContainable *val);
      /**<
      *  @returns The old second value.
      */

/* ============================ ACCESSORS ================================= */

      /// Get the first value wrapped by this object.
    UtlContainable* getFirst() const;    

      /// Get the second value wrapped by this object.
    UtlContainable* getSecond() const;    

      /// Calculate a unique hash code for this object.
    virtual unsigned hash() const;
      /**<
      *  If the equals operator returns true for another object, then both of
      *  those objects must return the same hashcode.
      */    

      /// Get the ContainableType for a UtlContainable derived class.
    virtual UtlContainableType getContainableType() const;

/* ============================ INQUIRY =================================== */

      /// Compare the this object to another like-objects. 
    virtual int compareTo(UtlContainable const *) const;
      /**<
      *  Compares mpFirst values, and return corresponding value if they're not
      *  equal. If mpFirst values are equal, return result of mpSecond values
      *  comparison.
      *
      *  Results for designating a non-like object are undefined.
      *
      *  @returns 0 if equal, < 0 if less then and >0 if greater.
      */

  
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlContainable *mpFirst;   ///< The first value, wrapped by this object.
    UtlContainable *mpSecond;  ///< The second value, wrapped by this object.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _UtlContainablePair_h_
