/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc. Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp. Licensed to SIPfoundry under a Contributor Agreement.
 *  $
 */
package org.sipfoundry.sipxconfig.upload;

import java.io.File;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditUploadTestUi extends WebTestCase {
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditUploadTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("link:resetUpload");
        clickLink("link:newUpload");
    }

    public void testNewUpload() throws Exception {
        setFormElement("name", "new upload");
        File f = File.createTempFile("new-upload-test", ".dat");
        assertTrue(getDialog().getForm().hasParameterNamed("promptUpload"));
        getDialog().getForm().setParameter("promptUpload", f);
        clickButton("form:ok");
        File uploadDir = new File(SiteTestHelper.getTftpDirectory());
        assertTrue(new File(uploadDir, f.getName()).exists());
    }
}
