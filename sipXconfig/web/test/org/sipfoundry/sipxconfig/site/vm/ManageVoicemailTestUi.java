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


public class ManageVoicemailTestUi extends WebTestCase {
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageVoicemailTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        SiteTestHelper.home(getTester());    
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("resetVoicemail");              
        clickLink("loginFirstTestUser");              
        clickLink("ManageVoicemail");              
    }
    
    public void testDisplay() throws Exception {
        SiteTestHelper.assertNoException(tester);        
        SiteTestHelper.assertNoUserError(tester);        
    }    
    
    public void testNavigation() throws Exception {
        assertTextPresent("Voice Message 00000002");
        clickLink("link:deleted");
        assertTextPresent("Voice Message 00000001");
    }
    
    public void testMove() throws Exception {
        assertTextPresent("Voice Message 00000002");
        checkCheckbox("checkbox");
        getDialog().setFormParameter("actionSelection", "org.sipfoundry.sipxconfig.site.vm.MoveVoicemailActiondeleted");
        assertTextNotPresent("Voice Message 00000002");
        clickLink("link:deleted");
        assertTextPresent("Voice Message 00000001");
        assertTextPresent("Voice Message 00000002");
    }
    
    public void testEdit() throws Exception {
        clickLinkWithText("Voice Message 00000002");
        assertElementPresent("voicemail:edit");
    }   
}


