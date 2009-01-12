//
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsTimer_h_
#define _OsTimer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsgQ.h>
#include <os/OsDateTime.h>
#include <os/OsNotification.h>
#include <os/OsStatus.h>
#include <os/OsTime.h>
#include <utl/UtlContainable.h>
#include <os/OsBSem.h>


// DEFINES

/// Macro to check that 'x' is an OsTimer* by testing its
/// getContainableType value.  This is to catch misuses of the OsTimer
/// methods.
#define CHECK_VALIDITY(x) \
            assert((x)->getContainableType() == OsTimer::TYPE)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * This class implements one-shot and periodic timers.
 *
 * Once a timer is created, it must be started.  After the specified time,
 * the timer expires or "fires", at which point (depending on how the
 * timer was created) an OsNotification object is used to signal an
 * event, or a message is posted to a specified queue.
 *
 * A timer may be stopped at any time (except when the timer is being
 * destroyed).  The destructor calls stop() before freeing the timer.
 *
 * If the stop() is synchronous, it may block, but it ensures that any
 * event routine call will have finished before stop() returns.  If
 * the stop() is asynchronous, it will not block, but an event routine
 * execution that has been previously committed may execute after stop()
 * returns.  (For one-shot timers, this can be detected by examining the
 * return value of stop().)
 *
 * Once a timer is stopped with stop() or by firing (if it is a one-shot
 * timer), it can be started again.  The time interval of a timer can be
 * changed every time it is started, but its notification information is
 * fixed when it is created.
 *
 * All methods can be used concurrently, except that no other method may be
 * called concurrently with the destructor (which cannot be made to work,
 * as the destructor deletes the timer's memory).  Note that a timer may
 * fire while it is being deleted; the destructor handles this situation
 * correctly, the timer is guaranteed to exist until after the event
 * routine returns.
 *
 * An event routine should be non-blocking, because it is called on
 * the timer task thread.  Within an event routine, all non-blocking
 * methods may be executed on the timer.  When the event routine of a
 * one-shot timer is entered, the timer is in the stopped state.  When
 * the event routine of a periodic timer is entered, the timer is
 * still in the running state.
 *
 * (If mbManagedNotifier is set, the timer may not be destroyed (using
 * deleteAsync, which is non-blocking), as that destroys the
 * OsNotifier object whose method is the event notifier that is
 * currently running.  But there is no current interface for creating
 * that situation.)
 *
 * Most methods are non-blocking, except to seize the timer's mutex
 * and to post messages to the timer task's message queue.  The
 * exceptions are the destructor and synchronous stops, which must
 * block until they get a response from the timer task.
 *
 * If VALGRIND_TIMER_ERROR is defined, additional code is created to
 * detect and backtrace errors in timer usage.  This code causes run-time
 * errors that Valgrind can detect to produce backtraces of where the
 * invalid method invocations were made.
 *
 * If NDEBUG is defined, some checking code that is used only to trigger
 * asserts is omitted.
 *
 * @nosubgrouping
 */
class OsTimer : public UtlContainable
{
   friend class OsTimerTask;
   friend class OsTimerTest;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// The states a timer can be in.
   enum OsTimerState
   {
         STOPPED,                ///< Timer has not been started, or has fired
                                 ///< or been stopped.
         STARTED                 ///< Timer is running and will fire.
   };

   static const UtlContainableType TYPE;  /**< Class type used for runtime checking */

/* ============================ CREATORS ================================== */

   /** @name Constructors
    *
    * Constructors specify how fired timers will signal the application.
    * The event specification does not change over the lifetime of the
    * timer.  The timer period information is specified by the start
    * method, and can be different for different starts.
    *
    * @{
    */

   /** Construct a timer that signals by calling
    *  @code
    *  rNotifier.signal((int) this)
    *  @endcode
    */
   OsTimer(OsNotification& rNotifier ///< OsNotification object to report event
      );

   /** Construct a timer that signals by calling
    *  @code
    *  pQueue->doSendEventMsg(OsEventMsg::NOTIFY, (int) this)
    *  @endcode
    */
   OsTimer(OsMsgQ* pQueue,      ///< Queue to send OsEventMsg::NOTIFY message
           intptr_t userData    ///< userData value to store in OsQueuedEvent
      );

   /// @}

   /// Destructor
   virtual ~OsTimer();

   /// Non-blocking asynchronous delete operation
   virtual void deleteAsync();
   /**<
    * Stops the timer, then sends a message to the timer task, which will
    * eventually delete it.  Provides a non-blocking way to delete an
    * OsTimer.
    */

/* ============================ MANIPULATORS ============================== */

   /** @name Start methods
    *
    * These methods start the timer.  They may be called when the timer is
    * in any state, but if the timer is already started, they have no
    * effect. They return a value that reflects that state:  OS_SUCCESS if
    * the start operation was successful and OS_FAILED if it failed
    * (because the timer was already started).
    *
    * @{
    */

   /// Start the timer to fire once at the indicated date/time
   virtual OsStatus oneshotAt(const OsDateTime& when);

   /// Start the timer to fire once at the current time + offset
   virtual OsStatus oneshotAfter(const OsTime& offset);

   /// Start the timer to fire periodically starting at the indicated date/time
   virtual OsStatus periodicAt(const OsDateTime& when, const OsTime &period);

   /// Start the timer to fire periodically starting at current time + offset
   virtual OsStatus periodicEvery(const OsTime &offset, const OsTime &period);

   /// @}

   /// Stop the timer if it has been started
   virtual OsStatus stop(UtlBoolean synchronous = TRUE);
   /**<
    * stop() can be called when the timer is in any state, and returns a
    * value that reflects that state:
    *
    * @returns OS_SUCCESS if the timer was started and OS_FAILED if the
    * timer was not started, was already stopped, or is a one-shot
    * timer and has fired.
    *
    * Thus, if it is a one-shot timer, and there are one or more calls to
    * stop(), if the event has been signaled, all calls will return
    * OS_FAILED.  But if the event has not been signaled, exactly one call
    * will return OS_SUCCESS.  This allows the caller of stop() to know
    * whether to clean up or not.
    *
    * If synchronous is TRUE, the call will block if necessary to
    * ensure that any event routine execution for this timer will
    * finish before stop() returns.  If synchronous is FALSE, the call
    * will not block, but a previously committed event routine
    * execution may happen after stop() returns.
    */

/* ============================ ACCESSORS ================================= */

   /// Return the OsNotification object for this timer
   virtual OsNotification* getNotifier(void) const;
   /**<
    * If the timer was constructed with OsTimer(OsMsgQ*, const int),
    * it returns the address of an internally allocated OsNotification.
     */

   /// Calculate a unique hash code for this object.
   virtual unsigned hash() const;
   /**<
    * If the equals operator returns true for another object, then both of
    * those objects must return the same hashcode.
     */

   /// Get the ContainableType for a UtlContainable derived class.
   virtual UtlContainableType getContainableType() const;

   /// Returns TRUE if timer was fired
   UtlBoolean getWasFired();
   /**<
    * This flag is set to FALSE when timer is first started, and also
    * if stop operation succeeds. It is set to TRUE when timer is fired.
    * Note that stop fails on a stopped timer.
    */

/* ============================ INQUIRY =================================== */

   /// Compare the this object to another like-objects.
   virtual int compareTo(UtlContainable const *) const;
   /**<
    * Results for comparing with a non-like object are undefined.
    *
    * @returns 0 if equal, < 0 if less-than and > 0 if greater-than.
    */

   /// Return the state value for this OsTimer object
   virtual OsTimerState getState();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsBSem          mBSem;      ///< Semaphore to lock access to members.

   unsigned int    mApplicationState;
   UtlBoolean      mWasFired;  ///< TRUE if timer is stopped because it was fired
                               ///< state as seen by application methods.
   unsigned int    mTaskState; ///< State as seen by the timer task.
   UtlBoolean      mDeleting;  ///< TRUE if timer is being deleted.

   OsNotification* mpNotifier; ///< Used to signal timer expiration event.
   UtlBoolean      mbManagedNotifier; ///< TRUE if OsTimer destructor should
                               ///< delete *mpNotifier.

   OsTime          mExpiresAt; ///< Expire time of timer.
   UtlBoolean      mPeriodic;  ///< TRUE if timer fires repetitively.
   OsTime          mPeriod;    ///< Repetition time.

   // Copies of time values for use by timer task.
   OsTime          mQueuedExpiresAt; ///< Expire time of timer (copy).
   UtlBoolean      mQueuedPeriodic; ///< TRUE if timer fires repetitively (copy).
   OsTime          mQueuedPeriod; ///< Repetition time (copy).

   int             mOutstandingMessages; ///< Number of messages for this timer
                               ///< in the timer task's queue.

   OsTimer*        mTimerQueueLink; ///< To chain together timers.

   /// Start a timer.
   OsStatus startTimer(OsTime start,
                       UtlBoolean periodic,
                       OsTime period);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   /// Copy constructor (not implemented for this class)
   OsTimer(const OsTimer& rOsTimer);

   /// Assignment operator (not implemented for this class)
   OsTimer& operator=(const OsTimer& rhs);

/* ============================ INLINE METHODS ============================ */
  protected:

   /// Test whether a status indicates the timer has been started.
   inline static UtlBoolean isStarted(int status)
   {
      return (status & 1) == 1;
   }

   /// Test whether a status indicates the timer has been stopped.
   inline static UtlBoolean isStopped(int status)
   {
      return (status & 1) == 0;
   }

};

#endif  // _OsTimer_h_
