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


package org.sipfoundry.sipxphone.sys ;
import org.sipfoundry.util.CountingSemaphore ;

/**
 * Provides a generic and simple event queue.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class GenericEventQueue
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Event queue where Applications send/post events */
    private   icEventQueueItem m_queue = null ;
    /** object used to synchronize post/sends */
    private CountingSemaphore m_semContent ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs a new Event Queue
     */
    public GenericEventQueue()
    {
        m_semContent = new CountingSemaphore(0, false) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Determine if this event queue is empty (no events have been posted)
     *
     * @return true if the event queue is empty otherwise false
     */
    public boolean isEmpty()
    {
        synchronized (this) {
            return (m_queue == null) ;
        }
    }



    /**
     * Get the next event queued for processing.  If not events are present,
     * the invocators thread context is blocked until an event is posted.
     *
     * @return The next event queued for processing
     */
    public Object getNextEvent()
        throws InterruptedException
    {
        Object oldEvent = null ;
        Object event = null ;

        // Wait for an event to get posted...
        m_semContent.down() ;
        synchronized (this) {
            oldEvent = event ;
            event = m_queue.event ;
            m_queue = m_queue.next ;
            oldEvent = null ;
        }
        return event  ;
    }


    /**
     * Post a event into this queue.
     *
     * @param event the new event which should be processed
     */
    public void postEvent(Object event)
    {
        synchronized (this) {
            if (m_queue == null) {
                // create a new queue
                m_queue = new icEventQueueItem(event) ;
            } else {
                icEventQueueItem q = m_queue ;

                // traverse to end
                while (q.next != null)
                    q = q.next ;

                // add
                q.next = new icEventQueueItem(event) ;
            }
        }
        m_semContent.up() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////
    /**
     * The icEventQueueItem provide a simple linked list style data structure.
     */
    private class icEventQueueItem
    {
        Object event ;
        icEventQueueItem next ;
        Thread thread ;

        public icEventQueueItem(Object event)
        {
            this.event = event ;
            this.next  =  null ;
        }
    }
}
