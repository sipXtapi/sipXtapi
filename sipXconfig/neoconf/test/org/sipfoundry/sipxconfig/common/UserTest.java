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

import java.util.List;

import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;

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

    public void testGetPintokenHash() throws Exception {
        User user = new User();
        user.setDisplayId("displayId");
        user.setPintoken("xxx");
        String hash = Md5Encoder.digestPassword("displayId", "sipfoundry.org", "xxx");

        assertEquals(hash, user.getPintokenHash("sipfoundry.org"));
    }

    public void testGetPintokenHashEmpty() throws Exception {
        User user = new User();
        user.setDisplayId("displayId");
        user.setPintoken(null);
        String hash = Md5Encoder.digestPassword("displayId", "sipfoundry.org", "");

        assertEquals(hash, user.getPintokenHash("sipfoundry.org"));
    }

    public void testGetPintokenHashMd5() throws Exception {
        User user = new User();
        user.setDisplayId("displayId");
        String hash = Md5Encoder.digestPassword("displayId", "sipfoundry.org", "");
        user.setPintoken(hash);

        assertEquals(hash, user.getPintokenHash("sipfoundry.org"));
    }

    public void testGetSipPasswordHash() throws Exception {
        User user = new User();
        user.setDisplayId("displayId");
        user.setSipPassword("xxx");
        String hash = Md5Encoder.digestPassword("displayId", "sipfoundry.org", "xxx");

        assertEquals(hash, user.getSipPasswordHash("sipfoundry.org"));
    }

    public void testGetSipPasswordHashEmpty() throws Exception {
        User user = new User();
        user.setDisplayId("displayId");
        user.setSipPassword(null);
        String hash = Md5Encoder.digestPassword("displayId", "sipfoundry.org", "");

        assertEquals(hash, user.getSipPasswordHash("sipfoundry.org"));
    }

    public void testGetSipPasswordHashMd5() throws Exception {
        User user = new User();
        user.setDisplayId("displayId");
        String hash = Md5Encoder.digestPassword("displayId", "sipfoundry.org", "");
        user.setSipPassword(hash);

        String newHash = Md5Encoder.digestPassword("displayId", "sipfoundry.org", hash);

        assertFalse(hash.equals(newHash));
        assertEquals(newHash, user.getSipPasswordHash("sipfoundry.org"));
    }
    
    public void testGetAliases() {
        User user = new User();
        user.setDisplayId("displayId");
        user.setExtension("4444");
        List aliases = user.getAliases("sipfoundry.org");
        assertEquals(1, aliases.size());
        AliasMapping alias = (AliasMapping) aliases.get(0);
        assertEquals("4444@sipfoundry.org", alias.getIdentity());
        assertEquals("sip:displayId@sipfoundry.org", alias.getContact());
    }
    
    public void testGetAliasesNoExtension() {
        User user = new User();
        user.setDisplayId("displayId");
        user.setExtension(null);
        List aliases = user.getAliases("sipfoundry.org");
        assertEquals(0, aliases.size());
    }    
}
