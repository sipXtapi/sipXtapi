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
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;

/**
 * CallSequenceTest
 */
public class CallSequenceTest extends TestCase {

    public void testGenerateAliases() {        
        final int N = 7;
        List rings = new ArrayList(N);        
        for(int i = 0; i < N; i++) {
            Ring ring = new Ring("2" + i, i, Ring.Type.DELAYED);
            rings.add(ring);
        }
        CallSequence sequence = new CallSequence();
        sequence.setCalls(rings);
        sequence.setDomain("sipfoundry.org");
        sequence.setUser("abc");

        List aliases = sequence.generateAliases();
        assertEquals(N, aliases.size());
        for (Iterator i = aliases.iterator(); i.hasNext();) {
            AliasMapping a = (AliasMapping) i.next();
            assertEquals("abc@sipfoundry.org", a.getIdentity());
            String contact = a.getContact();
            assertTrue(contact.matches("<sip:\\d+@sipfoundry.org\\?expires=\\d+>;;q=[01]\\.\\d+"));
        }
    }

    public void testGenerateAliasesEmpty() {
        CallSequence sequence = new CallSequence();
        List list = sequence.generateAliases();
        assertEquals(0, list.size());
    }

}
