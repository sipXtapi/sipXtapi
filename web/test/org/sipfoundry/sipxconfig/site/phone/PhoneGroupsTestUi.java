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
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class PhoneGroupsTestUi extends WebTestCase {
    
    PhoneTestHelper tester;
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PhoneGroupsTestUi.class);
    }
    
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        tester = new PhoneTestHelper(getTester());
    }

    public void testDisplay() {
        tester.reset();
        tester.seedGroup(2);
        SiteTestHelper.home(getTester());
        clickLink("PhoneGroups");
        SiteTestHelper.assertNoException(getTester());
        String[][] table = new String[][] {
                { "seedGroup0" },                
                { "seedGroup1" },                
            };
        assertTextInTable("group:list", table[0]);        
    }
}
