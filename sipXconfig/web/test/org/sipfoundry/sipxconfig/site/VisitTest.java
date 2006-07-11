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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;

public class VisitTest extends TestCase {

    public void testLogin() {
        User user = new User();
        UserSession userSession = new UserSession();
        assertNull(userSession.getUserId());

        userSession.login(user.getId(), false, true);

        assertSame(user.getId(), userSession.getUserId());
        assertFalse(userSession.isAdmin());

        userSession.login(user.getId(), true, false);
        assertSame(user.getId(), userSession.getUserId());
        assertTrue(userSession.isAdmin());
    }

    public void testNavigationVisible() throws Exception {
        // by default new Visit object has navigation enabled
        assertTrue(new UserSession().isNavigationVisible());
    }
}
