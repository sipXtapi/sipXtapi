/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.File;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class MusicOnHoldTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(MusicOnHoldTestUi.class);
    }

    private File m_tempFile;

    public void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        m_tempFile = File.createTempFile("MusicOnHoldTestUi", null);
    }

    public void testDisplay() throws Exception {
        clickLink("MusicOnHold");
        SiteTestHelper.assertNoException(tester);
        getDialog().getForm().setParameter("promptUpload", m_tempFile);
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        assertOptionEquals("prompt", m_tempFile.getName());
    }
}
