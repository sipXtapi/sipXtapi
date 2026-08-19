//
// Copyright (C) 2026 SIP Spectrum Inc.  All rights reserved.
//
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


    // Includes
#include "rtcp/RTCPTimer.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"
#include "os/OsTimerTask.h"
#ifdef INCLUDE_RTCP /* [ */

/**
 *
 * Method Name:  CRTCPTimer() - Constructor
 *
 *
 * Inputs:   unsigned long ulTimerPeriod - Amount of time before alarming
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
CRTCPTimer::CRTCPTimer(unsigned long ulTimerPeriod)
           : m_pTimeout(NULL), m_pCallback(NULL), m_pTimer(NULL)
{

    // Store the arguments passed in the constructor as internal data members
    SetReportTimer(ulTimerPeriod);

}


/**
 *
 * Method Name: ~CRTCPTimer() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocate and/or release all resources which were
 * acquired over the course of runtime.
 *
 * Usage Notes: This shall override the virtual destructor in the base class
 *   so that deallocation specific to the derived class will be done despite
 *   the destruction being performed in the base class as part of the release.
 *
 *
 */
CRTCPTimer::~CRTCPTimer(void)
{

    // Shutdown the processing thread and deallocate all related resources
    Shutdown();

}

/**
 *
 * Method Name: CRTCPTimer::Initialize
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     Boolean True/False
 *
 * Description: Create a timer thread that shall wakeup periodically and
 *             perform some operation and go back to sleep.
 *
 * Usage Notes: This overrides the virtual method in the base class.
 *
 */
bool CRTCPTimer::Initialize(void)
{

    // Re-initializing is allowed, so drop anything a previous call left behind.
    Shutdown();

    // OsTime takes seconds and microseconds; m_ulTimerPeriod is milliseconds.
    m_pTimeout  = new OsTime(m_ulTimerPeriod / 1000,          // whole seconds
                             (m_ulTimerPeriod % 1000) * 1000); // remainder as usec
    m_pCallback = new OsCallback((intptr_t)this, ReportingAlarm);
    m_pTimer    = new OsTimer(*m_pCallback);

    m_pTimer->periodicEvery(*m_pTimeout, *m_pTimeout);

    return(TRUE);

}


/**
 *
 * Method Name: CRTCPTimer::Shutdown
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
bool CRTCPTimer::Shutdown( void )
{

    if(m_pTimer)
    {
        // A synchronous stop does not return until any ReportingAlarm() call
        // already in progress has finished, which is what makes it safe to
        // free this object afterwards.  It gets that guarantee by blocking on
        // the timer task, so calling it FROM the timer task deadlocks.
        //
        // That is a reachable path, not a theoretical one: ReportingAlarm()
        // runs on the timer task and reaches code that can tear the connection
        // down, which lands here by way of ~CRTCPTimer().  In that case the
        // only callback in flight is the one calling us and it is about to
        // return -- precisely the state a synchronous stop would have waited
        // for -- so an asynchronous stop is equivalent.  OsTimer::deleteAsync()
        // exists for the same reason, the destructor blocking on the timer task
        // just as a synchronous stop does.
        const bool bOnTimerTask =
            (OsTask::getCurrentTask() == (OsTaskBase *)OsTimerTask::getTimerTask());

        if(bOnTimerTask)
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
               "CRTCPTimer::Shutdown(%p): running on the timer task, stopping asynchronously",
               this);
            m_pTimer->stop(FALSE);
            m_pTimer->deleteAsync();
        }
        else
        {
            m_pTimer->stop(TRUE);
            delete m_pTimer;
        }
        m_pTimer = NULL;
    }

    // Safe to free once the timer is stopped: the callback reaches these only
    // by way of the timer, and deleteAsync() has already stopped it.
    delete m_pCallback;
    m_pCallback = NULL;
    delete m_pTimeout;
    m_pTimeout = NULL;

    return (TRUE);
}


/**
 *
 * Method Name: CRTCPTimer::ReportingAlarm
 *
 *
 * Inputs:      const intptr_t userData   - The CRTCPTimer this alarm belongs to
 *              const intptr_t eventData  - Unused
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Called by the timer task each time the report period elapses.
 *
 * Usage Notes: Runs on the shared OsTimerTask thread.  Shutdown() cannot
 *              complete a synchronous stop while this is running, so the object
 *              is guaranteed to still be alive for the duration of the call.
 *
 */
void CRTCPTimer::ReportingAlarm(const intptr_t userData, const intptr_t eventData)
{

    CRTCPTimer *poRTCPTimer = (CRTCPTimer *) userData;

    // One line per report period, so a handful per call.  Nothing else on the
    // report generation path logs, which makes a silently unarmed timer look
    // exactly like a healthy one -- outbound RTCP simply stops and no error is
    // reported anywhere.  The log header carries the task name, so this also
    // shows which thread the reports are being generated on.
    OsSysLog::add(FAC_MP, PRI_DEBUG,
       "CRTCPTimer::ReportingAlarm(%p): report period elapsed", poRTCPTimer);

    poRTCPTimer->RTCPReportingAlarm();

}

#endif /* INCLUDE_RTCP ] */
