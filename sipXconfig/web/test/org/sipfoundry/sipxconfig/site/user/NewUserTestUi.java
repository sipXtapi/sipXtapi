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
        clickLink("seedTestUser");
    }
    
    
    /** FIXME XCF-560 */    
    public void test___DeleteThisTestWhenFixmeResolved___() {} //  test fails unless at least one test
    public void _testListUsers() throws Exception {
        // Pick a username that is likely to be unique so we can run the test
        //  repeatedly without resetting the DB
        final String NEW_USER_USERNAME = TestPage.TEST_USER_USERNAME + Long.toString(System.currentTimeMillis());
        final String NEW_USER_FNAME = "NewUserFname";
        final String NEW_USER_LNAME = "NewUserLname";
        final String NEW_USER_PWORD = "1234";
        final String NEW_USER_ALIASES = "lazyboy, 993";
        
        SiteTestHelper.home(tester);
        clickLink("NewUser"); 
        setFormElement("userId", NEW_USER_USERNAME);
        setFormElement("firstName", NEW_USER_FNAME);
        setFormElement("lastName", NEW_USER_LNAME);
        setFormElement("password", NEW_USER_PWORD);
        setFormElement("confirmPassword", NEW_USER_PWORD);
        setFormElement("aliases", NEW_USER_ALIASES);
        clickButton("user:save");
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);

        SiteTestHelper.home(tester);
        clickLink("ManageUsers");
        String[][] table = new String[][] {
                { NEW_USER_FNAME, NEW_USER_LNAME, NEW_USER_USERNAME, NEW_USER_ALIASES },
                { TestPage.TEST_USER_FIRSTNAME, TestPage.TEST_USER_LASTNAME,
                    TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_ALIASES }                
            };
        assertTableRowsEqual("user:list", 1, table);                        
    }
}
