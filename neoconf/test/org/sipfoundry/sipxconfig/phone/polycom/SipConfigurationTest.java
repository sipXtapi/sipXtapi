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
import org.sipfoundry.sipxconfig.setting.Setting;


public class SipConfigurationTest  extends XMLTestCase {
    
    public void setUp() {
        XMLUnit.setIgnoreWhitespace(true);
    }
    
    public void testGenerateProfile() throws Exception {
        PolycomTestHelper helper = PolycomTestHelper.plainEndpointSeed();
        helper.plainSettingsSeed();
        
        // settings selected at random, there are too many
        // to test all.  select a few.
        Setting endpointSettings = helper.phone[0].getSettings();
        endpointSettings.getSetting("log").getSetting("sip").getSetting("level.change.sip").setValue("3");

        ConfigurationFile cfg = new ConfigurationFile(helper.phone[0]);
        
        cfg.setTemplate(helper.phone[0].getSipTemplate());
        CharArrayWriter out = new CharArrayWriter();
        cfg.generateProfile(out);       
        
        InputStream expectedPhoneStream = getClass().getResourceAsStream("cfgdata/expected-sip.cfg");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);            
        Reader generatedXml = new CharArrayReader(out.toCharArray());

        // helpful debug
        System.out.println(new String(out.toCharArray()));

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();        
    }
}
