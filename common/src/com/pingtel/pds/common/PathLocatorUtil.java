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

package com.pingtel.pds.common;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

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


    ///////////////////////////////////////////////////////////////////////////
    // Properties file names constants
    ///////////////////////////////////////////////////////////////////////////
    public static final String PGS_PROPS = "pgs.props";
    
    ///////////////////////////////////////////////////////////////////////////
    // Properties names constants
    ///////////////////////////////////////////////////////////////////////////
    public static final String PGS_SIPXCHANGE_REALM = "sipxchange.realm";  
    public static final String PGS_SIPXCHANGE_DOMAIN_NAME = "sipxchange.domain.name";  
    
    /** Singleton Instance */
    private static PathLocatorUtil g_instance = null;

    private String m_datadir;

    private String m_confdir;

    private String m_tmpdir;

    private String m_logdir;
    
    private String m_phonedir;

    /** Singleton Private Constructor */
    PathLocatorUtil(VersionInfo versionInfo) {
        m_datadir = versionInfo.getProperty("sipxdata");
        m_confdir = versionInfo.getProperty("sipxconf");
        m_tmpdir = versionInfo.getProperty("sipxtmp");
        m_logdir = versionInfo.getProperty("sipxlog");
        m_phonedir = versionInfo.getProperty("sipxphone");
    }

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
                g_instance = new PathLocatorUtil(versionInfo);
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

    /** path to main configuration (e.g. /usr/local/var/sipx/log) */
    public PathBuffer log()
    {
        return new PathBuffer(m_logdir);
    }

    /** path to main configuration (e.g. /usr/local/var/tmp) */
    public PathBuffer tmp()
    {
        return new PathBuffer(m_tmpdir);
    }
    
    /** path to phone configuration (e.g. /usr/local/var/sipxdata/configserver/phone) */
    public PathBuffer phone()
    {
        return new PathBuffer(m_phonedir);
    }
    

    /**
     * Gets the path for a given path type
     * @return Fully Qualified Location for the path
     */
    public String getPath( int pathType, int source ) throws FileNotFoundException {
        String folderLocation = getPathSilent(pathType);


        if ( folderLocation != null ) {
            File folder = new File(folderLocation);
            if (!folder.isDirectory())
            {
                throw new FileNotFoundException("Folder " + folderLocation + " is not a folder");
            }
        } else {
            throw new FileNotFoundException();
        }

        return folderLocation;
    }

    /**
     * Gets the path for a given path type
     * Does not throw exceptions if path is not a directory
     * @param pathType
     * @return
     */
    String getPathSilent(int pathType) {
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
                folderLocation = phone().slash();
                break;
            }
        }
        return folderLocation.toString();
    }
}
