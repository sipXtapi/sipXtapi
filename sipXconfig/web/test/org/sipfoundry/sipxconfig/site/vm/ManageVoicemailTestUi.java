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
        clickLink("resetVoicemail");              
        clickLink("loginFirstTestUser");              
        clickLink("ManageVoicemail");              
    }
    
    public void testDisplay() throws Exception {
        SiteTestHelper.assertNoException(tester);        
        SiteTestHelper.assertNoUserError(tester);        
    }    
}
