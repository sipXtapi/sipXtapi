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

public class ExtensionPoolsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ExtensionPoolsTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("ExtensionPools");
    }
    
    public void testDisplayExtensionPool() throws Exception {
        assertTextNotPresent("An exception has occurred.");
        assertTextPresent("User Extension Pool");
    }
    
    public void testChangeExtensionPool() throws Exception {
        final String start = "100";
        final String end = "199";
        checkCheckbox("enableExtensionPool");
        setFormElement("start", start);
        setFormElement("end", end);
        clickButton("form:apply");        
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);
        
        // Go away and back and verify that our changes took effect
        SiteTestHelper.home(getTester());
        clickLink("ExtensionPools");
        assertCheckboxSelected("enableExtensionPool");
        assertFormElementEquals("start", start);
        assertFormElementEquals("end", end);
    }
    
}
