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

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;
import java.rmi.RMISecurityManager;
import java.rmi.RemoteException;
import java.util.Properties;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.PingtelXMLLayout;
import com.pingtel.pds.common.VersionInfo;
import com.pingtel.pds.sds.cache.SessionCache;
import com.pingtel.pds.sds.cache.SessionCacheException;

import org.apache.log4j.Category;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.NDC;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.Priority;
import org.apache.log4j.RollingFileAppender;

/**
 * The Software Delibery server is responsible for listening
 * for SIP requests and subscriptions.
 */
public class SoftwareDeliveryServer extends Object {

    private static Category m_logger;

    /** property definition */
    private static final String PROPERTIES_FILE  = "sds.props";
    private static final String DB_URL           = "sds.dbURL";
    private static final String DB_USERID        = "sds.dbUserID";
    private static final String DB_PASSWORD      = "sds.dbPassword";
    private static final String DB_DRIVERNAME    = "sds.dbDriverName";
    private static final String CATALOG_URL      = "sds.catalogURL";
    private static final String DB_CON_FACTORY   = "sds.dbConnectionFactoryClass";

    /** RMI Server listener for Profile updates from the profile writer */
    private ProfileListenerImpl m_profileListener;

    /**
     * Main Entry point for the SoftwareDeliveryServer
     */
    public static void main (String args[]) {

        if ( args != null && args.length !=0 && args [0].equalsIgnoreCase ("-v") ) {
            SoftwareDeliveryServer sds = new SoftwareDeliveryServer();
            System.exit(0);
        }

        // Ensure we can use RMI correctly
        System.setSecurityManager( new RMISecurityManager() );

        m_logger = Category.getInstance( "pds" );
        String debugValue = System.getProperty( "pds.debug");
        if ( debugValue != null && debugValue.equalsIgnoreCase( "true" ) ) {
            m_logger.setPriority( Priority.DEBUG );
        }
        else {
            m_logger.setPriority( Priority.INFO );
        }

        try {
            String path =
                PathLocatorUtil.getInstance().getPath( PathLocatorUtil.LOGS_FOLDER, PathLocatorUtil.NOT_PGS );

            RollingFileAppender fileAppender = new RollingFileAppender ( );
            fileAppender.setFile( path + "PDSMessages.log", true);

            fileAppender.setMaxFileSize( "1MB" );
            fileAppender.setMaxBackupIndex( 20 );
            fileAppender.setLayout( new PingtelXMLLayout( "SDS" ) );

            ConsoleAppender consoleAppender =
                new ConsoleAppender( new PatternLayout ( "%-5p [%t]: %m %l%n" ),
                                    "System.out" );

            m_logger.addAppender( fileAppender );

            if ( debugValue != null && debugValue.equalsIgnoreCase( "true" ) ) {
                m_logger.addAppender( consoleAppender );
            }

        }
        catch ( IOException ex ) {
            throw new RuntimeException ( ex.toString() );
        }

        NDC.push( "SDS" );
        NDC.push( Thread.currentThread().toString() );

        m_logger.info( "Starting SDS" );

        try {
            // set the VM System properties so that we can use JNDI,
            // RMI and the other security settings.  The RMI one is the most
            // interesting since the codebase VM property will not take relative
            // path names
            setSystemProperties();

            // Read the properties in from the config file
            Properties serverProperties = new Properties();

            String prop = System.getProperty( PROPERTIES_FILE );

            FileInputStream serverPropFile =
                new FileInputStream ( System.getProperty( PROPERTIES_FILE ) );
            serverProperties.load( serverPropFile );
            serverPropFile.close();

            m_logger.debug( "sucessfully read properties " + PROPERTIES_FILE );

            SoftwareDeliveryServer sds =
                    new SoftwareDeliveryServer(serverProperties);

        }  catch (Exception ex) {
            // Print out the exception to the Console as the logger uses a timer
            // internally and we need to see the log message somewhere
            m_logger.fatal( "problem Starting the SDS, Exception Occurred", ex );

            System.exit(-1);
        }
    }

    /**
     * JNT Service Layer Integreation - this method is called when NT
     * Shutsdown the service
     */
    public static void stopApplication() {
        m_logger.debug( "JNT Shutdown Request Received" );
        System.exit(0);
    }

