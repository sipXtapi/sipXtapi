/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ManagePresenceTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManagePresenceTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.seedUser(getTester());
        tester.clickLink("loginFirstTestUser");
        SiteTestHelper.home(getTester());
        tester.clickLink("managePresence");
    }
    
    public void testNotEnabled() {
        assertElementNotPresent("menu.managePresence");
        
        // not enabled by in unit test env.
        // system test should run tests
        assertElementPresent("presence-disabled");
    }
}
