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
package org.sipfoundry.sipxconfig.phone.polycom;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.setting.Setting;


public class PolycomLineTest extends TestCase {
    
    public void testGetSipPort() {
        assertEquals(5060, PolycomLine.getSipPort(null));
        assertEquals(5060, PolycomLine.getSipPort(""));
        assertEquals(5060, PolycomLine.getSipPort("5060"));
        assertEquals(1234, PolycomLine.getSipPort("1234"));
        // print stack trace expected
        assertEquals(5060, PolycomLine.getSipPort("bogus"));
    }
    
    public void testDefaults() throws Exception {
        PolycomTestHelper helper = PolycomTestHelper.plainEndpointSeed();
        PolycomLine line = new PolycomLine(helper.phone[0], helper.line[0]);
        Setting settings = line.getSettings();
        assertEquals("sipfoundry.org", settings.getSetting("reg").getSetting("server")
                .getSetting("1").getSetting("address").getValue());
    }
}
