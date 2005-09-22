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

import java.util.Map;

import junit.framework.Test;
import junit.framework.TestCase;

import org.apache.commons.beanutils.BeanUtils;
import org.codehaus.xfire.xml.client.SoapInvocatorFactory;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class UserServiceTest extends TestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserServiceTest.class);
    }

    public void testNop() throws Exception {
        UserService userService = (UserService) SoapInvocatorFactory.createStub(UserService.class);
        userService.echo();
    }

    public void testCreateUser() throws Exception {
        UserService userService = (UserService) SoapInvocatorFactory.createStub(UserService.class);
        User u = new User();
        u.setUserName("joe");
        u.setPintoken("pin");
        Map properties = BeanUtils.describe(u);
        userService.createUser(properties);
    }
}
