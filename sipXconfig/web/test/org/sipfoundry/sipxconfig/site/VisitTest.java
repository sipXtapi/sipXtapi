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

import org.sipfoundry.sipxconfig.common.User;

import junit.framework.TestCase;

public class VisitTest extends TestCase {

    /*
     * Test method for 'org.sipfoundry.sipxconfig.site.Visit.login(User, boolean)'
     */
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
        
        visit.logout();
        assertNull(visit.getUserId());
    }
}
