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
import org.sipfoundry.sipxconfig.site.TestPage;


public class NewUserTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(NewUserTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        tester.clickLink("resetCoreContext");
    }
    
    public void testListUsers() throws Exception {
        SiteTestHelper.home(tester);
        clickLink("NewUser"); 
        setFormElement("userId", "new-user-test");
        setFormElement("firstName", "NewUserFname");
        setFormElement("lastName", "NewUserLname");
        setFormElement("extension", "993");
        setFormElement("password", "1234");
        setFormElement("confirmPassword", "1234");
        clickButton("user:save");
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);

        SiteTestHelper.home(tester);
        clickLink("ManageUsers");
        String[][] table = new String[][] {
                { TestPage.TEST_USER.getUserName(), TestPage.TEST_USER.getFirstName(), 
                    TestPage.TEST_USER.getLastName(), TestPage.TEST_USER.getExtension()},                
                { "new-user-test" , "NewUserFname", "NewUserLname", "993"}
            };
        assertTableRowsEqual("user:list", 1, table);                        
    }
}
