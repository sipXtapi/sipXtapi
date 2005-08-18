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

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Field;
import java.util.Properties;

import junit.framework.Assert;
import junit.framework.AssertionFailedError;
import junit.framework.Test;
import junit.framework.TestSuite;
import net.sourceforge.jwebunit.HttpUnitDialog;
import net.sourceforge.jwebunit.WebTester;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxconfig.common.TestUtil;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;

import com.meterware.httpunit.WebForm;
import com.meterware.httpunit.WebResponse;

public class SiteTestHelper {

    /**
     * Same userName that TestPage creates. thought of referencing static variable but may pull
     * in unnec. dependencies
     */
    public static final String TEST_USER = "testuser";

    private static String s_buildDir;

    private static String s_baseUrl;

    private static String s_artificialSystemRoot;

    public static Test webTestSuite(Class webTestClass) {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(webTestClass);

        JettyTestSetup jetty = new JettyTestSetup(suite);
        s_baseUrl = jetty.getUrl();

        return jetty;
    }

    /**
     * Go to TestPage.html, includes hack for slow machines.
     */
    public static void home(WebTester tester) {
        tester.beginAt("/app?service=page/TestPage");
        tester.clickLink("login");
        // HACK: Webunit doesn't appear to fully load page, especialy
        // when the machine you're running it on is slow and you're
        // running a batch of tests, calling beginAt("/") twice seems
        // to get webunit to catch up.
        tester.beginAt("/app?service=page/TestPage");
        assertNoException(tester);
    }

    /**
     * Login - form based loging for our pages.
     * 
     */
    public static void login(WebTester tester, String username, String password) {
        try {
            if (tester.getDialog().hasForm("login:form")) {
                WebForm form = tester.getDialog().getForm();
                form.setParameter("userName", username);
                form.setParameter("password", password);
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
     * Works only for pages that use "user:error:text" id to display user errors. All pages with
     * ErrorMsg component belong to this category.
     */
    public static void assertNoUserError(WebTester tester) {
        Element element = tester.getDialog().getElement("user:error");
        if (null != element) {
            Assert.fail("User error on page: " + element.getFirstChild().getNodeValue());
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

    public static void assertOptionSelected(WebTester tester, String formElement, String expected) {
        String value = tester.getDialog().getSelectedOption(formElement);
        Assert.assertEquals(expected, value);
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

    /**
     * Get the root directory mimicking an installed sipx system. Useful when web pages need to
     * reference files from other sipx projects. Unittest should copy in seed test files.
     */
    public static String getArtificialSystemRootDirectory() {
        if (null == s_artificialSystemRoot) {
            s_artificialSystemRoot = TestUtil.getTestOutputDirectory("web")
                    + "/artificial-system-root";
        }
        return s_artificialSystemRoot;
    }

    /**
     * Create a dir if it doesn't exists and deletes all contents if it does exist
     */
    public static String cleanDirectory(String directory) {
        File dir = new File(directory);
        if (!dir.exists()) {
            dir.mkdirs();
        } else {
            try {
                FileUtils.cleanDirectory(dir);
            } catch (IOException ioe) {
                throw new RuntimeException("Could not clean directory " + directory, ioe);
            }
        }

        return directory;
    }

    /**
     * Write out sipxconfig.properties for testing arg 0 - any path in the testing classpath arg 1 -
     * path to artificial root directory arg 2 - where output is generated
     */
    public static void main(String[] args) {
        Properties sysProps = new Properties();
        s_artificialSystemRoot = args[0];

        String systemDirectory = cleanDirectory(args[1]);
        String etcDirectory = systemDirectory + "/etc";
        sysProps.setProperty("vxml.promptsDirectory", systemDirectory + "/prompts");
        sysProps.setProperty("vxml.scriptsDirectory", systemDirectory + "/aa_vxml");
        sysProps.setProperty("orbitsGenerator.audioDirectory", systemDirectory
                + "/parkserver/music");

        // generates sipxconfig.properties in classpath (arg 0)
        TestUtil.setSysDirProperties(sysProps, etcDirectory, args[2]);
        TestUtil.saveSysDirProperties(sysProps, args[0]);
    }

    /**
     * Utility function to click on Tapestry submit links from JWebUnit
     * 
     * The linkSubmit component is using a jaa script to set a value of a hiddent field and then
     * submit a form. HttpUnit/JWebUnit and rhino.jar do not support java script used by Tapesty
     * so we try to emulate the behavior by using HTTP unit function. See: XCF-349
     * 
     * In addition to that we need to make JWebUnit happy: it does not know we submitted form
     * request independently, we need to reinject the response back into the dialog. I could not
     * find any reasonable way of doing that, so I used reflection to set private field. I guess
     * being able to test more pages is the most important factor here.
     * 
     * There is no guarantee that it will work with new version of Tapestry or JWebUnit
     * 
     * @param linkName - name of the link component (link id will NOT work)
     */
    public static void clickSubmitLink(WebTester tester, String linkName) throws Exception {
        // submit the form after setting hidden field
        HttpUnitDialog dialog = tester.getDialog();
        WebForm form = dialog.getForm();
        form.getScriptableObject().setParameterValue("_linkSubmit", linkName);
        WebResponse response = form.submitNoButton();

        // set response directry in current JWebUnit object
        Class klass = dialog.getClass();
        Field respField = klass.getDeclaredField("resp");
        respField.setAccessible(true);
        respField.set(dialog, response);

        Assert.assertSame(tester.getDialog().getResponse(), response);
    }

    public static void seedUser(WebTester tester) {
        home(tester);
        tester.clickLink("resetCoreContext");
        tester.clickLink("seedTestUser");
    }

    /**
     * Create a new group, user or phone
     * 
     * @param pageLinkId From the TestPage, what link to click to get to new group page
     */
    public static void seedGroup(WebTester tester, String pageLinkId, int count) {
        SiteTestHelper.home(tester);
        for (int i = 0; i < count; i++) {
            tester.clickLink(pageLinkId);
            tester.setFormElement("name", "seedGroup" + i);
            tester.clickButton("group:ok");
            SiteTestHelper.home(tester);
        }
    }

}
