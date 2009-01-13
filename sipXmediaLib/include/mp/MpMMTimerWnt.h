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

#ifndef _MpMMTimerWnt_h_
#define _MpMMTimerWnt_h_

// SYSTEM INCLUDES
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <MMSystem.h>

// APPLICATION INCLUDES
#include "mp/MpMMTimer.h"
#include "os/OsMutex.h"

/**
*  @brief Windows implementation of High-precision periodic timer (MultiMedia timer).
*
*  This timer is implemented using Microsoft Multimedia timers (W95+, CE).
*  Other possible choices, which have yet to be implemented: 
*    1) Microsoft Waitable Timers (W98/NT+)
*    2) Microsoft Queue Timers (W2k+)
*  Queue timers would be good to support though! They have less overhead,
*  and similar high resolution.
*  See http://www.codeproject.com/system/timers_intro.asp
*/
class MpMMTimerWnt : public MpMMTimer
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

   static const char * const TYPE;

/* =============================== CREATORS =============================== */
///@name Creators
//@{

   MpMMTimerWnt(MpMMTimer::MMTimerType type);

   virtual ~MpMMTimerWnt();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// @copydoc MpMMTimer::setNotification()
   OsStatus setNotification(OsNotification* notification);

     /// @copydoc MpMMTimer::run()
   OsStatus run(unsigned usecPeriodic);

     /// @copydoc MpMMTimer::stop()
   OsStatus stop();

     /// @copydoc MpMMTimer::waitForNextTick()
   OsStatus waitForNextTick();

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

    /// @copydoc MpMMTimer::getResolution()
   OsStatus getResolution(unsigned& resolution);

    /// @copydoc MpMMTimer::getPeriodRange()
   OsStatus getPeriodRange(unsigned* pMinUSecs, unsigned* pMaxUSecs = NULL);

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:
   BOOL mbInitialized; ///< Whether we're fully initialized or not, or are in some failure state.
   BOOL mbTimerStarted; ///< Indicator of timer started or not.
   unsigned mPeriodMSec; ///< The current millisecond period being used.  0 when no timer.
   unsigned mResolution; ///< Cached timer resolution in ms, queried for and stored at startup.
   HANDLE mEventHandle; ///< Only valid in Linear mode, holds handle to an event.
   OsNotification* mpNotification; ///< Notification object used to signal a tick of the timer.
   MMRESULT mTimerId; ///< The ID of the MM timer we're using.

     /// Callback used by Windows Multimedia Timers.
   static void CALLBACK timeProcCallback(UINT uID, UINT uMsg, DWORD dwUser, 
                                         DWORD dw1, DWORD dw2);

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif //_MpMMTimerWnt_h_
