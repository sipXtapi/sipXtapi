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


public class EditPhoneSettingsTestUi extends WebTestCase {
    
    private PhoneTestHelper m_helper;
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPhoneSettingsTestUi.class);
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
    
    public void testEditSipSetttings() {
        final String holdParamName = "integerField";
        
        m_helper.seedPhone(1);
        clickLink("ManagePhones");        
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        // NOTE: Polycom only setting 
        clickLinkWithText("Hold Reminder");        
        // check seed data 
        assertEquals("0", getDialog().getFormParameterValue(holdParamName));
        setFormElement(holdParamName, "1");
        clickButton("setting:ok");

        // verify setting sticks
        SiteTestHelper.home(tester);
        clickLink("ManagePhones");        
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        clickLinkWithText("Hold Reminder");
        assertEquals("1", getDialog().getFormParameterValue(holdParamName));
    }
}
