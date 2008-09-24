//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

// SYSTEM INCLUDES
#ifdef WIN32 // [
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <MMSystem.h>
#endif // WIN32 ]
#include <os/OsStatus.h>
#include <os/OsSysLog.h>

// APPLICATION INCLUDES
#include "mp/MpMMTimer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// Static Methods
OsStatus MpMMTimer::getResolution(unsigned& resolution)
{
   OsStatus status = OS_NOT_YET_IMPLEMENTED;
#ifdef WIN32
   status = getPeriodRange(&resolution);
#endif
   return status;
}

OsStatus MpMMTimer::getPeriodRange(unsigned* pMinUSecs, 
                                  unsigned* pMaxUSecs)
{
   OsStatus status = OS_NOT_YET_IMPLEMENTED;

#ifdef WIN32
   status = OS_FAILED;
   TIMECAPS timecaps;
   // timeGetDevCaps deals in msecs, not usecs.. as does the 
   // rest of the windows MMTimers - thus multiply by 1000.
   if(timeGetDevCaps(&timecaps, sizeof(TIMECAPS)) == TIMERR_NOERROR)
   {
      status = OS_SUCCESS;
      if(pMinUSecs)
      {
         *pMinUSecs = (unsigned)(timecaps.wPeriodMin) * 1000;
      }
      if(pMaxUSecs)
      {
         *pMaxUSecs = (unsigned)(timecaps.wPeriodMax) * 1000;
      }
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "Couldn't get windows MMTimer capabilities!");
   }
#endif
   return status;
}

// Non-static methods
OsStatus MpMMTimer::setNotification(OsNotification* notification)
{
   return OS_INVALID_STATE;
}

OsStatus MpMMTimer::waitForNextTick()
{
   OsStatus ret = 
      (mTimerType != Linear) ? OS_INVALID_STATE : OS_NOT_YET_IMPLEMENTED;
   return ret;
}

int MpMMTimer::getUSecSinceLastFire() const
{
   return -1;
}

int MpMMTimer::getUSecDeltaExpectedFire() const
{
   return -1;
}

OsTime MpMMTimer::getAbsFireTime() const
{
   return OsTime::OS_INFINITY;
}

