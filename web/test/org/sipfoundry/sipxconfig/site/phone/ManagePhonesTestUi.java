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

public class ManagePhonesTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManagePhonesTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        PhoneTestHelper.reset(tester);
    }

    protected void tearDown() throws Exception {
        super.tearDown();
        dumpResponse(System.err);
    }

    public void testGenerateProfiles() {
        PhoneTestHelper.seedPhone(tester);

        clickLink("ManagePhones");          
        checkCheckbox("selectedRow");
        clickButton("phone:sendProfiles");
        // present if no exception
        assertLinkPresent("AddPhone");
    }    
}
