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
package org.sipfoundry.sipxconfig.site.dialplan;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditAttendantDefaultsTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditAttendantDefaultsTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
        clickLink("ManageAttendants");
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testEditSetting() {
        clickLink("defaultAttendantGroup");
        SiteTestHelper.assertNoException(tester);
        setFormElement("integerField_0", "5");
        clickButton("setting:apply");
        SiteTestHelper.assertNoException(tester);
        assertFormElementEquals("integerField_0", "5");
        clickButton("setting:cancel");
        assertTablePresent("list:attendant");
    }
}
