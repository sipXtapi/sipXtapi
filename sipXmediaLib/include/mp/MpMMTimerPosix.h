//  
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <sergey DOT kostanbaev AT SIPez DOT com>

#ifndef _MpMMTimerPosix_h_
#define _MpMMTimerPosix_h_

// SYSTEM INCLUDES
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>


// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include "mp/MpMMTimer.h"
#include <os/OsMutex.h>

#ifdef __APPLE__
#warning PosixTimer is not implimented in MacOS X
#define timer_t     int
#endif

/**
*  @brief Posix implementation of periodic timer
*
*  WARNING: You MUST be sure that exists only one object of this class.
*           To support several MpMMTimer objects we need to circumvent
*           sync sigwait() peculiarities, which will complicate the code
*           a lot and is not necessary right now.
*/
class MpMMTimerPosix : public MpMMTimer
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:
   class PosixSignalReg; // Forward declaration

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

     /// Returns signal descriptor for thread blocking/unblocking operations
   static PosixSignalReg* getSignalDescriptor();

//@}


     /// Help class for signal registering
   class PosixSignalReg
   {
   public:
      PosixSignalReg(int sigNum, void (*)(int, siginfo_t *, void *));
      ~PosixSignalReg();

        /// Get number of the signaling signal
      int getSignalNum() const;

      int blockThreadSig();
      int unblockThreadSig();

   private:
      struct sigaction mOldAction; ///< Old signal action
      int mSigNum;                 ///< Signal number
      sigset_t mBlockSigMask;      ///< Block mask for only this signal
   };

     /// Signal callback
   static
   void signalHandler(int signum, siginfo_t *siginfo, void *context);

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:
   OsNotification* mpNotification; ///< Notification object used to signal a tick of the timer.
   UtlBoolean mbTimerStarted;      ///< Is timer started.
   timer_t mTimer;                 ///< Timer object.
   sem_t mSyncSemaphore;           ///< Synchronization semaphore for linear operation.

   void callback();


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:   
   static PosixSignalReg sPosixTimerReg;
   pthread_t mThread;              ///< Sync-signal wait thread
   sem_t mIoSem;                   ///< Startup initialization semaphore
   UtlBoolean mbTerminate;         ///< Should the timer thread exit?

   static void* threadIoWrapper(void* arg);
};

/* ============================ INLINE METHODS ============================ */
#endif //_MpMMTimerPosix_h_
