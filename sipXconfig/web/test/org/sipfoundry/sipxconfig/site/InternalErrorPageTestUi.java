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

import junit.framework.AssertionFailedError;
import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

public class InternalErrorPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(InternalErrorPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
    }

    public void testShowExceptionPage() throws Exception {
        clickLink("ShowExceptionPage");
        try {
            SiteTestHelper.assertNoException(tester);
            // cannot call fail here it'll get caught by next catch
            throw new IllegalArgumentException();
        } catch (AssertionFailedError e) {
            // passed
        } catch (IllegalArgumentException e) {
            fail("SiteTestHelper.assertNoException did not properly detect the error page");
        }
    }
}
