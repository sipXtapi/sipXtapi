//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsTime_h_
#define _OsTime_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "utl/UtlDefs.h"

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


class OsTime
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const OsTime OS_INFINITY;
   static const OsTime NO_WAIT;
   static const long MSECS_PER_SEC ; 
   static const long USECS_PER_MSEC ;
   static const long USECS_PER_SEC ;

/* ============================ CREATORS ================================== */

   OsTime();
     //:Default constructor (creates a zero duration interval)

   OsTime(const long msecs);
     //:Constructor specifying time/duration in terms of milliseconds

   OsTime(const long seconds, const long usecs);
     //:Constructor specifying time/duration in terms of seconds and microseconds

   OsTime(const OsTime& rOsTime);
     //:Copy constructor

   virtual
   ~OsTime();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

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

   bool operator==(const OsTime& rhs);
     //:Test for equality operator

   bool operator!=(const OsTime& rhs);
     //:Test for inequality operator

   bool operator>(const OsTime& rhs);
     //:Test for greater than

   bool operator>=(const OsTime& rhs);
     //:Test for greater than or equal

   bool operator<(const OsTime& rhs);
     //:Test for less than

   bool operator<=(const OsTime& rhs);
     //:Test for less than or equal

/* ============================ ACCESSORS ================================= */

   virtual long seconds(void) const;
     //:Return the seconds portion of the time interval

   virtual long usecs(void) const;
     //:Return the microseconds portion of the time interval

   virtual long cvtToMsecs(void) const;
     //:Convert the time interval to milliseconds

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isInfinite(void) const;
     //:Return TRUE if the time interval is infinite

   virtual UtlBoolean isNoWait(void) const;
     //:Return TRUE if the time interval is zero (no wait)

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   long mSeconds;
   long mUsecs;

   void init(void);
     //:Initialize the instance variables for a newly constructed object

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTime_h_

