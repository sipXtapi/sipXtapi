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
package org.sipfoundry.sipxconfig.site.user_portal;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

/**
 * UserCallForwardingTestUi
 */
public class UserCallForwardingTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserCallForwardingTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
    }

    public void testDisplayUserCallForwarding() throws Exception {
        clickLink("UserCallForwarding");
        assertTextNotPresent("An exception has occurred.");
        assertTextPresent("Call Forwarding");
    }
}
