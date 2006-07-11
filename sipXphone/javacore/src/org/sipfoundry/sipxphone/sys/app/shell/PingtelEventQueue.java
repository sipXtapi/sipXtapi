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


package org.sipfoundry.sipxphone.sys.app.shell ;

import org.sipfoundry.util.CountingSemaphore ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.util.SysLog;

/**
 * The Pingtel Event queue is responsible for queueing and firing off events
 * to our various GUI components.  It models after the AWT event queue with a
 * few alterations.
 * <br><br>
 */
public class PingtelEventQueue
{

//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static boolean DEBUG_POST_EVENTS     = false ;
    public static boolean DEBUG_QUEUE_LENGTH    = false ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** the one and only pingtel event queue */
    public static icEventQueueItem m_queue = null ;
    public static int m_iQueueLength = 0 ;

    /** singleton reference to ourself */
    public static PingtelEventQueue m_reference = null ;
    private CountingSemaphore m_semContent ;


    /**
     * post a pingtel event into the queue
     */
    public static void postEvent(PingtelEvent event)
    {
        if (DEBUG_POST_EVENTS) {
            System.out.println("++++") ;
            System.out.println("++ <BEGIN> PingtelEventQueue: postEvent **") ;
            System.out.println(event) ;
        }

        getInstance().enqueueEvent(event) ;

        if (DEBUG_POST_EVENTS) {
            System.out.println("++ <END>   PingtelEventQueue: postEvent **") ;
            System.out.println("++++") ;
        }
    }

    /**
     * send an pingtel event into the queue
     */
    public static void sendEvent(PingtelEvent event)
    {
        if (DEBUG_POST_EVENTS) {
            System.out.println("++ <BEGIN> PingtelEventQueue: sendEvent **") ;
        }

        CountingSemaphore semaphore = new CountingSemaphore(0) ;
        event.setSemaphore(semaphore) ;

        postEvent(event) ;

        // We we should wait for a response...
        try {
            if (DEBUG_POST_EVENTS) {
                System.out.println("++ <BEGIN> PingtelEventQueue: sendEvent Wait **") ;
            }
            semaphore.down() ;
            if (DEBUG_POST_EVENTS) {
                System.out.println("++ <END>   PingtelEventQueue: sendEvent Wait**") ;
            }
        } catch (Exception e) {
            SysLog.log(e) ;
        }

        if (DEBUG_POST_EVENTS) {
            System.out.println("++ <END> PingtelEventQueue: sendEvent **") ;
        }
    }


    /**
     * get the next event from our queue
     */
    public static  PingtelEvent nextEvent()
        throws InterruptedException
    {
        return getInstance().getNextEvent() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    private PingtelEventQueue()
    {
        m_semContent = new CountingSemaphore(0, false) ;
    }


    /**
     * get a reference to the Event Queue
     */
    private static PingtelEventQueue getInstance()
    {
        if (m_reference == null) {
            m_reference = new PingtelEventQueue() ;
        }
        return m_reference ;
    }


    /**
     * post a pingtel event into the queue
     */
    public void enqueueEvent(PingtelEvent event)
    {
        m_iQueueLength++ ;
        if (DEBUG_QUEUE_LENGTH) {
            System.out.println("Queue Length: " + m_iQueueLength) ;
        }

        synchronized (this) {
            if (m_queue == null) {
                // create a new queue
                if (DEBUG_POST_EVENTS)
                    System.out.println("enqueueEvent : *********************************** CREATING EVENT QUEUE!");
                m_queue = new icEventQueueItem(event) ;
            } else {
                icEventQueueItem q = m_queue ;

                // traverse to end
                while (q.next != null)
                    q = q.next ;

                q.next = new icEventQueueItem(event) ;
            }
        }
        m_semContent.up() ;
    }


    /**
     * get the next event from our queue.
     */
    public PingtelEvent getNextEvent()
        throws InterruptedException
    {
        PingtelEvent oldEvent ;
        PingtelEvent event = null ;

        // Wait for an event to get posted...
        if (DEBUG_POST_EVENTS)
            System.out.println("%%%%%%%%%%%%% WAITING FOR EVENT TO BE POSTED...");

        m_semContent.down() ;

        if (DEBUG_POST_EVENTS)
            System.out.println("%%%%%%%%%%%%% IT WAS POSTED");
        synchronized (this) {
            oldEvent = event ;
            event = m_queue.event ;
            m_queue = m_queue.next ;
            oldEvent = null ;
            m_iQueueLength-- ;
        }

        return event  ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner classes
////

    /**
     *  A queue item
     */
    private class icEventQueueItem
    {
        PingtelEvent     event ;
        icEventQueueItem next ;

        public icEventQueueItem(PingtelEvent event)
        {
            this.event = event ;
            this.next  =  null ;
        }
    }
}