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


public class PhoneTestUiBase extends WebTestCase {
    
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        beginAt("/");
        clickLink("resetPhoneContext");        
    }

    protected void seedNewPhone() {
        beginAt("/");
        clickLink("NewPhone");
        setFormElement("serialNumber", "000000000000");
        setFormElement("phoneModel", "1");
        clickButton("phone:ok");
        String[][] table = new String[][] {
            { "000000000000", "", "SoundPoint IP 500" },                
        };
        assertTextInTable("phone:list", table[0]);    
        beginAt("/");
    }
}
