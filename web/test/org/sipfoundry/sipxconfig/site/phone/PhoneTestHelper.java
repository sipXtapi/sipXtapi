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

import net.sourceforge.jwebunit.WebTester;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.PhoneData;
import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

/**
 * Helper methods for phone base unittesting.
 * 
 * Thought about making this a helper w/static functions but webunit methods are
 * protected, as they probably should be.
 */
public class PhoneTestHelper {
    
    public PhoneData[] endpoint;
    
    public LineData[] line;
    
    public User[] user;
    
    private WebTester m_tester;
    
    public PhoneTestHelper(WebTester tester) {
        m_tester = tester;
    }
        
    public void reset() {
        SiteTestHelper.home(m_tester);
        m_tester.clickLink("resetPhoneContext");
        SiteTestHelper.assertNoException(m_tester);
    }
    
    public void seedUser() {
        // hack, use the user created in neoconf db unittests, assumes those
        // tests have been run prior to running unittes.  should be possible
        // to call Add User in JSP web interface
        user = new User[] { new User() };
        user[0].setDisplayId("testuser");
    }

    public void seedPhone(int count) {
        SiteTestHelper.home(m_tester);
        endpoint = new PhoneData[count];
        for (int i = 0; i < endpoint.length; i++) {
            endpoint[i] = new PhoneData();
            String serNum = "000000000000" + i;
            endpoint[i].setSerialNumber(serNum.substring(serNum.length() - 12));
            m_tester.clickLink("NewPhone");
            m_tester.setFormElement("serialNumber", endpoint[i].getSerialNumber());
            m_tester.setFormElement("phoneModel", "1");
            m_tester.clickButton("phone:ok");
            SiteTestHelper.home(m_tester);
        }
    }
    
    public void seedLine(int count) {
        seedUser();
        seedPhone(1);
        SiteTestHelper.home(m_tester);
        m_tester.clickLink("ManagePhones");        
        m_tester.clickLinkWithText(endpoint[0].getSerialNumber());
        m_tester.clickLinkWithText("Lines");
        line = new LineData[count];
        for (int i = 0; i < line.length; i++) {
            line[0] = new LineData();
            line[0].setUser(user[0]);
            line[0].setPhoneData(endpoint[0]);
            m_tester.clickLink("AddUser");        
            m_tester.clickButton("user:search");
            // first (should be only?) row
            m_tester.checkCheckbox("selectedRow");
            m_tester.clickButton("user:select");
        }
        SiteTestHelper.home(m_tester);
    }
}
