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


public class PolycomEndpointTest extends TestCase {
    
    public void testDefaults() throws Exception {
        PolycomTestHelper helper = PolycomTestHelper.plainEndpointSeed();
        PolycomEndpoint endpoint = new PolycomEndpoint(helper.phone[0], helper.endpoint[0]);
        Setting settings = endpoint.getSettings();
        assertEquals("sipfoundry.org", settings.getSetting("voIpProt")
                .getSetting("server").getSetting("1").getSetting("address").getValue());
    }
}
