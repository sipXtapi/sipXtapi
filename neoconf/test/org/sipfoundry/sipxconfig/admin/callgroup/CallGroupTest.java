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

import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class CallGroupTest extends TestCase {

    /*
     * Class under test for UserRing insertRing(User)
     */
    public void testInsertRingUser() {
        User u = new User();
        u.setDisplayId("testUser");
        CallGroup group = new CallGroup();
        UserRing ring = group.insertRing(u);
        assertSame(u, ring.getUser());
        List calls = group.getCalls();
        assertEquals(1, calls.size());
        assertSame(ring, calls.get(0));
    }

    public void testActivate() {
        MockControl control = MockControl.createControl(PhoneContext.class);
        control.setDefaultMatcher(MockControl.ALWAYS_MATCHER);
        PhoneContext phoneContext = (PhoneContext) control.getMock();
        phoneContext.generateProfilesAndRestart(Collections.EMPTY_LIST);
        control.replay();

        CallGroup cg = new CallGroup();
        cg.setEnabled(true);
        cg.activate(phoneContext);
        control.verify();

        control.reset();
        // nothing gets called when call group is disabled
        control.replay();
        cg.setEnabled(false);
        cg.activate(phoneContext);
        control.verify();
    }

    /*
     * Class under test for List generateAliases()
     */
    public void testGenerateAliases() {
        CallGroup group = new CallGroup();
        group.setName("sales");
        group.setExtension("401");

        final int ringsLen = 5;
        for (int i = 0; i < ringsLen; i++) {
            User u = new User();
            u.setDisplayId("testUser" + i);
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
}
