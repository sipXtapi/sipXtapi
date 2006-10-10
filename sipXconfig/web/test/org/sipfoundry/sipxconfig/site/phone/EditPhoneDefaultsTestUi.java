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

public class EditPhoneDefaultsTestUi extends WebTestCase {
    
    PhoneTestHelper tester;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPhoneDefaultsTestUi.class);
    }
    
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        tester = new PhoneTestHelper(getTester());
    }
    
    public void testBooleanField() {
        tester.reset();
        tester.seedGroup(1);
        clickLink("PhoneGroups");
        clickLinkWithText("seedGroup0");
        clickLinkWithText("Polycom SoundPoint IP 300/301");        
        checkCheckbox("booleanField"); //24HourFormat
        clickButton("setting:ok");
        clickLinkWithText("Polycom SoundPoint IP 300/301");        
        assertCheckboxSelected("booleanField");
    }
}
