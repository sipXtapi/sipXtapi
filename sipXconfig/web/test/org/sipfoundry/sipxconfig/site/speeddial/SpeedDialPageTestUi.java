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
package org.sipfoundry.sipxconfig.site.speeddial;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class SpeedDialPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(SpeedDialPageTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester(), true);
    }

    public void testDisplay() throws Exception {
        clickLink("SpeedDialPage");
        SiteTestHelper.assertNoUserError(tester);
        assertButtonPresent("form:ok");
        assertButtonPresent("form:cancel");
        assertButtonPresent("form:apply");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);        
    }

    public void testUpdatePhones() throws Exception {
        // just exercises page for error
        clickLink("SpeedDialPage");
        clickButton("form:updatePhones");
        SiteTestHelper.assertNoUserError(tester);        
    }
}
