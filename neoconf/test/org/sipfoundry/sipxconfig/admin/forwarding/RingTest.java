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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;

/**
 * RingTest
 */
public class RingTest extends TestCase {

    public void testCalculateContact() {
        ForkQueueValue q = new ForkQueueValue(3);
        Ring ring = new Ring();
        ring.setNumber("555");
        ring.setExpiration(45);
        ring.setType(Ring.Type.IMMEDIATE);
        
        String contact = ring.calculateContact("sipfoundry.org", q, false);
        assertEquals("<sip:555@sipfoundry.org?expires=45>;;q=1.0",contact);
    }

}
