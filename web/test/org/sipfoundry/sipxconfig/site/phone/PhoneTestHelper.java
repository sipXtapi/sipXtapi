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

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

/**
 * Helper methods for phone base unittesting.
 * 
 * Thought about making this a helper w/static functions but webunit methods are
 * protected, as they probably should be.
 */
public class PhoneTestHelper {
        
    public static void reset(WebTester tester) {
        SiteTestHelper.home(tester);
        tester.clickLink("resetPhoneContext");        
    }
    
    public static void seedUser(WebTester tester_) {
        // hack, use the user created in neoconf db unittests, assumes those
        // tests have been run prior to running unittes.  should be possible
        // to call Add User in JSP web interface
    }

    public static void seedPhone(WebTester tester) {
        SiteTestHelper.home(tester);
        tester.clickLink("NewPhone");
        tester.setFormElement("serialNumber", "000000000000");
        tester.setFormElement("phoneModel", "1");
        tester.clickButton("phone:ok");
        String[][] table = new String[][] {
            { "000000000000", "", "SoundPoint IP 500" },                
        };
        tester.assertTextInTable("phone:list", table[0]);
        SiteTestHelper.home(tester);
    }
    
    public static void seedLine(WebTester tester) {
        SiteTestHelper.home(tester);
        tester.clickLink("ManagePhones");        
        tester.clickLinkWithText("000000000000");
        tester.clickLinkWithText("Lines");        
        tester.clickLink("AddUser");        
        tester.clickButton("user:search");
        // first (should be only?) row
        tester.checkCheckbox("selectedRow");
        tester.clickButton("user:select");
        SiteTestHelper.home(tester);
    }
}
