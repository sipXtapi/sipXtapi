/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/test/AppClassLoaderTest.java#1 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

import junit.framework.TestCase;

import java.net.*;
import java.io.*;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import org.sipfoundry.sipxphone.sys.appclassloader.*;


/**
 * Resource loading in particular for phone applications
 */
public class AppClassLoaderTest extends TestCase 
    implements InvocationHandler
{

    private PingerConfigMock m_cfg;

    public void setUp()
    {
        m_cfg = new PingerConfigMock(this);
    }

    public void tearDown()
    {
        m_cfg.restoreOriginal();
    }

    /**
     * Route mock object calls to here
     */
    public Object invoke(Object o, Method m, Object[] args)
    {
        if (m.getName().equals("getValue"))
        {
            return "1000";
        }

        fail("Unexpected call to mock object " + o.getClass().getName()  
             + "." + m.getName());

        return null;
    }

    /**
     * Can we load a application jar
     */
    public void testLoadJar()
    {
        try
        {
            URL jarUrl = new URL("file://" + getBaseDir() + "/meta/test/dummyapp.jar");

            ApplicationManager mgr = ApplicationManager.getInstance();
            assertNotNull("application mananager loaded", mgr);

            AppClassLoader loader = mgr.getAppClassLoader(jarUrl);
            assertNotNull("created application loader", loader);

            Class c = loader.loadClass("org.sipfoundry.sipxphone.apps.phonebook.Phonebook");
            assertNotNull("Phone app class loaded", c);
        }
        catch (ClassNotFoundException cnfe)
        {
            cnfe.printStackTrace();
            fail("application class not found");
        }
        catch (MalformedURLException mue)
        {
            mue.printStackTrace();
            fail("dummy phone application not found");
        }
    }

    private String getBaseDir()
    {
        String path = System.getProperty("basedir");
        assertNotNull("-Dbasedir not defined, should point to directory where "
            + "build.xml is found", path);
        return path;
    }
}
