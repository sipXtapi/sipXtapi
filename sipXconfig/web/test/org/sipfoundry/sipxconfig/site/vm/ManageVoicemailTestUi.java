/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;


import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;


public class ManageVoicemailTestUi extends WebTestCase {
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageVoicemailTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
    }
    
    private void gotoManageVoicemail() {
        SiteTestHelper.home(getTester());    
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("resetVoicemail");              
        clickLink("loginFirstTestUser");              
        clickLink("ManageVoicemail");                      
    }
    
    public void testDisplay() throws Exception {
        gotoManageVoicemail();
        SiteTestHelper.assertNoException(tester);        
        SiteTestHelper.assertNoUserError(tester);        
        assertElementPresent("voicemail:list");
    }    
    
    public void testNavigation() throws Exception {
        gotoManageVoicemail();
        assertTextPresent("Voice Message 00000002");
        clickLink("link:deleted");
        assertTextPresent("Voice Message 00000001");
    }
    
    public void testMove() throws Exception {
        gotoManageVoicemail();
        assertTextPresent("Voice Message 00000002");
        checkCheckbox("checkbox");
        getDialog().setFormParameter("actionSelection", "org.sipfoundry.sipxconfig.site.vm.MoveVoicemailActiondeleted");
        assertTextNotPresent("Voice Message 00000002");
        clickLink("link:deleted");
        assertTextPresent("Voice Message 00000001");
        assertTextPresent("Voice Message 00000002");
    }
    
    public void testEdit() throws Exception {
        gotoManageVoicemail();
        clickLinkWithText("Voice Message 00000002");
        assertElementPresent("voicemail:edit");
    }   
    
    public void testPageServiceShouldntRedirect() throws Exception {
        SiteTestHelper.home(getTester());    
        clickLink("seedTestUser");
        clickLink("Logout");
        clickLink("ManageVoicemail");
        login(TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_PIN);
        assertElementNotPresent("voicemail:edit");
    }

    public void testExternalServiceShouldRedirect() throws Exception {
        SiteTestHelper.home(getTester());    
        clickLink("seedTestUser");
        clickLink("Logout");        
        gotoPage(String.format("mailbox/%s/inbox", TestPage.TEST_USER_USERNAME));
        login(TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_PIN);
        assertElementPresent("voicemail:list");
    }

    public void testExternalServiceShouldRedirectSecondPasswordAttempt() throws Exception {
        SiteTestHelper.home(getTester());    
        clickLink("seedTestUser");
        clickLink("Logout");
        gotoPage(String.format("mailbox/%s/inbox", TestPage.TEST_USER_USERNAME));
        login(TestPage.TEST_USER_USERNAME, "Bogus");
        login(TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_PIN);
        assertElementPresent("voicemail:list");
    }
    
    public void testPlayFriendlyUrl() throws Exception {
        gotoManageVoicemail();
        gotoPage(String.format("mailbox/%s/inbox/00000002-00", TestPage.TEST_USER_USERNAME));
        assertEquals("audio/x-wav", getDialog().getResponse().getContentType());                
    }
    
    public void testDeleteFriendlyUrl() throws Exception {
        gotoManageVoicemail();
        assertTextPresent("00000002-00");
        gotoPage(String.format("mailbox/%s/inbox/00000002-00/delete", TestPage.TEST_USER_USERNAME));
        assertTextNotPresent("00000002-00");
    }

    
    private void login(String username, String password) {
        assertElementPresent("login:form");
        setFormElement("userName", username);
        setFormElement("password", password);
        clickButton("login:submit");        
    }
}


