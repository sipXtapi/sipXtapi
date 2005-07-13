/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

public class UserTest extends TestCase {

    public void testGetDisplayName() {
        User u = new User();
        assertNull(u.getDisplayName());
        u.setDisplayId("bob");
        assertNull(u.getDisplayName());
        u.setFirstName("First");
        assertEquals("First", u.getDisplayName());
        u.setLastName("Last");
        assertEquals("First Last", u.getDisplayName());
    }

    public void testGetUri() {
        User user = new User();
        user.setDisplayId("displayId");
        String uri = user.getUri("mycomp.com");

        assertEquals("sip:displayId@mycomp.com", uri);

        user.setLastName("Last");
        uri = user.getUri("mycomp.com");
        assertEquals("Last<sip:displayId@mycomp.com>", uri);

        user.setFirstName("First");
        uri = user.getUri("mycomp.com");
        assertEquals("First Last<sip:displayId@mycomp.com>", uri);
    }
}
