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


public class AddPhoneUserTestUi extends PhoneTestUiBase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AddPhoneUserTestUi.class);
    }

    /** 
     * does not actually add user
     */
    public void testUserSearch() {
        seedUser();
        seedNewPhone();
        clickLink("ManagePhones");        
        clickLinkWithText("000000000000");
        clickLinkWithText("Lines");        
        clickLink("AddUser");        
        clickButton("user:search");        
        clickButton("user:cancel");
        assertLinkPresent("AddUser");
    }
}
