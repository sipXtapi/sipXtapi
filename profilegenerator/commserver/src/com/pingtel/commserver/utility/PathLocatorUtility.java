/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/commserver/src/com/pingtel/commserver/utility/PathLocatorUtility.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.commserver.utility;


import java.io.File;

/**
 * A basic class for logging information. 
 * Right now, it just writes to standard out.
 * This can be extended to write to approriate log files, etc.
 * 
 * @author Harippriya Sivapatham
 */
public class PathLocatorUtility {
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    
    // Name of different servers that use this helper file
    private static final String AUTH_PROXY      = "authproxy";
    private static final String PROXY           = "proxy";
    private static final String REGISTRAR       = "registrar";
    private static final String STATUS_SERVER   = "statusserver";
    private static final String CONFIG_DEFS     = "globalconfig";

    private static final String FILE_SEPARATOR  = System.getProperty("file.separator");

    public static final int COMMSERVER_CONFIG_FOLDER    = 0;
    public static final int GLOBAL_CONFIG_FOLDER        = 1;
    public static final int XML_FOLDER                  = 2;




//////////////////////////////////////////////////////////////////////////////
// Construction
////
    
    public PathLocatorUtility()
    {
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public static String getConfigFileLocation( String servername )
    {
        String fileLocation = "-1" ;
        if( getConfigFileName(servername) != "-1" )
            fileLocation = getPath( COMMSERVER_CONFIG_FOLDER ) + getConfigFileName(servername);

        return fileLocation;
    }

    public static String getConfigFileName( String servername )
    {
        String filename = "-1" ;
        if ( servername.equalsIgnoreCase(AUTH_PROXY) )
            filename = "authproxy-config.in";
        else if ( servername.equalsIgnoreCase(PROXY) )
            filename = "proxy-config.in";
        else if ( servername.equalsIgnoreCase(REGISTRAR) )
            filename = "registrar-config.in";
        else if ( servername.equalsIgnoreCase(STATUS_SERVER) )
            filename = "status-config.in";
        else if ( servername.equalsIgnoreCase(CONFIG_DEFS) )
            filename = "config.defs";

        return filename;
    }


    public static String getConfigDefinitionFileLocation( String servername )
    {
        String fileLocation = "-1" ;
        if( getConfigDefinitionFileName(servername) != "-1" )
            fileLocation = getPath( XML_FOLDER ) + getConfigDefinitionFileName(servername);

        return fileLocation;
    }

    public static String getConfigDefinitionFileName( String servername )
    {
        String filename = "-1" ;
         if ( servername.equalsIgnoreCase(AUTH_PROXY) )
            filename = "authproxy-config-defs.xml";
        else if ( servername.equalsIgnoreCase(PROXY) )
            filename = "proxy-config-defs.xml";
        else if ( servername.equalsIgnoreCase(REGISTRAR) )
            filename = "registrar-config-defs.xml";
        else if ( servername.equalsIgnoreCase(STATUS_SERVER) )
            filename = "statusserver-config-defs.xml";
        else if ( servername.equalsIgnoreCase(CONFIG_DEFS) )
            filename = "configdefs-config-defs.xml";

        return filename;
    }


    public static String getDefaultConfigFileLocation( String configFileLocation)
    {
        // Default config file is in the same location as the config file
        // Default config file name = <config file name>.default
        String fileLocation=configFileLocation + ".default" ;

        return fileLocation;
    }

    public static String getGlobalConfigFileLocation()
    {
        String fileLocation = getPath( GLOBAL_CONFIG_FOLDER ) + "config.defs" ;
        return fileLocation;
    }

    public static String getPath( int pathType )
    {
        String m_isDevelopment = System.getProperty("pds.development");

        String prodBase =   ".." + FILE_SEPARATOR + 
                            ".." + FILE_SEPARATOR + 
                            ".." + FILE_SEPARATOR +
                            ".." + FILE_SEPARATOR ;

        String devBase  =   ".." + FILE_SEPARATOR + 
                            ".." + FILE_SEPARATOR + 
                            ".." + FILE_SEPARATOR + 
                            ".." + FILE_SEPARATOR + 
                            ".." + FILE_SEPARATOR +
                            ".." + FILE_SEPARATOR ;

        String sipxHome = "home" + FILE_SEPARATOR + "sipxchange" + FILE_SEPARATOR ;
        
        boolean g_devRelease = false ;
        if ( m_isDevelopment != null && m_isDevelopment.equalsIgnoreCase( "true" ) )
            g_devRelease = true;

        String path = "" ;

        switch( pathType )
        {
            case COMMSERVER_CONFIG_FOLDER:
                if( g_devRelease )
                    path = devBase + sipxHome + "commserver" + FILE_SEPARATOR + "etc" + FILE_SEPARATOR ;
                else
                    path = prodBase + "commserver" + FILE_SEPARATOR + "etc" + FILE_SEPARATOR ; 
                break;
            case GLOBAL_CONFIG_FOLDER:
                if( g_devRelease )
                    path = devBase + sipxHome + "shared" + FILE_SEPARATOR + "etc" + FILE_SEPARATOR ;
                else
                    path = prodBase + "shared" + FILE_SEPARATOR + "etc" + FILE_SEPARATOR ; 
                break;
            case XML_FOLDER:
                if( g_devRelease )
                {
                    path =  devBase + 
                            "main" + FILE_SEPARATOR + 
                            "product" + FILE_SEPARATOR +
                            "sw" + FILE_SEPARATOR + 
                            "sipxchange" + FILE_SEPARATOR + 
                            "commserver" + FILE_SEPARATOR + 
                            "etc" + FILE_SEPARATOR ;
                }
                    
                else
                {
                    path = prodBase + "commserver" + FILE_SEPARATOR + "etc" + FILE_SEPARATOR ;
                }
                break;
        }
        return path;
    }

    
}

