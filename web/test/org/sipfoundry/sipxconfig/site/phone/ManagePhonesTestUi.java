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

    private PhoneTestHelper m_helper;
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManagePhonesTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    public void testGenerateProfiles() {
        m_helper.seedPhone(1);

        clickLink("ManagePhones");
        SiteTestHelper.checkCheckbox(tester, "selectedRow", 0);
        checkCheckbox("selectedRow");
        clickButton("phone:sendProfiles");
        SiteTestHelper.assertNoException(tester);
        // test if profiles were actually generated
    }    

    public void testRestart() {
        m_helper.seedPhone(1);

        clickLink("ManagePhones");   
        SiteTestHelper.checkCheckbox(tester, "selectedRow", 0);
        clickButton("phone:restart");
        SiteTestHelper.assertNoException(tester);        
        // test if SIP messages were sent
    }    
    
    public void testDelete() {
        m_helper.seedPhone(1);

        clickLink("ManagePhones");          
        SiteTestHelper.checkCheckbox(tester, "selectedRow", 0);
        clickButton("phone:delete");
        // 2 = 1 thead (columns) + 1 tfoot (pager)
        assertEquals(2, SiteTestHelper.getRowCount(tester, "phone:list"));
        
        SiteTestHelper.assertNoException(tester);
    }    

    public void testGroupFilter() throws Exception {
        m_helper.seedPhone(1);
        SiteTestHelper.seedGroup(tester, "NewPhoneGroup", 1);
        clickLink("ManagePhones");
        
        // all users
        int allTableCount = SiteTestHelper.getRowCount(tester, "phone:list");
        
        // empty group, no users
        setFormElement("groupFilter", "0");
        SiteTestHelper.submitNoButton(tester);
        SiteTestHelper.assertNoException(tester);
        int emptyTableCount = SiteTestHelper.getRowCount(tester, "phone:list");
        assertTrue(allTableCount > emptyTableCount);

        // back to all users
        setFormElement("groupFilter", "");
        SiteTestHelper.submitNoButton(tester);
        int allTableCountAgain = SiteTestHelper.getRowCount(tester, "phone:list");
        assertEquals(allTableCount, allTableCountAgain);
    }    
}
