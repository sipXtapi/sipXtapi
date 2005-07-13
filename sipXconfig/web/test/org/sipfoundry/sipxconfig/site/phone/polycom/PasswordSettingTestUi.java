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
package org.sipfoundry.sipxconfig.site.phone.polycom;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;
import org.w3c.dom.Element;

public class PasswordSettingTestUi extends WebTestCase {
 
    private PhoneTestHelper m_helper;
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PasswordSettingTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }
    
    public void testEditSipSetttings() {
        m_helper.seedPhone(1);
        m_helper.seedLine(1);
        clickLink("ManagePhones");        
        clickLinkWithText(SiteTestHelper.TEST_USER);
        clickLinkWithText("Registration");
        Element passwordField = getDialog().getElement("setting:auth.password");        
        assertEquals("password", passwordField.getAttribute("type"));
    }
}
