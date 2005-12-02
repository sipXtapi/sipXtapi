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
package org.sipfoundry.sipxconfig.conference;

import junit.framework.TestCase;

public class BridgeTest extends TestCase {

    public void testInsertConference() {
        Conference c = new Conference();
        c.setUniqueId();
        
        Bridge bridge = new Bridge();
        assertTrue(bridge.getConferences().isEmpty());
        bridge.insertConference(c);
        
        assertEquals(1, bridge.getConferences().size());
        assertSame(c, bridge.getConferences().iterator().next());
        
        assertSame(bridge, c.getBridge());
    }

    public void testRemoveConference() {
        Conference c = new Conference();
        c.setUniqueId();
        
        Conference c1 = new Conference();
        c1.setUniqueId();

        
        Bridge bridge = new Bridge();
        assertTrue(bridge.getConferences().isEmpty());
        bridge.insertConference(c);
        
        bridge.removeConference(c1);
        assertEquals(1, bridge.getConferences().size());
        
        bridge.removeConference(c);
        assertTrue(bridge.getConferences().isEmpty());
        assertNull(c.getBridge());
    }
}
