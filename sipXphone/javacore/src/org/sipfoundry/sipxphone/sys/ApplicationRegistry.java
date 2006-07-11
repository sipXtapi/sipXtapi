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

import java.io.* ;
import java.util.* ;
import java.net.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;
import org.sipfoundry.sipxphone.sys.util.NetUtilities ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.util.* ;


/**
 * This class is used to keep track of what applications are "installed".
 * Installed applications may be in JAR files located on the local filesystem
 * or they may be a URL pointing to a JAR file on the network somewhere.
 * <p>
 * This class is used by the Shell to determine the core applications
 * to kick off at system startup (LoadList).
 * <p>
 * This class is also used by the TaskForm to determine what applications
 * are installed. The user can use the TaskForm to manually start applications
 * in the list.
 * <p>
 * If the LoadList is not available from the PropertyManager, the default
 * LoadList is populated using hardcoded settings in createRegisteredAppList.
 * <p>
 * Applications can be registered as various types:
 * <ul>
 * <li><b>CORE</b>: A core application.  This is something that a user cannot
 *     install/uninstall and must live within pingtel.jar</li>
 * <li><b>*_APP</b>: A mapped core application.  This is something that a
 *     user can change... in theory  We don't have and provision for changing
 *     or installing replacements for these yet, but it is coming...</li>
 * <li><b>USER_*</b>: User installed applications.  These must be located in
 *     some side jar.  A user can install/uninstall these on the fly.</li>
 * </ul>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ApplicationRegistry
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** App Type: User Application */
    public static final int UNKNOWN           = -1 ;
    /** Core App Type: Random Core Application */
    public static final int CORE_APP          = 0 ;
    /** App Type: User Application */
    public static final int USER_APP          = 1 ;
