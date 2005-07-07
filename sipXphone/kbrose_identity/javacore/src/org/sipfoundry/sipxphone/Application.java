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

package org.sipfoundry.sipxphone ;

import java.io.* ;
import java.util.* ;
import java.net.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.startup.PingerApp ;

import org.sipfoundry.sipxphone.sys.appclassloader.ApplicationManager ;

/**
 * An application object provides an entry point or starting point for an
 * xpression application. The application object is equivalent to an applet
 * in an HTML browser; however, xpressions are not tied to a
 * Graphical User Interface like applets.
 * <p>
 *
 * Developers derive their own application from this class to execute on
 * any of the sipXphone platforms.
 * <p>
 *
 * An application has three methods that can be overridden:
 * <p>
 * <i>onLoad</i> - The <i>onLoad</i> method is called by the framework when the
 *      application's .jar file is loaded from the network. Applications cannot
 *      display any forms at this point, however, timely initialization could
 *      be performed. Hooks can also be installed at this point.
 * <br>
 * <i>onUnload</i> - The <i>onUnload</i> method is called when an application is
 *      being unloaded. This may happen if the framework is low on memory or
 *      if the application is uninstalled. Applications should remove any
 *      hooks and clean up after themselves.
 * <br>
 * <i>main</i> - The bulk of the application logic should be performed
 *      here.
 * <br>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class Application
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** our application thread context */
    private icApplicationContext m_context ;

    /** number of applications that have been created */
    private static int m_iAppCreationCnt = 1 ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Default Constructor.  Developers should not be concerned with
     * application construction.  The framework will create an application
     * object and an application thread behind the scenes.  Developers should
     * add their logic to the <i>main</i>, <i>onLoad</i>, and
       <i>onUnload</i> methods.
     */
    public Application()
    {

    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Hook for one-time initialization. This method is called when an
     * application is first loaded. Applications are generally
     * loaded on startup and unloaded on shutdown.
     * <p>
     * Applications should perform lengthy application initialization at this
     * point.
     *
     * @see #onUnload
     */
    public static void onLoad() { }


    /**
     * Cleanup hook. This method is called when an application is being
     * unloaded. Applications are generally loaded on startup and
     * unloaded on shutdown. However, low memory conditions may force a
     * non-running application to be unloaded.
     * <p>
     * Applications should perform final cleanup.
     *
     * @see #onLoad
     */
    public static void onUnload() { }


    /**
     * Application entry point. Application developers should add their
     * program logic here.
     */
    public void main(String argv[]) { }



    /**
     * Cause the application thread to terminate.
     */
    public void exit()
    {
        // Say "bye-bye" to Mr. Thread
        if (m_context != null) {
            m_context.stop() ;
            ApplicationManager.getInstance().notifyThreadDeath(m_context) ;
            m_context = null ;
        } else {
            throw new IllegalStateException("application thread is not running") ;
        }
    }



    /**
     * Start the application thread and begin execution. This is called by
     * the framework; developers need not use this.
     *
     * @deprecated do not expose.
     */
    public void start(String params[])
    {
        if (m_context == null) {
            m_context = new icApplicationContext(this, params) ;
            m_context.start() ;
        } else {
            throw new IllegalStateException("application thread already running") ;
        }
    }


    /**
     * Display a message box with the passed title and text.
     *
     * @param strTitle Title text to display in title bar of alert.
     * @param strText Text to display in alert.
     */
    public int showMessageBox(String strTitle, String strText)
    {
        return Shell.getInstance().showMessageBox(MessageBox.TYPE_INFORMATIONAL, strTitle, strText, this) ;
    }


    /**
     * Display a specified type of message box with the passed title and text.

     *
     * @param iType Type of message box, see constants in {@link org.sipfoundry.sipxphone.awt.form.MessageBox}.
     * @param strTitle Title text to display in title bar of alert.
     * @param strText Text to display in alert.
     */
    public int showMessageBox(int iType, String strTitle, String strText)
    {
        return Shell.getInstance().showMessageBox(iType, strTitle, strText, this) ;
    }


    /**
     * Return the application thread context. Each running application has an
     * associated thread.
     *
     * @deprecated do not expose.
     */
    public Thread getThread()
    {
        return m_context ;
    }


    /**
     * Return the full class name of this application.
     *
     */
    public String getName()
    {
        return getClass().getName() ;
    }


    /**
     * Place an object into non-volatile storage.
     *
     * @param strKey Unique identifer for the object.
     * @param obj Object you would like to store, or null to remove the key.
     */
    public void putObject(String strKey, Object obj)
        throws IOException
    {
        PropertyManager manager = PropertyManager.getInstance() ;
        if (manager != null) {
            manager.putObject(getName() + "." + strKey, obj) ;
        }
    }


    /**
     * Retrieve an object from non-volatile storage.
     *
     * @param strKey Unique identifer for the object.
     * @return Object previously stored, or null if not found.
     */
    public Object getObject(String strKey)
        throws IOException
    {
        Object objRC = null ;
        try {
            PropertyManager manager = PropertyManager.getInstance() ;
            if (manager != null) {
                objRC = manager.getObject(getName() + "." + strKey) ;
            }
        } catch (IllegalArgumentException e) { }

        return objRC ;
    }


    /**
     * Get a resource from the application's .jar file and return it as a stream.
     *
     * @param strResource Resource name.
     *
     * @return InputStream of specified resource, or null if not found.
     */
    public InputStream getResourceAsStream(String strResource)
    {
        //return null if a null string is passed or is an empty string.
        if (strResource == null || strResource.equals(""))
            return null;

        ClassLoader cl = getClass().getClassLoader() ;
        InputStream stream = null ;

        if (cl != null)
            stream = cl.getResourceAsStream(strResource) ;

        return stream  ;
    }


    /**
     * Get URL of a resource from the application's .jar file.
     *
     * @param strResource Resource name.
     *
     * @return URL of specified resource, or null if not found.
     */
    public URL getSystemResource(String strResource)
    {
        ClassLoader cl = getClass().getClassLoader() ;
        URL url = null ;

        if (cl != null)
            url = cl.getResource(strResource) ;

        return url  ;
    }

/**
 * Gets value out of jar
 *
 * @since 1.2
 *
 * @param propKey key to look up
 *
 * @return String
 * @author dwinsor
 * @deprecated <B>NOT EXPOSED</B>

 */
    public String getPropertyValue(String propKey)
    {
        String strValue = null ;

        // Get Version Information
        try
        {
            PropertyResourceBundle bundle = new PropertyResourceBundle(getResourceAsStream("ApplicationDescriptor.properties"));
            strValue = bundle.getString(propKey);
        }
        catch (Exception e)
        {
            SysLog.log(e) ;
        }

        return strValue ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Our application / thread context.  This is used to hide the thread from
     * the outside world and the application developers.  It also allows us
     * to control how the thread actually dies.  We never want a thread to
     * simply exit- instead we want to force the issue by calling 'stop'.
     * Calling stop causes the VM to throw an ThreadDeath exception.  We then
     * catch the exception in the ApplicationThreadGroup and start the clean
     * up process.
     *
     * Rumor has it that this may be a bad approach as some system resource
     * maybe left open.  I need to check up on this.
     */
    private class icApplicationContext extends Thread
    {
        private Application m_app ;
        private String m_strParams[];


        public icApplicationContext(Application app, String[] params)
        {
            super(  ApplicationThreadGroup.getInstance(),
                    ApplicationManager.buildAppThreadName(app, m_iAppCreationCnt++)) ;



            m_app = app ;
            m_strParams = params;

        }


        /**
         * Called by the system when it is time for our application to
         * execute.  This code simply calls the application at specific
         * entry points.  The way that this is done is subject to change.
         * perhaps we want to hold off deinitApplication until idle or
         * init the application on startup, but not actually let it run.
         */
        public void run()
        {
            // and do it's thing
            main(m_strParams) ;

            // and finally tell the application manager to cleanup...
            ApplicationManager.getInstance().notifyThreadDeath(m_context) ;
            m_context = null ;
        }
    }
}
