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
package org.sipfoundry.sipxconfig.site.service;

import junit.extensions.TestDecorator;
import junit.framework.Test;
import junit.framework.TestResult;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ListConfiguredServicesTestUi extends WebTestCase {
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListConfiguredServicesTestUi.class);
    }

    public void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("resetConfiguredServices");
        clickLink("link:configuredServices");
    }

    public void testDisplay() {
        assertElementPresent("service:list");
        SiteTestHelper.assertNoException(getTester());
    }
    
    public void testNewService() {
        seedNtpService("new ntp service");
        assertElementPresent("service:list");
        String[][] expected = new String[][] {
                {"new ntp service", "Disabled", "1.1.1.1", "NTP" }
        };
        assertTableRowsEqual("service:list", 1, expected);        
    }
    
    private void seedNtpService(String name) {
        setFormElement("newService", "0");
        assertElementPresent("server:form");
        setFormElement("name", name);
        setFormElement("address", "1.1.1.1");
        clickButton("form:ok");        
    }

    public void testEditService() {
        seedNtpService("edit test");
        clickLinkWithText("edit test");
        assertElementPresent("server:form");        
    }
}
