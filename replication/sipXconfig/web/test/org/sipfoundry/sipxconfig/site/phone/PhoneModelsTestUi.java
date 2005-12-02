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
package org.sipfoundry.sipxconfig.site.phone;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

public class PhoneModelsTestUi extends WebTestCase {
    PhoneTestHelper tester;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PhoneModelsTestUi.class);
    }
    
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        tester = new PhoneTestHelper(getTester());
    }

    public void testDisplay() {
        tester.reset();
        tester.seedGroup(1);        
        clickLink("PhoneGroups");
        clickLinkWithText("seedGroup0");
        SiteTestHelper.assertNoException(getTester());
        clickLinkWithText("Polycom SoundPoint IP 300");
        SiteTestHelper.assertNoException(getTester());
    }

    public void testEditGroup() {
        tester.reset();
        tester.seedGroup(1);        
        clickLink("PhoneGroups");
        clickLinkWithText("seedGroup0");
        SiteTestHelper.assertNoException(getTester());
        clickLink("group:edit");
        SiteTestHelper.assertNoException(getTester());
        assertFormElementEquals("name", "seedGroup0");
        clickButton("group:ok");
        assertLinkPresentWithText("Polycom SoundPoint IP 300");
    }

}
