/*
 * $Id: //depot/OPENDEV/sipXconfig/common/src/com/pingtel/pds/common/PathLocatorUtil.java#7 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.common;

import java.io.*;

/**
 * <p>Title: Boundary Interface Objects</p>
 * <p>Description: This is used to build the inter cmponet library</p>
 * <p>Copyright: Copyright (c) 2001</p>
 * <p>Company: Pingtel Corporation</p>
 * @author unascribed
 * @version 1.0
 */

public final class PathLocatorUtil {

    ////////////////////////////////////////////////////////////////////////////
    //
    // The following two constants are used by clients of the PathLocatorUtil.
    // The specify whether or not the client is the PGS (aka) JBoss.   This is
    // important as the relative path for the PGS is way down in the JBoss bin
    // directory rather than the the configserver/bin (production) or
    // individual development directories
    //
    ////////////////////////////////////////////////////////////////////////////
    public static final int PGS = 0;
    public static final int NOT_PGS = 1;

    ///////////////////////////////////////////////////////////////////////////
    //
    // The following constants are used to specify which folder/directory you
    // want.  They are used with the PGS/NOT_PGS constants described above.
    ///////////////////////////////////////////////////////////////////////////
    public static final int LICENSE_FOLDER = 0;
    public static final int XSLT_FOLDER = 1;
    public static final int LOGS_FOLDER = 2;
    public static final int CONFIG_FOLDER = 3;
    public static final int PHONEDEFS_FOLDER = 4;
    public static final int PGS_LOGS_FOLDER = 5;
    public static final int PATCHES_FOLDER = 6;
    public static final int DATASET_ADDINS_FOLDER = 7;
    public static final int DATA_FOLDER = 8;


    /** Singleton Instance */
    private static PathLocatorUtil g_instance = null;

    private String m_datadir;

    private String m_confdir;

    private String m_vardir;

    /** Singleton Private Constructor */
    private PathLocatorUtil() {}

    /** Singleton Accessor */
    public static PathLocatorUtil getInstance() 
    {
        if ( g_instance == null ) 
        {
            try 
            {
                InputStream versionStream = PathLocatorUtil.class.getClassLoader()
                    .getResourceAsStream( "buildVersion.properties" );
                VersionInfo versionInfo = new VersionInfo(versionStream);
                g_instance = new PathLocatorUtil();
                g_instance.m_datadir = versionInfo.getProperty("sipxdata");
                g_instance.m_confdir = versionInfo.getProperty("sipxconf");
                g_instance.m_vardir = versionInfo.getProperty("sipxvar");
            }
            catch (IOException ex) 
            {
                throw new RuntimeException (ex.toString());
            }
        }

        return g_instance;
    }

    /** path to main configuration (e.g. /usr/local/shared/sipxconfig) */
    public PathBuffer data()
    {
        return new PathBuffer(m_datadir);
    }


    /** path to main configuration (e.g. /usr/local/etc/sipxpbx) */
    public PathBuffer conf()
    {
        return new PathBuffer(m_confdir);
    }

    /** path to main configuration (e.g. /usr/local/var/log) */
    public PathBuffer log()
    {
        return var().slash().dir("log");
    }

    /** path to main configuration (e.g. /usr/local/var) */
    public PathBuffer var()
    {
        return new PathBuffer(m_vardir);
    }

    /**
     * Gets the path for a given path type
     * @return Fully Qualified Location for the path
     */
    public String getPath( int pathType, int source ) throws FileNotFoundException {
        PathBuffer folderLocation = null;

        switch ( pathType ) {
            case LICENSE_FOLDER:
                folderLocation = data().slash();
                break;

            case XSLT_FOLDER:
                folderLocation = data().dir("xslt").slash();
                break;

            case LOGS_FOLDER:
                folderLocation = log().slash();
                break;

            case CONFIG_FOLDER:
                folderLocation = conf().slash();
                break;

            case PHONEDEFS_FOLDER:
                folderLocation = data().dir("devicedefs").slash();
                break;

            case PGS_LOGS_FOLDER:
                folderLocation = log().slash();
                break;

            case PATCHES_FOLDER:
                folderLocation = data().dir("patches").slash();
                break;

            case DATASET_ADDINS_FOLDER:
                folderLocation = data().slash();
                break;

            case DATA_FOLDER: {
                folderLocation = var().append("data").slash();
                break;
            }
        }


        if ( folderLocation != null ) {
            File folder = new File(folderLocation.toString());
            if (!folder.isDirectory())
                throw new FileNotFoundException("Folder " + folderLocation + " is not a folder");
        } else {
            throw new FileNotFoundException();
        }

        return folderLocation.toString();
    }
}
