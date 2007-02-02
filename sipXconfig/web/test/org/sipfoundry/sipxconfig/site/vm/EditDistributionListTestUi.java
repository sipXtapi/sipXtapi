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
package org.sipfoundry.sipxconfig.site.vm;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

public class EditDistributionListTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditDistributionListTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        SiteTestHelper.home(getTester());    
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("resetVoicemail");
        clickLink("loginFirstTestUser"); 
        clickLink("toggleNavigation");         
        clickLink("menu.distributionLists");
    }
    
    public void testSetList() throws Exception {
        SiteTestHelper.assertNoException(tester);
        setFormElement("subject", "200 201 202");
        clickButton("form:apply");
        SiteTestHelper.assertNoUserError(tester);
        assertFormElementEquals("subject", "200 201 202");
    }
}