    /**
     * Sets the critical System properties for the application
     * if they are not already set.
     */
    private static void  setSystemProperties () {
        // Determine the current working directory
        m_logger.info( "setting system properties" );
        File tempFile = new File("");
        String cwd = tempFile.getAbsolutePath();
        tempFile.delete();

        // JNDI Settings 3
        if ( System.getProperty("sds.props") == null ) {
            System.setProperty("sds.props", "../conf/sds.props");
        }
        // JAAS Security Setting for Clients
        if ( System.getProperty( "java.security.auth.login.config" ) == null ) {
            // @JC it remains to be seen if relative path names work here
            System.setProperty( "java.security.auth.login.config",
                                "file:../appserver/jboss/client/auth.conf" );
        }

        // This property is very interesting as we cannot use relative paths from
        // the current working directory. The work around this is to calculate
        // the fully qualified URL's.
        if ( System.getProperty("java.rmi.server.codebase") == null ) {
            String sep = System.getProperty("file.separator");
            String libDir = cwd.substring(cwd.indexOf(sep), cwd.lastIndexOf(sep) + 1) + "lib/";
            libDir = "file:" + libDir.replace('\\', '/');
            String codeBase = libDir + "sds.jar" + " " + libDir + "common.jar";
            System.setProperty( "java.rmi.server.codebase", codeBase);
            System.out.println ("Setting java.rmi.server.codebase to: " + codeBase);
        }

        // Allow an open security policy (for RMI etc.)  Batton down the hatches
        // on this one eventually.
        if ( System.getProperty("java.security.policy") == null ) {
            System.setProperty("java.security.policy", "file:../conf/security.policy");
        }

        // JNDI Setting 1
        if ( System.getProperty("java.naming.factory.initial") == null ) {
            System.setProperty( "java.naming.factory.initial",
                                "org.jnp.interfaces.NamingContextFactory" );
        }

        // JNDI Settings 2
        if ( System.getProperty("java.naming.factory.url.pkgs") == null ) {
            System.setProperty( "java.naming.factory.url.pkgs",
                                "org.jboss.naming:org.jnp.interfaces" );
        }

        // JNDI Settings 3
        if ( System.getProperty("java.naming.provider.url") == null ) {
            System.setProperty("java.naming.provider.url", "localhost:1099");
        }
    }

    /**
     * Constructor
     * - reads config file for SDS server parameters
     * - Initializes the user agent (a C++ somponent that is
     *   responsible for  that listening for SIP supscribe requests)
     * @exception SDSException
     */
    public SoftwareDeliveryServer (Properties serverProperties) throws PDSException {
        try {
            // This utility object is a database implementatino for the
            // subscription database, launch it with the garbage collector enabled
            m_logger.debug( "Instantiating SoftwareDeliveryServer" );
            SessionCache sessionCache =
                new SessionCache (
                    serverProperties.getProperty(DB_URL),
                    serverProperties.getProperty(DB_USERID),
                    serverProperties.getProperty(DB_PASSWORD),
                    serverProperties.getProperty(DB_DRIVERNAME),
                    serverProperties.getProperty(DB_CON_FACTORY) );

            // Determine the IP address of this server as it is used as a fallback default
            String myIPAddress = InetAddress.getLocalHost().getHostAddress();

            // There is a potential race condition where the Subscriptions
            // and the projections may be updating the cache at the same time
            // this object will serialize access to these sensitive resources
            Object cacheMutex = new Object();

            // Start the RMI Service listening for new rendered profiles
            // This service is called from the profile writer
            m_profileListener = new ProfileListenerImpl (
                "rmi://" + myIPAddress + ":2001/SDSProfileListener",
                sessionCache,
                serverProperties.getProperty(CATALOG_URL),
                cacheMutex );

            m_logger.debug( "Created ProfileListenerImpl" );

            // Pass along the Database Enrollment Helper to the Subscription listener
            SubscriptionListener listener =
                new SubscriptionListener (
                    sessionCache,
                    m_profileListener, // Used for access to the document root
                    cacheMutex );

            m_logger.debug( "created SubscriptionListener" );

            // Initialize the lower C++ layers (loading the sds library) and
            // setting up the JNI callbacks, SIP Subscribe's come in through this path
            SipConfigServerAgent.getInstance().
                addSubscriptionListener( listener );
        } catch (SessionCacheException ex) {
            throw new PDSException("Unable to Create the Subscription Cache, " +
                                   "check database settings in ssd.properties", ex);
        } catch (RemoteException ex) {
ex.printStackTrace();
            throw new PDSException("Unable to Bind to the RMI Registry", ex);
        } catch (IOException ex) {
            throw new PDSException("Problem Loading the sip stack", ex);
        }

        m_logger.info( "SDS Started, Waiting for Profiles...." );
    }

    private SoftwareDeliveryServer () {

        InputStream versionStream =
                getClass().getClassLoader().getResourceAsStream( "buildVersion.properties" );

        try {
            VersionInfo versionInfo = new VersionInfo ( versionStream );
            System.out.println( versionInfo.getVersion() );
        }
        catch ( IOException ex ) {
            throw new RuntimeException ( ex.toString() );
        }
    }

}

