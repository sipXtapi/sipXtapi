//  
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <sergey.kostanbaev AT SIPez DOT com>

#ifndef _MpMMTimerPosix_h_
#define _MpMMTimerPosix_h_

// SYSTEM INCLUDES
#include <time.h>
#include <signal.h>

// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include "mp/MpMMTimer.h"
#include <os/OsMutex.h>

/**
*  @brief Posix implementation of periodic timer
*/
class MpMMTimerPosix : public MpMMTimer
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

   static const char * const TYPE;

/* =============================== CREATORS =============================== */
///@name Creators
//@{

   MpMMTimerPosix(MpMMTimer::MMTimerType type);

   ~MpMMTimerPosix();

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

     /// @copydoc MpMMTimer::getAbsFireTime() const
   OsTime getAbsFireTime() const;

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}


   /// Help class for signal registring
   class PosixSignalReg
   {
   public:
      PosixSignalReg(int sigNum, void (*)(int, siginfo_t *, void *));
      ~PosixSignalReg();
   private:
      struct sigaction mOldAction; ///< Old signal action
      int mSigNum;                 ///< Signal number
   };

   static
   void signalHandler(int signum, siginfo_t *siginfo, void *context);

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:
   OsNotification* mpNotification; ///< Notification object used to signal a tick of the timer.   
   UtlBoolean mbTimerStarted;
   timer_t mTimer; ///< Timer object

   void callback();


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:   
   static PosixSignalReg sPosixTimerReg;
};

/* ============================ INLINE METHODS ============================ */
#endif //_MpMMTimerPosix_h_
