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
package org.sipfoundry.sipxconfig.site.acd;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class AcdHistoryPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AcdHistoryPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("acdReports");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(tester);
    }    

    public void testReports() {        
        clickLink("report0");
        clickLink("report1");
        clickLink("report2");
        clickLink("report3");
        clickLink("report4");
        clickLink("report5");
        clickLink("report6");
    }    
}
