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

import java.net.* ;
import java.io.* ;
import java.util.* ;

import org.sipfoundry.sipxphone.service.logger.* ;
import org.sipfoundry.util.PingerConfig ;

/**
 *
 *
 * @author Robert J. Andreasen, Jr.
 */
public class Logger
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** default port number for the log server */
    public static final int LOGGER_PORT = 8123 ;

    /** verbosity level/serverity: this is purely informational */
    public static final int Informational = 1 ;
    /** verbosity level/serverity: this is a warning */
    public static final int Warning = 2 ;
    /** verbosity level/serverity: something bad... real bad */
    public static final int Fatal = 3 ;

    public static final int LOG_INFORMATION     = 0 ;
    public static final int LOG_WARNING         = 1 ;
    public static final int LOG_ERROR           = 2 ;

    public static final int TRAIL_COMMAND       = 100 ;
    public static final int TRAIL_NOTIFICATION  = 101 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** thread that sits around and waits for connections */
    protected static LoggerDaemon m_daemon = null ;
    /** thread that sits around and dispatches log message */
    protected static LogDispatcher m_dispatcher = null ;
    /** Used to store the singleton for this class */
    protected static Logger m_reference = null;

    protected boolean   m_bSubscribeAll ;
    protected Hashtable m_htSubscriptions ;
    protected boolean   m_bDebugFlag ;



//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default constructor
     */
    protected Logger()
    {
        int iPort = getPort() ;
        m_bDebugFlag = getDebugFlag() ;

        if (iPort >= 0)
        {
            m_dispatcher = new LogDispatcher() ;
            m_daemon = new LoggerDaemon(m_dispatcher, iPort) ;

            // Start up the dispatcher and then the daemon...
            m_dispatcher.setDaemon(true) ;
            m_dispatcher.start() ;

            m_daemon.setDaemon(true) ;
            m_daemon.start() ;

            m_htSubscriptions = new Hashtable() ;
            m_bSubscribeAll = false ;

            while (!m_dispatcher.isStarted() && !m_daemon.isStarted()) {
                try {
                    Thread.currentThread().sleep(500) ;
                } catch (Exception e) {

                }
            }
        }
    }



    /**
     * Method used to get reference to the singleton for this class
     */
    public static Logger getInstance()
    {
        if (m_reference == null)
            m_reference = new Logger() ;

        return m_reference ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public static boolean isEnabled()
    {
        return (getInstance().m_dispatcher != null) ;
    }


    /**
     *
     */
    public boolean getDebugFlag()
    {
        boolean bDebug = false ;

        String strDebugFlag = PingerConfig.getInstance().getValue(
                PingerConfig.JAVA_LOGGER_DEBUG) ;

        if ((strDebugFlag != null) && strDebugFlag.equalsIgnoreCase("ENABLE"))
        {
            bDebug = true ;
        }

        return bDebug ;
    }


    /**
     * Get the configured port number for the logger daemon.
     *
     * @return the port number or -1 if the logger is disabled.
     */
    public int getPort()
    {
        int iPort = -1 ;    // return code: port number

        String strPort = PingerConfig.getInstance().getValue(PingerConfig.JAVA_LOGGER_PORT) ;
        if (strPort != null)
        {
            try
            {
                iPort = Integer.parseInt(strPort) ;
                if (iPort <= 0)
                    iPort = -1 ;
            }
            catch (NumberFormatException nfe)
            {
                System.out.println("JAVA_LOGGER_PORT is invalid: " + strPort) ;
            }
        }

        return iPort ;
    }

    /**
     * Starts up the Logger if not already started.
     */
    public static void startup()
    {
        getInstance() ;
    }


    public static void refreshSubscriptionList()
    {
        Logger logger = getInstance() ;
        if (logger != null)
        {
            logger.generateSubscriptionList() ;
        }
    }


    public static void post(String strCategory, int iType, String strDescription)
    {
        Logger logger = getInstance() ;
        if (logger != null)
        {
            logger.postEvent(strCategory, iType, strDescription, null, null, null, null) ;
        }
    }


    public static void post(String strCategory, int iType, String strDescription, Object objParam1)
    {
        Logger logger = getInstance() ;
        if (logger != null)
        {
            logger.postEvent(strCategory, iType, strDescription, objParam1, null, null, null) ;
        }
    }


    public static void post(String strCategory, int iType, String strDescription, Object objParam1, Object objParam2)
    {
        Logger logger = getInstance() ;
        if (logger != null)
        {
            logger.postEvent(strCategory, iType, strDescription, objParam1, objParam2, null, null) ;
        }
    }

    public static void post(String strCategory, int iType, String strDescription, Object objParam1, Object objParam2, Object objParam3)
    {
        Logger logger = getInstance() ;
        if (logger != null)
        {
            logger.postEvent(strCategory, iType, strDescription, objParam1, objParam2, objParam3, null) ;
        }
    }


    public static void post(String strCategory, int iType, String strDescription, Object objParam1, Object objParam2, Object objParam3, Object objParam4)
    {
        Logger logger = getInstance() ;
        if (logger != null)
        {
            logger.postEvent(strCategory, iType, strDescription, objParam1, objParam2, objParam3, objParam4) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    protected void generateSubscriptionList()
    {
        // Clear the existing list
        m_htSubscriptions.clear() ;

        // Generate a new list
        Vector vClients = m_dispatcher.getClientList() ;
        Enumeration enumClients = vClients.elements() ;
        while (enumClients.hasMoreElements())
        {
            LogClient client = (LogClient) enumClients.nextElement() ;
            if (client != null)
            {
                Hashtable hash = client.getSubscriptions() ;
                if (hash != null)
                {
                    Enumeration enumKeys = hash.keys() ;
                    while (enumKeys.hasMoreElements())
                    {
                        String strKey = (String) enumKeys.nextElement() ;
                        if (strKey != null)
                        {
                            m_htSubscriptions.put(strKey, strKey) ;
                        }
                    }
                }
            }
        }


        if (m_htSubscriptions.containsKey("*"))
        {
            m_bSubscribeAll = true ;
        }
    }


    protected boolean subscribed(String strCategory)
    {
        boolean bRC = false ;

        // Only spin our wheels if someone has subscribed to the category.
        // NOTE: "logger" is a special category that gets send to all.
        if (    m_bSubscribeAll ||
                m_htSubscriptions.contains(strCategory) ||
                strCategory.equalsIgnoreCase("logger"))
        {
            bRC = true ;
        }

        return bRC ;
    }


    protected void postEvent(String strCategory, int iType, String strDescription, Object objParam1, Object objParam2, Object objParam3, Object objParam4)
    {
        if ((m_dispatcher != null) && subscribed(strCategory))
        {
            if (m_bDebugFlag)
            {
                System.out.println("JLogger: posting: " + strCategory + ": " + strDescription) ;
            }

            m_dispatcher.postEvent(strCategory, iType, strDescription, objParam1, objParam2, objParam3, objParam4) ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////
}
