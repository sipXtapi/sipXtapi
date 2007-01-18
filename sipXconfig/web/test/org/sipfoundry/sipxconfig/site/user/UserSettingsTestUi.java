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
package org.sipfoundry.sipxconfig.site.user;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class UserSettingsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserSettingsTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        SiteTestHelper.seedUser(tester);
        clickLink("ManageUsers");
        clickLinkWithText(SiteTestHelper.TEST_USER);        
    }
    
    public void testDisplay() {
        clickLink("permission.application.label");        
        clickLink("permission.call-handling.label");        
        SiteTestHelper.assertNoException(tester);        
    }

    public void testSaveSetting() {
        clickLink("permission.application.label");
        SiteTestHelper.enableCheckbox(tester, "booleanField", 0, true);
        
        clickButton("setting:ok");
        
        // click ok, then navigate back.  apply would work but this is more thurough.
        clickLinkWithText(SiteTestHelper.TEST_USER);        
        clickLink("permission.application.label");
        assertCheckboxSelected("booleanField");
    }    
}
