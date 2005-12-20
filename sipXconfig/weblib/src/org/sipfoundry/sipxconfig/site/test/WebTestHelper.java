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
package org.sipfoundry.sipxconfig.site.test;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Field;

import junit.extensions.TestSetup;
import junit.framework.Assert;
import junit.framework.AssertionFailedError;
import junit.framework.TestSuite;
import net.sourceforge.jwebunit.HttpUnitDialog;
import net.sourceforge.jwebunit.WebTester;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxconfig.common.TestUtil;
import org.springframework.context.support.ClassPathXmlApplicationContext;
import org.w3c.dom.Element;

import com.meterware.httpunit.WebForm;
import com.meterware.httpunit.WebResponse;

public class WebTestHelper {

    /**
     * Same userName that TestPage creates. thought of referencing static variable but may pull in
     * unnec. dependencies
     */
    public static final String TEST_USER = "testuser";

    /**
     * The name of the checkbox used in standard tables
     */
    public static final String ROW_CHECKBOX = "checkbox";
    
    private String m_baseUrl;

    private String m_project;
    
    private WebTestSetupFactory m_testSetupFactory;
    
    private static WebTestHelper s_instance;
    
    public static WebTestHelper getInstance() {
        if (s_instance == null) {
            String project = "weblib";
            System.setProperty("project", project);            
            System.setProperty("buildDirectory", TestUtil.getBuildDirectory(project));
            ClassPathXmlApplicationContext context = new ClassPathXmlApplicationContext("web.test.beans.xml");
            s_instance = (WebTestHelper) context.getBean("webTestHelper");            
        }
        
        return s_instance;
    }
    
    public static void main(String[] args) {
        getInstance().m_testSetupFactory.runMain(args);
    }
    
    public String getProject() {
        return m_project;
    }

    public void setProject(String project) {
        m_project = project;
    }
    
    public void setTestSetupFactory(WebTestSetupFactory testSetupFactory) {
        m_testSetupFactory = testSetupFactory;
        m_baseUrl = m_testSetupFactory.getUrl();
    }

    public TestSetup createWebTestSuite(Class webTestClass) {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(webTestClass);

        TestSetup webSetup = m_testSetupFactory.createTestSetup(suite);

        return webSetup;
    }

    /**
     * Go to TestPage.html and log in. Includes hack for slow machines.
     */
    public void home(WebTester tester) {
        tester.beginAt("/app?service=page/TestPage");
        
        // HACK: Webunit doesn't appear to fully load page, especially
        // when the machine you're running it on is slow and you're
        // running a batch of tests, calling beginAt("/") twice seems
        // to get webunit to catch up.
        tester.beginAt("/app?service=page/TestPage");
        
        assertNoException(tester);
    }

    /**
     * Looks for exception stack on tapestry error page. Dumps response if there was an exception.
     */
    public void assertNoException(WebTester tester) {
        try {
            tester.assertElementNotPresent("exceptionDisplay");
        } catch (AssertionFailedError e) {
            tester.dumpResponse(System.err);
            throw e;
        }
    }

    /**
     * Works only for pages that use "user:error:text" id to display user errors. All pages with
     * ErrorMsg component belong to this category.
     */
    public void assertNoUserError(WebTester tester) {
        Element element = tester.getDialog().getElement("user:error");
        if (null != element) {
            Assert.fail("User error on page: " + element.getFirstChild().getNodeValue());
        }
    }

    public void assertUserError(WebTester tester) {
        Element element = tester.getDialog().getElement("user:error");
        if (null == element) {
            Assert.fail("Expected user error on the page.");
        }
    }

    /**
     * Returns the row count in a table. Don't forget to include +1 in assert count if you have a
     * table header.
     */
    public int getRowCount(WebTester tester, String table) {
        return tester.getDialog().getWebTableBySummaryOrId(table).getRowCount();
    }

    /**
     * Translates between Tapestry index and normal index
     * 
     * @param id
     * @param index
     */
    public void enableCheckbox(WebTester tester, String id, int index, boolean enable) {
        String field = getIndexedId(id, index);
        if (enable) {
            tester.checkCheckbox(field);
        } else {
            tester.uncheckCheckbox(field);
        }
    }

    /**
     * Select/unselect rows in the table Only works if there is a single table on the screen.
     * 
     * @param tester
     * @param index row number starting from 0
     * @param enable True to select checkbox, false otherwise
     */
    public void selectRow(WebTester tester, int index, boolean enable) {
        String field = getIndexedId(ROW_CHECKBOX, index);
        if (enable) {
            tester.checkCheckbox(field);
        } else {
            tester.uncheckCheckbox(field);
        }
    }

    public void assertRowSelected(WebTester tester, int index) {
        String field = getIndexedId(ROW_CHECKBOX, index);
        tester.assertCheckboxSelected(field);
    }

    public void assertRowNotSelected(WebTester tester, int index) {
        String field = getIndexedId(ROW_CHECKBOX, index);
        tester.assertCheckboxNotSelected(field);
    }

    /**
     * Translates between Tapestry index and normal index
     * 
     * @param id HTML element id
     * @param index tapestry index
     */
    public String getIndexedId(String id, int index) {
        String suffix = "";
        if (index > 0) {
            suffix = "$" + (index - 1);
        }
        return id + suffix;
    }

    public void assertOptionSelected(WebTester tester, String formElement, String expected) {
        String value = tester.getDialog().getSelectedOption(formElement);
        Assert.assertEquals(expected, value);
    }

    public String getBaseUrl() {
        return m_baseUrl;
    }

    /**
     * Create a dir if it doesn't exists and deletes all contents if it does exist
     */
    public String cleanDirectory(String directory) {
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
     * Utility function to click on Tapestry submit links from JWebUnit
     * 
     * The linkSubmit component is using a java script to set a value of a hidden field and then
     * submit a form. HttpUnit/JWebUnit and rhino.jar do not support java script used by Tapestry
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
    public void clickSubmitLink(WebTester tester, String linkName) throws Exception {
        // submit the form after setting hidden field
        HttpUnitDialog dialog = tester.getDialog();
        WebForm form = dialog.getForm();
        if (linkName != null) {
            form.getScriptableObject().setParameterValue("_linkSubmit", linkName);
        }
        WebResponse response = form.submitNoButton();

        // set response directly in current JWebUnit object
        Class klass = dialog.getClass();
        Field respField = klass.getDeclaredField("resp");
        respField.setAccessible(true);
        respField.set(dialog, response);

        Assert.assertSame(tester.getDialog().getResponse(), response);
    }

    public void submitNoButton(WebTester tester) throws Exception {
        clickSubmitLink(tester, null);
    }
}
