//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsSysSoftTimer_h_
#define _OsSysSoftTimer_h_

// SYSTEM INCLUDES
#include <signal.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsMsgQ.h"
#include "os/OsTask.h"
#include "os/OsSysTimer.h"
#include "os/shared/OsTimerMessage.h"
#include "os/linux/OsLinuxDefs.h"

#if defined(sun)
   #include <sys/time.h>
   typedef hrtime_t pt_tick_t;
   #define pt_get_ticks() gethrtime()
   /* Solaris gethrtime() returns nanoseconds */
   #define TICKS_PER_SECOND ((pt_tick_t) 1000000000)
   #define TICKS_PER_MSEC   ((pt_tick_t) 1000000)
#elif defined(__pingtel_on_posix__)
   #include <sys/time.h>
   #include <unistd.h>
   typedef long long pt_tick_t;
   /* to be defined in OsSysSoftTimer.cpp in #ifdef __pingtel_on_posix__ */
   pt_tick_t pt_get_ticks(void);
   /* gettimeofday() has microsecond precision */
   #define TICKS_PER_SECOND ((pt_tick_t) 1000000)
   #define TICKS_PER_MSEC   ((pt_tick_t) 1000)
#elif defined(_WIN32)
   #include <windows.h>
   typedef LONG pt_tick_t;
   #define pt_get_ticks() GetTickCount()
   /* Windows GetTickCount() returns milliseconds */
   #define TICKS_PER_SECOND ((pt_tick_t) 1000)
   #define TICKS_PER_MSEC   ((pt_tick_t) 1)
#else
   #error Unsupported target platform.
#endif

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

/* we can't just use OsMsgQ because we need a smarter queue */
struct TIMER_NODE {
   OsTimer * pTimer;
   pt_tick_t expiry;
   pt_tick_t period;
   struct TIMER_NODE * prev;
   struct TIMER_NODE * next;
};

// TYPEDEFS
// FORWARD DECLARATIONS

//:Software-based timers for Solaris and WIN32
class OsSysSoftTimer : public OsSysTimerBase, public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int MAX_REQUEST_MSGS;   // Maximum number of request messages

/* ============================ CREATORS ================================== */

   static OsSysSoftTimer* getSysTimer(void);
     //:Return a pointer to the singleton object, creating it if necessary

   virtual
   ~OsSysSoftTimer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   static void showTimerInfo(void);
     //:Display information about the active timers in the system.

   void startTimer(OsTimer* pTimer);
     //:Start an emulated low-level system timer

   void stopTimer(OsTimer* pTimer);
     //:Stop and release an emulated low-level system timer

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsSysSoftTimer();
     //:Default constructor (only available internal to the class)

   int run(void* pArg);
     //:The timer management code runs as a thread

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   static OsSysSoftTimer* spInstance;  // Pointer to the instance of this
                                       // singleton class
   static OsBSem sLock;                // semaphore used to ensure that there is
                                       //  only one instance of this class
   static struct TIMER_NODE * timerQ;  // Queue of active timers
   static OsMsgQ* requestQ;            // Queue of timer operation requests

   OsSysSoftTimer(const OsSysSoftTimer& rOsSysSoftTimer);
     //:Copy constructor (not implemented for this class)

   OsSysSoftTimer& operator=(const OsSysSoftTimer& rhs);
     //:Assignment operator (not implemented for this class)

   static void doAddTimer(OsTimerMessage * msg);
     //:Add a new timer according to "msg"

   static void doRemoveTimer(OsTimerMessage * msg);
     //:Remove an old timer according to "msg"
   
   static bool doServiceRequests(OsMsg* pFirstMessage);
     //:Service all timer requests and return a boolean indicating whether
     //: the task should continue running.

   static struct TIMER_NODE * OsSysSoftTimer::peekQ(void);
     //:Peek at the first element of the timer queue

   static struct TIMER_NODE * OsSysSoftTimer::advanceQ(void);
     //:Advance the timer queue, returning the first element

   static int OsSysSoftTimer::insertQ(struct TIMER_NODE * node);
     //:Insert an element into the queue, returning 1 if it was at the head

   static void OsSysSoftTimer::flushQ(void);
     //:Flush the queue, removing and freeing all timers

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsSysSoftTimer_h_

