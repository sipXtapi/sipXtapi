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


public class AddPhoneUserTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AddPhoneUserTestUi.class);
    }
   
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        PhoneTestHelper.reset(tester);
    }

    /** 
     * does not actually add user
     */
    public void testUserSearch() {
        PhoneTestHelper.seedUser(tester);
        PhoneTestHelper.seedNewPhone(tester);
        clickLink("ManagePhones");        
        clickLinkWithText("000000000000");
        clickLinkWithText("Lines");        
        clickLink("AddUser");        
        clickButton("user:search");        
        clickButton("user:cancel");
        assertLinkPresent("AddUser");
    }
}
