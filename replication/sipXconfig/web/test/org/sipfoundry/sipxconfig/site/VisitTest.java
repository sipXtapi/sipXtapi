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
        Visit visit = new Visit();
        assertNull(visit.getUserId());

        visit.login(user.getId(), false);

        assertSame(user.getId(), visit.getUserId());
        assertFalse(visit.isAdmin());

        visit.login(user.getId(), true);
        assertSame(user.getId(), visit.getUserId());
        assertTrue(visit.isAdmin());
        
        // Call "clear" rather than "logout" because we don't have the request cycle
        // object that would be needed to call "logout".  
        visit.clear();
        assertNull(visit.getUserId());
    }

    public void testNavigationVisible() throws Exception {
        // by default new Visit object has navigation enabled
        assertTrue(new Visit().isNavigationVisible());
    }
}
