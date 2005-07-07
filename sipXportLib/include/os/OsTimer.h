// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _OsTimer_h_
#define _OsTimer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsDateTime.h"
#include "os/OsNotification.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:One-shot and periodic timers
// When the timer expires, an OsNotification object (passed to the
// constructor) is used to signal the event.

class OsTimer
{

//!rschaaf: This is ugly, but I couldn't come up with a better way to do it

#if defined(_WIN32)
   friend class OsSysSoftTimer;
#elif defined(_VXWORKS)
   friend class OsSysTimerVxw;
#elif defined(__pingtel_on_posix__)
   friend class OsSysSoftTimer;
#else
#  error Unsupported target platform.
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsTimer(OsNotification& rNotifier);
     //:Constructor
     // Timer expiration event notification happens using the OsNotification
     // object. The address of "this" OsTimer object is the eventData that is
     // conveyed to the Listener when the notification is signaled.

   virtual
   ~OsTimer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus oneshotAt(const OsDateTime& when);
     //:Arm the timer to fire once at the indicated date and time

   virtual OsStatus oneshotAfter(const OsTime& offset);
     //:Arm the timer to fire once at the current time + offset

   virtual OsStatus periodicAt(const OsDateTime& when, OsTime period);
     //:Arm the timer to fire periodically starting at the indicated date/time

   virtual OsStatus periodicEvery(OsTime offset, OsTime period);
     //:Arm the timer to fire periodically starting at current time + offset

   virtual OsStatus stop(void);
     //:Disarm the timer

/* ============================ ACCESSORS ================================= */

   virtual OsNotification* getNotifier(void) const;
     //:Return the OsNotification object for this timer

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum OsTimerType
   {
      UNSPECIFIED = 0x0,      // not initialized
      ONESHOT     = 0x1,      // expire at the specified offset
      PERIODIC    = 0x2,      // expire every period
   };

   // Possible states for the OsTimer object.  The state progression is from
   // CREATED -> STARTED -> STOPPED.  The meaning of the states and state
   // transitions is as follows:
   // 
   // CREATED                    The object has been constructed
   // CREATED->STARTED           A timer start request has been sent to the
   //                             OsTimerTask. As part of this transition, the
   //                             client acquires the mSemaphore lock.
   // STARTED->STOPPED           Either the timer has been explicitly stopped
   //                             (by sending a STOP message to the
   //                             OsTimerTask), or we have a oneshot timer
   //                             that has expired. As part of this
   //                             transition, the OsTimerTask releases the
   //                             mSemaphore lock.
   // STOPPED->STARTED           The client sends another start request for
   //                             this OsTimer object (and again acquires the
   //                             mSemaphore lock.
   enum OsTimerState
   {
      CREATED,                // initial state
      STARTED,                // timer start request has been submitted
      STOPPED                 // timer has been stopped
   };

   virtual void doStartTimer(void);
     //:Start the OsTimer object and modify its object state accordingly

   virtual void doStopTimer(void);
     //:Stop the OsTimer object and modify its object state accordingly

   virtual const OsTime& getPeriod(void) const;
     //:Return the period of a periodic timer
     // If the timer is not periodic, return an infinite period.
   
   virtual int getState(void) const;
     //:Return the state value for this OsTimer object

   virtual int getTimerId(void) const;
     //:Return the system timer id for this object.
     // This method should only be called by the OsSysTimer object.

   virtual int getType(void) const;
     //:Return the timer type.

   virtual UtlBoolean isOneshot(void) const;
     //:Return TRUE if this is a one-shot timer, otherwise FALSE.

   virtual UtlBoolean isPeriodic(void) const;
     //:Return TRUE if this is a periodic timer, otherwise FALSE.

   virtual void setState(int state);
     //:Set the state value for this OsTimer object

   virtual void setTimerId(int id);
     //:Set the system timer id for this object
     // This method should only be called by the OsSysTimer object.

   virtual void setTimerType(int timerType);
     //:Set the timer type for this object.

   OsTimer(const OsTimer& rOsTimer);
     //:Copy constructor (not implemented for this class)

   OsTimer& operator=(const OsTimer& rhs);
     //:Assignment operator (not implemented for this class)

   OsNotification* mpNotifier; // used to signal timer expiration event
   OsTime          mExpiresAt; // when the timer is due to expire (relative to
                               //  when the timer was requested)
   OsTime          mPeriod;    // interval between timer expirations
   int             mState;     // object state (see the OsTimerState enum)
   int             mTimerId;   // system (low-level) timer id
   int             mType;      // timer type

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTimer_h_