//    /** Core App Type: Phonebook */
//    public static final int PHONEBOOK_APP     = 2 ;
    /** Core App Type: Call Log */
    public static final int CALL_LOG_APP      = 2 ;
    /** Core App Type: Speed Dial */
    public static final int SPEED_DIAL_APP    = 3 ;
    /** Core App Type: Dial By URL */
    public static final int DIAL_BY_URL_APP   = 4 ;
    /** Core App Type: Adjust Volume */
    public static final int VOLUME_ADJUST_APP = 5 ;
    /** Core App Type: Preferences */
    public static final int PREFERENCES_APP   = 6 ;
    /** Core App Type: Conference */
    public static final int CONFERENCE_APP    = 7 ;
    /** Core App Type: Transfer */
    public static final int TRANSFER_APP      = 8 ;


    /** App Type: Min Value */
    public static final int APP_TYPE_MIN = CORE_APP ;
    /** App Type: Max Value */
    public static final int APP_TYPE_MAX = TRANSFER_APP ;

    /** constant to denote unlimited quota to install USER apps */
    private static final int UNLIMITED_QUOTA_TO_INSTALL_USER_APPS = -1;

    ///
    // These contants are the return codes for the
    // cgiInstallApplication method

  // NOTE::::::
  // IF YOU ADD OR EDIT THESE RETURN CONSTANTS starting with CGI, MAKE
  // SURE YOU CHANGE THEM IN ApplicationRegistry.h
  //and the classes that use that header file.
  //
    public static final int CGI_INSTALL_APPLICATION_OK        = 0;
    public static final int CGI_INSTALL_APPLICATION_ERROR_URL = 1;

    /**if the application from the url is already installed */
    public static final int
        CGI_INSTALL_APPLICATION_ERROR_APP_ALREADY_INSTALLED = 2;

    /**if USER app quota is exceeded */
    public static final int
        CGI_INSTALL_APPLICATION_ERROR_EXCEEDED_MAXIMUM_ALLOWED = 3;
    public static final int CGI_INSTALL_APPLICATION_ERROR_TIMEOUT = 4;
    public static final int CGI_INSTALL_APPLICATION_ERROR_VERSIONFAILED = 5;
    public static final int CGI_INSTALL_APPLICATION_ERROR = -1;

    //
    // These contants are the return codes for
    // the cgiUninstallApplication method
    //
    public static final int CGI_UNINSTALL_APPLICATION_OK        =  0;
    public static final int CGI_UNINSTALL_APPLICATION_ERROR_URL =  1;
    public static final int CGI_UNINSTALL_APPLICATION_ERROR     = -1;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** currently implemented as a singleton: singleton instance/references */
    private static ApplicationRegistry m_instance ;

    /*
     * This class uses a hash table to store the application descriptors.  The
     * key is the class path.  I'm guessing that we are going to do look ups
     * by class name more often than not.
   */

    /** hash table of application descriptors */
    private Hashtable htRegistry = new Hashtable() ;

    /** registered application list */
    private icRuntimeApplicationList m_vRegisteredApps ;

    /** list of Application Registry Listeners */
    private Vector                   m_vListeners ;

    /** boolean to indicate if max user application limit
        was tried to be exceeded */
    private boolean m_bMaxUserAppExceeded = false;

    ///////////////////////////////////////////////////////////////////////////
    // Construction
    ////
    private ApplicationRegistry()
    {
        m_vListeners = new Vector() ;

        Repository repository = Repository.getInstance() ;
        m_vRegisteredApps = new icRuntimeApplicationList() ;

        checkForOutdatedAppRegistry() ;
        checkForUpgrade() ;

        m_vRegisteredApps.dump() ;
    }


    /**
     * Checks for an old-style application registry and then converts it to a
     * new style application registry format.
     */
    protected void checkForOutdatedAppRegistry()
    {
        try {
            Vector vOldApps = (Vector) PropertyManager.getInstance().getObject("ApplicationRegistry_registered_apps") ;
            if ((vOldApps != null) && (vOldApps.size() > 0)) {
                System.out.println("Performing Old ApplicationList Registry Conversion") ;
                for (int i=0; i<vOldApps.size(); i++) {
                    AppListItem item = (AppListItem) vOldApps.elementAt(i) ;
                    if (item.getType() == USER_APP) {
                        m_vRegisteredApps.removeAppListItem(item) ;
                        m_vRegisteredApps.addAppListItem(item) ;
                    }
                }
                PropertyManager.getInstance().removeObject("ApplicationRegistry_registered_apps") ;
            }
        } catch (IllegalArgumentException iae) {
            /* burp */
        } catch (Exception e) {
            SysLog.log(e) ;
        }
    }


    /**
     * This code will check the version and remove any outdated/bad code
     */
    protected void checkForUpgrade()
    {
        Repository repository = Repository.getInstance() ;

        String oldVersion = repository.getOldRepositoryVersion() ;
        String newVersion = repository.getNewRepositoryVersion() ;

        if ( (oldVersion == null) || (!oldVersion.equals(newVersion)))
        {
            if ((newVersion != null) && newVersion.equals("2.0.0.1"))
            {
                System.out.println("UPGRADING APPLICATION LIST TO 2.0.0.1") ;

                m_vRegisteredApps.removeAppListItem("org.sipfoundry.sipxphone.sys.app.DeploymentConfigApp") ;

                // Remove outdated Version Checks
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.6.0/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.6.1/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.7.0/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.8.0/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.9.0/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.9.1/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.0/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.0.0/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.1.0/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.2.0/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.3.0/VersionCheck.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/2.0.0/VersionCheck.jar") ;


                // Remove outdated News Apps
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.6.0/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.6.1/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.7.0/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.8.0/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.9.0/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.9.1/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.0/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.0.0/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.1.0/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.2.0/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.3.0/PingtelNews.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/2.0.0/PingtelNews.jar") ;

                // Remove outdated PingtelNet Apps
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.9.0/PingtelNet.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appdev.pingtel.com/pingtelapps/0.9.1/PingtelNet.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.0/PingtelNet.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.0.0/PingtelNet.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.1.0/PingtelNet.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.2.0/PingtelNet.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.3.0/PingtelNet.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/2.0.0/PingtelNet.jar") ;

                // Remove outdated myxpressa
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.1.0/MyPingtelApp.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.2.0/MyPingtelApp.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/1.3.0/MyPingtelApp.jar") ;
                m_vRegisteredApps.removeAppListItem("http://appsrv.pingtel.com/pingtelapps/2.0.0/MyPingtelApp.jar") ;

                // Add Default Core Application
                m_vRegisteredApps.addAppListItem(new AppListItem(USER_APP, "http://appsrv.pingtel.com/pingtelapps/2.0.0/MyPingtelApp.jar")) ;
                m_vRegisteredApps.addAppListItem(new AppListItem(USER_APP, "http://appsrv.pingtel.com/pingtelapps/2.0.0/PingtelNews.jar")) ;


                // HACK: We changed the LCD Contrast range before release v1.0.
                // This change leaves most users with an invalid contrast
                // setting.  For this reason, we are blowing away all of the saved
                // volume and contrast settings.
                //DWW - Only do this if the old version is < 1.0

                if ((oldVersion == null) || oldVersion.charAt(0) == '0')
                {
                    System.out.println("Clearing AudioLevels...") ;
                    try
                    {
                        PropertyManager manager = PropertyManager.getInstance() ;
                        manager.clearOwnedBean(new AudioLevels()) ;
                    }
                    catch (IOException ioe)
                    {
                        System.out.println("Clearing AudioLevels...") ;
                    }
                }
            }
        }
    }



    /**
     * Get the singleton reference to this object
     */
    public static ApplicationRegistry getInstance()
    {
        if (m_instance == null)
            m_instance = new ApplicationRegistry() ;

        return m_instance ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Find a specific application by the passed type.  This method works only
     * for system defined applications- not user applications.
     *
     * @param iType application type
     *
     * @return ApplicationDescriptor of specified application type or null if not
     *         found.
     *
     * @exception IllegalArgumentException thrown if iType is invalid or set
     *            to USER
     * @exception SecurityException thrown FIXME
     */
    public ApplicationDescriptor getCoreApplicationByType(int iType)
            throws IllegalArgumentException, SecurityException
    {
        ApplicationDescriptor rc = null ;

        // Validate iType
        if ((iType < APP_TYPE_MIN) || (iType > APP_TYPE_MAX)) {
            throw new IllegalArgumentException("application type is invalid") ;
        } else if ((iType == USER_APP) || (iType == CORE_APP)) {
            throw new IllegalArgumentException("USER and CORE types are not valid for this operation") ;
        }

        // Start searching...
        synchronized (htRegistry) {
            Enumeration enum = htRegistry.elements() ;
            while (enum.hasMoreElements()) {
                ApplicationDescriptor desc = (ApplicationDescriptor) enum.nextElement() ;
                if (desc.getType() == iType) {
                    rc = desc ;
                    break ;
                }
            }
        }

        return rc ;
    }


    /**
     * look up all user applications
     *
     * @return ApplicationDescriptor[] of all registered user applications.
     */
    public ApplicationDescriptor[] getUserApplications()
        throws SecurityException
    {
        ApplicationDescriptor[] rc = null ;
        int iItems, i ;

        synchronized (htRegistry) {
            /*
             * Pass 1: Count number of items
             */
            iItems = 0 ;
            Enumeration enum = htRegistry.elements() ;
            while (enum.hasMoreElements()) {
                ApplicationDescriptor desc = (ApplicationDescriptor) enum.nextElement() ;
                if (desc.getType() == USER_APP) {
                    iItems++ ;
                }
            }

            /*
             * Pass 2: Build Array
             */
            rc = new ApplicationDescriptor[iItems] ;
            i = 0 ;
            enum = htRegistry.elements() ;
            while (enum.hasMoreElements()) {
                ApplicationDescriptor desc = (ApplicationDescriptor) enum.nextElement() ;
                if (desc.getType() == USER_APP) {
                    rc[i++] = desc ;
                }
            }
        }
        return rc ;
    }


    /**
     * look up all applications
     *
     * @return ApplicationDescriptor[] of all applications.
     */
    public ApplicationDescriptor[] getApplications()
        throws SecurityException
    {
        ApplicationDescriptor[] rc = null ;
        int iItems, i ;

        synchronized (htRegistry) {
            rc = new ApplicationDescriptor[htRegistry.size()] ;
            Enumeration enum = htRegistry.elements() ;
            i = 0 ;
            while (enum.hasMoreElements()) {
                ApplicationDescriptor desc = (ApplicationDescriptor) enum.nextElement() ;
                rc[i++] = desc ;
            }
        }
        return rc ;
    }

    /**
     * This method is useful to find out if an application of
     * type iAppType is registered. If the iAppType is USER_TYPE,
     * it will always return false as it needs more information to
     * find that out. Perhaps an exception should be thrown as
     * returning false is misleading.
     */
    public boolean isApplicationRegistered(int iAppType)
    {
        boolean bRet = false;
        if( iAppType > USER_APP )
        {
            bRet = m_vRegisteredApps.containsAppOfType( iAppType );
        }
        return bRet;
    }


    /**
     * Register an application as a particular application type
     *
     * @param strClassName the class name of the application
     * @param iType type constant to register as
     *
     * @exception IllegalArgumentException thrown if the iType is invalid or
     *            if the strClassName is null
     * @exception SecurityException thrown if the user/application does not
     *            have permissions to register as the specified type.
     *
     */
    public void registerApplication(int iType, String strClassName)
            throws IllegalArgumentException, SecurityException
    {
        // Validate iType
        if ((iType < APP_TYPE_MIN) || (iType > APP_TYPE_MAX)) {
            throw new IllegalArgumentException("application type is invalid") ;
        }

        // Validate the class name
        if (strClassName == null) {
            throw new IllegalArgumentException("class name is null") ;
        }

//System.out.println("register application: type=" + iType + ", name=" + strClassName) ;
        unregisterApplication(strClassName) ;
        synchronized (m_vRegisteredApps) {
            AppListItem item = new AppListItem(iType, strClassName) ;
            m_vRegisteredApps.removeAppListItem(item) ;
            m_vRegisteredApps.addAppListItem(item) ;
        }
    }


    /**
     * Removes an application from application registry
     *
     * @param strClassName the class name of the application
     *
     * @exception IllegalArgumentException thrown if the strClassName is null
     * @exception SecurityException thrown if the user/application does not
     *            have permissions to unregister the specified application
     */
    public void unregisterApplication(String strClassName)
            throws IllegalArgumentException, SecurityException
    {
        // Validate the class name
        if (strClassName == null) {
            throw new IllegalArgumentException("class name is null") ;
        }

        // Remove from load list
        if (m_vRegisteredApps != null) {
            m_vRegisteredApps.removeAppListItem(strClassName) ;
        }

        // Remove from Registry
        if (htRegistry != null) {
            ApplicationDescriptor descRemoved = null ;
            synchronized (htRegistry) {
                descRemoved = (ApplicationDescriptor) htRegistry.get(strClassName) ;
                htRegistry.remove(strClassName) ;
            }
            fireApplicationRemoved(descRemoved) ;
        }
    }


    /**
     * What is the registered application list?
     */
    protected Vector getAppList()
    {
        return m_vRegisteredApps.getApplicationList() ;
    }


    /**
     * retries loading applications listed in appURLS.
     */
    public static void retryLoadingApplications( String[] appURLs ){
        loadMultipleApplications( appURLs );
    }


    /**
     * This method is called from the embedded web server through its CGI
     * interface.
     */
    public static int cgiInstallApplication(String appURL)
    {
        //default return value is OK
        int iRet = CGI_INSTALL_APPLICATION_OK;


        if( isApplicationAlreadyInstalled( appURL ) ){
            iRet = CGI_INSTALL_APPLICATION_ERROR_APP_ALREADY_INSTALLED;

        }else if( isMaxUserApplicationQuotaReached() ){
            iRet =  CGI_INSTALL_APPLICATION_ERROR_EXCEEDED_MAXIMUM_ALLOWED;

        }else{
            iRet = loadApplication( appURL );
        }
         //System.out.println("Return code from java: " + iRet);
        return iRet;
    }

    /**
     * loads a single application with the URL appURL.
     */
    private static int loadApplication( String appURL){
        // See if we can install the application.
        int iRet = CGI_INSTALL_APPLICATION_OK;
        ApplicationRegistry registry = ApplicationRegistry.getInstance() ;
        ApplicationManager  appManager = ApplicationManager.getInstance() ;
        try {

            BulkApplicationLoader appLoader =
                new BulkApplicationLoader(appManager, registry) ;

            AppListItem item = new AppListItem(USER_APP, appURL) ;
            registry.registerApplication(USER_APP, appURL) ;
            try {
                if (registry.shouldLoadApplication(item)) {
                    appLoader.addApplication
                        (item.iAppType, item.strClassNameOrURL) ;
                    appLoader.load() ;
                }
                if (appLoader.getNumLoaded() == 1) {
                    appManager.preloadApplication(appURL) ;
                    appManager.startRegisteredApplication(appURL) ;
                } else {
                    if (appLoader.getNumFailed() > 0)
                    {
                      int iBulkRetCode = appLoader.getFailureCode(0); //get the failure code for this one app

                      //when retrieving the value from here Bulk app loader
                      //sets one of three values:
                      // 0 = OK
                      // 1 = TIMEOUT
                      // 2 = VERSIONFAILED
                      // -1 = general error
                      switch(iBulkRetCode)
                      {
                        case 0 : iRet =  CGI_INSTALL_APPLICATION_OK;
                                  break;
                        case 1 : iRet =  CGI_INSTALL_APPLICATION_ERROR_TIMEOUT;
                                  break;
                        case 2 : iRet =  CGI_INSTALL_APPLICATION_ERROR_VERSIONFAILED;
                                  break;
                        default : iRet =  CGI_INSTALL_APPLICATION_ERROR;
                      }
                    }
                }
            } catch (Exception e) {
                System.out.println
                    ("<<ERROR>> ApplicationRegistry: Unable to "+
                     " load Application: " + item.strClassNameOrURL) ;
                SysLog.log(e) ;
                iRet =  CGI_INSTALL_APPLICATION_ERROR;
            }
        } catch (Exception e) {
            System.out.println("cgiInstallApplication: Error installing application");

            // Dump out the stack trace so we have a
            //hope of figuring out what happened.
            SysLog.log(e);
            iRet =  CGI_INSTALL_APPLICATION_ERROR;
        }
        return iRet;
    }

    /**
     * loads multiple "user" applications identified by  appUrl in appUrls array.
     */
     private static void loadMultipleApplications( String[] appURLs){
        // See if we can install the application.
        ApplicationRegistry registry = ApplicationRegistry.getInstance() ;
        ApplicationManager  appManager = ApplicationManager.getInstance() ;

            BulkApplicationLoader appLoader =
                new BulkApplicationLoader(appManager, registry) ;
            for( int i=0; i<appURLs.length; i++){
                AppListItem item = new AppListItem(USER_APP, appURLs[i]) ;
                registry.registerApplication(USER_APP, appURLs[i]) ;
                if (registry.shouldLoadApplication(item)) {
                    appLoader.addApplication
                        (item.iAppType, item.strClassNameOrURL) ;
                }
            }
            try{
                appLoader.load() ;
                int iLoadedApps = appLoader.getNumLoaded();
                String[] loadedApps = appLoader.getLoadedApps();
                if( loadedApps != null ){
                    for( int i = 0; i<loadedApps.length; i++){
                        appManager.preloadApplication(loadedApps[i]) ;
                        appManager.startRegisteredApplication(loadedApps[i]) ;
                    }
                }
            } catch (Exception e) {
                SysLog.log(e) ;
            }
    }


    /**
     * This method is called from the embedded web server through its CGI
     * interface.
     */
    public static int cgiUninstallApplication(String appURL)
    {

        ApplicationRegistry registry = ApplicationRegistry.getInstance() ;
        int iRet =  CGI_UNINSTALL_APPLICATION_OK;
        // See if we can uninstall the application.
        try {
            registry.unregisterApplication(appURL) ;
            ApplicationManager.getInstance()
                .flushApplicationDescriptor(appURL) ;
        } catch (MalformedURLException e) {

            // They gave us a bogus URL
            System.out.println("cgiUninstallApplication: Error uninstalling application: Bad URL");
            iRet = CGI_UNINSTALL_APPLICATION_ERROR_URL;

        } catch (Exception e) {

            System.out.println("cgiUninstallApplication: Error uninstalling application");

            // Dump out the stack trace so we have
            // a hope of figuring out what happened.
            SysLog.log(e);
            iRet = CGI_UNINSTALL_APPLICATION_ERROR;
        }

        return iRet;
    }

    /**
     * Is the USER application as denoted by the strURL already installed?
     */
    public static boolean isApplicationAlreadyInstalled( String strURL ){
        boolean bRet = false;
        if( strURL != null ){
            String[] appList = cgiQueryInstalledApplications();
            if( appList != null ){
                for( int i = 0; i< appList.length; i++ ){
                    if( appList[i].equals(strURL.trim()) ){
                        bRet = true;
                        break;
                    }
                }
            }
        }
        return bRet;
    }

    /**
     * does the number of User Applications installed equals
     * the number allowed( this is set by MAX_USER_APPLICATIONS key
     * in settings.properties file ).
     * -1 denotes no limit.
     */
    public static boolean isMaxUserApplicationQuotaReached(){
        boolean bRet = false;
        String[] appList = cgiQueryInstalledApplications();
        int maxAllowed = getMaxUserApplicationsLimit();
        if( maxAllowed !=  UNLIMITED_QUOTA_TO_INSTALL_USER_APPS ){
            if( appList.length >= maxAllowed ){
                bRet = true;
            }
        }
        return bRet;
    }

    /**
     * gets the max quota of the user applications
     * that can be installed. If its a hard phone,
     * it will always return unlimited i.e. -1.
     */
    public static int getMaxUserApplicationsLimit(){
        int iRet = UNLIMITED_QUOTA_TO_INSTALL_USER_APPS;
        if( PingerApp.isTestbedWorld() ){
            //bypassing for developers in Pingtel
            //if KB equals yes
//            String keyBypass = System.getProperty("KB");
//            if (keyBypass == null)
//                keyBypass = "";
//            if( !(keyBypass.equalsIgnoreCase("YES"))){
              iRet = PingerInfo.getInstance().getAppLimit();
//                iRet = Settings.getInt("MAX_USER_APPLICATIONS",
//                                       UNLIMITED_QUOTA_TO_INSTALL_USER_APPS);
//            }
        }
        return iRet;
    }

    /**
     *  This gives you a list of USER APPS installed .
     */
    public static String[] cgiQueryInstalledApplications()
    {
        ApplicationRegistry registry = ApplicationRegistry.getInstance() ;
        Vector m_vRegisteredApps = registry.getAppList() ;
        String strRC[] = null ;

        if (m_vRegisteredApps != null) {
            synchronized (m_vRegisteredApps) {
                int iItems = 0 ;
                if (m_vRegisteredApps != null) {
                    for (int i=0; i<m_vRegisteredApps.size(); i++) {
                        AppListItem item = (AppListItem) m_vRegisteredApps.elementAt(i) ;
                        if (item.iAppType == USER_APP) {
                            iItems++ ;
                        }
                    }
                }

                if (iItems > 0) {
                    strRC = new String[iItems] ;

                    iItems = 0 ;
                    for (int i=0; i<m_vRegisteredApps.size(); i++) {
                        AppListItem item = (AppListItem) m_vRegisteredApps.elementAt(i) ;
                        if (item.iAppType == USER_APP) {
                            strRC[iItems++] = item.strClassNameOrURL ;
                        }
                    }
                }
            }
        }

        if (strRC == null)
            strRC = new String[0] ;

        return strRC ;
    }


    /**
     *  This gives you a list of all USER apps and the apps that have title
     *  and  icons.
     *  These follow the logic that shows the apps in
     *  the APPS tab in the phone.
     */
    public static String[] cgiQueryAllInstalledApplications()
    {
        String strRC[] = null ;
        Hashtable appListHash = new Hashtable();

        //first get the USER apps
        String[] userApps =
          ApplicationManager.getInstance().getLoadedApplications();
        for( int i = 0; i<userApps.length; i++ ){
          appListHash.put(userApps[i], userApps[i]);
        }

        //then get all the apps that have icons and titles
        //if they are already in the hash, just overwrite them.
        ApplicationDescriptor desc[] = ApplicationRegistry.getInstance().getApplications() ;
        for (int i=0; i<desc.length; i++) {
                String strTitle = desc[i].getTitle() ;
                String strIcon = desc[i].getLauncherIcon() ;

                // We cannot continue without an icon and title
                if ((strTitle != null) && (strTitle.trim().length() != 0) &&
                    (strIcon != null)  && (strIcon.trim().length() != 0)) {
                      appListHash.put(strTitle, strTitle);
                }
        }
        strRC = new String[appListHash.size()];
        Enumeration enum = appListHash.keys();
        int count = 0;
        while( enum.hasMoreElements() ){
          strRC[count++] = (String)( enum.nextElement() );
        }
        if (strRC == null)
            strRC = new String[0] ;

        return strRC ;
    }



    /**
     * This gives you an array of arrays( that consist of title and url of
     *  the USER apps installed. First it gets the list(urls) of installed apps.
     *  Please note that all of these apps may not have been loaded. Then it
     *  looks at the loaded applications and gets the titles from them.
     */
    public static String[][] cgiQueryInstalledApplicationsForTitleAndURL()
    {
        ApplicationRegistry registry = ApplicationRegistry.getInstance() ;
        ApplicationManager manager = ApplicationManager.getInstance() ;

        Vector m_vRegisteredApps = registry.getAppList() ;
        Vector vList = new Vector();

        if (m_vRegisteredApps != null) {
            synchronized (m_vRegisteredApps) {
              int iRegisteredAppSize = m_vRegisteredApps.size();
              for (int i=0; i<iRegisteredAppSize; i++) {
                  AppListItem item = (AppListItem) m_vRegisteredApps.elementAt(i) ;
                  if (item.iAppType == USER_APP) {
                      ApplicationDescriptor itemDescriptor = null;
                      try{
                        itemDescriptor =
                          manager.getApplicationDescriptor(item.strClassNameOrURL ) ;
                      }catch(Exception e ){
                         System.out.println
                          ("couldn't get title for the App with the url <"
                              + item.strClassNameOrURL + "> as it was not loaded");
                         //SysLog.log(e);
                      }
                      if( itemDescriptor != null ){
                        String[] itemTitleURL = new String[2];
                        itemTitleURL[0] = itemDescriptor.getTitle();
                        itemTitleURL[1] = item.strClassNameOrURL ;
                        vList.addElement( itemTitleURL );
                      }
                  }
               }
             }
         }
        String strRC[][] = new String[vList.size()][] ;
        Enumeration enum = vList.elements();
        int i = 0;
        while( enum.hasMoreElements()){
          strRC[i++] = (String[])(enum.nextElement());
        }
        return strRC ;
    }


    public void addApplication(String strClassNameOrURL)
    {
        BulkApplicationLoader appLoader = new BulkApplicationLoader(ApplicationManager.getInstance(), this) ;

        if (m_vRegisteredApps != null) {
            AppListItem item = new AppListItem(USER_APP, strClassNameOrURL) ;

// System.out.println("addApplication: " + item) ;

            try {
                if (shouldLoadApplication(item)) {
                    appLoader.addApplication(item.iAppType, item.strClassNameOrURL) ;
                }
            } catch (Exception e) {
                System.out.println("<<ERROR>> ApplicationRegistry: Unable to load Application: " + item.strClassNameOrURL) ;
                SysLog.log(e) ;
            }
            appLoader.load() ;
        }
    }


    public void addApplicationDescriptor(ApplicationDescriptor descriptor, String strClassNameOrURL)
        throws MalformedURLException
    {
        synchronized (htRegistry) {
            htRegistry.put(strClassNameOrURL, descriptor) ;
/*
            System.out.println("") ;
            System.out.println("addApplicationDescriptor: " + descriptor) ;
            System.out.println("  url      =" + strClassNameOrURL) ;
            System.out.println("  title    =" + descriptor.getTitle()) ;
            System.out.println("  codebase =" + descriptor.getCodebase()) ;
            System.out.println("  mainclass=" + descriptor.getMainClassName()) ;
*/

        }
        fireApplicationAdded(descriptor) ;
    }

    /**
     * Should we load the specified application?
     */
    public boolean shouldLoadApplication(AppListItem item)
    {
        boolean bRC = false ;

        if  (item.strClassNameOrURL.startsWith("http:")) {
            bRC = true ;
        } else if (item.strClassNameOrURL.startsWith("file:")) {
            bRC = true ;
        }

        return bRC ;
    }

    /**
     * Coordinate with the Application manager to obtain an application
     * descriptor for each application
     * If the USER apps number exceeds the quota, don't bother to
     * load the remaining USER apps and set boolean m_bMaxUserAppExceeded
     * to true.
     */
    public void populateApplicationDescriptors()
    {
        BulkApplicationLoader appLoader =
            new BulkApplicationLoader(ApplicationManager.getInstance(), this) ;
        int iMaxUserAppLimit = getMaxUserApplicationsLimit();
        if (m_vRegisteredApps != null) {
            Vector vApps = m_vRegisteredApps.getApplicationList() ;
            int iUserAppCount = 0;
            for (int i=0; i<vApps.size(); i++) {
                AppListItem item = (AppListItem) vApps.elementAt(i) ;
                System.out.println("Load List #" + i + ": type="
                                   + item.iAppType + ", name="
                                   + item.strClassNameOrURL) ;

                boolean bLoadApp = true;
                if( iMaxUserAppLimit != UNLIMITED_QUOTA_TO_INSTALL_USER_APPS ){
                    if( item.getType() == USER_APP ){
                        if( iUserAppCount >= iMaxUserAppLimit){
                            System.out.println
                                ("didn't load "+item.getClassNameOrURL() );
                            m_bMaxUserAppExceeded = true;
                            bLoadApp = false;
                        }
                        iUserAppCount++;
                    }
                }

                if (bLoadApp) {
                    try {
                        // Does the application NEED loading?
                        if (shouldLoadApplication(item)) {

                            // If so, then check system state and load if we
                            // should/can.
                            if (item.strClassNameOrURL.startsWith("file:") || item.strClassNameOrURL.startsWith("http:")) {
                                appLoader.addApplication(item.iAppType, item.strClassNameOrURL) ;
                            } else {
                                System.out.println("Network Unavailable, ignoring: " + item.strClassNameOrURL) ;
                            }
                        } else {
                            populateApplicationDescriptor
                                (item.getType(), item.getClassNameOrURL()) ;
                        }
                    } catch (Exception e) {
                        System.out.println("<<ERROR>> ApplicationRegistry: Unable to load Application: " + item.strClassNameOrURL) ;
                        SysLog.log(e) ;
                    }
                }
            }
            appLoader.load() ;
        }
    }


    /**
     * Add an application registry listener.  This listener will be notified
     * changes in applications.
     */
    public void addApplicationRegistryListener(ApplicationRegistryListener listener)
    {
        synchronized (m_vListeners) {
            if (!m_vListeners.contains(listener)) {
                m_vListeners.addElement(listener) ;
            }
        }
    }


    /**
     * Remove an application registry listener.  This listener will be
     * notified changes in applications.
     */
    public void removeApplicationRegistryListener(ApplicationRegistryListener listener)
    {
        synchronized (m_vListeners) {
            m_vListeners.removeElement(listener) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * Fire of an applicationAdded notification to all interested parties
     */
    protected void fireApplicationAdded(ApplicationDescriptor desc)
    {
        synchronized (m_vListeners) {
            int iLength = m_vListeners.size() ;
            for (int i=0; i<iLength; i++) {
                ApplicationRegistryListener listener = (ApplicationRegistryListener) m_vListeners.elementAt(i) ;
                if (listener != null) {
                    try {
                        listener.applicationAdded(desc) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }


    /**
     * Fire of an applicationRemoved notification to all interested parties
     */
    protected void fireApplicationRemoved(ApplicationDescriptor desc)
    {
        synchronized (m_vListeners) {
            int iLength = m_vListeners.size() ;
            for (int i=0; i<iLength; i++) {
                ApplicationRegistryListener listener = (ApplicationRegistryListener) m_vListeners.elementAt(i) ;
                if (listener != null) {
                    try {
                        listener.applicationRemoved(desc) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }


    protected ApplicationDescriptor populateApplicationDescriptor(int iType, String strClassNameOrURL)
        throws MalformedURLException
    {
        ApplicationDescriptor descriptor = null ;
        ApplicationManager    appManager = ApplicationManager.getInstance() ;


        // REMOTE Applications
        if ((strClassNameOrURL.startsWith("http:")) || (strClassNameOrURL.startsWith("file:"))) {
            // Have the App Manager look up the application descriptor
            URL url = new URL(strClassNameOrURL) ;
            descriptor = appManager.getApplicationDescriptor(url) ;
            if (descriptor != null) {
                descriptor.setType(iType) ;
                addApplicationDescriptor(descriptor, strClassNameOrURL) ;
            } else
                System.out.println("<<ERROR>> ApplicationRegistry: unable to get application descriptor: " + url) ;
        }
        // LOCAL or embedded applications
        else {
            descriptor = appManager.getApplicationDescriptor(strClassNameOrURL) ;
            try {
                if (descriptor != null) {
                    descriptor.setType(iType) ;
                    addApplicationDescriptor(descriptor, strClassNameOrURL) ;
                } else
                    System.out.println("<<ERROR>> ApplicationRegistry: unable to get application descriptor: " + strClassNameOrURL) ;
            } catch (IllegalArgumentException e) {
                SysLog.log(e) ;
            }
        }

        return descriptor ;
    }

    /** Is the maximum user-applications limit exceeded
     * while loading apps?
     */
    public boolean isMaxUserAppLimitExceeded(){
        return  m_bMaxUserAppExceeded;
    }

    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////

    private class icRuntimeApplicationList
    {
        Vector m_vList ;
        ApplicationRegistryParser m_parser ;


        public icRuntimeApplicationList()
        {
            m_vList = new Vector(32) ;
            m_parser = ApplicationRegistryParser.getInstance() ;

            initializeInternalApplications() ;
            overlayInstalledApplications() ;
            //also overlay applications from app-config that resides in the
            //classpath e.g. pingtel.jar or resources.jar .
            overlayApplicationsFromResourcesDotJar();
        }


        public Vector getApplicationList()
        {
            return  m_vList ;
        }

        /**
         * This method is useful to find out if an application of
         * type iAppType is registered. If the iAppType is USER_TYPE,
         * it will always return false as it needs more information to
         * find that out. Perhaps an exception should be thrown as
         * returning false is misleading.
         */
        public boolean containsAppOfType(int iAppType)
        {
            boolean bRet = false;
            if( iAppType > USER_APP )
            {
                for (int i=0; i<m_vList.size(); i++)
                {
                    AppListItem item = (AppListItem) m_vList.elementAt(i) ;
                    if ((item != null) && item.getType() == iAppType)
                    {
                        bRet = true;
                        break;
                    }
                }
            }
            return bRet;
        }


        protected void overlayInstalledApplications()
        {
            AppListItem items[] = m_parser.getAppList() ;
            if (items != null)
            {
                for (int i=0; i<items.length; i++) {
                    addAppListItem(items[i]) ;
                }
            }
        }

        /**
         * The default value for whether to flush the app-config file is true.
         */
        public synchronized void addAppListItem(AppListItem item)
        {
            addAppListItem( m_parser, item, true );

        }




        public synchronized void removeAppListItem(AppListItem item)
        {
// System.out.println("icRuntimeApplicationList.removeAppListItem: " + item) ;
            for (int i=0; i<m_vList.size(); i++) {
                AppListItem itemReplace = (AppListItem) m_vList.elementAt(i) ;
                if ((itemReplace != null) && item.equals(itemReplace)) {
// System.out.println("Removing: " + itemReplace.getClassNameOrURL()) ;
                    m_vList.removeElementAt(i) ;
                    i-- ;
                }
            }
            m_parser.removeAppListItem(item) ;
            m_parser.flush() ;
        }


        public synchronized void removeAppListItem(String strClassnameOrURL)
        {
            for (int i=0; i<m_vList.size(); i++) {
                AppListItem itemReplace = (AppListItem) m_vList.elementAt(i) ;
                if ((itemReplace != null) && strClassnameOrURL.equals(itemReplace.getClassNameOrURL())) {
                    m_vList.removeElementAt(i) ;
                    m_parser.removeAppListItem(itemReplace) ;
                    i-- ;
                }
            }
            m_parser.flush() ;
        }


        protected synchronized void addAppListItem
            (ApplicationRegistryParser appRegistryParser, AppListItem item, boolean bFlushFile)
        {
        //System.out.println("icRuntimeApplicationList.addAppListItem: " + item) ;

            if  (   (item.getType() != UNKNOWN) &&
                    (item.getType() != CORE_APP) &&
                    (item.getType() != USER_APP)) {

                // Before adding, check to see if this is overriding any existing
                // items.  If we are overriding  something, remove that item.
                for (int i=0; i<m_vList.size(); i++) {
                    AppListItem itemReplace = (AppListItem) m_vList.elementAt(i) ;
                    if ((itemReplace != null) && (itemReplace.getType() == item.getType())) {
                        System.out.println("Application Replacement: " + item.getClassNameOrURL() + " overrides " + itemReplace.getClassNameOrURL()) ;

                        m_vList.removeElementAt(i) ;
                        i-- ;
                    } else if (itemReplace.equals(item)) {
                        m_vList.removeElementAt(i) ;
                        i-- ;
                    }
                }
                //if USER_APP, and the item is same, replace it too.
                //would be efficient not to add, rather than replacing and
                //adding
            }else if( item.getType() == USER_APP){
                for (int i=0; i<m_vList.size(); i++) {
                    AppListItem itemReplace = (AppListItem) m_vList.elementAt(i) ;
                    if (itemReplace.equals(item)) {
                        m_vList.removeElementAt(i) ;
                        i-- ;
                    }
                }
            }
            //if the url is null, remove that item. We already removed the
            //item with same type( if not USER type)  and/or same value.
            //don't add it again if the url is null.
            if( item.getClassNameOrURL() != null )
            {
                m_vList.addElement(item) ;
                appRegistryParser.addAppListItem(item) ;
            }
            if( bFlushFile )
            {
                appRegistryParser.flush() ;
            }
        }

        protected void initializeInternalApplications()
        {
            m_vList.addElement(new AppListItem(CALL_LOG_APP,  "org.sipfoundry.sipxphone.app.CallLogApp")) ;
            m_vList.addElement(new AppListItem(SPEED_DIAL_APP, "org.sipfoundry.sipxphone.app.SpeeddialApp")) ;
            m_vList.addElement(new AppListItem(DIAL_BY_URL_APP, "org.sipfoundry.sipxphone.app.DialByURLApp")) ;
            m_vList.addElement(new AppListItem(VOLUME_ADJUST_APP, "org.sipfoundry.sipxphone.app.ChangeVolumeApp")) ;
            m_vList.addElement(new AppListItem(CONFERENCE_APP, "org.sipfoundry.sipxphone.app.ConferenceApp")) ;
            m_vList.addElement(new AppListItem(PREFERENCES_APP, "org.sipfoundry.sipxphone.app.DevicePreferencesApp")) ;
            m_vList.addElement(new AppListItem(CORE_APP, "org.sipfoundry.sipxphone.app.LogoApp")) ;

            String strDefaultTransferMethod = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_TRANSFER_METHOD) ;
            if ((strDefaultTransferMethod != null) && strDefaultTransferMethod.equalsIgnoreCase("BLIND"))
            {
                m_vList.addElement(new AppListItem(TRANSFER_APP, "org.sipfoundry.sipxphone.app.BlindTransferApp")) ;
            }
            else
            {
                m_vList.addElement(new AppListItem(TRANSFER_APP, "org.sipfoundry.sipxphone.app.ConsultativeTransferApp")) ;
            }
        }

        /**
         * couldn't come up with a better name for this method.
         * This loads the app-config that resides in the classpath e.g.
         * pingtel.jar or resources.jar. The app names loaded from here
         * do not get written to the default app-config.
         */
        protected void overlayApplicationsFromResourcesDotJar()
        {
            /**
             * Is app-config file in the classpath? Then get the inputstream if it is in the classpath.
             * If the inputstream is not null, we will load this app-config which resides
             * in the classpath i.e. the one which may reside in resources.jar or pingtel.jar.
             */
            InputStream inputStream = ClassLoader.getSystemResourceAsStream
                    (ApplicationRegistryParser.APPLICATION_LIST);
            //remember we are not using the singleton instance of ApplicationRegistryParser.
            //we are creating a new instance.
            ApplicationRegistryParser appConfigParser = new ApplicationRegistryParser(inputStream);
            AppListItem items[] = appConfigParser.getAppList() ;
            if (items != null)
            {
                final boolean bDoNotFlushFile = false;
                for (int i=0; i<items.length; i++)
                {
                    addAppListItem(appConfigParser, items[i], bDoNotFlushFile) ;
                }
            }
        }


        public void dump()
        {
            System.out.println("") ;
            System.out.println("") ;
            System.out.println("Running Application List Dump:") ;
            System.out.println("") ;
            for (int i=0; i<m_vList.size(); i++) {
                AppListItem item = (AppListItem) m_vList.elementAt(i) ;
                System.out.println(Integer.toString(i+1) + ": " + item) ;
            }

        }
    }

//////////////////////////////////////////////////////////////////////////////
// Debug Methods
////
    public void dump()
    {
        Enumeration enumDescriptors = htRegistry.elements() ;
        while (enumDescriptors.hasMoreElements()) {
            ApplicationDescriptor descriptor = (ApplicationDescriptor) enumDescriptors.nextElement() ;
        }
    }


}
