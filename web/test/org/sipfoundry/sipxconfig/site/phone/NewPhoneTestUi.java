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

import org.sipfoundry.sipxconfig.site.SiteTestHelper;


public class NewPhoneTestUi extends PhoneTestUiBase {
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(NewPhoneTestUi.class);
    }
    
    public void testAddPhone() {
        clickLink("NewPhone");  
        setFormElement("serialNumber", "000000000000");
        setFormElement("phoneModel", "1");
        clickButton("phone:ok");
        String[][] table = new String[][] {
            { "000000000000", "", "SoundPoint IP 500" },                
        };
        assertTextInTable("phone:list", table[0]);
    }
}
