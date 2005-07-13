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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class RestartReminderTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(RestartReminderTestUi.class);
    }
    
    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink(RestartReminder.PAGE);
    }
    
    public void testRestartNow() {
        SiteTestHelper.assertNoException(tester);
        assertFormPresent("restart:form");
        
        uncheckCheckbox("checkRestartLater");
        
        clickButton("restart:save");

        // should be back at the test page
        assertLinkPresent(RestartReminder.PAGE);
        // there will be some exceptions in the log - topology file is not found
    }
    
    public void testRestartLater() {
        SiteTestHelper.assertNoException(tester);
        assertFormPresent("restart:form");

        checkCheckbox("checkRestartLater");
        
        clickButton("restart:save");
        
        // should be back at the test page
        assertLinkPresent(RestartReminder.PAGE);
    }

}
