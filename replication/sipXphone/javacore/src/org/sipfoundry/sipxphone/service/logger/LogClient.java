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

import java.io.* ;
import java.net.* ;
import java.util.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.util.CountingSemaphore;



/**
 * Object representing a logger client
 */
public class LogClient extends Thread
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected Socket             m_socket ;
    protected ObjectOutputStream m_outputStream ;
    protected ObjectInputStream  m_inputStream ;
    protected boolean            m_bDone ;
    protected Hashtable          m_htSubscriptions ;
    protected CountingSemaphore  m_semWaitForResponse ;


//////////////////////////////////////////////////////////////////////////////
// Constructions
////
    /**
     * Default Construction
     */
    public LogClient(Socket socket, ObjectInputStream inputStream, ObjectOutputStream outputStream)
    {
        super("JLogClient") ;
        setPriority(Thread.MIN_PRIORITY) ;


        m_socket = socket ;
        m_outputStream = outputStream ;
        m_inputStream = inputStream ;
        m_semWaitForResponse = new CountingSemaphore(0, false) ;

        m_bDone = false ;
        m_htSubscriptions = new Hashtable() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * sends a message to the client and returns true/false depending on
     * success
     */
    public boolean sendMessage(LogMessage msg)
    {
        boolean bRC = false ;

        // Send the message if the log client has subscribed to the all
        // channel (*) or the specific channel.  Also, if the category is
        // logger, send it through.
        if (    m_htSubscriptions.containsKey("*") ||
                m_htSubscriptions.containsKey(msg.strCategory) ||
                msg.strCategory.equalsIgnoreCase("logger"))
        {
            if (Logger.getInstance().getDebugFlag())
            {
                System.out.println("JLogger: sending: " + msg.strCategory + ": " + msg.strDescription) ;
            }

            try
            {
                m_outputStream.writeObject(msg) ;
                m_outputStream.reset() ;
                m_outputStream.flush() ;

                bRC = m_semWaitForResponse.down(30*1000) ;
                if (!bRC)
                {
                    System.out.println("JLogger: Failed to receive response to log msg: " + msg) ;
                }
            }
            catch (Exception e)
            {
                System.out.println("JLogger: Error sending message to logger client: " + e.getMessage()) ;
            }
        }
        else
        {
            bRC = true ;
        }
        return bRC ;
    }


    /**
     * try to clean up as much as possible
     */
    public void close()
    {
        m_bDone = true ;

        // Close the input stream
        try {
            if (m_inputStream != null)
                m_inputStream.close() ;
            m_inputStream = null ;
        } catch (IOException ioe) {
            /* eat it */
        }


        // Close the output stream
        try {
            if (m_outputStream != null)
            {
                m_outputStream.close() ;
            }
            m_outputStream = null ;
        } catch (IOException ioe) {
            /* eat it */
        } ;

        // Close the socket
        try {
            if (m_socket != null)
                m_socket.close() ;
            m_socket = null ;
        } catch (IOException ioe) {
            /* eat it */
        } ;
    }


    public Hashtable getSubscriptions()
    {
        return m_htSubscriptions ;
    }


    /**
     * Main Thead Execution loop
     */
    public void run()
    {
        while (!m_bDone)
        {
            try
            {
                LogCommand cmd = null ;
                Object obj = m_inputStream.readObject() ;
                if (obj instanceof Boolean)
                {
                    m_semWaitForResponse.up() ;
                }
                else
                {
                    cmd = (LogCommand) obj ;
                    m_outputStream.writeObject(new Boolean(true));
                    m_outputStream.reset() ;
                    m_outputStream.flush() ;
                    processCommand(cmd) ;
                }
            }
            catch (InterruptedIOException iioe)
            {
                try
                {
                    sleep(900) ;
                }
                catch (Exception e) { }
            }
            catch (ClassNotFoundException e) {
                System.out.println("LogClient: Unknown command: " + e) ;
            }
            catch (IOException e) {
                System.out.println("LogClient: Error obtaining command: " + e) ;
                m_bDone = true ;
            }
        }

        close() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected void processCommand(LogCommand cmd)
    {
        if (cmd.strCommand.equalsIgnoreCase(LogCommand.CMD_SUBSCRIBE)) {
            m_htSubscriptions.put(cmd.strOption, cmd.strOption) ;
            Logger.refreshSubscriptionList() ;
        } else if (cmd.strCommand.equalsIgnoreCase(LogCommand.CMD_UNSUBSCRIBE)) {
            m_htSubscriptions.remove(cmd.strOption) ;
            Logger.refreshSubscriptionList() ;
        } else {
            System.out.println("LogClient: " + cmd) ;
        }
    }
}
