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

package com.pingtel.pds.profilewriter;

import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.PingtelXMLLayout;
import com.pingtel.pds.common.VersionInfo;
import com.pingtel.pds.profilewriter.protocol.tftp.TFTPStreamHandlerFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;
import java.rmi.RMISecurityManager;
import java.util.Properties;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.NDC;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.RollingFileAppender;


/**
 *  ProfileWriterServer class registers a servant implementation object
 *  that does the file writing for profiles.
 */
public class ProfileWriterServer {
    /** The Optional property file containing the configuration settings */
    private static final String PROPERTIES_FILE = "profilewriter.props";

    /** HTTP Mechanism - Pingtel Phones */
    private static final String PROPERTIES_HTTPSERVERNAME = "httpservername";
    private static final String PROPERTIES_HTTPBASEURL    = "httpbaseurl";

    /** HTTP Mechanism - Cisco Phones */
    private static final String PROPERTIES_TFTPSERVERNAME = "tftpservername";
    private static final String PROPERTIES_TFTPBASEURL    = "tftpbaseurl";

    /** Potentially Remote SDS Host */
    private static final String PROPERTIES_SDSHOSTNAME    = "sdshostname";

    /* This property specifies the URL which is send to the SDS.  It is
       prepended to all URLs sent in NOTIFY messages */
    private static final String PROPERTIES_SDS_BASEURL = "publishedHttpDocRoot";
    
    private static final String PROPERTIES_MYIPADDR = "myIpAddress";


    private static Logger m_logger;


