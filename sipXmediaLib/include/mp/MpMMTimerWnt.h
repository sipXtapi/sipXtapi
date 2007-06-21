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

#ifndef _MpMMTimerWnt_h_
#define _MpMMTimerWnt_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpMMTimer.h"
#include "os/OsMutex.h"

class MpMMTimerWnt : public MpMMTimer
{
private:
   static void CALLBACK timeProcCallback(UINT uID, UINT uMsg, DWORD dwUser, 
                                         DWORD dw1, DWORD dw2);
     /**< 
      *  @brief callback used by windows multimedia timers
      *
      *  This should only be called by the windows multimedia timer.
      */


public:
   typedef enum 
   {
      Multimedia = 0,         ///< Microsoft Multimedia timers (W95+, CE)
      WaitableTimer = 1,      ///< Microsoft Waitable Timers (W98/NT+) (Not Implemented)
      QueueTimer = 2          ///< Microsoft Queue Timers (W2k+) (Not Implemented)
   } MMTimerWntAlgorithms;

   MpMMTimerWnt(MpMMTimer::MMTimerType type);
   virtual ~MpMMTimerWnt();

     /// @copydoc MpMMTimer::setCallback(OsCallback*)
   virtual inline OsStatus setCallback(OsCallback* callbackFn);

   /// @copydoc MpMMTimer::run(unsigned, unsigned)
   virtual OsStatus run(unsigned usecPeriodic, 
                        unsigned uAlgorithm = MPMMTIMER_ALGORITHM_DEFAULT);

     /// @copydoc MpMMTimer::stop()
   virtual OsStatus stop();

     /// @copydoc MpMMTimer::onFire(UtlBoolean);
   virtual void onFire(UtlBoolean bFirstFire);

     /// @copydoc MpMMTimer::waitForNextTick()
   OsStatus waitForNextTick();

     /// @copydoc MpMMTimer::getUSecSinceLastFire() const
   int getUSecSinceLastFire() const;

     /// @copydoc MpMMTimer::getUSecDeltaExpectedFire() const
   int getUSecDeltaExpectedFire() const;

     /// @copydoc MpMMTimer::getAbsFireTime() const
   OsTime getAbsFireTime() const;

protected:
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

private:
   MMTimerWntAlgorithms mAlgorithm; ///< The current or last algorithm used.
   BOOL mbInitialized; ///< Whether we're fully initialized or not, or are in some failure state.
   BOOL mbTimerStarted; ///< Indicator of timer started or not.
   unsigned mPeriodUSec; ///< The current period being used.  0 when no timer.
   BOOL mbTimerFired; ///< Set to false until the timer first fires, then set to true thereafter. reset on stop.
   unsigned mResolution; ///< Cached timer resolution, queried for and stored at startup.
   HANDLE mEventHandle; ///< Only valid in Linear mode, holds handle to an event.
};



// Inline Function Implementation

OsStatus MpMMTimerWnt::setCallback(OsCallback* callbackFn)
{ 
   return OS_NOT_YET_IMPLEMENTED;
}


#endif //_MpMMTimerWnt_h_
