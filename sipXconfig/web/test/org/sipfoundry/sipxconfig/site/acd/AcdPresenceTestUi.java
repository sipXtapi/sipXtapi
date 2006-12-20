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
package org.sipfoundry.sipxconfig.site.acd;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

public class AcdPresenceTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AcdPresenceTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("acdPresenceServer");
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testDisplay() {
        assertButtonPresent("signInButton");        
        assertButtonPresent("signOutButton");        
        assertButtonPresent("refreshButton");        
    }
    
    public void testRefresh() {
        clickButton("refreshButton");
    }
}
