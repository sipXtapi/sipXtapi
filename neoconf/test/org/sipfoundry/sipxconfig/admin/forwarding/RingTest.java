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

import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;

import junit.framework.TestCase;

/**
 * RingTest
 */
public class RingTest extends TestCase {

    public void testCalculateContact() {
        ForkQueueValue q = new ForkQueueValue(3);
        Ring ring = new Ring();
        ring.setNumber("444");
        ring.setExpiration(45);
        ring.setType(Ring.Type.IMMEDIATE);
        
        String contact = ring.calculateContact("sipfoundry.org", q);
        assertEquals("<sip:444@sipfoundry.org?expires=45>;;q=1.0",contact);
        
        Ring ring2 = new Ring("333", 25, Ring.Type.DELAYED);
        String contact2 = ring2.calculateContact("sipfoundry.org", q);
        assertEquals("<sip:333@sipfoundry.org?expires=25>;;q=0.95",contact2);
        
        // with new q value - ring2 is delayed, q mustbe < 1.0
        ForkQueueValue q1 = new ForkQueueValue(3);
        contact2 = ring2.calculateContact("sipfoundry.org", q1);
        assertEquals("<sip:333@sipfoundry.org?expires=25>;;q=0.95",contact2);
    }

}
