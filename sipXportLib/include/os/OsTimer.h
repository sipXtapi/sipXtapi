//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsTimer_h_
#define _OsTimer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include "os/OsDateTime.h"
#include "os/OsNotification.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"
#include "utl/UtlContainable.h"


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

class OsTimer : public UtlContainable
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

/* ============================ CREATORS ================================== */

    //:Constructor
    // Timer expiration event notification happens using a newly created
    // OsQueuedEvent object.
    // The address of "this" OsTimer object is the eventData that is
    // conveyed to the Listener when the notification is signaled.
   OsTimer(OsMsgQ* pQueue, const int userData);

    //:Constructor
    // *Deprecated*
    // Timer expiration event notification happens using the OsNotification
    // object. The address of "this" OsTimer object is the eventData that is
    // conveyed to the Listener when the notification is signaled.
    // *Deprecated*
   OsTimer(OsNotification& rNotifier);

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

    /**
     * Calculate a unique hash code for this object.  If the equals
     * operator returns true for another object, then both of those
     * objects must return the same hashcode.
     */
    virtual unsigned hash() const ;

    /**
     * Get the ContainableType for a UtlContainable derived class.
     */
    virtual UtlContainableType getContainableType() const;

    virtual const int getUserData() { return mUserData; }
    
/* ============================ INQUIRY =================================== */

    /**
     * Compare the this object to another like-objects.  Results for 
     * designating a non-like object are undefined.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    virtual int compareTo(UtlContainable const *) const ;

    virtual int getState(void) const;
     //:Return the state value for this OsTimer object


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static UtlContainableType TYPE ;   /** < Class type used for runtime checking */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

 enum OsTimerType
   {
      UNSPECIFIED = 0x0,      // not initialized
      ONESHOT     = 0x1,      // expire at the specified offset
      PERIODIC    = 0x2,      // expire every period
   };

   virtual void doStartTimer(void);
     //:Start the OsTimer object and modify its object state accordingly

   virtual void doStopTimer(void);
     //:Stop the OsTimer object and modify its object state accordingly

   virtual const OsTime& getPeriod(void) const;
     //:Return the period of a periodic timer
     // If the timer is not periodic, return an infinite period.
   
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
   bool            mbManagedNotifier;
   const int        mUserData;  // user data supplied in the constructor 
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTimer_h_

