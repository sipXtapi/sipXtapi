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

import com.meterware.httpunit.WebForm;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

public class LoginPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(LoginPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
    }    
    

    public void testLogin() throws Exception {
        tester.beginAt("/");
        
        SiteTestHelper.assertNoException(getTester());
        SiteTestHelper.assertNoUserError(getTester());
        
        WebForm form = tester.getDialog().getForm();
        form.setParameter("userName", TestPage.ADMIN);
        form.setParameter("password", TestPage.PINTOKEN);
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
