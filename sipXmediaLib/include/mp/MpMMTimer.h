//  
// Copyright (C) 2007-2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpMMTimer_h_
#define _MpMMTimer_h_

// SYSTEM INCLUDES
#include <os/OsStatus.h>
#include <os/OsTime.h>
#include <os/OsNotification.h>
#include <utl/UtlString.h>

// APPLICATION INCLUDES
#include "mp/MpTypes.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief High-precision periodic timer (MultiMedia timer) base class.
*
*  This class is not supposed to be a generic purpose timer. You MUST read
*  documentation for specific implementation of the timer and clearly understand
*  what you're doing before using this. Media timers are very sensitive to
*  incorrect usage and may cause deadlocks and crashes easily when used
*  incorrectly. Main purpose of these timers is to provide ticks for MpMediaTask
*  when there are no enabled sound card.
*
*  Use create() static method to get an instantiate of a timer.
*/
class MpMMTimer
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:
   typedef enum 
   {
      Linear = 0,         ///< For use with waitForNextTick()
      Notification = 1    ///< For use with setNotification()
   } MMTimerType;

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Factory method to create timer class instance of the given type.
   static MpMMTimer *create(MMTimerType type, const UtlString &name = "");
     /**<
     *  @param[in] type = Do we want linear timer or notifications timer?
     *  @param[in] name - type of a timer to create. Empty string means default
     *             timer for this platform. To date we support "Windows Multimedia"
     *             timers on Windows and "Posix" timers on Linux.
     */

     /// Destructor
   virtual ~MpMMTimer();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

   virtual
   OsStatus setNotification(OsNotification* notification);
     /**<
      * @brief Set notification for the OsNotification timer type.
      * 
      * Signals will be sent to this notification when the timer ticks.
      * Pointer to this timer instance will be set in \p eventData member.
      * @param[in] notification - object to notify when timer fires/ticks.
      * @retval OS_SUCCESS Returned when notification is set.
      * @retval OS_INVALID_STATE Returned when calling this function for 
      *                          non-Notification timer type.
      * @retval OS_FAILED Returned when mandatory initialization failed 
      *                   (maybe in the constructor) and later calls will fail
      *                   for this instance also.
      */

   virtual 
   OsStatus run(unsigned usecPeriodic) = 0;
     /**<
      * @brief Start periodical firing.
      * 
      * Start periodical firing with given period and fire type.
      * @param[in] usecPeriodic timer period in microseconds.
      *                      
      * @retval OS_SUCCESS Returns when timer has been created and ran.
      * @retval OS_INVALID_ARGUMENT Returns when either value of \p uAlgorithm 
      *                             doesn't support in current realization or 
      *                             running OS couldn't create timer with
      *                             specified \p usecPeriodic, but with another 
      *                             value of it, it could.
      * @retval OS_INVALID_STATE Returns when timer has already been started or
      *                          if the timer type is not supported.
      * @retval OS_LIMIT_REACHED Returns when timer couldn't run due the resource 
      *                          limitation.
      * @retval OS_FAILED Returns when mandatories initializations failed 
      *                   (maybe in constructor) and later calls will also fail 
      *                   for this instance.
      */

   virtual 
   OsStatus stop() = 0;
     /**<
      * @brief Stop periodical firing. 
      *
      * Stop periodical timer.
      *
      * @retval OS_SUCCESS Returns when timer has been stopped.
      * @retval OS_FAILED Returns when mandatory initialization failed 
      *                   (maybe in constructor) and later calls for this 
      *                   instance will fail.
      * @retval OS_INVALID_STATE Returns when timer has not been started.
      */


   virtual
   OsStatus waitForNextTick();
     /**< 
      * Block this thread until the next tick occurs
      * 
      * @retval OS_SUCCESS Returns when next tick period occurs.
      * @retval OS_INVALID_STATE Returns when calling this function for 
      *                          non-Linear timer type.
      * @retval OS_FAILED Returns when mandatory initialization failed 
      *                   (maybe in constructor) and later calls for this 
      *                   instance will fail too.
      */

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

   virtual
   OsStatus getResolution(unsigned& resolution);
     /**<
      *  @brief Get resolution of timer in microseconds
      *
      *  @param[out] resolution - set to the finest resolution timer we can generate.  
      *  @retval OS_SUCCESS - the resolution was able to be retrieved.
      *  @retval OS_FAILED - the resolution wasn't able to be retrieved.
      */

   virtual
   OsStatus getPeriodRange(unsigned* pMinUSecs, unsigned* pMaxUSecs = NULL);
     /**<
      *  @brief Get the range of timer periods that can be generated.
      *
      *  @param[out] pMinUSecs - set to the smallest period we can generate.  
      *  @param[out] pMaxUSecs - set to the largest period we can generate.  
      *  @retval OS_SUCCESS - the min and max periods were able to be retrieved.
      *  @retval OS_FAILED - the min and max periods weren't able to be retrieved.
      */

     /// @brief Get the type of timer fire.
   inline MMTimerType getTimerType() const;

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:
   MMTimerType mTimerType;

     /// @brief protected constructor, as this is an abstract class.
   inline MpMMTimer(MMTimerType type);

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

MpMMTimer::MpMMTimer(MMTimerType type)
: mTimerType(type)
{
}

MpMMTimer::MMTimerType MpMMTimer::getTimerType() const
{
   return mTimerType;
}

#endif //_MpMMTimer_h_
