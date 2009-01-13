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
#include <os/OsIntTypes.h>
#include <os/OsStatus.h>
#include <os/OsSysLog.h>

// APPLICATION INCLUDES
#include "mp/MpMMTimer.h"

#if defined(WIN32) // [ WIN32
#  include "mp/MpMMTimerWnt.h"
#  define DEFAULT_TIMER_CLASS MpMMTimerWnt
#elif defined(__pingtel_on_posix__) // ][ POSIX
#  include "mp/MpMMTimerPosix.h"
#  define DEFAULT_TIMER_CLASS MpMMTimerPosix
#else // ][ Unsupported platform
#  error Unsupported target platform.
#endif // ] Unsupported platform

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpMMTimer *MpMMTimer::create(MMTimerType type, const UtlString &name)
{
   if (name.isNull())
   {
      return new DEFAULT_TIMER_CLASS(type);
   }
#ifdef WIN32 // [
   else if (name.compareTo(MpMMTimerWnt::TYPE, UtlString::ignoreCase))
   {
      return new MpMMTimerWnt(type);
   }
#endif // WIN32 ]
#ifdef __pingtel_on_posix__ // [
   else if (name.compareTo(MpMMTimerPosix::TYPE, UtlString::ignoreCase))
   {
      return new MpMMTimerPosix(type);
   }
#endif // __pingtel_on_posix__ ]
   return NULL;
}

MpMMTimer::~MpMMTimer()
{
}

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

/* =============================== INQUIRY ================================ */

/* ////////////////////////////// PROTECTED /////////////////////////////// */

/* /////////////////////////////// PRIVATE //////////////////////////////// */

/* ============================== FUNCTIONS =============================== */
