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

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

/**
 * Helper methods for phone base unittesting.
 * 
 * Thought about making this a helper w/static functions but webunit methods are
 * protected, as they probably should be.
 */
public class PhoneTestUiBase extends WebTestCase {
    
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        beginAt("/");
        //getDialog().getResponseText();
        dumpResponse(System.err);
        //beginAt("/");
        assertTextNotPresent("An exception has occurred");
        clickLink("resetPhoneContext");        
    }
    
    protected void seedUser() {
        // hack, use the user created in neoconf db unittests, assumes those
        // tests have been run prior to running unittes.  should be possible
        // to call Add User in JSP web interface
    }

    protected void seedNewPhone() {
        beginAt("/");
        assertTextNotPresent("An exception has occurred");
        clickLink("NewPhone");
        setFormElement("serialNumber", "000000000000");
        setFormElement("phoneModel", "1");
        clickButton("phone:ok");
        String[][] table = new String[][] {
            { "000000000000", "", "SoundPoint IP 500" },                
        };
        assertTextInTable("phone:list", table[0]);    
        beginAt("/");
        assertTextNotPresent("An exception has occurred");
    }
}
