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
    
    public void testDisplay() throws Exception {
        clickLink("ManageUsers");              
        SiteTestHelper.assertNoException(tester);        
    }
    
    public void testAddUser() throws Exception {
        SiteTestHelper.seedUser(tester);
        clickLink("ManageUsers");
        clickLink("AddUser");
        clickButton("form:cancel");
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testGroupFilter() throws Exception {
        SiteTestHelper.seedUser(tester);
        SiteTestHelper.seedGroup(tester, "NewPhoneGroup", 1);
        clickLink("ManageUsers");
        
        // all users
        int allTableCount = SiteTestHelper.getRowCount(tester, "user:list");
        
        // empty group, no users
        setFormElement("groupFilter", "0");
        SiteTestHelper.submitNoButton(tester);
        SiteTestHelper.assertNoException(tester);
        int emptyTableCount = SiteTestHelper.getRowCount(tester, "user:list");
        assertTrue(allTableCount > emptyTableCount);

        // back to all users
        setFormElement("groupFilter", "");
        SiteTestHelper.submitNoButton(tester);
        int allTableCountAgain = SiteTestHelper.getRowCount(tester, "user:list");
        assertEquals(allTableCount, allTableCountAgain);
    }
    
}
