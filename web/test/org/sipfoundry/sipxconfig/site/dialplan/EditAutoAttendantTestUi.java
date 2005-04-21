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
import net.sourceforge.jwebunit.WebTestCase;

import org.apache.commons.io.CopyUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.common.TestUtil;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditAutoAttendantTestUi extends WebTestCase {
    
    private static final String PROMPT_TEST_FILE = "thankyou_goodbye.wav";
    
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
        String[][] expectedMenuItems = {
                { "0123456789*#", "Cancel", "" }, 
                { "0123456789*#", "Operator", "" }, 
        };
        assertTableRowsEqual("attendant:menuItems", 1, expectedMenuItems);        

        setFormElement("name", "New Attendant Test");
        setFormElement("description", "created by EditAutoAttendantTestUi.testNewAttendant");
        setFormElement("extension", "301");
        selectOption("prompt", PROMPT_TEST_FILE);
        setFormElement("name", "New Attendant");
        clickButton("attendant:ok");
        SiteTestHelper.assertNoException(tester);
    }
        
    public void testUpload() {
        File expectedFile = new File(getCleanPromptsDir(), PROMPT_TEST_FILE);
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

    public static final String seedPromptFile() throws IOException {
        File promptsDir = getCleanPromptsDir();
        copyFileToDirectory(PROMPT_TEST_FILE, promptsDir);
        
        return PROMPT_TEST_FILE;
    }
    
    private static final File getCleanPromptsDir() {
        File promptsDir = new File(SiteTestHelper.getArtificialSystemRootDirectory() + "/prompts");
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
