//
// Copyright (C) 2026 SIP Spectrum Inc.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


//  Border Guard
#ifndef _RTCPTimer_h
#define _RTCPTimer_h

#include "rtcp/RtcpConfig.h"

//  Includes
#ifndef WIN32
#include <time.h>
#endif

#include "IRTCPConnection.h"
#include "IRTCPSession.h"

// OsTimer carries no platform conditionals of its own and is built on every
// target, which is what lets this class have a single timing implementation.
#include "os/OsTime.h"
#include "os/OsCallback.h"
#include "os/OsTimer.h"


//  Defines
#define MILLI2SECS      1000
#define MILLI2NANO      1000000

/**
 *
 * Class Name:  CRTCPTimer
 *
 * Inheritance: CBaseClass       - Generic Base Class
 *
 *
 * Interfaces:  IRTCPTimer       - RTP Header Interface
 *
 * Description: The CRTCPTimer is a class that implements a renewable timer
 *              used to signal the periodic generation of RTCP Reports.
 *
 * Notes:
 *
 */
class CRTCPTimer
{

//  Public Methods
public:

/**
 *
 * Method Name:  CRTCPTimer() - Constructor
 *
 *
 * Inputs:   unsigned long ulTimerPeriod - Amount of time to run before alarming
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine CRTCPTimer object initialization.
 *
 * Usage Notes:  The argument passed in the constructor list shall be
 *               stored as an attribute and used establish the alarm period.
 *
 *
 */

    CRTCPTimer(unsigned long ulTimerPeriod);


/**
 *
 * Method Name: ~CRTCPTimer() - Destructor
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Shall deallocate and/or release all resources which were
 *              acquired over the course of runtime.
 *
 * Usage Notes: This shall override the virtual destructor in the base class so
 *              that deallocation specific to the derived class will be done
 *              despite the destruction being performed in the base class as
 *              part of the release.
 *
 */
    virtual ~CRTCPTimer(void);

/**
 *
 * Method Name: CRTCPTimer::Initialize
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  Boolean True/False
 *
 * Description: Create a timer thread that shall wakeup periodically and
 *              perform some operation and go back to sleep.
 *
 * Usage Notes: This shall override the virtual destructor in the base class so
 *              that deallocation specific to the derived class will be done
 *              despite the destruction being performed in the base class as
 *              part of the release.
 *
 */
    bool Initialize(void);

/**
 *
 * Method Name: SetReportTimer
 *
 *
 * Inputs:   unsigned long ulTimerPeriod
 *                          - Number of milliseconds to elapse before alarming
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SetReportTimer method sets a timer to expire at a
 *              specified period.
 *
 *
 * Usage Notes:
 *
 *
 */
    virtual void SetReportTimer(unsigned long ulTimerPeriod);

/**
 *
 * Method Name: GetReportTimer
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of milliseconds to elapse before alarming
 *
 * Description: The GetReportTimer method returns the value of the report timer.
 *
 * Usage Notes:
 *
 */
    virtual unsigned long GetReportTimer(void);

/**
 *
 * Method Name: RTCPReportingAlarm
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                                  - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                                  - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The RTCPReportingAlarm() event method shall inform the
 *              recipient of the expiration of the RTCP Reporting Timer.  This
 *              signals the generation of the next round of RTCP Reports.
 *
 * Usage Notes: This method shall be overloaded by the superior class.
 *
 *
 */
    virtual void RTCPReportingAlarm(IRTCPConnection *piRTCPConnection=NULL,
                                    IRTCPSession *piRTCPSession=NULL) {};


protected:  // Protected Methods

/**
 *
 * Method Name: Shutdown
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     Boolean True/False
 *
 * Description: Perform RTCPTimer shutdown operations in preparation for
 *              terminating the network interface.
 *
 * Usage Notes:
 *
 *
 */
    bool Shutdown(void);

private:    // Private Methods

/**
 *
 * Method Name: ReportingAlarm
 *
 *
 * Inputs:      const intptr_t userData   - The CRTCPTimer this alarm belongs to
 *              const intptr_t eventData  - Unused
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Static method handed to OsCallback, called by the timer task
 *              each time the report period elapses.  Recovers the object from
 *              userData and forwards to the virtual RTCPReportingAlarm().
 *
 * Usage Notes: Runs on the shared OsTimerTask thread, not a thread of this
 *              object's own.  Shutdown() performs a synchronous OsTimer::stop(),
 *              which does not return until any call in progress here has
 *              finished, so this cannot still be running once teardown has got
 *              past that point.
 *
 */
    static void ReportingAlarm(const intptr_t userData, const intptr_t eventData);

private:    // Private Data Members

/**
 *
 * Attribute Name:  m_ulTimerPeriod
 *
 * Type:            unsigned long
 *
 * Description:     This member shall store the RTCP Report Period value
 *              used to set the timer.
 *
 */
    unsigned long    m_ulTimerPeriod;

/**
 *
 * Attribute Names: m_pTimeout / m_pCallback / m_pTimer
 *
 * Type:            OsTime * / OsCallback * / OsTimer *
 *
 * Description:     The periodic timer, the callback it fires, and the period it
 *                  fires at.  Allocated by Initialize() and torn down by
 *                  Shutdown().
 *
 */
    OsTime         * m_pTimeout;
    OsCallback     * m_pCallback;
    OsTimer        * m_pTimer;

};

/**
 *
 * Method Name: SetReportTimer
 *
 *
 * Inputs:  unsigned long ulTimerPeriod  - Number of mSec between alarms
 *
 * Outputs: None
 *
 * Returns: None

 *
 * Description: The SetReportTimer method sets a timer to expire at a
 *              specified period.
 *
 * Usage Notes:
 *
 */
inline void CRTCPTimer::SetReportTimer(unsigned long ulTimerPeriod)
{

    m_ulTimerPeriod = ulTimerPeriod;

}

/**
 *
 * Method Name: GetReportTimer
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of milliseconds to elapse before alarming
 *
 * Description: Returns the value of the report timer.
 *
 * Usage Notes:
 *
 */
inline unsigned long CRTCPTimer::GetReportTimer(void)
{

    return(m_ulTimerPeriod);

}

#endif
