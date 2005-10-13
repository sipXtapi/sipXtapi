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
package org.sipfoundry.sipxconfig.api;

import junit.framework.Test;
import junit.framework.TestCase;

import org.codehaus.xfire.xml.client.SoapInvocatorFactory;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class UserServiceTestUi extends TestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserServiceTestUi.class);
    }

    public void testNop() throws Exception {
        UserService userService = (UserService) SoapInvocatorFactory.createStub(UserService.class);
        userService.echo();
    }
}
