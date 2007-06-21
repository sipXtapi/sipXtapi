//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: 

#ifndef _MpMMTimer_h_
#define _MpMMTimer_h_

// SYSTEM INCLUDES
#include <os/OsStatus.h>
#include <os/OsTime.h>
#include <os/OsCallback.h>

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

#define  MPMMTIMER_ALGORITHM_DEFAULT  0
class MpMMTimer
{
public:
   static OsStatus getResolution(unsigned& resolution);
     /**<
      *  @brief Get resolution of timer in microseconds
      *
      *  @param[out] resolution - set to the finest resolution timer we can generate.  
      *  @retval OS_SUCCESS - the resolution was able to be retrieved.
      *  @retval OS_FAILED - the resolution wasn't able to be retrieved.
      */

   static OsStatus getPeriodRange(unsigned* pMinUSecs, 
                                  unsigned* pMaxUSecs = NULL);
     /**<
      *  @brief Get the range of timer periods that can be generated.
      *
      *  @param[out] pMinUSecs - set to the smallest period we can generate.  
      *  @param[out] pMaxUSecs - set to the largest period we can generate.  
      *  @retval OS_SUCCESS - the min and max periods were able to be retrieved.
      *  @retval OS_FAILED - the min and max periods weren't able to be retrieved.
      */

public:
   typedef enum 
   {
      Linear = 0,         ///< For use with lockForNextTick()
      Os_Callback = 1,     ///< For use with setOsCallback()
      On_Fire = 2          ///< For use with onFire()
   } MMTimerType;

   virtual
   OsStatus setCallback(OsCallback* callbackFn);
     /**<
      * @brief Set callback function for the OsCallback timer type.
      * 
      * Signals will sent with pointer to this timer instance in \p eventData member.
      * @param[in] callbackFn Callback function will be use for signaling.
      * @retval OS_SUCCESS Returns when callback was set.
      * @retval OS_INVALID_STATE Returns when calling this function for non-OsCallback timer type.
      * @retval OS_FAILED Returns when mandatories initializations failed (maybe in constructor) and
      *                   later calls will fails too for this instance.
      */

   virtual 
   OsStatus run(unsigned usecPeriodic, 
                unsigned uAlgorithm = MPMMTIMER_ALGORITHM_DEFAULT) = 0;
     /**<
      * @brief Start periodical firing.
      * 
      * Start periodical firing with given period and fire type.
      * @param[in] usecPeriodic timer period in microseconds.
      * @param[in] uAlgorithm timer algorithm. Depending OS exists
      *                       many ways to organize the file. Using
      *                       special type could give better performance 
      *                       in special application. Value of MPMMTIMER_TYPE_DEFAULT 
      *                       is a default timer type is suitable for most applications.
      *                       All inherited types MUST support this default type.
      *                       For example, windows has multimedia timers, 
      *                       waitable timers, and queue timers.  varying values
      *                       of \p uAlgorithm would (if implemented) be used to 
      *                       specify which of these timer types to use.
      *                      
      * @retval OS_SUCCESS Returns when timer has been created and ran.
      * @retval OS_INVALID_ARGUMENT Returns when either value of \p uAlgorithm 
      *                             dosen't support in current realization or 
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
   void onFire(UtlBoolean bFirstFire) = 0;
     /**< 
      * Callback Function that should be implemented in the derived class
      * 
      * @param[in] bFirstFire Set to TRUE for first signal after stopping state.
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

     /// @brief Get the type of timer fire.
   inline MMTimerType getTimerType() const;

     /// @brief Get the number of microseconds elapsed since the timer last fired.
   int getUSecSinceLastFire() const;

     /// @brief Get the number of microseconds until the timer is next expected to fire.
   int getUSecDeltaExpectedFire() const;

   OsTime getAbsFireTime() const;
   
protected:
     /// @brief protected constructor, as this is an abstract class.
   inline MpMMTimer(MMTimerType type);

     /// @brief protected destructor, as this is an abstract class.
   virtual inline ~MpMMTimer();

protected:
   MMTimerType mTimerType;
};



// Inline Function Implementation

MpMMTimer::MpMMTimer(MMTimerType type)
   : mTimerType(type)
{}

MpMMTimer::~MpMMTimer()
{}

MpMMTimer::MMTimerType MpMMTimer::getTimerType() const
{
   return mTimerType;
}

#endif //_MpMMTimer_h_
