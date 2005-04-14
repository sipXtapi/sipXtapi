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
import java.io.FileInputStream;
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
    
    private File m_promptsDir;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditAutoAttendantTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("EditAutoAttendant");
        SiteTestHelper.assertNoException(tester);

        m_promptsDir = new File(SiteTestHelper.getArtificialSystemRootDirectory() + "/prompts");
        SiteTestHelper.cleanDirectory(m_promptsDir.getPath());
    }
    
    public void testNewAttendant() throws IOException {
        String promptFile = "thankyou_goodbye.wav";
        copyFileToDirectory(promptFile, m_promptsDir);

        // need to rerender page after prompt is copied in
        SiteTestHelper.home(getTester());
        clickLink("EditAutoAttendant");

        setFormElement("name", "New Attendant Test");
        setFormElement("description", "created by EditAutoAttendantTestUi.testNewAttendant");
        setFormElement("extension", "301");
        selectOption("prompt", "thankyou_goodbye.wav");
        setFormElement("name", "New Attendant");
        clickButton("attendant:ok");
        SiteTestHelper.assertNoException(tester);
    }
        
    public void testUpload() {
        File expectedFile = new File(m_promptsDir, "thankyou_goodbye.wav");
        assertFalse(expectedFile.exists());
        
        setFormElement("name", "Upload Prompt Test");
        setFormElement("description", "created by EditAutoAttendantTestUi.testUpload");
        setFormElement("extension", "301");
        String actualFilename = TestUtil.getTestSourceDirectory(getClass()) + "/thankyou_goodbye.wav";
        File actualFile = new File(actualFilename);
        assertTrue(actualFile.exists());
        getDialog().getForm().setParameter("promptUpload", actualFile);
        clickButton("attendant:ok");        
        SiteTestHelper.assertNoException(tester);
        assertTrue(expectedFile.exists());
        assertEquals(actualFile.length(), expectedFile.length());
    }

    private void copyFileToDirectory(String filename, File dir) throws IOException {
        InputStream in = getClass().getResourceAsStream(filename);
        SiteTestHelper.cleanDirectory(dir.getPath());
        FileOutputStream out = new FileOutputStream(new File(dir, filename));
        CopyUtils.copy(in, out);
        IOUtils.closeQuietly(in);
        IOUtils.closeQuietly(out);
    }
}
