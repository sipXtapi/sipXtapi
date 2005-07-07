// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <windows.h>
#include <time.h>

// APPLICATION INCLUDES
#include "os/Wnt/OsDateTimeWnt.h"
#include "os/OsTime.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
static int MILLISECS_PER_SEC      = 1000;
static int MICROSECS_PER_MILLISEC = 1000;
static int FILETIME_UNITS_PER_SEC   = 10000000;  // 100 nanosecs per sec
static int FILETIME_UNITS_PER_USEC  = 10;        // 100 nanosecs per microsec

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default constructor
OsDateTimeWnt::OsDateTimeWnt()
 : OsDateTimeBase()
{
   // no further work required
   //  all of the work is done by the parent constructor
}

// Constructor
OsDateTimeWnt::OsDateTimeWnt(const unsigned short year,
                             const unsigned char  month,
                             const unsigned char  day,
                             const unsigned char  hour,
                             const unsigned char  minute,
                             const unsigned char  second,
                             const unsigned int   microsecond)
 : OsDateTimeBase(year, month, day, hour, minute, second, microsecond)
{
   // no further work required
   //  all of the work is done by the parent constructor
}

// Copy constructor
OsDateTimeWnt::OsDateTimeWnt(const OsDateTimeWnt& rOsDateTimeWnt)
 : OsDateTimeBase((const OsDateTimeBase&) rOsDateTimeWnt)
{
   // no further work required
   //  all of the work is done by the parent copy constructor
}

// Destructor
OsDateTimeWnt::~OsDateTimeWnt()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsDateTimeWnt& 
OsDateTimeWnt::operator=(const OsDateTimeWnt& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsDateTimeBase::operator=((const OsDateTimeBase&) rhs);
   // no further work required
   //  all of the "heavy lifting" is done by the parent assignment operator

   return *this;
}

// Set the system time.  For now, this is a no-op for Windows platforms.
void OsDateTimeWnt::setTime(struct timespec gmt, int tzOffsetSecs,
                            DstRule dstRule)
{
   // no-op for now
}

// Set the system timezone.  For now, this is a no-op for Windows platforms.
void OsDateTimeWnt::setTimeZone(int tzOffsetSecs, DstRule dstRule)
{
   // no-op for now
}

/* ============================ ACCESSORS ================================= */

// Convert the OsDateTimeBase value to an OsTime value
// The OsTime value is relative to when the system was booted.
OsStatus OsDateTimeWnt::cvtToTimeSinceBoot(OsTime& rTime) const
{
   UtlBoolean     ntRes;

   FILETIME   bootFileTime;      // boot time in various representations
   __int64    bootTime_i64;

   FILETIME   thisFileTime;      // this time in various representations
   SYSTEMTIME thisSysTime;
   __int64    thisTime_i64;

   __int64    deltaTime_i64;     // (this time - boot time) in various
   __int64    deltaSecs_i64;     //  representations
   __int64    deltaUsecs_i64;

   // get the system boot time as a 64-bit integer recording the number of 
   //  100-nanosecond intervals since January 1, 1601
   GetSystemTimeAsFileTime(&bootFileTime);          // get as FILETIME
   bootTime_i64 = (bootFileTime.dwHighDateTime << 32) +
                  bootFileTime.dwLowDateTime;       // convert to __int64

   // convert this OsDateTime object to a 64-bit integer
   thisSysTime.wYear         = mYear;               // represent as SYSTEMTIME
   thisSysTime.wMonth        = mMonth+1;  // because we subtracted previously(just like vxw)
   thisSysTime.wDay          = mDay;
   thisSysTime.wHour         = mHour;
   thisSysTime.wMinute       = mMinute;
   thisSysTime.wSecond       = mSecond;
   thisSysTime.wMilliseconds = mMicrosecond / 1000;

                                                    // convert to FILETIME 
   ntRes = SystemTimeToFileTime(&thisSysTime, &thisFileTime);
   assert(ntRes == TRUE);

   thisTime_i64 = (thisFileTime.dwHighDateTime << 32) +
                  thisFileTime.dwLowDateTime;       // convert to __int64

   // compute (thisFileTime - bootFileTime) and convert to an OsTime value
   deltaTime_i64  = thisTime_i64 - bootTime_i64;
   deltaSecs_i64  = (long) (deltaTime_i64 / FILETIME_UNITS_PER_SEC);
   deltaUsecs_i64 = (deltaTime_i64 % FILETIME_UNITS_PER_SEC) /
                     FILETIME_UNITS_PER_USEC;

   assert((deltaSecs_i64 >> 32)  == 0);
   assert((deltaUsecs_i64 >> 32) == 0);

   OsTime deltaOsTime((long) deltaSecs_i64, (long) deltaUsecs_i64);
   rTime = deltaOsTime;

   return OS_SUCCESS;
}

// Return the current time as an OsDateTime value
void OsDateTimeWnt::getCurTime(OsDateTimeWnt& rDateTime)
{
   SYSTEMTIME sysTime;

   GetSystemTime(&sysTime);

   rDateTime.mYear        = sysTime.wYear;
   rDateTime.mMonth       = (unsigned char) sysTime.wMonth-1; //month should start with 0 just like vxw
   rDateTime.mDay         = (unsigned char) sysTime.wDay;
   rDateTime.mHour        = (unsigned char) sysTime.wHour;
   rDateTime.mMinute      = (unsigned char) sysTime.wMinute;
   rDateTime.mSecond      = (unsigned char) sysTime.wSecond;
   rDateTime.mMicrosecond = sysTime.wMilliseconds * MICROSECS_PER_MILLISEC;
}

// Return the current time as an OsTime value
// The OsTime value is relative to when the system was booted.
void OsDateTimeWnt::getCurTimeSinceBoot(OsTime& rTime)
{
   DWORD msecs;
   long  secs;
   long  usecs;
   static long  start_msecs = 	GetTickCount();
   msecs = GetTickCount()-start_msecs;
   secs  =  msecs / MILLISECS_PER_SEC;
   usecs = (msecs % MILLISECS_PER_SEC) * MICROSECS_PER_MILLISEC;

   OsTime timeSinceBoot(secs, usecs);
   rTime = timeSinceBoot;
}

// Current time as the number of seconds since midnight (0 hour) 01/01/1970
unsigned long OsDateTimeWnt::getSecsSinceEpoch(void)
{
   return time(NULL);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


