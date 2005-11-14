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

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class SnapshotPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(SnapshotPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("SnapshotPage");
    }

    /**
     * Does not check if operation was successful - just checks if no Tapestry exceptions show up
     * would have to send mock backup shell script to artificial root.
     */
    public void testDisplay() {
        SiteTestHelper.assertNoException(getTester());
        assertCheckboxSelected("wwwCheck");
        assertCheckboxSelected("logsCheck");
        assertCheckboxNotSelected("credentialsCheck");
        clickButton("form:apply");
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.assertNoUserError(getTester());
    }
}
