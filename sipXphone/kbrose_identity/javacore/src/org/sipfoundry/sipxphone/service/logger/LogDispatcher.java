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

package org.sipfoundry.sipxphone.service.logger ;

import org.sipfoundry.sipxphone.service.* ;
import java.io.* ;
import java.net.* ;
import java.util.* ;
import org.sipfoundry.util.CountingSemaphore ;


/**
 * This is a log dispatcher thread that sits around and waits for log
 * messages.  Whenever a log message is posted- it queues it and then
 * processes/sends it whenever it is convenient.
 */
public class LogDispatcher extends Thread
{
//////////////////////////////////////////////////////////////////////////////
// Attribute
////
    /** the queue of outgoing log messages */
    protected icMessageQueueItem m_queue ;
    /** reference to it's own thread */
    protected Thread m_dispather = null ;
    /** connected clients / people listening to the log */
    protected Vector m_vLogClients = null ;
    /** are we started and initialized? */
    protected boolean m_bStarted ;
    /** guard for the queue data structures */
    protected Object m_objQueueGuard ;
    /** Synchronization mechanism for */
    protected CountingSemaphore m_semQueue ;


//////////////////////////////////////////////////////////////////////////////
// Constrution
////
    public LogDispatcher()
    {
        super("JLogDispatch") ;

        setPriority(Thread.MIN_PRIORITY) ;

        m_vLogClients = new Vector() ;
        m_semQueue = new CountingSemaphore(0, false) ;
        m_objQueueGuard = new Object() ;

        m_bStarted  = false ;
        m_queue = null ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     *
     */
    public void postEvent(String strCategory, int iType, String strDescription, Object objParam1, Object objParam2, Object objParam3, Object objParam4)
    {
        LogMessage msg = new LogMessage(strCategory,  iType, strDescription, objParam1, objParam2, objParam3, objParam4) ;

        synchronized (m_objQueueGuard)
        {
            icMessageQueueItem q = m_queue ;

            if (q == null)
            {
                m_queue = new icMessageQueueItem(msg) ;
            }
            else
            {
                // traverse to end of queue
                while (q.next != null)
                    q = q.next ;

                // Add the message
                q.next = new icMessageQueueItem(msg) ;
            }
        }
        m_semQueue.up() ;
    }


    /**
     *
     */
    public boolean isStarted()
    {
        return m_bStarted ;
    }


    /**
     * get the next message from the queue.  It will return null if
     * no messages are available
     */
    public LogMessage nextMessage()
    {
        LogMessage msgRC = null ;

        m_semQueue.down() ;

        synchronized (m_objQueueGuard)
        {
            if (m_queue != null)
            {
                msgRC = m_queue.msg ;
                m_queue = m_queue.next ;
            }
            else
            {
                System.out.println("PANIC: semQueue semaphore doesn't match queue state") ;
            }
        }

        return msgRC ;
    }


    /**
     * Adds a client to the logger.  There is currenly no way to remove
     * a client.  This class will detect a disconnection / problem and
     * simply (rudely?) drop them!
     */
    public void addClient(LogClient client)
    {
        m_vLogClients.addElement(client) ;
    }


    /**
     * Get a list of logger clients
     */
    public Vector getClientList()
    {
        return m_vLogClients ;
    }


    /**
     * Thread processing loop.  It sits around and basically waits for log
     * additions.
     */
    public void run()
    {
        LogMessage msg ;

        m_dispather = Thread.currentThread() ;
        m_bStarted = true ;

        while (true)
        {
            msg = nextMessage() ;
            if (msg != null)
            {
                if (Logger.getInstance().getDebugFlag())
                {
                    System.out.println("JLogger: posting: " + msg.strCategory + ": " + msg.strDescription) ;
                }

                // otherwise tell everyone about it
                synchronized (this)
                {
                    for (int i=0; i<m_vLogClients.size(); i++)
                    {
                        LogClient client = (LogClient) m_vLogClients.elementAt(i) ;

                        // Send the message to the client (and kill the
                        // client if it acts up!)
                        if (client.sendMessage(msg) == false)
                        {
                            System.out.println("JLogger: Removing client: " + client.toString()) ;
                            client.close() ;
                            m_vLogClients.removeElementAt(i) ;
                            Logger.refreshSubscriptionList() ;
                            i-- ;
                        }
                    }
                }
            }
        }
    }


    /**
     * Very simple data encapsulation object...
     */
    private class icMessageQueueItem
    {
        public LogMessage        msg ;
        public icMessageQueueItem  next  ;

        public icMessageQueueItem(LogMessage msg)
        {
            this.msg = msg ;
            next = null ;
        }
    }
}