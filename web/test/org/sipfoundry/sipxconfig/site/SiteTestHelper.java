/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import java.util.Properties;

import junit.framework.Test;
import junit.framework.TestSuite;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.common.TestUtil;
import org.springframework.context.ApplicationContext;
import org.springframework.context.support.ClassPathXmlApplicationContext;

public class SiteTestHelper {
    
    private static final String APPLICATION_CONTEXT_FILE = "org/sipfoundry/sipxconfig/applicationContext-sipxconfig.xml";
    
    private static Properties s_sysDirProps;
    
    private static ApplicationContext s_appContext;
    
    private static String s_buildDir;
    
    private static String s_baseUrl;
    
    public static Test webTestSuite(Class webTestClass) {
        TestSuite suite = new TestSuite();        
        suite.addTestSuite(webTestClass);
        
        JettyTestSetup jetty = new JettyTestSetup(suite);
        s_baseUrl = jetty.getUrl();
        
        return jetty;        
    }
    
    public static String getBaseUrl() {
        return s_baseUrl;
    }

    public static ApplicationContext getApplicationContext() {
        if (s_appContext == null) {
            getSysDirProperties();
            s_appContext = new ClassPathXmlApplicationContext(
                TestUtil.APPLICATION_CONTEXT_FILE);
        }
        
        return s_appContext;
    }
    
    public static String getClasspathDirectory() {        
        return TestUtil.getClasspathDirectory(SiteTestHelper.class);
    }

    public static String getTestDirectory() {
        return getClasspathDirectory() + "/test-output";
    }
    
    public static String getBuildDirectory() {
        if (s_buildDir == null) {
            s_buildDir = TestUtil.getBuildDirectory("web");
        }
        
        return s_buildDir;
    }
    
    public static Properties getSysDirProperties() {
        if (s_sysDirProps == null) {
            String etcDir = TestUtil.getProjectDirectory() + "/../etc";
            String outDir = getTestDirectory();
            s_sysDirProps = TestUtil.getSysDirProperties(getClasspathDirectory(), etcDir, outDir);
        }
        return s_sysDirProps;
    }    
}
