/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/common/ejb/BaseEJB.java#7 $
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

import javax.ejb.EJBContext;
import javax.ejb.EJBException;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import java.util.Properties;

import java.io.IOException;
import java.io.FileInputStream;
import java.io.FileNotFoundException;

import org.apache.log4j.Category;
import org.apache.log4j.Priority;
import org.apache.log4j.RollingFileAppender;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.PatternLayout;

import com.pingtel.pds.common.*;


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
        Category cat = Category.getInstance( "pgs" );
        String debugValue = System.getProperty( "pds.debug");
        if ( debugValue != null && debugValue.equalsIgnoreCase( "true" ) ) {
            cat.setPriority( Priority.DEBUG );
        }
        else {
            cat.setPriority( TXPriority.INFO );
        }

        try {
            String path =
                PathLocatorUtil.getInstance().getPath( PathLocatorUtil.PGS_LOGS_FOLDER, PathLocatorUtil.PGS );

            RollingFileAppender fileAppender = new RollingFileAppender ( );
            fileAppender.setFile( path + "PDSMessages.log", true);

            fileAppender.setMaxFileSize( "1MB" );
            fileAppender.setMaxBackupIndex( 20 );
            fileAppender.setLayout( new PingtelXMLLayout ( "PGS" ) );

            ConsoleAppender consoleAppender =
                new ConsoleAppender( new PatternLayout ( "%-5p [%t]: %m %l%n" ), "System.out" );

            cat.addAppender( fileAppender );

            if ( debugValue != null && debugValue.equalsIgnoreCase( "true" ) ) {
                cat.addAppender( consoleAppender );
            }

        }
        catch ( IOException ex ) {
            throw new RuntimeException ( ex.toString() );
        }

        try {
            String configFolder =
                PathLocatorUtil.getInstance().getPath(
                    PathLocatorUtil.CONFIG_FOLDER,
                    PathLocatorUtil.PGS );

            m_pgsProperties.load( new FileInputStream ( configFolder + "pgs.props" )  );
        }
        catch ( IOException ex ) {
            throw new RuntimeException ( ex.toString() );
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
        Category.getInstance( "pgs" ).info( message );
    }

    /**
     * logFatal logs a message to the PDS message log.
     * @param message String message you want to log.
     */
    protected void logFatal (  String message ) {

        Category.getInstance( "pgs" ).fatal( message );
    }

    protected void logFatal ( String message, Throwable t ) {
        Category.getInstance( "pgs" ).fatal( message, t );
    }

    /**
     * logWarning logs a message to the PDS message log.
     * @param message String message you want to log.
     */
    protected void logWarning ( String message ) {
        //LogManager.getCategory( "pgs" ).warn( message );
        Category.getInstance( "pgs" ).warn( message );
    }

    /**
     * logError logs a message to the PDS message log.
     *
     * @param message String message you want to log.
     */
    protected void logError ( String message ) {
        Category.getInstance( "pgs" ).error( message );
    }

    protected void logError ( String message, Throwable t ) {
        Category.getInstance( "pgs" ).error( message, t );
    }

    protected void logDebug ( String message ) {
        Category.getInstance( "pgs" ).debug( message );
    }

    protected void logTransaction ( EJBContext context, String message ) {
        Category.getInstance( "pgs" ).log(  TXPriority.TX_MESSAGE,
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



    protected String getPGSProperty ( String propertyName ) {
        return BaseEJB.m_pgsProperties.getProperty( propertyName );
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
                            "pgs.props",
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
}
