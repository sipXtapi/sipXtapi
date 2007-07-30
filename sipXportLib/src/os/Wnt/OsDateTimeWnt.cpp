//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#define WIN32_LEAN_AND_MEAN
#include <assert.h>
#include <windows.h>
#include <MMSystem.h>
#include <time.h>

#ifndef WINCE // [
   // winmm.lib is required for OsDateTimeWnt::getCurTime() implementation
   // (for timeGetTime() and timeBeginPeriod() functions).
#  pragma comment(lib, "winmm.lib")
#endif // WINCE ]

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/Wnt/OsDateTimeWnt.h"
#include "os/OsTime.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const __int64 WINDOWSTIME2UNIXTIME = 11644473600L;
const int MILLISECS_PER_SEC      = 1000;
const int MICROSECS_PER_MILLISEC = 1000;
const int FILETIME_UNITS_PER_SEC   = 10000000;  // 100 nanosecs per sec
const int FILETIME_UNITS_PER_USEC  = 10;        // 100 nanosecs per microsec

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

/// Convert an OsTime to an OsDateTime
OsDateTimeWnt::OsDateTimeWnt(const OsTime& toTime)
{
   // first convert the OsTime to a Windows FILETIME
   __int64 winTime_64;
   winTime_64 =  toTime.seconds();
   winTime_64 += WINDOWSTIME2UNIXTIME;   // adjust for epoch difference
   winTime_64 *= FILETIME_UNITS_PER_SEC; // scale to windows ticks
   winTime_64 += toTime.usecs() * FILETIME_UNITS_PER_USEC;
   FILETIME winTime;
   winTime.dwHighDateTime = (unsigned long)(winTime_64 >> 32);
   winTime.dwLowDateTime  = (unsigned long)(winTime_64 & 0xFFFFFFFF);

   // then the FILETIME to a broken out SYSTEMTIME
   SYSTEMTIME sysTime;
   FileTimeToSystemTime(&winTime, &sysTime);

   // and last, SYSTEMTIME to OsDateTime
   mYear        = sysTime.wYear;
   mMonth       = sysTime.wMonth - 1; // windows is 1-based
   mDay         = (unsigned char)sysTime.wDay;
   mHour        = (unsigned char)sysTime.wHour;
   mMinute      = (unsigned char)sysTime.wMinute;
   mSecond      = (unsigned char)sysTime.wSecond;
   mMicrosecond = sysTime.wMilliseconds * MICROSECS_PER_MILLISEC;
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
   bootTime_i64 = (((__int64)bootFileTime.dwHighDateTime) << 32) +
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

   thisTime_i64 = (((__int64)thisFileTime.dwHighDateTime) << 32) +
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


// Return the current time as an OsTime value
void OsDateTimeWnt::getCurTime(OsTime& rTime)
{
#if WINCE
    typedef union 
    {
        FILETIME         ft ;
        unsigned __int64 int64 ;        
    } g_FILETIME ;

    unsigned __int64 ticks ;
    unsigned __int64 freq ;
    static bool       sbInitialized = false ;
    static g_FILETIME sOsFileTime ;
    static unsigned __int64 sLastTicks = 0 ;
    static unsigned __int64 sResetTime = 0 ;

    QueryPerformanceCounter((LARGE_INTEGER*) &ticks) ;
    QueryPerformanceFrequency((LARGE_INTEGER*) &freq) ;

    if (!sbInitialized || sOsFileTime.int64 > sResetTime)
    {
        sbInitialized = true ;
        GetSystemTimeAsFileTime(&sOsFileTime.ft);
        sResetTime = -1 ; // sOsFileTime.int64 + (freq - 1) ;
        sLastTicks = ticks ;
    }
    else
    {
        unsigned __int64 delta = ticks - sLastTicks ;

        sLastTicks = ticks ;
        sOsFileTime.int64 = sOsFileTime.int64 + 
                (((unsigned __int64) 10000000) * (delta / freq)) + 
                (((unsigned __int64) 10000000) * (delta % freq)) / freq ;    
        
        SYSTEMTIME si ;
        FileTimeToSystemTime(&sOsFileTime.ft, &si) ;
    }

   OsTime curTime((long)  ((sOsFileTime.int64 - ((unsigned __int64) 116444736000000000)) / ((unsigned __int64) 10000000)), 
                  (long) ((sOsFileTime.int64 / ((unsigned __int64) 10)) % ((unsigned __int64) 1000000)));
   rTime = curTime;
#else
    typedef union 
    {
        FILETIME         ft ;
        unsigned __int64 int64 ;        
    } g_FILETIME ;

    DWORD systemMSecs;
    static bool       sbInitialized = false ;
    static g_FILETIME sOsFileTime ;
    static DWORD sLastSystemMSecs = 0 ;

    systemMSecs = timeGetTime();

    if (!sbInitialized)
    {
        sbInitialized = true ;
        FILETIME sft;
        // Set the precision of timings got from timeGetTime.
        timeBeginPeriod(1);
        // Resample time, since we changed the precision.
        systemMSecs = timeGetTime();

        GetSystemTimeAsFileTime(&sft);
        // Store in a temp and copy over to prevent datatype misalignment issues.
        sOsFileTime.ft = sft;
        sLastSystemMSecs = systemMSecs ;
    }
    else
    {
        DWORD delta = systemMSecs - sLastSystemMSecs ;

        sLastSystemMSecs = systemMSecs;
        sOsFileTime.int64 = sOsFileTime.int64 + 
                            10000 * delta;  // convert delta msec to 100ns units
        
        SYSTEMTIME si ;
        FileTimeToSystemTime(&sOsFileTime.ft, &si) ;
    }

   OsTime curTime((long) ((sOsFileTime.int64 - 
                           ((unsigned __int64) WINDOWSTIME2UNIXTIME * 10000000)) 
                          / ((unsigned __int64) 10000000)), 
                  (long) ((sOsFileTime.int64 / ((unsigned __int64) 10)) % 
                          ((unsigned __int64) 1000000)));
   rTime = curTime;
#endif
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
   static long  start_msecs =   GetTickCount();
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
