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


public class EditPhoneTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;
        
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPhoneTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
        m_helper.seedPhone(1);
        clickLink("ManagePhones");        
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        setFormElement("serialNumber", "a00000000000");
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testEditPhone() {
        setFormElement("serialNumber", "a00000000001");
        clickButton("phone:ok");
        String[][] table = new String[][] {
            { "a00000000001", "", "Polycom SoundPoint IP 500" },                
        };
        assertTextInTable("phone:list", table[0]);        
    }

    public void testAddLine() {
        clickLink("AddLine");
        assertElementPresent("user:list");
    }
}
