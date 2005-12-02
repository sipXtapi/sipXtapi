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
package org.sipfoundry.sipxconfig.components;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class BorderTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(BorderTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
    }    
    
    /**
     * Checks if navigation disappears/re-appears when we click the toggle 
     */
    public void testToggleNavigation() {
        SiteTestHelper.assertNoException(getTester());
        assertElementNotPresent("navigation");
        assertElementNotPresent("content");
        assertElementPresent("_content");

        clickLink("toggleNavigation");

        SiteTestHelper.assertNoException(getTester());
        assertElementPresent("navigation");
        assertElementPresent("content");
        assertElementNotPresent("_content");
        
        clickLink("toggleNavigation");

        SiteTestHelper.assertNoException(getTester());
        assertElementNotPresent("navigation");
        assertElementNotPresent("content");
        assertElementPresent("_content");
    }

    
    public void testLogout() {
        // display navigation and click logout link
        clickLink("toggleNavigation");
        clickLink("link.logout");
        
        // login form should be visible
        SiteTestHelper.assertNoException(getTester());
        assertFormPresent("login:form");
        SiteTestHelper.assertNoUserError(getTester());
    }
    
}
