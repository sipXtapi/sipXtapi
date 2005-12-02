/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.service ;

import java.util.* ;
import java.awt.event.* ;

import org.sipfoundry.sipxphone.sys.Shell ;
import org.sipfoundry.sipxphone.sys.util.PingerInfo ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.SysLog;
/**
 * Very basic Timer / Scheduler like singleton class that works much like
 * the swing timers.  It isn't quite as complex... or perhaps more complex
 * depending on how you look at it.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class Timer extends Thread implements Cloneable
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** action command sent whenever a time fires */
    public final static String ACTION_TIMER_FIRED = "action_timer_fired" ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to our singleton timer instance */
    protected static Timer  m_instance = null ;

    /** last fired time stamp */
    protected static long   m_lTimeStamp = 0 ;

    /** our scheduled list */
    protected Vector    m_vSchedule ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    protected Timer()
    {
        super("JTimerSrv") ;
        setPriority(Thread.NORM_PRIORITY+1) ;

        m_vSchedule = new Vector(15) ;
    }


    /**
     * Obtain a reference to our singleton timer instance.
     */
    public static Timer getInstance()
    {
        try {
            if (m_instance == null) {
                m_instance = new Timer() ;
                m_instance.start() ;
            }
        } catch (Exception e) {
            SysLog.log(e) ;
        }

        return m_instance ;
    }


    /**
     * Startup the Timer service...
     */
    public static void startup()
    {
        getInstance() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Set a timer listener
     *
     * @param lDelay time in ms until the timer should fire
     * @param listener The listener that will be notified on fire
     * @param objData Optional data object
     */
    public void addTimer(long lDelay, PActionListener listener, Object objData)
    {
        addTimer(lDelay, listener, objData, false) ;
    }

    /**
     * set a timer listener
     *
     * @param iDelay time in ms until the timer should fire
     * @param listener The listener that will be notified on fire
     * @param objData Optional data object
     * @param bAutoRearm Should the timer automatically rearm after it fires?
     */
    public void addTimer(long lDelay, PActionListener listener, Object objData, boolean bAutoRearm)
    {
        try
        {
// System.out.println("addTimer: " + lDelay + " " + listener + " " + objData) ;
            long lAdjustment = 0 ;

            // We are enforcing a minimum timer granularity of 32 ms.
            if (lDelay < 32)
            {
                lDelay = 32 ;
            }

            // Add however long the timer has been running to our item
            lAdjustment = (PingerInfo.getInstance().getTimeInMS() - m_lTimeStamp) ;

            // Add our event
            icTimedEvent event = new icTimedEvent(lDelay, listener, objData, bAutoRearm) ;
            event.m_lFireTime = m_lTimeStamp + lDelay + lAdjustment ;
            event.m_lTimeLeft = lAdjustment + lDelay ;

            synchronized (m_vSchedule)
            {
                m_vSchedule.addElement(event) ;
            }

            // now reschedule our new entry
            interrupt() ;
        } catch (Exception e) {
            SysLog.log(e) ;
        }
    }



    /**
     * remove all timers for the specified listener
     */
    public void removeTimers(PActionListener listener)
    {
//System.out.println("removeTimers: " + listener)  ;

        synchronized (m_vSchedule)
        {
            Enumeration enumEvents = m_vSchedule.elements() ;
            while (enumEvents.hasMoreElements()) {
                icTimedEvent event = (icTimedEvent) enumEvents.nextElement() ;

                if (event.m_actionListener == listener) {
                    m_vSchedule.removeElement(event) ;
                }
            }
        }
    }


    /**
     * reset timer will remove all existing timers registered to the listener
     * and then add the passed one.
     *
     * @param iDelay time in ms until the timer should fire
     * @param listener The listener that will be notified on fire
     * @param objData Optional data object
     */
    public void resetTimer(long lDelay, PActionListener listener, Object objData)
    {
        resetTimer(lDelay, listener, objData, false) ;
    }


    /**
     * reset timer will remove all existing timers registered to the listener
     * and then add the passed one.
     *
     * @param iDelay time in ms until the timer should fire
     * @param listener The listener that will be notified on fire
     * @param objData Optional data object
     * @param bAutoRearm Should the timer automatically rearm after it fires?
     */
    public void resetTimer(long lDelay, PActionListener listener, Object objData, boolean bAutoRearm)
    {
//System.out.println("resetTimer: " + iDelay + " " + listener + " " + objData) ;

        synchronized (m_vSchedule)
        {
            removeTimers(listener) ;
            addTimer(lDelay, listener, objData, bAutoRearm) ;
        }
    }




    /**
     *
     */
    public void run()
    {
        try {
            long        lNextEventDelay ;
            long        lRealDelay ;
            PingerInfo  pingerInfo = PingerInfo.getInstance() ;

            m_lTimeStamp = pingerInfo.getTimeInMS() ;
            for (;;) {
                // Figure out what should fire
                lNextEventDelay = nextEventToFire() ;

                // Wait, ignoring interruptions
                try {
                    sleep(lNextEventDelay) ;
                } catch (InterruptedException e) {
                    // Ignore interruptions
                    e = null ;
                }

                // figure out how much time passed, fire, and repeat.
                lRealDelay = (int) (pingerInfo.getTimeInMS() - m_lTimeStamp) ;
                m_lTimeStamp = pingerInfo.getTimeInMS() ;
                if (lRealDelay < 0)
                    lRealDelay = 0 ;
                fireTimers(lRealDelay) ;
            }
        } catch (Exception e) {
            SysLog.log(e) ;
            e = null ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * fire our what the next delay is.. given what needs to fire.
     */
    protected long nextEventToFire()
    {
        long lMaxSleep = Integer.MAX_VALUE ;

        synchronized (m_vSchedule)
        {
            Enumeration enumEvents = m_vSchedule.elements() ;
            while (enumEvents.hasMoreElements()) {
                icTimedEvent event = (icTimedEvent) enumEvents.nextElement() ;
                if (event.m_lTimeLeft < lMaxSleep) {
                    lMaxSleep = event.m_lTimeLeft ;
                }
            }
        }

//System.out.println("nextEventToFire: " + iMaxSleep) ;
        return lMaxSleep ;
    }


    /**
     * send timer notifications for any timers which have expired.
     */
    protected void fireTimers(long lTimeExpired)
    {
        Vector vEventsToFire = new Vector() ;

        // Step 1: Update all the bookwork for each scheduled item
        //         and note who is past-due and should be fired NOW.
        synchronized (m_vSchedule)
        {
            Enumeration enumEvents = m_vSchedule.elements() ;
            while (enumEvents.hasMoreElements()) {
                icTimedEvent event = (icTimedEvent) enumEvents.nextElement() ;
                if (event.m_lTimeLeft > lTimeExpired) {
                    event.m_lTimeLeft -= lTimeExpired ;
                } else {
                    vEventsToFire.addElement(event) ;

                    if (event.isRearming()) {
                        event.m_lFireTime += event.m_lDelay ;
                        // Look for a timewarp forward...
                        if (event.m_lFireTime < m_lTimeStamp)
                        {
                            System.out.println("JTimeSrv: Getting behind on REARM or Timewarp: " + (m_lTimeStamp - event.m_lFireTime)  + " ms") ;
                            event.m_lFireTime = m_lTimeStamp + event.m_lDelay ;
                        }

                        // Calculate time until next fire...
                        event.m_lTimeLeft = (int) (event.m_lFireTime - m_lTimeStamp) ;
    //System.out.println("RE-ARM " + event.m_actionListener + " " + event.m_lDelay) ;
                    } else {
                        m_vSchedule.removeElement(event) ;
                    }
                }
            }
        }

        // Step 2: Fire all expired timers
        PActionEvent actionEvent = new PActionEvent(this, ACTION_TIMER_FIRED) ;
        Enumeration enumEvents = vEventsToFire.elements() ;
        while (enumEvents.hasMoreElements())
        {
            icTimedEvent event = (icTimedEvent) enumEvents.nextElement() ;
            try
            {
//System.out.println("firing: " + event.m_actionListener) ;
                actionEvent.setObjectParam(event.m_objData) ;
                event.m_actionListener.actionEvent(actionEvent) ;
            }
            catch (Throwable t)
            {
                Shell.getInstance().showUnhandledException(t, false) ;
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Inner classes
////
    /**
     * icTimedEvent is a storage / data encapsulation class that stores
     * information about a particular scheduled event
     */
    public class icTimedEvent
    {
        long              m_lDelay ;         // Delay until next fire
        long              m_lFireTime ;      // Next firing time (used for rearming)
        long              m_lTimeLeft ;      // time left until firing

        PActionListener  m_actionListener ; // Who we should notify
        Object           m_objData ;
        boolean          m_bAutoRearm ;

        public icTimedEvent(int iDelay, PActionListener listener, Object objData, boolean bAutoRearm)
        {
            m_lDelay = (long) iDelay ;

            if (m_lTimeLeft < 0)
                m_lTimeLeft = 0 ;

            m_actionListener = listener ;
            m_objData = objData ;
            m_bAutoRearm = bAutoRearm ;
        }

        public icTimedEvent(long iDelay, PActionListener listener, Object objData, boolean bAutoRearm)
        {
            m_lDelay = iDelay ;

            if (m_lTimeLeft < 0)
                m_lTimeLeft = 0 ;

            m_actionListener = listener ;
            m_objData = objData ;
            m_bAutoRearm = bAutoRearm ;
        }


        public boolean isRearming()
        {
            return m_bAutoRearm ;
        }
    }
}
