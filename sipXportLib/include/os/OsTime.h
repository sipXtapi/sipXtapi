// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsTime_h_
#define _OsTime_h_

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

//:Time or time interval
// If necessary, this class will adjust the seconds and microseconds values
// that it reports such that 0 <= microseconds < USECS_PER_SEC.


class OsTime : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Time quantity enum for special time values
   typedef enum
   {
      OS_INFINITY = -1,
      NO_WAIT_TIME = 0
   } TimeQuantity;

   static const long MSECS_PER_SEC;
   static const long USECS_PER_MSEC;
   static const long USECS_PER_SEC;

/* ============================ CREATORS ================================== */

   OsTime();
     //:Default constructor (creates a zero duration interval)

   OsTime(const long msecs);
     //:Constructor specifying time/duration in terms of milliseconds

   OsTime(TimeQuantity quantity);
     //:Constructor specifying time/duration in terms of TimeQuantity enum

   OsTime(const long seconds, const long usecs);
     //:Constructor specifying time/duration in terms of seconds and microseconds

   OsTime(const OsTime& rOsTime);
     //:Copy constructor

   virtual
   ~OsTime();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsTime& operator=(TimeQuantity rhs);
     //:Assignment operator

   OsTime& operator=(const OsTime& rhs);
     //:Assignment operator

   OsTime operator+(const OsTime& rhs);
     //:Addition operator

   OsTime operator-(const OsTime& rhs);
     //:Subtraction operator

   OsTime operator+=(const OsTime& rhs);
     //:Increment operator

   OsTime operator-=(const OsTime& rhs);
     //:Decrement operator

   bool operator==(const OsTime& rhs) const;
     //:Test for equality operator

   bool operator!=(const OsTime& rhs) const;
     //:Test for inequality operator

   bool operator>(const OsTime& rhs) const;
     //:Test for greater than

   bool operator>=(const OsTime& rhs) const;
     //:Test for greater than or equal

   bool operator<(const OsTime& rhs) const;
     //:Test for less than

   bool operator<=(const OsTime& rhs) const;
     //:Test for less than or equal

/* ============================ ACCESSORS ================================= */

   virtual long seconds(void) const
   {
      return mSeconds;
   }
     //:Return the seconds portion of the time interval

   virtual long usecs(void) const
   {
      return mUsecs;
   }
     //:Return the microseconds portion of the time interval

   virtual long cvtToMsecs(void) const;
     //:Convert the time interval to milliseconds

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

   virtual UtlBoolean isInfinite(void) const;
     //:Return TRUE if the time interval is infinite

   virtual UtlBoolean isNoWait(void) const;
     //:Return TRUE if the time interval is zero (no wait)

    /**
     * Compare the this object to another like-objects.  Results for 
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
    static UtlContainableType TYPE ;    /** < Class type used for runtime checking */ 

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   long mSeconds;
   long mUsecs;

   void init(void);
     //:Initialize the instance variables for a newly constructed object

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTime_h_

