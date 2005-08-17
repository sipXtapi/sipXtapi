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
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.User;

public class CallGroupTest extends TestCase {

    public void testInsertRingUser() {
        User u = new User();
        u.setUserName("testUser");
        CallGroup group = new CallGroup();
        UserRing ring = group.insertRing(u);
        assertSame(u, ring.getUser());
        List calls = group.getCalls();
        assertEquals(1, calls.size());
        assertSame(ring, calls.get(0));
        assertTrue(ring.isFirst());
    }

    public void testGenerateAliases() {
        CallGroup group = new CallGroup();
        group.setName("sales");
        group.setExtension("401");

        final int ringsLen = 5;
        for (int i = 0; i < ringsLen; i++) {
            User u = new User();
            u.setUserName("testUser" + i);
            group.insertRing(u);
        }

        List aliases = group.generateAliases("kuku");
        // disabled group should not generate aliases
        assertTrue(aliases.isEmpty());

        group.setEnabled(true);
        aliases = group.generateAliases("kuku");

        assertEquals(ringsLen + 1, aliases.size());
        for (int i = 0; i < ringsLen; i++) {
            AliasMapping am = (AliasMapping) aliases.get(i);
            assertEquals(am.getIdentity(), group.getName() + "@kuku");
            assertTrue(am.getContact().startsWith("<sip:testUser" + i + "@kuku"));
        }

        // the last alias is an extension => identity
        AliasMapping am = (AliasMapping) aliases.get(aliases.size() - 1);
        assertEquals(am.getIdentity(), group.getExtension() + "@kuku");
        assertTrue(am.getContact().startsWith(group.getName() + "@kuku"));
    }

    public void testClone() {
        CallGroup group = new CallGroup();
        group.setName("sales");
        group.setExtension("401");

        final int ringsLen = 5;
        for (int i = 0; i < ringsLen; i++) {
            User u = new User();
            u.setUserName("testUser" + i);
            group.insertRing(u);
        }
        assertEquals(ringsLen, group.getCalls().size());

        CallGroup clonedGroup = (CallGroup) group.duplicate();
        assertEquals("sales", clonedGroup.getName());
        assertEquals("401", clonedGroup.getExtension());
        List clonedCalls = clonedGroup.getCalls();
        assertEquals(ringsLen, clonedCalls.size());
        for (int i = 0; i < ringsLen; i++) {
            UserRing ring = (UserRing) clonedCalls.get(i);
            assertEquals("testUser" + i, ring.getUser().getUserName());
            assertSame(clonedGroup, ring.getCallGroup());
        }

    }
}
