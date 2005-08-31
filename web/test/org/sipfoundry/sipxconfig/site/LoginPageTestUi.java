/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import com.meterware.httpunit.WebForm;

public class LoginPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(LoginPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
    }    
    
    public void testLoginWithUserName() throws Exception {       
        checkLogin(TestPage.TEST_USER_USERNAME);
    }
    
    /** FIXME XCF-560 */
    public void _testLoginWithFirstAlias() throws Exception {       
        checkLogin(TestPage.TEST_USER_ALIAS1);
    }
    
    /** FIXME XCF-560 */
    public void _testLoginWithSecondAlias() throws Exception {       
        checkLogin(TestPage.TEST_USER_ALIAS2);
    }
    
    private void checkLogin(String userId) {        
        tester.beginAt("/");        
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.assertNoUserError(getTester());
        
        WebForm form = tester.getDialog().getForm();
        form.setParameter("userName", userId);
        form.setParameter("password", TestPage.TEST_USER_PIN);
        clickButton("login:submit");
                
        // we are on the home page now - no errors no login form
        SiteTestHelper.assertNoException(getTester());
        assertFormNotPresent("login:form");
        assertElementNotPresent("user:error");        
    }
    
    // successful login is tested by "home" function
    public void testLoginFailed() throws Exception {
        tester.beginAt("/");
        
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.assertNoUserError(getTester());
        
        WebForm form = tester.getDialog().getForm();
        form.setParameter("userName", "xyz");
        form.setParameter("password", "abc");
        clickButton("login:submit");
        
        // still on the same page
        SiteTestHelper.assertNoException(getTester());
        assertFormPresent("login:form");
        assertElementPresent("user:error");
    }
}
