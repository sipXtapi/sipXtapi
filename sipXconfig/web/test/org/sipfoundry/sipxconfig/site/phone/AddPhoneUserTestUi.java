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

    private PhoneTestHelper m_helper;
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AddPhoneUserTestUi.class);
    }
   
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }
    
    protected void tearDown() throws Exception {
        super.tearDown();
        dumpResponse(System.err);
    }

    /** 
     * does not actually add user
     */
    public void testUserSearch() {
        m_helper.seedUser();
        m_helper.seedPhone(1);
        clickLink("ManagePhones");        
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        clickLinkWithText("Lines");        
        clickLink("AddUser");        
        clickButton("user:search");        
        clickButton("user:cancel");
        assertLinkPresent("AddUser");
    }
}
