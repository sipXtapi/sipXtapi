/*
 * $Id$
 *
 * Copyright (C) 2004 Pingtel Corp.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys.appclassloader ;

import java.util.* ;
import java.net.*;
import java.lang.* ;
import java.io.*;

import org.sipfoundry.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.app.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.hook.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.app.shell.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.*;
import org.sipfoundry.util.*;

import java.lang.reflect.* ;

/**
 * <p>The application manager keeps track of which applications are running
 * and a list of what forms are up (and in what stacking order) for each
 * application.
 *
 * <p>All application loads are performed through this manager.  This is the
 * place where an application would be serialized from disk.  Generally,
 * applications will serialize themself to disk or the application may force
 * a save if a low memory conditions are detected.
 *
 * <p>All of the methods in this class should be package protected and only
 * accessible from within the teleping.sys package.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ApplicationManager
{

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** current list of loaded application */
    protected icApplicationDescriptorList m_vApplications ;

    /** singleton instance of the application manager */
    protected static ApplicationManager m_reference = null ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * protected constructor.  This class is implemented as a singleton and
     * use getInstance() to obtain a reference to the application manager.
     */
    protected ApplicationManager()
    {
        m_vApplications = new icApplicationDescriptorList() ;
        m_ApplicationDescriptors = new Hashtable();
        m_ApplicationMainClassInstances = new Hashtable();
        classloaders = new Hashtable() ;

        // Commented out as part of OPENDEV PORT, may not need
        URL.setURLStreamHandlerFactory(new XpressaStreamHandlerFactory());
    }


    /**
     * get a reference to our application manager
     */
    public static ApplicationManager getInstance()
    {
        if (m_reference == null) {
            m_reference = new ApplicationManager() ;
        }
        return m_reference ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Method
////
    public boolean activateApplication(String strAppName)
    {
        return activateApplication(strAppName,null);
    }


    /**
     */
    public boolean activateApplication(String strAppName, String[] params)
    {
        ShellApp shellApp = ShellApp.getInstance() ;

        boolean bRC = false ;

        Application app = getApplicationByKey(strAppName) ;
        if (app != null) {
            // The application was already running, just activate it.
            PForm form = getActiveForm(app) ;
            if (form != null) {      // It is possible that the app does not have a GUI...
                if (((PAbstractForm) form).isStacked())
                {
                    shellApp.postEvent(new ShellEvent(ShellEvent.SE_SHOWFORM, this, form, null)) ;
                }
            }
            bRC = true ;
        } else {
            app = loadApplication(strAppName, params) ;
            if (app != null) {
                bRC = true ;
            }
        }

        return bRC ;
    }

    /**
     *
     */
    public boolean activateApplication(ApplicationDescriptor descriptor)
    {
        return activateApplication(descriptor,null);
    }


    /**
     *
     */
    public boolean activateApplication(ApplicationDescriptor descriptor, String[] params)
    {
        ShellApp shellApp = ShellApp.getInstance() ;
        boolean bRC = false ;
        String strKey ;

        strKey = descriptor.getCodebase() ;
        if (strKey == null)
            strKey = descriptor.getMainClassName() ;

        Application app = getApplicationByKey(strKey) ;
        if (app != null) {
            // The application was already running, just activate it.
            PForm form = getActiveForm(app) ;
            if (form != null) {      // It is possible that the app does not have a GUI...
                if (((PAbstractForm) form).isStacked())
                {
                    shellApp.postEvent(new ShellEvent(ShellEvent.SE_SHOWFORM, this, form, null)) ;
                }
            }
            bRC = true ;
        } else {
            app = loadApplication(descriptor, params) ;
            if (app != null) {
                bRC = true ;
            }
        }

        return bRC ;
    }




    /**
     */
    public boolean closeApplication(String strAppName)
    {
        boolean bRC = false ;
        Application app = getApplicationByKey(strAppName) ;
        if (app != null) {
            PAbstractForm form = (PAbstractForm) getActiveForm(app) ;
            while (form != null) {
                ShellApp.getInstance().sendEvent(new ShellEvent(ShellEvent.SE_CLOSEFORM, this, form, null)) ;
                form = (PAbstractForm) getActiveForm(app) ;
            }
            bRC = true ;
        }
        return bRC ;
    }


    /**
     */
    public boolean isApplicationActive(String strAppName)
    {
        boolean bRC = false ;

        Application app = getApplicationByKey(strAppName) ;
        if (app != null) {
            bRC = true ;
        }

        return bRC ;

    }


    /**
     */
    public boolean isApplicationInFocus(String strAppName)
    {
        // Only the shell really knows this one...
        return ShellApp.getInstance().isApplicationInFocus(strAppName) ;
    }


    /**
     */
    public boolean waitForApplication(String strAppName)
    {
        boolean bRC = false ;
        Application app = getApplicationByKey(strAppName) ;
        if (app != null) {
            Thread appThread = app.getThread() ;
            if (appThread != null) {
                ShellApp.getInstance().safeJoin(appThread) ;
                bRC = true ;
            }
        }
        return bRC ;
    }



    /**
     */
    public boolean activateCoreApplication(int iType)
        throws IllegalArgumentException
    {
        String[] astrParams = null ;
        return activateCoreApplication(iType, astrParams) ;
    }


    public boolean activateCoreApplication(int iType, String param)
        throws IllegalArgumentException
    {
        String[] astrParams = null ;

        if (param != null)
        {
            astrParams = new String[1] ;
            astrParams[0] = param ;
        }
        return activateCoreApplication(iType, astrParams) ;
    }




    public boolean activateCoreApplication(int iType, String[] params)
        throws IllegalArgumentException
    {
        boolean bRC = false ;

        if ((iType < ApplicationRegistry.APP_TYPE_MIN) || (iType > ApplicationRegistry.APP_TYPE_MAX)) {
            throw new IllegalArgumentException("application type is invalid") ;
        } else if ((iType == ApplicationRegistry.USER_APP) || (iType == ApplicationRegistry.CORE_APP)) {
            throw new IllegalArgumentException("USER and CORE types are not valid for this operation") ;
        }

        ApplicationDescriptor desc = ApplicationRegistry.getInstance().getCoreApplicationByType(iType) ;
        if (desc != null) {
            bRC = activateApplication(desc, params) ;
        } else {
            //if it a TRANSFER or CONFERENCE, throw an error message.
            if( iType == ApplicationRegistry.TRANSFER_APP  ||
                iType == ApplicationRegistry.CONFERENCE_APP )
            {
                String[] strParamArray = new String[2];
                strParamArray[0]=("Sorry...");
                strParamArray[1]="This feature is not available";
                if( iType == ApplicationRegistry.TRANSFER_APP )
                {
                    strParamArray[1] = "Transfer feature is not available." ;
                }
                else if( iType == ApplicationRegistry.CONFERENCE_APP )
                {
                    strParamArray[1] = "Conference feature is not available." ;
                }
                activateApplication("org.sipfoundry.sipxphone.app.ModalMessageBoxApp", strParamArray);
            }
            System.out.println("Core Application is not registered!") ;
        }

        return bRC ;
    }


    /**
     */
    public boolean closeCoreApplication(int iType)
        throws IllegalArgumentException
    {
        boolean bRC = false ;

        if ((iType < ApplicationRegistry.APP_TYPE_MIN) || (iType > ApplicationRegistry.APP_TYPE_MAX)) {
            throw new IllegalArgumentException("application type is invalid") ;
        } else if ((iType == ApplicationRegistry.USER_APP) || (iType == ApplicationRegistry.CORE_APP)) {
            throw new IllegalArgumentException("USER and CORE types are not valid for this operation") ;
        }

        ApplicationDescriptor desc = ApplicationRegistry.getInstance().getCoreApplicationByType(iType) ;
        if (desc != null) {
            bRC = closeApplication(desc.getMainClassName()) ;
        }
        return bRC ;
    }


    /**
     */
    public boolean isCoreApplicationActive(int iType)
        throws IllegalArgumentException
    {
        boolean bRC = false ;

        if ((iType < ApplicationRegistry.APP_TYPE_MIN) || (iType > ApplicationRegistry.APP_TYPE_MAX)) {
            throw new IllegalArgumentException("application type is invalid") ;
        } else if ((iType == ApplicationRegistry.USER_APP) || (iType == ApplicationRegistry.CORE_APP)) {
            throw new IllegalArgumentException("USER and CORE types are not valid for this operation") ;
        }

        ApplicationDescriptor desc = ApplicationRegistry.getInstance().getCoreApplicationByType(iType) ;
        if (desc != null) {
            bRC = isApplicationActive(desc.getMainClassName()) ;
        }

        return bRC ;
    }


    /**
     */
    public boolean isCoreApplicationInFocus(int iType)
        throws IllegalArgumentException
    {
        boolean bRC = false ;

        if ((iType < ApplicationRegistry.APP_TYPE_MIN) || (iType > ApplicationRegistry.APP_TYPE_MAX)) {
            throw new IllegalArgumentException("application type is invalid") ;
        } else if ((iType == ApplicationRegistry.USER_APP) || (iType == ApplicationRegistry.CORE_APP)) {
            throw new IllegalArgumentException("USER and CORE types are not valid for this operation") ;
        }

        ApplicationDescriptor desc = ApplicationRegistry.getInstance().getCoreApplicationByType(iType) ;
        if (desc != null) {
            bRC = isApplicationInFocus(desc.getMainClassName()) ;
        }

        return bRC ;
    }


    /**
     */
    public boolean waitForCoreApplication(int iType)
        throws IllegalArgumentException
    {
        boolean bRC = false ;

        if ((iType < ApplicationRegistry.APP_TYPE_MIN) || (iType > ApplicationRegistry.APP_TYPE_MAX)) {
            throw new IllegalArgumentException("application type is invalid") ;
        } else if ((iType == ApplicationRegistry.USER_APP) || (iType == ApplicationRegistry.CORE_APP)) {
            throw new IllegalArgumentException("USER and CORE types are not valid for this operation") ;
        }

        ApplicationDescriptor desc = ApplicationRegistry.getInstance().getCoreApplicationByType(iType) ;
        if (desc != null) {
            bRC = waitForApplication(desc.getMainClassName()) ;
        }

        return bRC ;
    }

    /**
     * Load an application
     *
     * @param strClassname fully qualified class name of the application
     */
    public Application loadApplication(String strClassnameOrURL, String param)
    {
        String[] astrParams = null ;

        if (param != null)
        {
            astrParams = new String[1] ;
            astrParams[0] = param ;
        }
        return loadApplication(strClassnameOrURL, astrParams) ;
    }



    /**
     * Load an application
     *
     * @param strClassname fully qualified class name of the application
     */
    public Application loadApplication(String strClassnameOrURL, String[] params)
    {
        Application appRC = null ;
        ApplicationDescriptor appDescriptor = null ;

        if (strClassnameOrURL.startsWith("http:") || strClassnameOrURL.startsWith("file:")) {
            try {
                URL url = new URL(strClassnameOrURL) ;
                appDescriptor = getApplicationDescriptor(url) ;
                if (appDescriptor != null) {
                    appRC = loadApplication(appDescriptor, params) ;
                }
            } catch (MalformedURLException mue) {
                SysLog.log(mue) ;
            }
        } else {
            appDescriptor = getApplicationDescriptor(strClassnameOrURL) ;
            if (appDescriptor != null) {
                appRC = loadApplication(appDescriptor, params) ;
            } else {
                try {
                    Class classRef = Class.forName(strClassnameOrURL) ;
                    if (classRef != null) {
                        appRC = (Application) classRef.newInstance() ;

                        // add the application to our app list and then let it run
                        if (appRC != null) {
                            m_vApplications.addApplication(appRC, strClassnameOrURL) ;
                            appRC.start(params) ;
                        }
                    }
                } catch (Throwable t) {
                    System.out.println("ApplicationManager: Unable to load Application:" + strClassnameOrURL) ;
                    SysLog.log(t) ;
                }
            }
        }
        return appRC ;
    }


    /**
     * Load an application
     *
     * @param strClassname fully qualified class name of the application
     */
    public Application loadApplication(ApplicationDescriptor descriptor, String[] params)
    {
        String strClassname = descriptor.getMainClassName() ;
        String strCodebase = descriptor.getCodebase() ;
        Application objApp = null ;

        try {
            if (strCodebase != null) {
                objApp = getMainClassInstance(descriptor);
            } else {
                Class classRef = Class.forName(strClassname) ;
                if (classRef != null) {
                    objApp = (Application) classRef.newInstance() ;
                }
            }

            // add the application to our app list and then let it run
            if (objApp != null) {
                if (strCodebase != null )
                    m_vApplications.addApplication(objApp, strCodebase) ;
                else
                    m_vApplications.addApplication(objApp, strClassname) ;

                objApp.start(params) ;
            }
        } catch (ClassNotFoundException cnfe) {
            System.out.println("Unable to load " + strClassname + ": " + cnfe.toString()) ;
            SysLog.log(cnfe) ;
        } catch (InstantiationException ie) {
            System.out.println("Unable to instantiate " + strClassname + ": " + ie.toString()) ;
            SysLog.log(ie) ;
        } catch (IllegalAccessException iae) {
            System.out.println("Illegal instantiation " + strClassname + ": " + iae.toString()) ;
            SysLog.log(iae) ;
        } catch (Throwable t) {
            System.out.println("Unknown Error " + strClassname + ": " + t.toString()) ;
            SysLog.log(t) ;
        }
        return objApp ;
    }



    public void preloadApplication(String strClassNameOrURL)
    {
        ApplicationDescriptor desc = getApplicationDescriptor(strClassNameOrURL) ;
        if (desc != null) {
            preloadRegisteredApplication(desc) ;
        } else {
            System.out.println("ApplicationManager: unable to preload: " + strClassNameOrURL) ;
        }
    }


    /**
     * Invoke "onLoad" on a application
     */
    public void preloadRegisteredApplication(ApplicationDescriptor descriptor)
    {
        Class classApp = null ;

        try {
            if (descriptor.getCodebase() != null) {
                URL url = new URL(descriptor.getCodebase()) ;

                // This may fail because of network problems.
                AppClassLoader classLoader = getAppClassLoader(url) ;

                String strMainClassName = descriptor.getMainClassName() ;
                if (strMainClassName != null) {
                    classApp = classLoader.loadClass(strMainClassName) ;
                } else {
                    AppResourceManager manager = AppResourceManager.getInstance() ;
                    Shell.getInstance().showMessageBox
                        (MessageBox.TYPE_ERROR,
                        manager.getSystemString("lblAppLoadReportFailureTitle"),
                        manager.getSystemString("lblAppLoadReportFailureNoDescriptor") + descriptor.getCodebase(),
                        null) ;
                }
            } else {
                classApp = Class.forName(descriptor.getMainClassName()) ;
            }

            if (classApp != null)
            {
                Method method = classApp.getMethod("onLoad", null) ;
                if (method != null) {
                    method.invoke(classApp, null) ;
                }
            }
        } catch (Throwable t) {
            System.out.println(descriptor.getTitle() + " error while trying to preload: ") ;
            SysLog.log(t) ;
        }
    }


    /**
     * Invoke "onLoad" on all of our applications
     */
    public void preloadRegisteredApplications()
    {
        ApplicationDescriptor descriptors[] = ApplicationRegistry.getInstance().getApplications() ;

        if (descriptors != null) {
            for (int i=0;i<descriptors.length; i++) {
/*
System.out.println("") ;
System.out.println("Preloading #"+i+": " + descriptors[i].getTitle()) ;
System.out.println("  codebase =" + descriptors[i].getCodebase()) ;
System.out.println("  mainclass=" + descriptors[i].getMainClassName()) ;
*/
                preloadRegisteredApplication(descriptors[i]) ;
            }
        }
    }


    public void startApplication(String strClassnameOrURL)
    {
        ApplicationDescriptor desc = getApplicationDescriptor(strClassnameOrURL) ;
        if (desc != null) {
            startRegisteredApplication(desc, true) ;
        } else {
            System.out.println("ApplicationManager: unable to force start: " + strClassnameOrURL) ;
        }

    }

    public void startRegisteredApplication(String strClassnameOrURL)
    {
        ApplicationDescriptor desc = getApplicationDescriptor(strClassnameOrURL) ;
        if (desc != null) {
            startRegisteredApplication(desc, false) ;
        } else {
            System.out.println("ApplicationManager: unable to start: " + strClassnameOrURL) ;
        }
    }


    /**
     *
     */
    public void startRegisteredApplications()
    {
        ApplicationDescriptor descriptors[] = ApplicationRegistry.getInstance().getApplications() ;

        if (descriptors != null) {
            for (int i=0;i<descriptors.length; i++) {
                startRegisteredApplication(descriptors[i], false) ;
            }
        }
    }


    /**
     * Start an application only if the app descriptors say so
     */
    public void startRegisteredApplication(ApplicationDescriptor desc, boolean bForce)
    {
        if (desc != null) {
            try {
                String startRunParam = desc.getRunSetting() ;

                if (bForce) {
                    activateApplication(desc) ;
                } else if ((startRunParam != null) && startRunParam.equalsIgnoreCase("SystemStart")) {
                    activateApplication(desc) ;
                } else if ((startRunParam != null) && startRunParam.equalsIgnoreCase("RunOnce")) {
                    activateApplication(desc) ;
                    ApplicationRegistry.getInstance().unregisterApplication(desc.getCodebase()) ;
                }
            }
            catch (Throwable t)
            {
                Shell.getInstance().showMessageBox(MessageBox.TYPE_ERROR, "Error", "Unable to start " + desc.getTitle()+": \n\n" + t.toString(), null) ;
                SysLog.log(t) ;
            }
        }
    }



    /**
     * flush an application descriptor and all related resources.  This is
     * often called when an application is being uninstall.  We will flush
     * all memories of the application, descriptor, class loaders, etc. down
     * the drain.
     */
    public void flushApplicationDescriptor(String strClassnameOrURL)
        throws MalformedURLException
    {
        if (strClassnameOrURL.startsWith("http:") || strClassnameOrURL.startsWith("file:")) {
            URL url = new URL(strClassnameOrURL) ;

            if (m_ApplicationDescriptors != null) {
                m_ApplicationDescriptors.remove(url.toString()) ;
                m_ApplicationDescriptors.remove(strClassnameOrURL) ;
            }

            if (m_ApplicationMainClassInstances != null) {
                m_ApplicationMainClassInstances.remove(url.toString()) ;
                m_ApplicationMainClassInstances.remove(strClassnameOrURL) ;
            }

            if (classloaders != null) {
                classloaders.remove(url) ;
                classloaders.remove(strClassnameOrURL) ;
            }
        }
    }


    /**
     * This method is called whenever the ApplicationThreadGroup notices that
     * a thread has died.  It is the ApplicationManager's responsibility to
     * clean up the application descriptor and fire off notification so that
     * the rest of the world knows that's going on.  (The GUI should dump any
     * forms, etc).
     */
    public void notifyThreadDeath(Thread t)
    {
        Application app = m_vApplications.findByThread(t) ;
        if (app != null) {
            icApplicationDescriptor descriptor = m_vApplications.findApplication(app) ;
            if (descriptor != null) {
                AppClassLoader loader = (AppClassLoader) classloaders.get(descriptor.m_strAppKey) ;

                if (loader != null) {
                    m_ApplicationMainClassInstances.remove(descriptor.m_strAppKey) ;
                    // classloaders.remove(descriptor.m_strAppKey) ;
                    // System.out.println(classloaders) ;
                    // loader.purgeStaticResources(descriptor.m_strAppKey) ;
                    // loader.dumpStaticResources() ;
                }
            } else {
                System.out.println("Unable to find application descriptor") ;
            }
            m_vApplications.removeApplication(app) ;

        }
    }


    /**
     *
     */
    public boolean pushForm(PForm form, int iType)
    {
        boolean bRC = false ;

        icApplicationDescriptor descriptor = m_vApplications.findApplication(form.getApplication()) ;
        if (descriptor != null) {
            bRC = descriptor.pushForm(form, iType) ;
        } else {
            System.out.println("Unable to find application descriptor the form: " + form) ;
            System.out.println("Application: " + form.getApplication()) ;
            System.out.println("This may happen if your application exits (main is not blocking) or if you try to display a form as part of onLoad") ;
            throw new IllegalStateException("Invalid Application Request") ;
        }

        return bRC ;
    }


    public void dump(PrintStream out)
    {
        Enumeration enum = m_vApplications.elements() ;
        while (enum.hasMoreElements())
        {
            icApplicationDescriptor descriptor = (icApplicationDescriptor) enum.nextElement() ;
            if (descriptor != null)
            {

                descriptor.dump(out) ;
            }
        }
    }


    /**
     *
     */
    public PForm activeForm(PForm form)
    {
        PForm formRC = form ;
        icApplicationDescriptor descriptor = m_vApplications.findApplication(form.getApplication()) ;
        if (descriptor != null) {
            formRC = descriptor.activeForm(form) ;
        }

        return formRC ;
    }


    /**
     *
     */
    public boolean removeForm(PForm form)
    {
        boolean bRC = false ;

        icApplicationDescriptor descriptor = m_vApplications.findApplication(form.getApplication()) ;
        if (descriptor != null) {
            bRC = descriptor.removeForm(form) ;
        }

        return bRC ;
    }


    /**
     * What the passed application's active form?
     */
    public PForm getActiveForm(Application app)
    {
        PForm formRC = null ;

        icApplicationDescriptor descriptor = m_vApplications.findApplication(app) ;
        if (descriptor != null) {
            formRC = descriptor.getActiveForm() ;
        } else
            System.out.println("Unable to find descriptor for : " + app) ;

        return formRC ;
    }


    /**
     * look up an application given the passed name
     */
    public Application getApplicationByKey(String strAppKey)
    {
        Application app = m_vApplications.findByKey(strAppKey) ;

        return app ;
    }


    /**
     * Return the list of currently active applications
     */
    public String[] getLoadedApplications()
    {
        System.out.println("ApplicationManager::getLoadedApplications()") ;

        Enumeration enum = m_ApplicationDescriptors.elements() ;

        String strApps[] = null ;

        if (m_vApplications != null) {
            strApps = new String[m_ApplicationDescriptors.size()] ;
        }

        System.out.println("Number of loaded application: " + String.valueOf(m_ApplicationDescriptors.size())) ;

        int i = 0 ;
        ApplicationDescriptor des ;
        while (enum.hasMoreElements()==true) {
            des = (ApplicationDescriptor) enum.nextElement() ;
            if (des != null) {
                strApps[i] = des.getTitle();
                System.out.println("ApplicationManager loaded application: " + strApps[i]) ;
                i++ ;
            }

        }

        if (strApps == null) {
            strApps = new String[0] ;
        }

        return strApps ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner classes
///
    /**
     * This is an internal application descriptor.  This is where we hold
     * application specific data / references to the applications.  The
     * stack of forms for an application is also maintained in this list.
     */
    protected class icApplicationDescriptor
    {
        public String      m_strAppKey ;
        public Application m_application ;
        public Stack       m_formStack ;

        public icApplicationDescriptor(Application app, String strAppKey)
        {
            // Validate our params
            if ((strAppKey == null) || (app == null)) {
                throw new IllegalArgumentException("icApplicationDescriptor requires a non-null app and app key") ;
            }

            m_strAppKey = strAppKey ;
            m_application = app ;
            m_formStack = new Stack() ;
        }


        public PForm activeForm(PForm form)
        {
            PForm formRC = form ;

            if (!m_formStack.isEmpty()) {
                icFormDescriptor desc = (icFormDescriptor) m_formStack.peek() ;
                if (desc != null) {
                    if ((desc.iType & PForm.MODAL) == PForm.MODAL) {
                        formRC = desc.form ;
                    }
                }
            }
            return formRC ;
        }


        public boolean removeForm(PForm form)
        {
            boolean bRC = false ;
            // Remove our item if is already in the stack
            for (int i=0; i<m_formStack.size(); i++) {
                icFormDescriptor desc = (icFormDescriptor) m_formStack.elementAt(i) ;
                if (desc.form == form) {
                    m_formStack.removeElementAt(i) ;
                    bRC = true ;
                    break ;
                }
            }
            return bRC ;
        }


        public boolean pushForm(PForm form, int iType)
        {
            boolean bRC = true ;

            // Simple case: the stack is empty
            if (m_formStack.isEmpty()) {
                m_formStack.push(new icFormDescriptor(form, iType)) ;
            } else {
                try {
                    icFormDescriptor formDescriptor = (icFormDescriptor) m_formStack.peek() ;

                    if (formDescriptor.form != form)
                    {
                        // Remove our item if is already in the stack
                        for (int i=0; i<m_formStack.size(); i++) {
                            icFormDescriptor desc = (icFormDescriptor) m_formStack.elementAt(i) ;
                            if (desc.form == form)
                            {
                                m_formStack.removeElementAt(i) ;
                                break ;
                            }
                        }

                        if ((formDescriptor.iType & PForm.ALWAYSONTOP) == PForm.ALWAYSONTOP)
                        {
                            if ((iType & PForm.ALWAYSONTOP) == PForm.ALWAYSONTOP)
                                m_formStack.push(new icFormDescriptor(form, iType)) ;
                            else
                                m_formStack.insertElementAt(new icFormDescriptor(form, iType), 1) ;
                        }
                        else
                            m_formStack.push(new icFormDescriptor(form, iType)) ;
                    }
                }
                catch (EmptyStackException ese)
                {
                    m_formStack.push(new icFormDescriptor(form, iType)) ;
                }
            }
            return bRC ;
        }


        public PForm getActiveForm()
        {
            PForm form = null ;

            if (!m_formStack.isEmpty())
            {
                icFormDescriptor descriptor = (icFormDescriptor) m_formStack.peek() ;
                if (descriptor != null)
                {
                    form = descriptor.form ;
                }
            }

            return form ;
        }



        protected class icFormDescriptor
        {
            public PForm form ;
            public int   iType ;

            public icFormDescriptor(PForm form, int iType)
            {
                this.form = form ;
                this.iType = iType ;
            }


            public String toString()
            {
                return "icFormDescriptor(name="+form.getFormName()+", type="+iType ;
            }
        }



        /**
         * Dumps the key of this application descriptor and it's form stack
         * to the designated output stream
         */
        public void dump(PrintStream outputStream)
        {
            outputStream.println("****") ;
            outputStream.println("icApplicationDescription: " + m_strAppKey) ;
            outputStream.println("key : " + m_strAppKey) ;
            outputStream.println("app : " + m_application) ;

            for (int i=0; i<m_formStack.size(); i++) {
                icFormDescriptor formDesc = (icFormDescriptor) m_formStack.elementAt(i) ;
                if (formDesc != null) {
                    outputStream.println("    " + formDesc) ;
                }
            }
        }
    }


    /**
     * A Vector or application descriptors with some basic searching
     * and add/remove helper methods.
     */
    protected class icApplicationDescriptorList extends Vector
    {
        /**
         * add an application to our descriptor list
         */
        public synchronized void addApplication(Application app, String strAppKey)
        {
            addElement(new icApplicationDescriptor(app, strAppKey)) ;

// System.out.println("Adding application: "  + strAppKey) ;
// System.out.println("Adding application: " + app) ;
// dump(System.out) ;
        }


        /**
         * remove an application from our descriptor list
         */
        public synchronized void removeApplication(Application app)
        {
            icApplicationDescriptor descriptor = findApplication(app) ;

            removeElement(descriptor) ;

            // @TODO This is bad- the list is traversed twice, one
            // for the lookup and once for the removal.  We should do it all
            // at once.

// System.out.println("Removing application: " + app) ;
// dump(System.out) ;

        }


        /**
         * search our application descriptors for a specific application
         */
        public synchronized icApplicationDescriptor findApplication(Application app)
        {
            icApplicationDescriptor descriptorRC = null ;

            Enumeration enum = elements() ;
            while (enum.hasMoreElements()) {
                icApplicationDescriptor descriptor = (icApplicationDescriptor) enum.nextElement() ;
                if (descriptor.m_application == app) {
                    // we found the application
                    descriptorRC = descriptor ;
                    break ;
                }
            }
            return descriptorRC ;
        }


        /**
         * search our application descriptors for a specific application thread
         */
        public synchronized Application findByThread(Thread thread)
        {
            Application appRC = null ;

            Enumeration enum = elements() ;
            while (enum.hasMoreElements()) {
                icApplicationDescriptor descriptor = (icApplicationDescriptor) enum.nextElement() ;
                if (descriptor.m_application.getThread() == thread) {
                    // we found the application
                    appRC = descriptor.m_application ;
                    break ;
                }
            }
            return appRC ;
        }


        /**
         * Search for an application by key
         */
        public synchronized Application findByKey(String strAppKey)
        {
            Application appRC = null ;
            String      strCheckKey ;

            Enumeration enum = elements() ;
            while (enum.hasMoreElements()) {
                icApplicationDescriptor descriptor = (icApplicationDescriptor) enum.nextElement() ;
                if (descriptor.m_strAppKey.equals(strAppKey)) {
                    // we found the application
                    appRC = descriptor.m_application ;
                    break ;
                }
            }
            return appRC ;
        }



        /**
         * Dumps know list of application descriptors
         */
        public void dump(PrintStream outputStream)
        {
            for (int i=0; i<size(); i++) {
                icApplicationDescriptor descriptor = (icApplicationDescriptor) elementAt(i) ;
                if (descriptor != null) {
                    descriptor.dump(outputStream) ;
                }
            }
        }

    }


    //============================================================================
    //============================================================================


    // Here's the sequence:
    // A ClassLoader is created for the class
    //     The JAR file gets loaded into the local cache by the ClassLoader
    // The ApplicationDescriptor is populated
    // The init() method is called
    // the main() method is called

    // Used for debug messages
    private static final boolean debug = false ;

    /**
     * Hashtable of ApplicationDescriptors
     * Key is URL
     * Value is the ApplicationDescriptor Object.
     */
    private Hashtable m_ApplicationDescriptors;

    /**
     * Hashtable of Application Objects
     * Key is URL
     * Value is the Application Object.
     */
    private Hashtable m_ApplicationMainClassInstances;

    /**
     * Hashtable of ClassLoaders for the Applications
     * Key is URL
     * Value is the ClassLoader.
     */
    private Hashtable classloaders ;



    /**
     * Get the ApplicationDescriptor for this Application.
     * This method will create one for the Application if it doesn't yet exist.
     */
    public ApplicationDescriptor getApplicationDescriptor(URL url) throws IllegalArgumentException
    {
        ApplicationDescriptor appDescriptor = null ;

        // Look to see if the ApplicationDescriptor already exists for this app
        appDescriptor = (ApplicationDescriptor) m_ApplicationDescriptors.get(url.toString()) ;
        if (appDescriptor == null) {
            // Get the ClassLoader for this Application. This will create one if it doesn't exist.

            AppClassLoader classLoader = getAppClassLoader(url);
            InputStream appDescriptorStream = classLoader.getResourceAsStream("ApplicationDescriptor.properties");

            if (appDescriptorStream != null) {

                Properties prop = new Properties();

                try {
                    prop.load(appDescriptorStream) ;

                    appDescriptor = new ApplicationDescriptor() ;
                    appDescriptor.setCodebase(url.toString()) ;
                    appDescriptor.setProperties(prop) ;

                    /*
                    System.out.println( ("\n*********** Loading App ***********")
                       +("\nLoading: " + url)
                       +("\nTitle: " + prop.get(ApplicationDescriptor.TITLE))
                       +("\nCodebase: " + url.toString())
                       +("\nclasspath: " + prop.get(ApplicationDescriptor.MAIN_CLASS_NAME))
                       +("\nRequiredVersion: " + prop.get(ApplicationDescriptor.REQUIRED_VERSION))
                       +("\n**********************************")  );
                    */


                    m_ApplicationDescriptors.put(url.toString(), appDescriptor) ;

                }
                catch (IOException e)
                {
                    throw new IllegalArgumentException("Error loading ApplicationDescriptor.properties file in JAR");
                }
            }
        }

        return appDescriptor;
    }


    /**
     * Get the ApplicationDescriptor for this Application.
     * This method will create one for the Application if it doesn't yet exist.
     */
    public ApplicationDescriptor getApplicationDescriptor(String strClassNameOrURL)
            throws IllegalArgumentException
    {
        //Look to see if the ApplicationDescriptor already exists for this app
        ApplicationDescriptor appDescriptor = null;

        if (strClassNameOrURL.toLowerCase().startsWith("http:") || strClassNameOrURL.toLowerCase().startsWith("file:")) {
            try {
                return getApplicationDescriptor(new URL(strClassNameOrURL)) ;
            } catch (MalformedURLException e) {
                SysLog.log(e) ;
            }
        } else {
            // Get the ApplicationDescriptor from the JAR file
            InputStream appDescriptorStream = ClassLoader.getSystemResourceAsStream(strClassNameOrURL+".properties") ;

            if (appDescriptorStream != null) {

                Properties prop = new Properties();

                try {
                    prop.load(appDescriptorStream);
                } catch (IOException e) {
                    throw new IllegalArgumentException("Error loading "+strClassNameOrURL+".properties file");
                }

                appDescriptor = new ApplicationDescriptor();
                appDescriptor.setCodebase(null) ;
                appDescriptor.setProperties(prop) ;
            }

        }

        return appDescriptor;
    }


    /**
     * This class will get the class loader for the Application.
     * It also has the side effect of loading the JAR for the class
     * into the local cache.
     */
    public ClassLoader getClassLoader(String url) throws IllegalArgumentException
    {
        URL codebase;

        try {
            codebase = new URL(url);
        } catch (MalformedURLException e) {
            throw new IllegalArgumentException("Bogus URL");
        } catch (NullPointerException e) {
            throw new IllegalArgumentException("Bogus URL");
        }

        return getAppClassLoader(codebase) ;
    }

    /**
     * This is used to determine how long to wait for the JAR file to
     * load.
     */
    private int getJarLoadTimeout()
    {
        final String JAR_LOAD_TIMEOUT = "JAR_LOAD_TIMEOUT";

        // Default timeout
        int timeout = 60 ;

        try {
            String sTimeout = (String) PingerConfig.getInstance().getValue(JAR_LOAD_TIMEOUT);
            int iTimeout = Integer.parseInt(sTimeout);
            timeout = iTimeout;
        } catch (Exception e) {
        }

        return timeout;
    }

    /**
     * This class will get the class loader for the Application.
     * It also has the side effect of loading the JAR for the class
     * into the local cache.
     */
    public AppClassLoader getAppClassLoader(URL codebase) throws IllegalArgumentException
    {
        AppClassLoader c = (AppClassLoader) classloaders.get(codebase.toString());
        if (c == null) 
        {
            c = new AppClassLoader(codebase);
            /*
            AsyncAppClassLoader async = new AsyncAppClassLoader(c);
            async.start();
            try
            {
                async.join(getJarLoadTimeout() * 1000);
                if (!async.isSuccessful())
                {
                    throw new IllegalArgumentException("JAR not found: " +
                        codebase.toString());
                }
            }
            catch (InterruptedException e)
            {
                throw new IllegalArgumentException("Timeout loading JAR: " +
                    codebase.toString());
            }
            */

            classloaders.put(codebase.toString(), c);

            /*
            AppClassLoaderResourceLoader r = c.getResourceLoader();

            // Load the jar file for this app into the local cache
            icLoadJar loader = new icLoadJar(r, codebase) ;
            Waiter myWaiter = new Waiter(loader, getJarLoadTimeout() * 1000) ;

            if (myWaiter.isSuccessful()) {
                if (loader.isSuccessful()) {
                    classloaders.put(codebase.toString(), c);
                } else {
                    throw new IllegalArgumentException("JAR not found: " + codebase.toString()) ;
                }
            } else {
                throw new IllegalArgumentException("Timeout loading JAR: " + codebase.toString()) ;
            }
            */
        }

        return c;
    }


    public Application getMainClassInstance(ApplicationDescriptor descriptor)
    {
        try {

            if (descriptor.getCodebase() != null) {
                URL url = new URL(descriptor.getCodebase()) ;

                return getMainClassInstance(url) ;
            } else {
                Class classRef = Class.forName(descriptor.getMainClassName()) ;
                if (classRef != null) {
                    return  (Application) classRef.newInstance() ;
                }
            }
         } catch (Throwable t) {
            System.out.println("Error trying to get main class:" + t.toString()) ;
            SysLog.log(t) ;
        }
        return null ;
    }

    public Application getMainClassInstance(URL url) throws IllegalArgumentException
    {
        // This may throw an exception
        ApplicationDescriptor appDescriptor = getApplicationDescriptor(url);

//        System.out.println("getMainClassInstance: url=" + url) ;
//        appDescriptor.dump(System.out) ;

        String mainClass = null;

        if ((mainClass = (String) appDescriptor.getMainClassName()) != null)
        {
            debug("    ApplicationMainClass is " + mainClass);

            // See if we have already created an instance of the main class
            Object theInstance = m_ApplicationMainClassInstances.get(url.toString());

            // If we have not created an instance, create one.
            if (theInstance == null)
            {
                // This may throw an exception
                AppClassLoader classLoader = getAppClassLoader(url);

                Class theClass = null;

                try {
                    theClass = classLoader.loadClass(mainClass);
                } catch (Throwable t) {
                    throw new IllegalArgumentException("Could not load class " + mainClass);
                }

                try {
                    debug("Trying to instantiate " + theClass.getName());
                    theInstance = theClass.newInstance();
                } catch (Throwable t) {
                    throw new IllegalArgumentException("Could not instantiate class " + mainClass + ". Are you sure that your default contructor and class are declared as public?");
                }

                // Squirrel away the Application Object
                m_ApplicationMainClassInstances.put(url.toString(), theInstance);

                if (theInstance == null)
                {
                    throw new IllegalArgumentException("Could not create a new instance of " + url.toString());
                }
            }

            if (theInstance instanceof Application)
            {
                return ((Application) theInstance);
            } else {
                throw new IllegalArgumentException("The instance of the main class was not derived from Application");
            }

        } else {
            throw new IllegalArgumentException("The MainClassName key could not be found");
        }
    }

    private void debug(String message)
    {
        if (debug)
            System.out.println("ApplicationManager: " + message);
    }

    public static String buildAppThreadName(Application app, int count)
    {
        String strClassName = app.getClass().toString() ;
        int iLastDot = strClassName.lastIndexOf('.') ;
        if (iLastDot >= 0)
        {
            strClassName = strClassName.substring(iLastDot+1) ;
        }

        return strClassName + "-" + count ;
    }

//////////////////////////////////////////////////////////////////////////////
// Native methods
///

    public static String[] JNI_getLoadedApplications()
    {
        return ApplicationManager.getInstance().getLoadedApplications() ;
    }


    /**
     * Main used for unit test
     */
    static public void main(String args[])
    {
        ApplicationManager xm = ApplicationManager.getInstance();
        ApplicationRegistry registry = ApplicationRegistry.getInstance();

        String ApplicationURL = "http://localhost/MyTestApp.jar";

        try {
            registry.cgiInstallApplication(ApplicationURL) ;
            xm.activateApplication(ApplicationURL) ;

            String[] strApps = new String[10] ;
            int i = 0 ;
            strApps = xm.JNI_getLoadedApplications() ;
            while (strApps[i] != null) {
                System.out.println("App " + String.valueOf(i) + " = " + strApps[i]) ;
                i++ ;
            }

        } catch (Throwable t) {
            SysLog.log(t);
        }
    }


    /*
    class icLoadJar implements Runnable, WaiterControl
    {
        private AppClassLoaderResourceLoader m_r;
        private URL m_codebase;
        private boolean m_bSuccess ;
        private boolean m_bCanAbort ;

        icLoadJar(AppClassLoaderResourceLoader r, URL urlCodebase)
        {
            m_bSuccess = false ;
            m_r = r;
            m_codebase = urlCodebase;
            m_bCanAbort = true ;
        }

        public void run()
        {
            try
            {
                m_r.loadJar(m_codebase, "", this);
                m_bSuccess = true ;
            }
            catch (IOException e)
            {
                SysLog.log(e) ;
            }
        }

        public boolean isSuccessful()
        {
            return m_bSuccess ;
        }


        public boolean canAbort()
        {
            return m_bCanAbort ;
        }


        public void resetCanAbort(boolean bCanAbort)
        {
            m_bCanAbort = bCanAbort ;
        }
    }
    */

}
