//  
// Copyright (C) 2007-2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2009 SIPfoundry Inc.
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
#include <os/OsIntTypes.h>
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

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

/* ============================= MANIPULATORS ============================= */

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

/* ============================== ACCESSORS =============================== */

OsStatus MpMMTimer::getResolution(unsigned& resolution)
{
   return OS_NOT_YET_IMPLEMENTED;
}

OsStatus MpMMTimer::getPeriodRange(unsigned* pMinUSecs, 
                                   unsigned* pMaxUSecs)
{
   return OS_NOT_YET_IMPLEMENTED;
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

/* =============================== INQUIRY ================================ */

/* ////////////////////////////// PROTECTED /////////////////////////////// */

/* /////////////////////////////// PRIVATE //////////////////////////////// */

/* ============================== FUNCTIONS =============================== */
