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
package org.sipfoundry.sipxconfig.upload;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

public class ManageUploadTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageUploadTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
    }
    
    public void testDisplay() {
        clickLink("link:upload");
        SiteTestHelper.assertNoException(tester);
    }
}
