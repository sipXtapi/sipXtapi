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
*/
class MpMMTimerWnt : public MpMMTimer
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:
   typedef enum 
   {
      Multimedia = 0         ///< Microsoft Multimedia timers (W95+, CE)
      // Other possible choices, which have yet to be implemented: 
      // Microsoft Waitable Timers (W98/NT+) (Not Implemented)
      // Microsoft Queue Timers (W2k+) (Not Implemented)
   } MMTimerWntAlgorithms;

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
   OsStatus run(unsigned usecPeriodic, 
                        unsigned uAlgorithm = MPMMTIMER_ALGORITHM_DEFAULT);

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
   MMTimerWntAlgorithms mAlgorithm; ///< The current or last algorithm used.
   BOOL mbInitialized; ///< Whether we're fully initialized or not, or are in some failure state.
   BOOL mbTimerStarted; ///< Indicator of timer started or not.
   unsigned mPeriodMSec; ///< The current millisecond period being used.  0 when no timer.
   unsigned mResolution; ///< Cached timer resolution in ms, queried for and stored at startup.
   HANDLE mEventHandle; ///< Only valid in Linear mode, holds handle to an event.
   OsNotification* mpNotification; ///< Notification object used to signal a tick of the timer.
   MMRESULT mTimerId; ///< The ID of the MM timer we're using.

   static void CALLBACK timeProcCallback(UINT uID, UINT uMsg, DWORD dwUser, 
                                         DWORD dw1, DWORD dw2);
     /**< 
      *  @brief callback used by windows multimedia timers
      *
      *  This should only be called by the windows multimedia timer.
      */

   OsStatus runMultimedia(unsigned usecPeriodic);
     /**< 
      *  @brief start a multimedia timer.
      *
      *  This should only be used within the MpMMTimerWnt class.
      *  @see MpMMTimer::run(unsigned, unsigned)
      */

   OsStatus stopMultimedia();
     /**< 
      *  @brief stop a started multimedia timer.
      *
      *  This should only be used within the MpMMTimerWnt class.
      *  @see MpMMTimer::stop()
      */

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif //_MpMMTimerWnt_h_
