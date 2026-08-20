//
// Copyright (C) 2006-2026 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsTimerTask_h_
#define _OsTimerTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsMsgQ.h"
#include "os/OsServerTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsTimer;
class OsTimerMsg;

//:Timer service request manager (runs as a separate task)
// This task is responsible for managing timer service requests. Timer
// requests are received via a message queue.

class OsTimerTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum { TIMER_CENSUS_MAX_ENTRIES = 32 };

   /// Signature of one timer found still started when the timer task was destroyed
   struct OsTimerCensusEntry
   {
      unsigned long mId;             ///< OsTimer::getId of the timer.
      const char*   mCallerFunction; ///< Function that constructed the timer.
      const char*   mCallerFile;     ///< File that constructed the timer.
      int           mCallerLine;     ///< Line that constructed the timer.
      UtlBoolean    mPeriodic;       ///< TRUE if the timer fires repetitively.
      int           mPeriodMsec;     ///< Repetition period, 0 if not periodic.
   };

/* ============================ CREATORS ================================== */

   static OsTimerTask* getTimerTask(void);
     //:Return a pointer to the timer task, creating it if necessary

   static void destroyTimerTask(void);
     //: Destroy the singleton instance of the sys timer
     // Should only be called when timers are not being started or stopped.
     // All current timers are stopped.

   virtual
   ~OsTimerTask();
     //:Destructor
     // Should not be called directly.  Use destroyTimerTask().

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   /// Number of started timers recorded since the census was last cleared
   static int getCensusCount();
   /**
    * destroyTimerTask() stops every timer still in the timer queue, whether
    * or not its owner is finished with it.  The census records those timers
    * so that a caller can tell whether a teardown was clean.  It accumulates
    * across destructions, as sipxUnInitialize destroys the task more than
    * once.
    *
    * The census is written on the timer task thread while handling the
    * shutdown message, and ~OsTimerTask waits for that message to be
    * handled, so these values are stable once destroyTimerTask() returns.
    */

   /// Get a recorded timer, index must be < getCensusCount()
   static const OsTimerCensusEntry* getCensusEntry(int index);

   /// Number of started timers that did not fit in the census
   static int getCensusOverflowCount();

   /// Number of requests still queued when the timer task was destroyed
   static int getCensusPendingMessageCount();

   /// Number of times the timer task was destroyed since the census was cleared
   static int getCensusShutdownCount();

   /// Discard everything recorded in the census
   static void clearCensus();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// Constructor (called only indirectly via getTimerTask())
   OsTimerTask();
   /**< We identify this as a protected (rather than a private) method so
    *   that gcc doesn't complain that the class only defines a private
    *   constructor and has no friends.
    */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const int TIMER_MAX_REQUEST_MSGS;   // Maximum number of request messages
 
   /// The entry point for the task
   virtual int run(void* pArg);
   /**< We replace OsServerTask::run() so that it will simultaneously wait
    *   for an incoming message or for the next timer to fire.
    */

   /// Handle a timer service request.
   virtual UtlBoolean handleMessage(OsMsg& rMsg);
   ///< Return TRUE if the request was handled, otherwise FALSE.

   /** Fire a timer because it has expired.
    *  Calls the if notification routine, if the timer hasn't been stopped
    *  already.
    *  If the timer is periodic and hasn't been stopped, reinserts it into
    *  the queue.
    *  Advances the timer's state if it is one-shot or has been stopped.
    */
   virtual void fireTimer(OsTimer* timer);

   /// Pointer to the single instance of the OsTimerTask class.
   static volatile OsTimerTask* spInstance;
   ///< Declare as volatile because it is set and tested concurrently.

   /// Semaphore used to protect manipulations of spInstance.
   static OsBSem *sLock;

   /// Census of timers still started when the timer task was destroyed.
   static OsTimerCensusEntry sCensus[TIMER_CENSUS_MAX_ENTRIES];
   static int sCensusCount;
   static int sCensusOverflowCount;
   static int sCensusPendingMessageCount;
   static int sCensusShutdownCount;

   /// The queue of timer requests, ordered by increasing firing time.
   OsTimer* mTimerQueue;

   /// Timeout to use when signalling
   OsTime mSignalTimeout;

   /// Insert a timer into the timer queue.
   void insertTimer(OsTimer* timer);

   /// Remove a timer from the timer queue.
   void removeTimer(OsTimer* timer);

   /// Copy constructor (not implemented for this class)
   OsTimerTask(const OsTimerTask& rOsTimerTask);

   /// Assignment operator (not implemented for this class)
   OsTimerTask& operator=(const OsTimerTask& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTimerTask_h_
