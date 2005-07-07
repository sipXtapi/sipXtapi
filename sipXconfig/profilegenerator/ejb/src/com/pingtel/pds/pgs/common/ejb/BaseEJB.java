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

package com.pingtel.pds.pgs.common.ejb;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

import javax.ejb.EJBContext;
import javax.ejb.EJBException;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.RollingFileAppender;

import com.pingtel.pds.common.ConfigFileManager;
import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.ErrorMessageBuilder;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.PingtelXMLLayout;
import com.pingtel.pds.common.TXPriority;
import com.pingtel.pds.common.TemplatesCache;


public class BaseEJB {
    protected static Properties m_pgsProperties = new Properties();
    protected static final String EJB_BEAN_PROV_ENV = "java:/comp/env";

    private static ErrorMessageBuilder mErrorMessageBuilder = new ErrorMessageBuilder("pgs-strings");

    // force the class to be loaded to fire off static code in class
    static {
        TemplatesCache.getInstance();
        try {
            EJBHomeFactory.getInstance().init();
        }
        catch (NamingException e) {
            throw new EJBException (e.getMessage());
        }
    }

    static {
        // TODO: we need to move setting logging property to configuration files
        Logger cat = Logger.getLogger( "pgs" );
        String debugValue = System.getProperty( "pds.debug");
        if ( debugValue != null && debugValue.equalsIgnoreCase( "true" ) ) {
            cat.setLevel( Level.DEBUG );
        }
        else {
            cat.setLevel( Level.INFO );
        }

        try {
            final PathLocatorUtil plu = PathLocatorUtil.getInstance();
            String path =
                plu.getPath( PathLocatorUtil.PGS_LOGS_FOLDER, PathLocatorUtil.PGS );

            RollingFileAppender fileAppender = new RollingFileAppender ( );
            fileAppender.setFile( path + "PDSMessages.log" );
            fileAppender.setAppend( true );

            fileAppender.setMaxFileSize( "1MB" );
            fileAppender.setMaxBackupIndex( 20 );
            fileAppender.setLayout( new PingtelXMLLayout ( "PGS" ) );

            ConsoleAppender consoleAppender =
                new ConsoleAppender( new PatternLayout ( "%-5p [%t]: %m %l%n" ), "System.out" );

            cat.addAppender( fileAppender );

            if ( debugValue != null && debugValue.equalsIgnoreCase( "true" ) ) {
                cat.addAppender( consoleAppender );
            }

            String configFolder =
                plu.getPath( PathLocatorUtil.CONFIG_FOLDER, PathLocatorUtil.PGS );
            
            ConfigFileManager configFileManager = ConfigFileManager.getInstance();
            m_pgsProperties = 
                configFileManager.getProperties( configFolder + PathLocatorUtil.PGS_PROPS );
        }
        catch ( IOException ex ) {
            throw new RuntimeException ( ex );
        }
    }


    protected String formatLogMessage ( String businessErrorMessage, Exception ex ) {
        return businessErrorMessage + " " + ex.toString();
    }

    /**
     * logInfo logs a message to the PDS message log.
     *
     * @param message String message you want to log.
     */
    protected void logInfo (  String message ) {
        Logger.getLogger( "pgs" ).info( message );
    }

    /**
     * logFatal logs a message to the PDS message log.
     * @param message String message you want to log.
    protected void logFatal (  String message ) {

        Category.getInstance( "pgs" ).fatal( message );
    }
     */

    protected void logFatal ( String message, Throwable t ) {
        Logger.getLogger( "pgs" ).fatal( message, t );
    }

    /**
     * logWarning logs a message to the PDS message log.
     * @param message String message you want to log.
     */
    protected void logWarning ( String message ) {
        //LogManager.getCategory( "pgs" ).warn( message );
        Logger.getLogger( "pgs" ).warn( message );
    }

    /**
     * logError logs a message to the PDS message log.
     *
     * @param message String message you want to log.
     */
    protected void logError ( String message ) {
        Logger.getLogger( "pgs" ).error( message );
    }

    protected void logError ( String message, Throwable t ) {
        Logger.getLogger( "pgs" ).error( message, t );
    }

    protected void logDebug ( String message ) {
        Logger.getLogger( "pgs" ).debug( message );
    }

    protected void logTransaction ( EJBContext context, String message ) {
        Logger.getLogger( "pgs" ).log(  TXPriority.TX_MESSAGE,
                                            "User: " + context.getCallerPrincipal().getName() +
                                            " " + message );
    }


    public String collateErrorMessages (    String useCase,
                                            String errorMessageNumber,
                                            Object [] detailMsgSubstitutions ) {

        return mErrorMessageBuilder.collateErrorMessages(useCase, errorMessageNumber, detailMsgSubstitutions);
    }


    public String collateErrorMessages ( String useCase ) {
        return mErrorMessageBuilder.collateErrorMessages(useCase);
    }

    public String collateErrorMessages (    String errorMessageNumber,
                                            Object [] detailMsgSubstitutions ) {

        return mErrorMessageBuilder.collateErrorMessages(errorMessageNumber, detailMsgSubstitutions);
    }



    protected final String getPGSProperty ( String propertyName ) {
        return BaseEJB.m_pgsProperties.getProperty( propertyName );
    }

    protected final String getPGSProperty ( String propertyName, String defaultValue) {
        return BaseEJB.m_pgsProperties.getProperty( propertyName, defaultValue );
    }
    
    protected final Object setPGSProperty ( String propertyName, String value) {
        return BaseEJB.m_pgsProperties.setProperty( propertyName, value );
    }

    protected String getEnvEntry ( String envEntryName ) {

        try {
            Context initial = new InitialContext();
            Context env = (Context) initial.lookup( BaseEJB.EJB_BEAN_PROV_ENV );
            return (String) env.lookup( envEntryName );
        }
        catch ( NamingException ne ) {
            throw new EJBException ( ne );
        }
    }

    protected String getInstallStereoType() {
        String installStereotype = null;

        try {
            installStereotype =
                ConfigFileManager.getInstance().getProperty(
                        PathLocatorUtil.getInstance().getPath(
                            PathLocatorUtil.CONFIG_FOLDER, PathLocatorUtil.PGS)+
                            PathLocatorUtil.PGS_PROPS,
                        "installStereotype");
        }
        catch(FileNotFoundException e) {
            throw new RuntimeException(e);
        }

        if(installStereotype == null){
            installStereotype = PDSDefinitions.ENTERPRISE_ST;
        }

        return installStereotype;
    }

    /**
     * Log and rethrow exceptions helper. Does not eat exceptions.
     * 
     * @param error message to be passes
     * @param e caught exception - we'll be packaged in EJBException and rethrown
     */
    protected void logFatalAndRethrow(String error, Exception e) {
        logFatal(e.toString(), e);
        throw new EJBException(error, e);
    }
}