    /**
     * Main Startup for the ProfileWriterServer - I may wish to have a
     * base directory here to write profiles to??
     */
    public static void main(String[] args) {


        if ( args != null && args.length !=0 && args [0].equalsIgnoreCase ("-v") ) {
            ProfileWriterServer sds = new ProfileWriterServer();
            System.exit(0);
        }

        // Emnsure that we can use RMI in the way we need
        System.setSecurityManager( new RMISecurityManager() );


        /******************************************************/

        m_logger = Logger.getLogger( "pds" );
        String debugValue = System.getProperty( "pds.debug");
        if ( debugValue != null && debugValue.equalsIgnoreCase( "true" ) ) {
            m_logger.setLevel( Level.DEBUG );
        }
        else {
            m_logger.setLevel( Level.ERROR );
        }

        try {
            String path =
                PathLocatorUtil.getInstance().getPath( PathLocatorUtil.LOGS_FOLDER, PathLocatorUtil.NOT_PGS );

            RollingFileAppender fileAppender = new RollingFileAppender ( );
            fileAppender.setFile( path + "profilewriter.log" );
            fileAppender.setAppend( true );

            fileAppender.setMaxFileSize( "1MB" );
            fileAppender.setMaxBackupIndex( 20 );

            // XCF-45 : Uncomment but keep an eye on this. It was suspiciously commented out
            // causing no logging. I suspect it was commented out because it was flooding the 
            // log file but if that's the case, we will fix the extraneous logging entries 
            // instead.
            fileAppender.setLayout( new PingtelXMLLayout( "PW" ) );

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

        NDC.push( "ProfileWriter" );
        NDC.push( Thread.currentThread().toString() );


        /**************************************************************/



        // Set VM Arguments
        //setSystemProperties();

        // Seamless Support for the TFTP URL based protocol requires
        // us to set a Stream handler factory.  When the URL encounteres
        // a tftp:// protocol it asks this factory to create a
        // TFTPStreamHandler which in turn uses the TFTPURLConnection
        // to make the low level stream calls via NetComponents to a
        // tftp server.
        TFTPStreamHandlerFactory factory = new TFTPStreamHandlerFactory();
        java.net.URL.setURLStreamHandlerFactory(factory);
        // The JLog requires an object to be passed as 1st parameter
        // because we are in a static main method we cannot pass 'this'
        Object staticLogObject = new Object();
        boolean logFileOpened = false;

        try {

            m_logger.info( "ProfileWriter Starting up" );

            // Set VM Arguments @IB
            setSystemProperties();

            // Read the startup arguments (docroot etc.) from the configuration file
            Properties serverProperties = new Properties();

            String configFileName = System.getProperty( PROPERTIES_FILE );

            FileInputStream serverPropFile =
                new FileInputStream ( System.getProperty( PROPERTIES_FILE ) );
            serverProperties.load( serverPropFile );
            serverPropFile.close();

            // Determine the IP address of this server as it is used as a fallback default
            String myIPAddress = serverProperties.getProperty(PROPERTIES_MYIPADDR,
                    InetAddress.getLocalHost().getHostAddress());

            // Parse the Pingtel HTTP parameters
            String httpBaseURL = serverProperties.getProperty ( PROPERTIES_HTTPBASEURL /*, "file:/dms/docrooot" */ );
            String httpServerName = serverProperties.getProperty ( PROPERTIES_HTTPSERVERNAME /* , "localhost" */ );

            // Parse the Pingtel TFTP parameters
            String tftpBaseURL = serverProperties.getProperty ( PROPERTIES_TFTPBASEURL /* , "file:/dms/tftproot" */ );
            String tftpServerName = serverProperties.getProperty ( PROPERTIES_TFTPSERVERNAME /* , "localhost" */ );

            // RMI Connection to SDS Service uses configuration parameter - Defaults to LocalHost
            String sdsHostName = serverProperties.getProperty ( PROPERTIES_SDSHOSTNAME, myIPAddress );

            String publishedHttpBaseURL =
                serverProperties.getProperty ( PROPERTIES_SDS_BASEURL );

            // Create the Servant object and bind it to the rmi name 'ProfileWriter'
            // Eventually use a proper fully qualified name for location independence
            // Because the PriofileWriterImpl object is a subclass of UnicastRemoteObject
            // the construction process (superclass) exports itself - making it available
            // for remote business methods
            String rmiBoundName = "rmi://" + myIPAddress + ":2001/ProfileWriter";
            ProfileWriterImpl implementation =
                new ProfileWriterImpl(
                     rmiBoundName,
                     sdsHostName,                 // Name of Remote SDS Service Host
                     httpBaseURL,                 // (can only be null httpServerName is locahost)
                     httpServerName,              // for SIP Support
                     publishedHttpBaseURL,
                     tftpBaseURL,                 // (can only be null if tftpServerNAme is locahost)
                     tftpServerName );            // for CISCO Support

            m_logger.info( "ProfileWriter Started, Waiting for Projections..." );

        }  catch (Exception ex) {
            m_logger.fatal ( "Unable to start ProfileWriter " + ex.getMessage() );
            System.exit(1);
        }
    }

    /**
     * JNT Service Layer Integreation - this method is called when NT
     * Shutsdown the service
     */
    public static void stopApplication() {
        System.out.println("JNT Shutdown Request Received");
        System.exit(0);
    }

    /**
     * Sets the critical System properties for the application
     * if they are not already set.
     */
    private static void setSystemProperties () {
        // Determine the current working directory
        File tempFile = new File("");
        String cwd = tempFile.getAbsolutePath();
        tempFile.delete();

        // Set the configuration file if is not specified as a VM Argument
        if ( System.getProperty(PROPERTIES_FILE) == null ) {
            System.setProperty(PROPERTIES_FILE, "../conf/profilewriter.props");
        }

        // This property is very interesting as we cannot use relative paths from
        // the current working directory. The work around this is to calculate
        // the fully qualified URL's.
        if ( System.getProperty("java.rmi.server.codebase") == null ) {
            String sep = System.getProperty("file.separator");
            String libDir = cwd.substring(cwd.indexOf(sep), cwd.lastIndexOf(sep) + 1) + "lib/";
            libDir = "file:" + libDir.replace('\\', '/');
            String codeBase = libDir + "profilewriter.jar" + " " + libDir + "common.jar";
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


    private ProfileWriterServer () {

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
