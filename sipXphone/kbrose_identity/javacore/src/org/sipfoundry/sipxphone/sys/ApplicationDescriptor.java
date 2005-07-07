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


package org.sipfoundry.sipxphone.sys ;

import java.util.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.* ;

public class ApplicationDescriptor
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /**
     * This is the name of the Application. For example:
     * "Space Invaders"
     */
    public static final String TITLE = "Title";

    /**
     * This is the version of the Application. For example:
     * "1.0"
     */
    public static final String VERSION = "Version";

    /**
     * This is the version of the core software required for the app to run
     * "1.0.0"
     */
    public static final String REQUIRED_VERSION = "RequiredVersion";

    /**
     * This is the hint that appears when the user holds down the
     * button bound to this application in the ApplicationLauncher.
     */
    public static final String HINT = "Hint";

    /**
     * This is the long description of the Application. For example:
     * "This Application allows you to play a sound to someone that you
     * are talking with on the phone."
     */
    public static final String LONG_DESCRIPTION = "LongDescription";

    /**
     * This is the name of the author of the Application. For example:
     * "Acme Software Company"
     */
    public static final String AUTHOR = "Author";

    /**
     * This is the icon used by the Application launcher. The value for
     * this key is either a file name in the JAR or a URL. For example:
     * "earcon_launcher.gif"
     */
    public static final String LAUNCHER_ICON = "LauncherIcon";

    /**
     * This is the main class for the Application. This class needs to be derived
     * from Application. For example:
     * "com.acme.earcon"
     */
    public static final String MAIN_CLASS_NAME = "MainClassName";

    /**
     * This key tells the system whether to install the JAR locally on the phone
     * or reference it via URL. The possible values are:
     * Local = install locally on the phone (not supported)
     * URL = only reference the URL
     */
    public static final String INSTALLATION_METHOD = "InstallationMethod";


    /**
     * This key tells the system whether to load the application at system
     * start time or on demand. This amounts to loading application JAR file
     * into memory and calling the onLoad() method in the application.  If the
     * application is not frequently used, then you may wish to load on demand.
     * If the application start time is critical, or it is a frequently used
     * application, then you may wish to load the application at system
     * startup.  Possible values for this key are:
     *
     * SystemStart = Load application at system startup
     * OnDemand = Load application when the application is chosen from the
     *            Launcher
     */
    public static final String LOAD = "Load" ;

    /**
     * This key tells the system when to call the main() method on the
     * application.
     *
     * SystemStart = Call main when the system comes up.
     * OnDemand = Call main when the application is chosen from the Launcher
     */
    public static final String RUN = "Run" ;


    /**
     * This is the type of the Application. If there are multiple values
     * for a key, they should be separated by commas. For example:
     * Type=Hook,Normal
     */
    public static final String APP_CATEGORY = "AppCategory";



//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** the type of application, see constants in ApplicationRegistry */
    private int m_iType ;

    /** the URL from where the application was loaded */
    private String m_codebase;

    /** properties of other attributes: see constants for known keys */
    private Properties m_properties ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Construction requires type, application, and display name.
     *
     * @param iType type of application, see constants in ApplicationRegistry
     * @param strClassName fully qualified application class name
     *
     * @exception SecurityException thrown if a user does not have permissions
     *            to replace a system application.
     * @exception IllegalArgumentException thrown if the type is invalid or a
     *            null string is passed in as the application.
     */
    protected ApplicationDescriptor(int iType, String mainClassName)
            throws SecurityException, IllegalArgumentException
    {
        m_properties = new Properties();

        // Validate the type
        if ((iType < ApplicationRegistry.APP_TYPE_MIN) || (iType > ApplicationRegistry.APP_TYPE_MAX)) {
            throw new IllegalArgumentException("application type is invalid") ;
        }

        // Validate the class name
        if (mainClassName == null) {
            throw new IllegalArgumentException("main class name is null") ;
        }

        // if everything is ok then store
        m_iType = iType ;
        m_codebase = null;

        m_properties.put(MAIN_CLASS_NAME, mainClassName);
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public ApplicationDescriptor()
    {
        m_properties = new Properties();

        m_iType=ApplicationRegistry.UNKNOWN;
        m_codebase = null;
    }

    /**
     * query the type of application
     *
     * @return int constant application type
     */
    public int getType()
    {
        return m_iType ;
    }

    public void setType(int iType)
    {
        m_iType = iType ;
    }

    public String getCodebase()
    {
        return m_codebase;
    }

    public void setCodebase(String codebase)
    {
        m_codebase = codebase;
    }

    public String getTitle()
    {
        String title = m_properties.getProperty(TITLE);
        if( title != null )
            title = title.trim();
        return title;
    }

    public String getVersion()
    {
        String version = m_properties.getProperty(VERSION);
        if( version != null )
            version = version.trim();
        return version;
    }

    public String getHint()
    {
        String hint = m_properties.getProperty(HINT);
        if( hint != null )
            hint = hint.trim();
        return hint;
    }

    public String getLongDescription()
    {
        String longDescrip = m_properties.getProperty(LONG_DESCRIPTION);
        if( longDescrip != null )
            longDescrip = longDescrip.trim();
        return longDescrip;
    }

    public String getAuthor()
    {
        String author = m_properties.getProperty(AUTHOR);
        if( author != null )
            author = author.trim();
        return author;
    }

    public String getLauncherIcon()
    {
        String icon = m_properties.getProperty(LAUNCHER_ICON);
        if( icon != null )
            icon = icon.trim();
        return icon;
    }

    public String getMainClassName()
    {
        String mainClassName = m_properties.getProperty(MAIN_CLASS_NAME);
        if( mainClassName != null )
            mainClassName = mainClassName.trim();
        return mainClassName;
    }

    public String getInstallationMethod()
    {
        String method = m_properties.getProperty(INSTALLATION_METHOD);
        if( method != null )
            method = method.trim();
        return method;
    }

    public String getAppCategory()
    {
        String category = m_properties.getProperty(APP_CATEGORY);
        if( category != null )
            category = category.trim();
        return category;
    }

    public String getRequiredVersion()
    {
        String version =  m_properties.getProperty(REQUIRED_VERSION);
        if( version != null )
            version = version.trim();
        return version;
    }

    public String getRunSetting()
    {
        String runSetting =  m_properties.getProperty(RUN);
        if( runSetting != null )
            runSetting = runSetting.trim();
        return runSetting;
    }


    public void setProperties(Properties properties)
    {
        m_properties = properties;
    }

    public Properties getProperties()
    {
        return m_properties;
    }


    public void dump(java.io.PrintStream out)
    {
        out.println("") ;
        out.println(getTitle()) ;
        out.println(getCodebase()) ;
        out.println(getMainClassName()) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * temp hack method to populate registry with core applictions
     */
}
