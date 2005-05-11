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
package org.sipfoundry.sipxconfig.site.dialplan;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import junit.framework.Test;
import net.sourceforge.jwebunit.ExpectedTable;
import net.sourceforge.jwebunit.WebTestCase;
import net.sourceforge.jwebunit.WebTester;

import org.apache.commons.io.CopyUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.common.TestUtil;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditAutoAttendantTestUi extends WebTestCase {
    
    static public final String PROMPT_TEST_FILE = "thankyou_goodbye.wav";
    
    private static final String KEYS = "0123456789*#";
    
    private static final String[][] FACTORY_DEFAULT = {
        { KEYS, "Repeat Prompt", "" }, 
        { KEYS, "Operator", "" } 
    }; 
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditAutoAttendantTestUi.class);
    }

    protected void setUp() throws IOException {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
        SiteTestHelper.assertNoException(tester);
    }
        
    public void testNewAttendant() throws IOException {
        seedPromptFile();
        clickLink("NewAutoAttendant");

        // need to rerender page after prompt is copied in
        assertTableRowsEqual("attendant:menuItems", 1, FACTORY_DEFAULT);        

        setFormElement("name", "New Attendant");
        setFormElement("description", "created by EditAutoAttendantTestUi.testNewAttendant");
        setFormElement("extension", "301");
        selectOption("prompt", PROMPT_TEST_FILE);
        clickButton("attendant:ok");
        SiteTestHelper.assertNoException(tester);
    }
        
    public void testUpload() {
        File expectedFile = new File(getCleanPromptsDir("prompts"), PROMPT_TEST_FILE);
        assertFalse(expectedFile.exists());
        clickLink("NewAutoAttendant");
        
        setFormElement("name", "Upload Prompt Test");
        setFormElement("description", "created by EditAutoAttendantTestUi.testUpload");
        setFormElement("extension", "301");
        String actualFilename = TestUtil.getTestSourceDirectory(getClass()) + "/" + PROMPT_TEST_FILE;
        File actualFile = new File(actualFilename);
        assertTrue(actualFile.exists());
        getDialog().getForm().setParameter("promptUpload", actualFile);
        clickButton("attendant:ok");        
        SiteTestHelper.assertNoException(tester);
        assertTrue(expectedFile.exists());
        assertEquals(actualFile.length(), expectedFile.length());
    }
    
    public void testCancel() {
        clickLink("NewAutoAttendant");
        assertFormPresent("attendant:form");
        clickButton("attendant:cancel");
        assertTablePresent("list:attendant");
    }

    public void testReset() throws Exception {
        seedPromptFile();
        clickLink("NewAutoAttendant");
        selectOption("addMenuItemAction", "Voicemail Login");
        clickButton("attendant:addMenuItem");
        clickButton("attendant:reset");
        assertTableRowsEqual("attendant:menuItems", 1, FACTORY_DEFAULT);        
    }

    public void testRemoveMenuItems() throws Exception {
        seedPromptFile();
        clickLink("NewAutoAttendant");
        assertFormPresent("attendant:form");
        
        SiteTestHelper.checkCheckbox(tester, "selectedRow", 0);
        clickButton("attendant:removeMenuItems");
        String[][] expectedMenuItems = {
                { KEYS, "Operator", "" }, 
        };
        assertTableRowsEqual("attendant:menuItems", 1, expectedMenuItems);        
    }
    
    public void testAddMenuItems() throws Exception {
        seedAutoAttendant(tester);
        SiteTestHelper.home(tester);
        
        clickLink("NewAutoAttendant");
        assertFormPresent("attendant:form");
        
        selectOption("addMenuItemAction", "Auto Attendant");
        clickButton("attendant:addMenuItem");
        selectOption("attendantParameter", "New Attendant");
        clickButton("attendant:apply");

        ExpectedTable expected = new ExpectedTable(FACTORY_DEFAULT);
        String[][] defaultMenuItems = {
                { KEYS, "Auto Attendant", "select...New Attendant" }, 
        };
        expected.appendRows(defaultMenuItems);
        assertTableRowsEqual("attendant:menuItems", 1, expected);
        SiteTestHelper.assertOptionSelected(tester, "attendantParameter", "New Attendant");
        
        selectOption("addMenuItemAction", "Deposit Voicemail");
        clickButton("attendant:addMenuItem");
        setFormElement("extensionParameter", "3232");
        clickButton("attendant:apply");
        
        String[][] vmDepositRow = {
                // 3rd column - curious why text fields do not show up text in table when
                // all other form elements do. May be related to webunit version.
                { KEYS, "Deposit Voicemail", "" }, 
        };
        // verify 3rd column data here
        assertFormElementEquals("extensionParameter", "3232");
        
        expected.appendRows(vmDepositRow);
        assertTableRowsEqual("attendant:menuItems", 1, expected);
    }

    public static final String seedPromptFile(String dir) throws IOException {
        File promptsDir = getCleanPromptsDir(dir);
        copyFileToDirectory(PROMPT_TEST_FILE, promptsDir);
        
        return PROMPT_TEST_FILE;
    }

    public static final String seedPromptFile() throws IOException {
        return seedPromptFile("prompts");
    }
    
    private static void seedAutoAttendant(WebTester tester) throws Exception {
        seedPromptFile();
        tester.clickLink("NewAutoAttendant");
        tester.setFormElement("name", "New Attendant");
        tester.setFormElement("description",
                "created by EditAutoAttendantTestUi.seedAutoAttendant");
        tester.setFormElement("extension", "301");
        tester.selectOption("prompt", PROMPT_TEST_FILE);
        tester.clickButton("attendant:ok");
    }

    private static final File getCleanPromptsDir(String dir) {
        File promptsDir = new File(SiteTestHelper.getArtificialSystemRootDirectory(), dir);
        SiteTestHelper.cleanDirectory(promptsDir.getPath());
        return promptsDir;
    }

    private static final void copyFileToDirectory(String filename, File dir) throws IOException {
        InputStream in = EditAutoAttendantTestUi.class.getResourceAsStream(filename);
        SiteTestHelper.cleanDirectory(dir.getPath());
        FileOutputStream out = new FileOutputStream(new File(dir, filename));
        CopyUtils.copy(in, out);
        IOUtils.closeQuietly(in);
        IOUtils.closeQuietly(out);
    }
}
