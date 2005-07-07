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

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;

public class EditSipPasswordTestUi extends WebTestCase {
    
    private User user;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditSipPasswordTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        SiteTestHelper.home(getTester());
        clickLink("ManageUsers");
        PhoneTestHelper phoneTester = new PhoneTestHelper(tester);
        phoneTester.seedUser();  
        user = phoneTester.user[0];
        clickLinkWithText(user.getDisplayId());
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testDisplay() {
        assertFormPresent("user:sippassword");
    }

    public void testEditPassword() {
        setFormElement("sipPassword", "9999");
        clickButton("sippassword:save");
        assertTablePresent("user:criteria");
        clickLinkWithText(user.getDisplayId());
        assertFormElementEquals("sipPassword", "9999");
    }
}
