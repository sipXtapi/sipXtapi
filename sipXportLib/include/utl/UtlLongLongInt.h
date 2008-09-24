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
*  UtlContainable wrapper for a 64-bit long int.
*/
class UtlLongLongInt : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    static const UtlContainableType TYPE ;  ///< Class type used for runtime checking

/* ============================ CREATORS ================================== */

     /// Constructor accepting an optional default value.
    UtlLongLongInt(int64_t initialValue = 0) ;

     /// Destructor
    virtual ~UtlLongLongInt();

/* ============================ OPERATORS ============================== */

      /// Prefix increment operator
    UtlLongLongInt& operator++();
      /// Postfix increment operator
    UtlLongLongInt operator++(int);

      /// Prefix decrement operator
    UtlLongLongInt& operator--();
      /// Postfix decrement operator
    UtlLongLongInt operator--(int);

      /// Conversion to long long int
    operator int64_t() { return mValue; }

/* ============================ MANIPULATORS ============================== */

      /// Set a new long long int value for this object.
    int64_t setValue(int64_t iValue);
      /**<
      *  @returns the old value
      */

      /// Convert a ASCII string to long long int
    static int64_t stringToLongLong(const char* longLongString);

/* ============================ ACCESSORS ================================= */

      /// Get the long long int wrapped by this object.
    int64_t getValue() const ;

      /// Calculate a unique hash code for this object.
    virtual unsigned hash() const ;
      /**<
      *  If the equals operator returns true for another object, then both
      *  of those objects must return the same hash code.
      */

      /// Get the ContainableType for a UtlContainable derived class.
    virtual UtlContainableType getContainableType() const;

/* ============================ INQUIRY =================================== */

      /// Compare the this object to another like-object.
    virtual int compareTo(UtlContainable const *) const ;
      /**<
      *  Results for designating a non-like object are undefined.
      *
      *  @returns 0 if equal, < 0 if less then and >0 if greater.
      */

      /// Test this object to another like-object for equality.
    virtual UtlBoolean isEqual(UtlContainable const *) const ;
      /**<
      *  @returns false if unlike-objects are specified.
      */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    int64_t mValue ;    ///< The long long int wrapped by this object

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlLongLongInt_h_
