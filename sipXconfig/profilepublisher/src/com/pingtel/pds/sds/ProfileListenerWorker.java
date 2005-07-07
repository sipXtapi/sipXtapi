/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.sds;

import java.util.ArrayList;

import org.apache.log4j.*;

import com.pingtel.pds.sds.cache.SessionCache;
import com.pingtel.pds.sds.cmd.Command;
import com.pingtel.pds.sds.cmd.CommandException;
import com.pingtel.pds.sds.cmd.PhoneInvoker;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author      JC
 * @version 1.0
 */

public class ProfileListenerWorker implements Runnable {

    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
    }

    /** Doc Root */
    private String m_documentRoot;

    /** Prevent cseq update from subscripton and enrollments, this object is
    the same as the one used by subscriptions */
    private Object m_cacheMutex = null;

    /** flag used to shutdown the worker thread */
    private boolean m_shutdown = false;

    /** Database Helper utility to access the subscription Database */
    private SessionCache m_sessionCache;

    /** Store the worker thread for cleanup (join method) */
    private Thread m_workerThread = null;

    /** Command queue, written to by RMI thread, drained by worker thread */
    private ArrayList m_commandQueue = new ArrayList();

    /**
     * Constructor
     * @param sessionCache helper object to access the SIP subscription info
     */
    public ProfileListenerWorker( SessionCache sessionCache,
                                  Object cacheMutex ) {
        m_sessionCache = sessionCache;
        m_cacheMutex = cacheMutex;
    }

    /**
     * This is the workhorse method for the profile listener RMI
     * Service.  The thread does the following tasks:
     * - dequeues projected profiles from the ProfileWriter
     * - checks to see if projected profile is enrolled via SIP
     * - if enrolled the profile is sent to the phone
     */
    public void run() {
        // initialize the thread so member so that this
        // worker can be shutdown cleanly
        m_workerThread = Thread.currentThread();

        NDC.pop();
        NDC.push( "SDS" );
        NDC.push( m_workerThread.toString() );

        // Wait until we receive the shutdown command over RMI
        while ( !m_shutdown ) {
            // seriaze access to the command queue
            synchronized ( m_commandQueue ) {
                // Block until a command is posted to the queue
                while ( m_commandQueue.isEmpty() ) {
                    try {
                        m_commandQueue.wait();
                    } catch (InterruptedException iex) {}
                }

                // There is a commmand in the queue, but check first
                // as we may be in the midst of a shutdown command where
                // and interrupted exception was generated above
                if ( !m_commandQueue.isEmpty() ) {
                    // Extract projected profile from the front of the
                    // queue (the oldest entry), the commands are implemted
                    // as part of the command design pattern
                    // and as such they are intelligent
                    m_logger.debug( "Received command" );
                    Command nextCommand = (Command)m_commandQueue.get(0);
                    m_commandQueue.remove(0);
                    PhoneInvoker invoker = new PhoneInvoker( nextCommand );
                    try {
                        invoker.notifyPhone();
                    } catch (CommandException ex) {
                        m_logger.error("Error invoking command ", ex);
                    }
                }
            }
        }
        m_logger.debug( "Shutdown Received by Worker Thread" );
    }

    /** Sets the document root */
    public void setDocumentRoot ( String documentRoot ) {
        // ensire thread safe access
        synchronized ( m_commandQueue ) {
            m_logger.info( "Setting Document Root to: " + documentRoot );
            m_documentRoot = documentRoot;
        }
    }

    /** getter for the Document Root property */
    public String getDocumentRoot() {
        // ensire thread safe access
        synchronized ( m_commandQueue ) {
            return m_documentRoot;
        }
    }

    /** Posts a command on to the command queue */
    public void enqueueCommand( Command command ) {
        synchronized ( m_commandQueue ) {
            m_commandQueue.add (command);
            m_commandQueue.notifyAll();
        }
    }

    /** Shutdown the worker thread and wait for it to terminate */
    public void shutdown() {
        m_shutdown = true;
        m_sessionCache.shutdown();
        // Wait for the worker thread to accept the shutdown signal
        // and terminate (wait up to one second to shutdown)
        try { m_workerThread.join(1000); } catch (InterruptedException iex) {}
    }

    /**
     * ThreadEntryCleanip interface implementatino callback - Can be used to do
     * thread cleanup after the run method genreates a checked exception
     * Implementing this method is optional (this is a poolmanager interface
     * implementation)
     */
    public void cleanupOnException (Throwable t) {
        m_logger.debug( "PoolManager Exception callback handled" );
    }
}