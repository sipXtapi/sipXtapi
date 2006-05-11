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

import java.io.CharArrayReader;
import java.io.CharArrayWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;


public class SipConfigurationTest  extends XMLTestCase {
    
    PolycomPhone phone;
    
    PhoneTestDriver tester;
    
    protected void setUp() {
        XMLUnit.setIgnoreWhitespace(true);
        phone = new PolycomPhone(PolycomModel.MODEL_600);
        tester = new PhoneTestDriver(phone);        
    }
    
    public void testGenerateProfile() throws Exception {
        
        // settings selected at random, there are too many
        // to test all.  select a few.
        phone.setSettingValue("log/sip/level.change.sip", "3");
        phone.setSettingValue("call/rejectBusyOnDnd", "0");
        phone.setSettingValue("voIpProt.SIP/local/port", "5061");
        phone.setSettingValue("call/rejectBusyOnDnd", "0");

        tester.line.setSettingValue("call/serverMissedCall/enabled", "1");
        
        assertEquals("0", phone.getSettingValue("voIpProt.server.dhcp/available"));
        
        VelocityProfileGenerator cfg = new SipConfiguration(phone);
        cfg.setVelocityEngine(TestHelper.getVelocityEngine());
        
        CharArrayWriter out = new CharArrayWriter();
        cfg.generateProfile(phone.getSipTemplate(), out);       
        
        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sip.cfg.xml");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);            
        Reader generatedXml = new CharArrayReader(out.toCharArray());

        // helpful debug
        System.out.println(new String(out.toCharArray()));

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();        
    }
}
