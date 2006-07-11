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

/**
 * Threat thead sits around and waits for connections.  Once a connection
 * is made- this thread hands it over to the dispatcher.
 */
public class LoggerDaemon extends Thread
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** incoming connection socket */
    protected ServerSocket m_socket ;
    /** are we started and initialized? */
    protected boolean m_bStarted ;
    /** reference to the log dispatcher */
    protected LogDispatcher m_dispatcher ;
    /** the port number that the daemon should bind to */
    protected int m_iPort ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default Constructor
     */
    public LoggerDaemon(LogDispatcher dispatcher, int iPort)
    {
        super("JLogDaemon") ;

        if ((dispatcher == null) || (iPort <= 0))
            throw new IllegalArgumentException() ;

        setPriority(Thread.MIN_PRIORITY) ;
        m_bStarted = false ;
        m_dispatcher = dispatcher ;
        m_iPort = iPort ;

    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Is this daemon started?
     */
    public boolean isStarted()
    {
        return m_bStarted ;
    }

    /**
     * Wait for Clients...
     */
    public void run()
    {
        try {
            System.out.println("Starting Logger on port " + m_iPort) ;
            m_socket = new ServerSocket(m_iPort, 1, null) ;
            m_bStarted = true ;

            while (true) {
                try {
                    Socket s = m_socket.accept() ;

                    ObjectOutputStream outputStream = new ObjectOutputStream(s.getOutputStream()) ;
                    outputStream.flush() ;

                    ObjectInputStream inputStream = new ObjectInputStream(s.getInputStream()) ;

                    LogClient client = new LogClient(s, inputStream, outputStream) ;
                    client.start() ;

                    m_dispatcher.addClient(client) ;
                } catch (IOException ioe) {
                    // ignore accept exceptions...
                }
            }
        } catch (IOException ioe) {
            System.out.println("Unable to start Logger on port " + m_iPort + ": " + ioe.toString()) ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

}
