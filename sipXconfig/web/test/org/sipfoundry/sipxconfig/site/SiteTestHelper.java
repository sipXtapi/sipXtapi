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

import java.io.IOException;

import junit.framework.AssertionFailedError;
import junit.framework.Test;
import junit.framework.TestSuite;
import net.sourceforge.jwebunit.WebTester;

import org.sipfoundry.sipxconfig.common.TestUtil;
import org.xml.sax.SAXException;

import com.meterware.httpunit.WebForm;

public class SiteTestHelper {
    private static String s_buildDir;

    private static String s_baseUrl;

    public static Test webTestSuite(Class webTestClass) {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(webTestClass);

        JettyTestSetup jetty = new JettyTestSetup(suite);
        s_baseUrl = jetty.getUrl();

        return jetty;
    }

    /**
     * Go to Home.html, includes hack for slow machines.
     */
    public static void home(WebTester tester) {
        tester.beginAt("/");
        login(tester, "", "");
        // HACK! Webunit doesn't appear to fully load page, especialy
        // when the machine you're running it on is slow and you're
        // running a batch of tests, calling beginAt("/") twice seems
        // to get webunit to catch up.
        tester.beginAt("/");
        assertNoException(tester);
    }

    /**
     * Login - form based loging for our pages You can change which user names are passwords are
     * valid in JettyUserRealm
     * 
     * @see JettyUserRealm
     */
    public static void login(WebTester tester, String username, String password) {
        try {
            if (tester.getDialog().hasForm("login:form")) {
                WebForm form = tester.getDialog().getForm();
                form.setParameter("j_username", username);
                form.setParameter("j_password", password);
                form.submit();
            }
        } catch (IOException e) {
            throw new AssertionError(e);
        } catch (SAXException e) {
            throw new AssertionError(e);
        }
    }

    /**
     * Looks for exception stack on tapestry error page Dumps response if there was an exception.
     */
    public static void assertNoException(WebTester tester) {
        try {
            tester.assertTextNotPresent("An exception has occurred");
        } catch (AssertionFailedError e) {
            tester.dumpResponse(System.err);
            throw e;
        }
    }

    /**
     * Returns the row count in a table. Don't forget to include +1 in assert count if you have a
     * table header.
     */
    public static int getRowCount(WebTester tester, String table) {
        return tester.getDialog().getWebTableBySummaryOrId(table).getRowCount();
    }

    /**
     * Translates between Tapestry index and normal index
     * 
     * @param id
     * @param index
     */
    public static void checkCheckbox(WebTester tester, String id, int index) {
        String suffix = "";
        if (index > 0) {
            suffix = "$" + (index - 1);
        }
        tester.checkCheckbox(id + suffix);
    }

    public static String getBaseUrl() {
        return s_baseUrl;
    }

    public static String getClasspathDirectory() {
        return TestUtil.getClasspathDirectory(SiteTestHelper.class);
    }

    public static String getBuildDirectory() {
        if (s_buildDir == null) {
            s_buildDir = TestUtil.getBuildDirectory("web");
        }

        return s_buildDir;
    }
}
