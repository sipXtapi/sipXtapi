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
package org.sipfoundry.sipxconfig.site.user;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;


public class ManageUsersTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageUsersTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }
    
    public void testListUsers() throws Exception {
        clickLink("ManageUsers");              
        SiteTestHelper.assertNoException(tester);        
    }
    
    /**
     * DISABLED until sipXconfig 3.2
     */
    public void _testNewUser() throws Exception {
        clickLink("ManageUsers");
        clickLink("AddUser");
        setFormElement("userId", "new-user-test");
        setFormElement("firstName", "NewUserFname");
        setFormElement("lastName", "NewUserLname");
        setFormElement("extension", "993");
        setFormElement("pin", "1234");
        setFormElement("pinConfirm", "1234");
        clickButton("user:ok");
        SiteTestHelper.assertNoException(tester);
        // assert user is in table
    }
    
}
