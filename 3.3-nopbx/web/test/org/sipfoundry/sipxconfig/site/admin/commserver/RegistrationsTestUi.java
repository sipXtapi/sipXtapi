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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class RegistrationsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(RegistrationsTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);

        clickLink("Registrations");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("registrations:refresh");
        assertElementPresent("registrations:page");
        clickButton("registrations:refresh");
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("registrations:refresh");
        assertElementPresent("registrations:page");
    }
}
