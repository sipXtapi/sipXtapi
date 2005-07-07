/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/telephony/AbstractStateChangePoller.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.telephony ;

import org.sipfoundry.util.CountingSemaphore ;
import org.sipfoundry.util.SysLog;
/**
 * A template for a state change poller that will poll until a state change
 * is noted by the poll method, timeout is reached, or is aborted.  A poller
 * can be aborted by prematurely calling the abort() method.  Afterwards, the
 * wasSuccesful() method will return false.
 *
 * @author Robert J. Andreasen, Jr.
 */
public abstract class AbstractStateChangePoller extends Thread
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    protected final int SLEEP_INTERVAL_LENGTH = 500 ;   // in ms
    //this is now calculated dividing the timeout( JNI_getSipStateTransactionTimeout )
    //by  SLEEP_INTERVAL_LENGTH
    //protected final int TIMEOUT_TRIES = 41 ;           // times/attempts

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    CountingSemaphore m_semaphore ;   // Semaphore blocking the requester
    boolean           m_bSuccessful ; // Did the operation ultimately succeed?
    boolean           m_bCompleted ;  // Have we completed?
    boolean           m_bAborted ;    // Were we aborted?
    Thread            m_context ;     // Thread context of the polling thread

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public AbstractStateChangePoller()
    {
        super("JStatePoller") ;
        m_semaphore = new CountingSemaphore(0, true) ;
        m_bSuccessful = false ;
        m_bCompleted = false ;
        m_bAborted = false ;
        m_context = null ;
    }

//////////////////////////////////////////////////////////////////////////////
// Abstract Methods
////
    /**
     * Poll for the desired state and return true if the state has been met
     * otherwise false
     */
    public abstract boolean poll() ;


    /**
     * Perform and desired cleanup.
     */
    public abstract void cleanup() ;

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Was the state change ultimately successful?  This is not valid until
     * after the poller has completed and the original calling thread has
     * been resumed.
     */
    public boolean wasSuccessful()
    {
        return m_bSuccessful ;
    }


    /**
     * Was the polling aborted?
     */
    public boolean wasAborted()
    {
        return m_bAborted ;
    }


    /**
     * Get the sleep interval between polling attempts in ms.
     */
    public int getSleepInterval()
    {
        return SLEEP_INTERVAL_LENGTH ;
    }


    /**
     * Get the number of polling attempts before the poller gives up.
     */
    public int getTimeoutTries()
    {
        //return TIMEOUT_TRIES ;
        int iTimeout = JNI_getSipStateTransactionTimeout();
        return (iTimeout/getSleepInterval());
    }


    /**
     * Begins the active operation of polling.  This method block until a
     * either state changes, operation is aborted, or the timeout period is
     * exhausted.
     */
    public void beginPolling()
    {
        start() ;
        m_semaphore.down() ;
    }


    /**
     * Exposed as a side effect of the Thread implementation.  Here, we manage
     * the polling and timeouts.
     */
    public void run()
    {
        m_context  = Thread.currentThread() ;

        boolean bSuccessful = false ;

        int iCount = getTimeoutTries() ;
        //System.out.println("------------------------------timoeut tries is---"+ iCount);
        bSuccessful = poll() ;
        while ((iCount >= 0) && !bSuccessful && !m_bCompleted)
        {
            try
            {
                Thread.currentThread().sleep(getSleepInterval()) ;
            }
            catch (InterruptedException ie)
            {
                break ;
            }
            iCount-- ;
            bSuccessful = poll() ;
        }

        m_context = null ;

        if (!m_bCompleted)
            markCompleted(bSuccessful) ;
    }


    /**
     * Aborts the polling attempt if not already completed.
     */
    public void abort()
    {
        if (!m_bCompleted)
        {
            Thread pollingContext = m_context ;
            markCompleted(false) ;
            if (pollingContext != null)
            {
                try
                {
                    pollingContext.join() ;
                }
                catch (InterruptedException ie)
                {
                    SysLog.log(ie);
                }
            }
        }
    }



//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Mark the operation as completed and with a final success flag.  This
     * can be called from another thread context, where it will interrupt the
     * poller and force the thread to exit.
     */
    protected synchronized void markCompleted(boolean bSuccessful)
    {
        if (!m_bCompleted)
        {
            m_bSuccessful = bSuccessful ;
            m_bCompleted = true ;

            if (m_context != null)
            {
                m_bAborted = true ;
                m_context.interrupt() ;
                m_context = null ;
            }
        }
        m_semaphore.up() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native int JNI_getSipStateTransactionTimeout() ;

}
