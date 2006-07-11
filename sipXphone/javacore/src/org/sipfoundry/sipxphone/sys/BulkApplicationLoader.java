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

import java.util.Vector ;
import java.awt.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.Application ;
import org.sipfoundry.sipxphone.sys.appclassloader.ApplicationManager ;
import org.sipfoundry.sipxphone.sys.startup.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.util.NetUtilities ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.featureindicator.* ;


/**
 * The Bulk Application Loader is used by the application registry to load
 * multiple applications in parallel.  This is accomplished by creating
 * WORKER_THREADS worker threads which go ahead and load the applications.
 * <br><br>
 * This application also adds a status feature indicator to display status
 * on load completion.  If any applications fail to load, a failure indicator
 * is added which then allows the end user to uninstall the unloadable
 * applications.
 * <br><br>
 * NOTE: This only works if all of the applications are added before load() is
 * execute.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class BulkApplicationLoader
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /**
     * The number of worker thread that will be created to handle application loading
     */
    public static final int WORKER_THREADS = 4 ;

    public static final int COMPLETION_CODE_SUCCESS = 0 ;
    public static final int COMPLETION_CODE_TIMEOUT = 1 ;
    public static final int COMPLETION_CODE_VERSION_FAILED = 2 ;
    public static final int COMPLETION_CODE_APP_WITH_MAINCLASSNAME_ALREADY_LOADED = 3 ;
    public static final int COMPLETION_CODE_FILE_NOT_FOUND = 4 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private ApplicationManager      m_applicationManager ;  // Reference to Application Manager
    private ApplicationRegistry     m_applicationRegistry ;        // Reference to Application Registry
    private icWorkerThread          m_workers[] ;       // Active Worker Thread
    private CountingSemaphore       m_csLoadList ;      // Load list Semaphore
    private CountingSemaphore       m_csCompletion ;    // Synchronization for completion
    private Vector                  m_vLoadList ;       // THE load list
    private Vector                  m_vLoaded ;         // Apps that loaded successfully
    private Vector                  m_vFailed ;         // Apps that failed to load
    private icLoadingStatus         m_indicatorStatus ; // The Status/progress indicator
    private AppResourceManager      m_resourceManager ;          // Application Resource Manager


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public BulkApplicationLoader(ApplicationManager manager, ApplicationRegistry registry)
    {
        m_applicationManager = manager ;
        m_applicationRegistry = registry ;

        m_csLoadList = new CountingSemaphore(0, false) ;
        m_workers = new icWorkerThread[WORKER_THREADS] ;
        m_vLoadList = new Vector() ;
        m_vLoaded = new Vector() ;
        m_vFailed = new Vector() ;
        m_indicatorStatus = null ;

        for (int i=0; i<WORKER_THREADS; i++) {
            m_workers[i] = new icWorkerThread(i+1) ;
        }

        m_resourceManager = AppResourceManager.getInstance() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Add an application that should be loaded.  You should add all of the
     * applications that you want to load BEFORE invoking load()
     */
    public void addApplication(int iType, String strClassNameOrURL)
    {
        synchronized (m_vLoadList) {
            m_vLoadList.addElement(new icLoadRequest(iType, strClassNameOrURL)) ;
            m_csLoadList.up() ;
        }
    }


    /**
     * Load all of the previously added applications.  Do not call
     * addApplication after invoking load() or bad things may happen.
     */
    public void load()
    {
        int iApplications = m_vLoadList.size() ;

        if (iApplications > 0) {

            // Display Application Loading Status/progress
            m_indicatorStatus = new icLoadingStatus(iApplications) ;
            m_indicatorStatus.install() ;

            try {
                // We are down then all of the items in the load list are consumed.
                m_csCompletion = new CountingSemaphore(((WORKER_THREADS*-1) + 1), false) ;
                for (int i=0;i<WORKER_THREADS; i++) {
                    m_workers[i].start() ;

                    // Also perform one extra up per thread.  This will nudge the worker
                    // threads to exit when complete.
                    m_csLoadList.up() ;
                }

                // Wait for all the workers to complete
                m_csCompletion.down() ;
            } finally {
                // No matter what, remove the indicator status.

                m_indicatorStatus.remove() ;
            }

            // If anything failed, communicate that to the user
            if (getNumFailed() > 0) {
                icFailureStatus indicatorFailure = new icFailureStatus() ;
                indicatorFailure.installIndicator() ;
            }
        }
    }


    /**
     * How many applications have been loaded so far?
     */
    public int getNumLoaded()
    {
        return m_vLoaded.size() ;
    }

    /**
     * gets the array of urls of loaded apps.
     */
    public String[] getLoadedApps(){
        String[] loadedApps = null;
        synchronized(m_vLoaded){
            loadedApps = new String[m_vLoaded.size()];
            for( int i = 0; i<m_vLoaded.size(); i++ ){
                loadedApps[i] = ((icLoadRequest)m_vLoaded.elementAt(i)).getClassNameOrURL();
            }
        }
        return loadedApps;
    }

    /**
     * How many applications have failed to load so far?
     */
    public int getNumFailed()
    {
        return m_vFailed.size() ;
    }

    /**
     * From the failed apps, return the result for the indicated app
     * returns -1 if search failed  (ie 10 aps failed but you asked for 11)
     */
    public int getFailureCode(int appnum)
    {
        int retVal = -1;
        if (appnum < m_vFailed.size() && appnum >= 0)
        {
            try
            {
                icLoadRequest request = (icLoadRequest) m_vFailed.elementAt(appnum) ;
                if (request != null)
                {
                    retVal = request.m_iCode;
                }
            }
            catch (Exception e)
            {
                retVal = -1;
            }
        }

        return retVal;
    }

    /**
     * From the failed apps, return the required version for the specified app
     * returns null if search failed  (ie 10 aps failed but you asked for 11)
     */
    public String getRequiredVersion(int appnum)
    {
        String retVal = null;
        if (appnum < m_vFailed.size() && appnum >= 0)
        {
            try
            {
                icLoadRequest request = (icLoadRequest) m_vFailed.elementAt(appnum) ;
                if (request != null)
                {
                    retVal = request.m_strRequiredVersion;
                }
            }
            catch (Exception e)
            {
                retVal = null;
            }
        }

        return null;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * @return the load requests that have failed.
     */
    protected Vector getFailures()
    {
        return m_vFailed ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////

    /**
     * This class exists purely for data encapsulation.  You sort of miss
     * good old c-style structure at times like this.
     */
    private class icLoadRequest
    {

        public int      m_iAppType ;            // The type of application/load request
        public String   m_strClassNameOrURL ;   // Class name or URL
        public int      m_iCode ;   // completion code (see bulk loader for defines)
        public String   m_strRequiredVersion = "Unknown";

        /**
         * Convenience Constructor
         */
        public icLoadRequest(int iAppType, String strClassNameOrURL)
        {
            m_iCode = COMPLETION_CODE_SUCCESS;
            m_iAppType = iAppType ;
            m_strClassNameOrURL = strClassNameOrURL ;
        }

        /**
         * @return the application type
         */
        public int getType()
        {
            return m_iAppType ;
        }


        /**
         * @return the class name or url
         */
        public String getClassNameOrURL()
        {
            return m_strClassNameOrURL ;
        }
    }


    /**
     * This worker thread that is responsible for loading applications.  The
     * thread pulls its next job out of the vLoadList vector and simply exits
     * when the vector is empty.
     */
    private class icWorkerThread extends Thread
    {
        private int m_iID = 0 ;

        public icWorkerThread(int id)
        {
            m_iID = id ;
        }


        public void run()
        {
            boolean bDone = false ;
            debug("Worker " + m_iID + ": BEGIN") ;
            while (!bDone) {
                icLoadRequest request = null ;

                // Get the next load request
                m_csLoadList.down() ;
                synchronized (m_vLoadList) {
                    if (m_vLoadList.size() > 0) {
                        request = (icLoadRequest) m_vLoadList.elementAt(0) ;
                        m_vLoadList.removeElementAt(0) ;
                    }
                }

                // If not empty, process it, otherwise we are done.
                if (request != null) {
                    debug("Worker " + m_iID + ": GOT JOB: " + request.m_strClassNameOrURL) ;
                    debug("") ;
                    try {
                        String strClassNameOrURL = request.getClassNameOrURL() ;
                        int iType = request.getType() ;

                        ApplicationDescriptor descriptor
                            = m_applicationManager.getApplicationDescriptor(strClassNameOrURL) ;

                        debug("      Worker " + m_iID + ": Loading: " + strClassNameOrURL) ;

                        // a null descriptor is considered failure
                        if ( descriptor == null )
                            m_vFailed.addElement(request) ;
                        else{
                            debug( ("\n********* bulk app loader trying to load *************")
                                  +("\n      descriptor.title           : " + descriptor.getTitle())
                                  +("\n      descriptor.codebase        : " + descriptor.getCodebase())
                                  +("\n      descriptor.classname       : " + descriptor.getMainClassName())
                                  +("\n      descriptor.RequiredVersion : " + descriptor.getRequiredVersion()) ) ;

                            boolean bShouldBeLoaded = true;

                            //Compare the apps Required Version to the xpressa version
                            //getRequiredVersion will return null if the developer never put it in the jar
                            //In a future version of this code we will want to fail apps that return null
                            //for now, we let them go.
                            if ( descriptor.getRequiredVersion() != null &&
                                !compareAppVersionOnXpressa(descriptor.getTitle(),
                                                            descriptor.getRequiredVersion())  )
                            {
                                  System.out.println("***** APP FAILED REQUIRED VERSION: " + descriptor.getTitle());
                                  request.m_iCode = COMPLETION_CODE_VERSION_FAILED;  //set failure code
                                  String strRequiredVersion = descriptor.getRequiredVersion();
                                  request.m_strRequiredVersion = "Unknown";
                                  if (strRequiredVersion != null)
                                      request.m_strRequiredVersion = strRequiredVersion;
                                  bShouldBeLoaded = false;
                            }

                            //needs to synchronize on m_vLoaded as we need to track
                            //if there is already an app with same class name thats
                            //already installed.
                            synchronized( m_vLoaded ){

                                //check if an app with mainClassName is already loaded.
                                if( bShouldBeLoaded ){

                                    if( isApplicationWithSameClassNameAlreadyLoaded(descriptor) ){
                                        request.m_iCode =
                                                COMPLETION_CODE_APP_WITH_MAINCLASSNAME_ALREADY_LOADED;
                                        bShouldBeLoaded = false;
                                    }
                                }

                                //if the app can now be safely loaded, load it
                                if( bShouldBeLoaded ){
                                    descriptor.setType(iType) ;
                                    m_applicationRegistry.addApplicationDescriptor(descriptor, strClassNameOrURL) ;
                                    m_vLoaded.addElement(request) ;
                                }
                            }

                            //otherwise put ut in the failed app list.
                            if( ! bShouldBeLoaded ){
                                m_vFailed.addElement(request);
                            }

                        }
                    }

                    catch (IllegalArgumentException iae)
                    {
                        if (iae.getMessage().startsWith("JAR not found:"))
                        {
                            request.m_iCode = COMPLETION_CODE_FILE_NOT_FOUND;
                            m_vFailed.addElement(request) ;
                        }
                        iae.printStackTrace();
                    }
                    catch (Exception e) {
                        request.m_iCode = COMPLETION_CODE_TIMEOUT;
                        e.printStackTrace();
                        m_vFailed.addElement(request) ;
                    }

                    if (m_indicatorStatus != null) {
                        m_indicatorStatus.refresh() ;
                    }
                } else {
                    bDone = true ;
                }
            }
            m_csCompletion.up() ;
            debug("Worker " + m_iID + ": DONE") ;

            // It *appears* that TCP connections used for loading apps are
            // not closed until the objects are garbage collected.
            Runtime.getRuntime().gc() ;
            Runtime.getRuntime().runFinalization() ;
        }



    }//end of inner class icWorkerThread.


    /**
     * Feature Indicator that displays the load status.  This feature indicator
     * is very simple and displays a "loading... (x/max)" type of message.
     */
    private class icLoadingStatus implements FeatureIndicator
    {
        private int m_iApps ;   // number of apps that we are loading

        /**
         * Construct the status indicator with the max number of apps that
         * we will be loading.
         */
        public icLoadingStatus(int iApps)
        {
            m_iApps = iApps ;
        }


        /**
         * Install the feature indicator
         */
        public void install()
        {
            Shell.getFeatureIndicatorManager().installIndicator(this, FeatureIndicatorManager.VIEWSTYLE_LINE) ;
        }


        /**
         * Refresh the feature indicator
         */
        public void refresh()
        {
            Shell.getFeatureIndicatorManager().refreshIndicator(this) ;
        }


        /**
         * Remove/uninstall the feature indicator
         */
        public void remove()
        {
            Shell.getFeatureIndicatorManager().removeIndicator(this) ;
        }


        /**
         *
         */
        public Image getIcon()
        {
            return null ;
        }


        /**
         *
         */
        public String getShortDescription()
        {
            return null ;
        }


        /**
         *
         */
        public Component getComponent()
        {
            PLabel label = new PLabel(buildStatusString()) ;
            label.setAlignment(PLabel.ALIGN_WEST) ;
            label.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;

            return label ;
        }


        /**
         *
         */
        public String getHint()
        {
            return m_resourceManager.getString("lblAppLoadStatusHint") ;
        }


        /**
         *
         */
        public void buttonPressed()
        {
        }


        /**
         * Builds the status string presented to the end user
         */
        protected String buildStatusString()
        {
            String strRC = "" ;
            String strMaxApps = Integer.toString(m_iApps) ;

            int iLoaded = getNumLoaded() ;
            int iFailed = getNumFailed() ;

            strRC = m_resourceManager.getString("lblAppLoadStatusPrompt") +
                    " " + Integer.toString(m_iApps-iLoaded) ;

            return strRC ;
        }

    }


    /**
     * Feature Indicator displayed to inform the user of application load
     * failures.
     */
    private class icFailureStatus implements FeatureIndicator
    {
        /**
         * Install the feature indicator
         */
        public void installIndicator()
        {
            Shell.getFeatureIndicatorManager().installIndicator(this, FeatureIndicatorManager.VIEWSTYLE_LINE) ;
        }

        /**
         * remove the feature indicator
         */
         public void removeIndicator()
        {
            Shell.getFeatureIndicatorManager().removeIndicator(this) ;
        }


        /**
         * Install the feature indicator
         */
        public Image getIcon()
        {
            return null ;
        }



        /**
         *
         */
        public String getShortDescription()
        {
            return null ;
        }
        /**
         *
         */
        public Component getComponent()
        {
            PLabel label = new PLabel(buildStatusString()) ;
            label.setAlignment(PLabel.ALIGN_EAST) ;
            label.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;

            return label ;
        }


        /**
         *
         */
        public String getHint()
        {
            return m_resourceManager.getString("lblAppLoadFailureHint") ;
        }


        /**
         * On button press, tell the user about the various failures... and kick out.
         */
        public void buttonPressed()
        {
            icReportFailuresForm form = new icReportFailuresForm
                (ShellApp.getInstance().getCoreApp(),
                 m_resourceManager.getString("lblAppLoadReportFailureTitle")) ;

            // No need to be modal....
            form.showModeless() ;
            removeIndicator() ;
        }


        /**
         * Builds the status string presented to the end user
         */
        protected String buildStatusString()
        {
            String strRC = "" ;
            String strFailedApps = Integer.toString(getNumFailed()) ;

            if (getNumFailed() > 1)
                strRC = strFailedApps + " " + m_resourceManager.getString("lblMultipleAppLoadFailurePrompt") ;
            else
                strRC = strFailedApps + " " + m_resourceManager.getString("lblSingleAppLoadFailurePrompt") ;


            return strRC ;
        }
    }


    /**
     * Display the application loading failure information and gives the user
     * the change to uninstall failed applications
     */
    private class icReportFailuresForm extends SimpleTextForm
    {
        public icReportFailuresForm(Application parent, String strTitle)
        {
            super(parent, strTitle) ;

            initText() ;
            initMenus() ;
        }


        /**
         * Initialize the status/content text
         */
        private void initText()
        {
            setText(m_resourceManager.getString("lblAppLoadReportFailurePreload") + generateFailureList()) ;
        }


        /**
         * Initialize menus
         */
        private void initMenus()
        {
            PBottomButtonBar menuControl = getBottomButtonBar() ;
            PMenuComponent menuComponent = getLeftMenuComponent() ;
            menuComponent.removeAllItems() ;

            PActionItem actionRetry = new PActionItem( new PLabel("Retry"),
                    "Retry | Retry loading failed applications.",
                    new icRetryHandler(),
                    "ACTION_RETRY") ;
            menuControl.setItem(PBottomButtonBar.B2, actionRetry) ;
            menuComponent.addItem(actionRetry) ;

            PActionItem actionUninstall = new PActionItem( new PLabel("Uninstall"),
                    "Uninstall the listed applications",
                    new icUninstallHandler(),
                    "ACTION_UNINSTALL") ;
            menuControl.setItem(PBottomButtonBar.B1, actionUninstall) ;
            menuComponent.addItem(actionUninstall) ;
        }


        /**
         * @return string containing the list of failed applications
         */
        private String generateFailureList()
        {
            StringBuffer results = new StringBuffer() ;

            Vector vFailures = getFailures() ;
            for (int i=0; i<vFailures.size(); i++) {
                icLoadRequest request = (icLoadRequest) vFailures.elementAt(i) ;
                if (request != null) {
                    results.append(request.m_strClassNameOrURL) ;

                    if (request.m_iCode == COMPLETION_CODE_APP_WITH_MAINCLASSNAME_ALREADY_LOADED){
                      results.append("\nDuplicate Main classname.\n Another application with "+
                                       " the same main classname is already loaded. ");

                    }else if (request.m_iCode == COMPLETION_CODE_VERSION_FAILED){
                      results.append("\nIncompatible version.\nRequires sipXphone v" +
                                        request.m_strRequiredVersion + "\n") ;

                    }else if (request.m_iCode == COMPLETION_CODE_FILE_NOT_FOUND){
                      results.append("\napplication not found.") ;
                    }else if (request.m_iCode == COMPLETION_CODE_TIMEOUT){
                      results.append("\nTimeout loading.") ;
                    }
                    results.append("\n\n") ;
                }
            }
            return results.toString() ;
        }


        /**
         * Simple Exit handler
         */
        private class icRetryHandler implements PActionListener
        {
            public void actionEvent(PActionEvent event)
            {
                if (event.getActionCommand().equals("ACTION_RETRY")) {
                    closeForm() ;
                    Vector failedVector = m_vFailed;
                    if( failedVector.size() > 0 ){
                        String[] failedApps = new String[failedVector.size()];
                        for( int i = 0; i<failedVector.size(); i++ ){
                            icLoadRequest request = (icLoadRequest) failedVector.elementAt(i) ;
                            failedApps[i] = request.getClassNameOrURL();
                        }
                        //there is a little loop here, as this method  creates an
                        //instance of BulkApplicatonLoader and calls methods in it.
                        ApplicationRegistry.getInstance().retryLoadingApplications(failedApps);
                    }

                }
            }
        }


        /**
         * Uninstall Handler.  This will uninstall all of the applications in
         * the failure list (after confirmation from the user).
         */
        private class icUninstallHandler implements PActionListener
        {
            public void actionEvent(PActionEvent event)
            {
                if (event.getActionCommand().equals("ACTION_UNINSTALL")) {
                    MessageBox alert = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_WARNING) ;
                    alert.setMessage(m_resourceManager.getString("lblAppLoadReportFailureUninstallWarning")) ;

                    if (alert.showModal() == MessageBox.OK) {
                        Vector vFailures = getFailures() ;
                        for (int i=0; i<vFailures.size(); i++) {
                            icLoadRequest request = (icLoadRequest) vFailures.elementAt(i) ;
                            if (request != null) {
                                m_applicationRegistry.unregisterApplication(request.m_strClassNameOrURL) ;
                            }
                        }
                        closeForm() ;
                    }
                }
            }
        }
    }

     /**
     * Compares xpressa version against the version of the app that is passed in.
     * if the app version is greater than of equal to the version of xpressa then the
     * function returns true
     *
     * @author Daniel Winsor
     */

    private boolean compareAppVersionOnXpressa(String title, String appVersion)
    {
        String strXpressaVersionOnPhone;

        strXpressaVersionOnPhone = Shell.getInstance().getXpressaSettings().getVersion();
        int iBuildNumber = Shell.getInstance().getXpressaSettings().getBuildNumber();

        String strFullXpressaVersion = VersionUtils.buildVersionString(strXpressaVersionOnPhone,iBuildNumber);

        // the next function returns
        //         * @return < 0 if strVersion1 <  strVersion2,
        //         *         > 0 if strVersion1 >  strVersion2,
        //         *         = 0 if strVersion2 == strVersion2
        System.out.println("AppVersion: " + appVersion + "  Xpressa Version: " + strFullXpressaVersion);
        if (VersionUtils.compareVersions(strFullXpressaVersion, appVersion) < 0)
          return false;

        return true;
    }

    /**
     * Is any other application with the same main classname already loaded?
     */
    private  boolean isApplicationWithSameClassNameAlreadyLoaded
                                (ApplicationDescriptor descriptorToCheck){
        boolean bRet = false;
        ApplicationDescriptor[] alreadlyLoadedApps = m_applicationRegistry.getApplications();
        for( int i=0; i<alreadlyLoadedApps.length; i++ ){
            ApplicationDescriptor descriptor = alreadlyLoadedApps[i];
            if( descriptor.getMainClassName().equals(descriptorToCheck.getMainClassName()) ){
                bRet = true;
                break;
            }
        }
        return bRet;

    }



    private static boolean m_sDebug = false;
    private static void debug( String str ){
        if( m_sDebug )
            System.out.println(str);
    }


}
