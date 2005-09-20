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
package org.sipfoundry.sipxconfig.site.line;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;


public class EditLineTestUi extends WebTestCase {
    
    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditLineTestUi.class);
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

    public void testEditLine() {
        m_helper.seedLine(1);
        clickLink("ManagePhones");        
        clickLinkWithText(SiteTestHelper.TEST_USER);
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testReturnToEditPhone() {
        navigateToAddLine();
        clickButton("user:cancel");
        assertFormPresent("phone:edit");
        SiteTestHelper.assertNoException(tester);        
    }
    
    public void testPhoneLinkOnEditLinePage() {
        navigateToAddLine();
        
        // Click on the phone label at the top of the page, which is a link
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        assertFormPresent("phone:edit");
        
        // Cancel to go back to the add line page
        clickButton("form:cancel");
        assertFormPresent("line:add");        
    }
    
    private void navigateToAddLine() {
        m_helper.seedLine(1);
        clickLink("ManagePhones");        
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        clickLinkWithText("Add Line");
        SiteTestHelper.assertNoException(tester);        
        assertFormPresent("line:add");        
    }
    
}
