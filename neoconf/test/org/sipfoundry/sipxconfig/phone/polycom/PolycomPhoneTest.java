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

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.Organization;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.phone.SipService;
import org.sipfoundry.sipxconfig.setting.Setting;

public class PolycomPhoneTest extends TestCase {
            
    public void testDefaults() throws Exception {
        PolycomTestHelper helper = PolycomTestHelper.plainEndpointSeed();
        Setting settings = helper.phone[0].getSettings();
        assertEquals("sipfoundry.org", settings.getSetting("voIpProt")
                .getSetting("server").getSetting("1").getSetting("address").getValue());
    }

    public void testGenerateProfiles() throws Exception {
        Organization rootOrg = new Organization();
        rootOrg.setDnsDomain("localhost.localdomain");

        PolycomTestHelper helper = PolycomTestHelper.plainEndpointSeed();
        helper.phone[0].generateProfiles();
        
        // content of profiles is tested in individual base classes of ConfigurationTemplate
    }
    
    public void testRestartFailureNoLine() throws Exception {
        PolycomTestHelper helper = PolycomTestHelper.plainEndpointSeed();
        helper.phone[0].getPhoneMetaData().getLines().clear();
        try {
            helper.phone[0].restart();
            fail();
        } catch (RestartException re) {
            assertTrue(true);
        }
    }
    
    public void testRestart() throws Exception {
		String expected = "NOTIFY juser@sipfoundry.org SIP/2.0\r\n" 
				+ "Via: SIP/2.0/TCP [VIA]\r\n"
				+ "From: <sip:[SERVER_URI]>\r\n" 
				+ "To: <sip:juser@sipfoundry.org>\r\n"
				+ "Event: check-sync\r\n" 
				+ "Date: [DATE]\r\n" 
				+ "Call-ID: [CALL_ID]\r\n"
				+ "CSeq: 1 NOTIFY\r\n" 
				+ "Contact: <sip:[SERVER_URI]>\r\n"
				+ "Content-Length: 0\r\n" 
				+ "\r\n";        
        
        
        MockControl control = MockControl.createStrictControl(SipService.class);        
        SipService sip = (SipService) control.getMock();
        control.expectAndReturn(sip.getServerVia(), "[VIA]");
        control.expectAndReturn(sip.getServerUri(), "[SERVER_URI]");
        control.expectAndReturn(sip.getCurrentDate(), "[DATE]");
        control.expectAndReturn(sip.generateCallId(), "[CALL_ID]");
        sip.send("sipfoundry.org", 5060, expected);
        control.replay();        
        
        PolycomTestHelper helper = PolycomTestHelper.plainEndpointSeed();

        // "throw in" test when port explicitly set, not pertinent to rest of test
// KNOWN FAILURE        
//        PolycomLine pline = new PolycomLine(helper.phone[0], helper.line[0]);
//        pline.setPrimaryRegistrationServerPort("1234");
//        assertEquals(helper.line[0], helper.endpoint[0].getLines().get(0));

        helper.phone[0].setSipService(sip);
        helper.phone[0].restart();
        
        
        control.verify();
    }
}


